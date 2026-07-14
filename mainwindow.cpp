#include "mainwindow.h"
#include "SDL2/SDL.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(cameraLogic *camPtr, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_cam = camPtr;

    // Инициализация SDL
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        qDebug() << "SDL_Init Error:" << SDL_GetError();
        return;
    }else
    {
        qDebug() << "SDL_Init OK";
    }

    qDebug() << "SDL Version:" << SDL_GetRevision();
    qDebug() << "Joysticks found:" << SDL_NumJoysticks();

    // Инициализация джойстика
    initJoystick();

    // Таймер для опроса джойстика
    m_joystickTimer = new QTimer(this);
    connect(m_joystickTimer, &QTimer::timeout, this, &MainWindow::processJoystick);
    m_joystickTimer->start(1000/25); // ~25 раз в секунду


    // Подключаем сигнал логирования
    connect(m_cam, &cameraLogic::logMessage,
            this, [this](const QString &msg) {
                if (ui && ui->logTextEdit)
                    ui->logTextEdit->append(msg);
            });

    // Подключаем кнопки
    connect(ui->setServerButton, &QPushButton::clicked,
            this, &MainWindow::onSetServerClicked);

    connect(ui->zoomWideButton, &QPushButton::clicked,
            this, &MainWindow::on_ZoomWide_clicked);

    connect(ui->zoomTeleButton, &QPushButton::clicked,
            this, &MainWindow::on_zoomTele_clicked);

    ui->logTextEdit->append("✅ Приложение запущено.\n"
                            "Настройте сервер и отправляйте сообщения.");
}

MainWindow::~MainWindow()
{

    if (m_joystick) {
        SDL_JoystickClose(m_joystick);
    }
    SDL_Quit();

    delete ui;
}

void MainWindow::setCamPrt(cameraLogic *camPtr)
{
    this->m_cam = camPtr;
}

void MainWindow::onSetServerClicked()
{
    if (!m_cam) {
        QMessageBox::warning(this, "Ошибка", "cameraLogic не инициализирован!");
        return;
    }

    QHostAddress addr(ui->hostEdit->text().trimmed());

    if (addr.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Неверный IP-адрес!");
        return;
    }

    quint16 port = static_cast<quint16>(ui->portSpinBox->value());

    m_cam->setServer(addr, port);
}

void MainWindow::onError(const QString &message)
{
    if (ui && ui->logTextEdit)
        ui->logTextEdit->append("❌ " + message);
}

void MainWindow::on_ZoomWide_clicked()
{
    if (m_cam)
        m_cam->zoom_wide();
}

void MainWindow::on_zoomTele_clicked()
{
    if (m_cam)
        m_cam->zoom_tele();
}

void MainWindow::on_radioAutofokus_clicked()
{
    if ((m_cam)&&(ui->radioAutofokus->isChecked()))
    {
        m_cam->fokus_auto();
    }
}


void MainWindow::on_radioInfFokus_clicked()
{
    if ((m_cam)&&(ui->radioInfFokus->isChecked()))
    {
        m_cam->fokus_inf();
    }
}


void MainWindow::on_pushButton_Bright_minus_clicked()
{
    if (m_cam)
    {
        m_cam->bright_minus();
    }
}


void MainWindow::on_pushButton_Bright_plus_clicked()
{
    if (m_cam)
    {
        m_cam->bright_plus();
    }
}

void MainWindow::initJoystick()
{
    SDL_JoystickEventState(SDL_ENABLE);

    if (SDL_NumJoysticks() > 0) {
        m_joystick = SDL_JoystickOpen(0);
        if (m_joystick) {
            m_joystickID = SDL_JoystickInstanceID(m_joystick);
            updateJoystickStatus(QString("Джойстик подключён: %1").arg(SDL_JoystickName(m_joystick)));
            ui->JoyStickState->setText("ON");

        }
    } else {
        updateJoystickStatus("Джойстик не подключён");
        ui->JoyStickState->setText("OFF");
        ui->JoyStickState->setStyleSheet("color: #FF0000;");
    }
}

void MainWindow::processJoystick()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_JOYDEVICEADDED) {
            int deviceIndex = event.jdevice.which;
            if (!m_joystick) {
                m_joystick = SDL_JoystickOpen(deviceIndex);
                if (m_joystick) {
                    m_joystickID = SDL_JoystickInstanceID(m_joystick);
                    updateJoystickStatus(QString("Подключён: %1").arg(SDL_JoystickName(m_joystick)));
                    ui->JoyStickState->setText("ON");

                }
            }
        }
        else if (event.type == SDL_JOYDEVICEREMOVED) {
            SDL_JoystickID id = event.jdevice.which;
            if (m_joystick && id == m_joystickID) {
                SDL_JoystickClose(m_joystick);
                m_joystick = nullptr;
                m_joystickID = -1;
                updateJoystickStatus("Джойстик отключён");
                ui->JoyStickState->setText("OFF");
            }
        }
    }

    // === 1. Кнопки (уже было) ===
    for (int i = 0; i < SDL_JoystickNumButtons(m_joystick); ++i) {
        if (SDL_JoystickGetButton(m_joystick, i)) {
            QString msg = QString("Кнопка %1 нажата").arg(i);
            //ui->logTextEdit->append(msg);
             qDebug() << msg;
        }
    }

    // === 2. HAT (крестовина) ===
    for (int i = 0; i < SDL_JoystickNumHats(m_joystick); ++i) {
        Uint8 hat = SDL_JoystickGetHat(m_joystick, i);

        if (hat != SDL_HAT_CENTERED) {
            QString direction;

            if (hat & SDL_HAT_UP)    direction += "↑";
            if (hat & SDL_HAT_DOWN)  direction += "↓";
            if (hat & SDL_HAT_LEFT)  direction += "←";
            if (hat & SDL_HAT_RIGHT) direction += "→";

            QString msg = QString("HAT %1: %2 (%3)").arg(i).arg(direction).arg(hat);
            //ui->logTextEdit->append(msg);
            qDebug() << msg;
        }
    }

    for (int i = 0; i < SDL_JoystickNumAxes(m_joystick); ++i) {
        Sint16 rawValue = SDL_JoystickGetAxis(m_joystick, i);

        // Нормализация в диапазон -1.0 ... +1.0
        double normalized = rawValue / 32768.0;

        // Мёртвая зона (deadzone) — убираем шум от небольшого отклонения
        const double deadzone = 0.1;
        if (std::abs(normalized) < deadzone) {
            normalized = 0.0;
        }

        // QString msg = QString("Axis %1: raw=%2  norm=%3")
        //                   .arg(i)
        //                   .arg(rawValue)
        //                   .arg(normalized, 0, 'f', 3);

        // ui->logTextEdit->append(msg);
        // qDebug() << msg;
    }
}

void MainWindow::updateJoystickStatus(const QString &status)
{
    ui->logTextEdit->append(status);
    qDebug() << status;
}



