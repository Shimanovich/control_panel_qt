#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SDL2/SDL.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>

MainWindow::MainWindow(cameraLogic *camPtr, gimbalLogic *gimbalPtr, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_cam = camPtr;
    m_gimbal = gimbalPtr;

    // === Gimbal controls (programmatically added) ===
    QGroupBox *gimbalGroup = new QGroupBox("Gimbal Control", this);
    QVBoxLayout *gimbalLayout = new QVBoxLayout(gimbalGroup);

    QPushButton *basePosBtn = new QPushButton("Base Position", gimbalGroup);
    QPushButton *motorOnBtn = new QPushButton("Motors ON", gimbalGroup);
    QSlider *azSlider = new QSlider(Qt::Horizontal, gimbalGroup);
    QSlider *elSlider = new QSlider(Qt::Horizontal, gimbalGroup);
    azSlider->setRange(-180, 180);
    elSlider->setRange(-90, 90);
    QLabel *azLabel = new QLabel("Az: 0", gimbalGroup);
    QLabel *elLabel = new QLabel("El: 0", gimbalGroup);

    connect(basePosBtn, &QPushButton::clicked, this, &MainWindow::on_gimbalBasePos_clicked);
    connect(motorOnBtn, &QPushButton::clicked, this, &MainWindow::on_gimbalMotorOn_clicked);
    connect(azSlider, &QSlider::valueChanged, azLabel, [azLabel](int v){ azLabel->setText(QString("Az: %1").arg(v)); });
    connect(elSlider, &QSlider::valueChanged, elLabel, [elLabel](int v){ elLabel->setText(QString("El: %1").arg(v)); });

    gimbalLayout->addWidget(basePosBtn);
    gimbalLayout->addWidget(motorOnBtn);
    gimbalLayout->addWidget(azLabel);
    gimbalLayout->addWidget(azSlider);
    gimbalLayout->addWidget(elLabel);
    gimbalLayout->addWidget(elSlider);

    if (ui->centralwidget && ui->centralwidget->layout()) {
        ui->centralwidget->layout()->addWidget(gimbalGroup);
    } else {
        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(gimbalGroup);
        QWidget *central = new QWidget;
        central->setLayout(mainLayout);
        setCentralWidget(central);
    }

    // Camera connections
    connect(m_cam, &cameraLogic::logMessage, this, [this](const QString &msg) {
        if (ui && ui->logTextEdit) ui->logTextEdit->append(msg);
    });

    if (m_gimbal) {
        connect(m_gimbal, &gimbalLogic::logMessage, this, [this](const QString &msg) {
            if (ui && ui->logTextEdit) ui->logTextEdit->append(msg);
        });
        connect(m_gimbal, &gimbalLogic::realtimeDataReceived, this, [](const SBGC_cmd_realtime_data_t &d) {
            qDebug() << "Realtime Gimbal data received";
        });
    }

    // Joystick
    initJoystick();
    m_joystickTimer = new QTimer(this);
    connect(m_joystickTimer, &QTimer::timeout, this, &MainWindow::processJoystick);
    m_joystickTimer->start(1000/25);

    // Server button
    connect(ui->setServerButton, &QPushButton::clicked, this, &MainWindow::onSetServerClicked);

    // Camera buttons
    connect(ui->zoomWideButton, &QPushButton::clicked, this, &MainWindow::on_ZoomWide_clicked);
    connect(ui->zoomTeleButton, &QPushButton::clicked, this, &MainWindow::on_zoomTele_clicked);
    // ... add other camera buttons if needed

    ui->logTextEdit->append("✅ Приложение запущено с централизованным UDP и gimbal.");
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
    if (m_gimbal) m_gimbal->setServer(addr, port);
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

void MainWindow::on_gimbalBasePos_clicked() { if (m_gimbal) m_gimbal->sendBasePos(); }
void MainWindow::on_gimbalMotorOn_clicked() { if (m_gimbal) m_gimbal->sendMotorOn(); }

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

    // Joystick processing (axes, buttons, hats) can be extended here
}

void MainWindow::updateJoystickStatus(const QString &status)
{
    if (ui && ui->logTextEdit)
        ui->logTextEdit->append(status);
    qDebug() << status;
}
