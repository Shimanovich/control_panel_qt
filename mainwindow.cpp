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




