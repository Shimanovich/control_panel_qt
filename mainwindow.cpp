#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SDL2/SDL.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

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

    // Add to main layout (assuming ui has a central widget or layout)
    if (ui->centralwidget && ui->centralwidget->layout()) {
        ui->centralwidget->layout()->addWidget(gimbalGroup);
    } else {
        // fallback
        setCentralWidget(gimbalGroup);
    }

    // ... existing camera connections ...
    connect(m_cam, &cameraLogic::logMessage, this, [this](const QString &msg) {
        if (ui && ui->logTextEdit) ui->logTextEdit->append(msg);
    });

    if (m_gimbal) {
        connect(m_gimbal, &gimbalLogic::logMessage, this, [this](const QString &msg) {
            if (ui && ui->logTextEdit) ui->logTextEdit->append(msg);
        });
        connect(m_gimbal, &gimbalLogic::realtimeDataReceived, this, [](const SBGC_cmd_realtime_data_t &d) {
            qDebug() << "Realtime Gimbal data received";
            // Add UI update here
        });
    }

    // existing joystick and camera buttons...
    // (keep existing code)
    initJoystick();
    m_joystickTimer = new QTimer(this);
    connect(m_joystickTimer, &QTimer::timeout, this, &MainWindow::processJoystick);
    m_joystickTimer->start(1000/25);

    ui->logTextEdit->append("✅ Приложение запущено с централизованным UDP и gimbal.");
}

// ... existing destructor, onSetServerClicked, button slots for camera ...

void MainWindow::on_gimbalBasePos_clicked() { if (m_gimbal) m_gimbal->sendBasePos(); }
void MainWindow::on_gimbalMotorOn_clicked() { if (m_gimbal) m_gimbal->sendMotorOn(); }

// Keep other methods...
