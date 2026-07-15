#ifndef CAMERALOGIC_H
#define CAMERALOGIC_H

#include "Settings.h"
#include "mc108m3camera.hpp"
#include "udpworker.h"
#include <QThread>
#include <QObject>
#include <QList>
#include <QHostAddress>

class cameraLogic : public QObject
{
    Q_OBJECT

private:
    UdpWorker           *m_worker           = nullptr;
    QThread             *m_thread           = nullptr;
    MC108M3Camera       *cameraProtocol     = nullptr;
    bool                 m_ownsWorker       = true;
    quint8               m_deviceAddr       = 0x01;

public:
    // перечень возможных положений привода зума
    QList<int> cameraZoomPosList;
    int zoom_index = 0;

    void (*sendCallback)(const QByteArray &payload) = nullptr;

    // ip адрес и порт для камеры
    Settings::TargetControlInfo camNetInfo;

    void sendWithAddr(const QByteArray &payload);

public:
    cameraLogic();
    ~cameraLogic();

    int loadCameraSettings(Settings* settings, UdpWorker* sharedWorker = nullptr);

    // Метод для установки сервера (вызывается из MainWindow)
    void setServer(const QHostAddress &addr, quint16 port);

    // Функции зума
    void zoom_wide();
    void zoom_tele();
    void fokus_auto();
    void fokus_inf();
    void bright_minus();
    void bright_plus();

public slots:
    void onReceived(const QByteArray &data, const QHostAddress &sender, quint16 port);

signals:
    void logMessage(const QString &message);

    void zoomWideRequested();
    void zoomTeleRequested();
};

#endif // CAMERALOGIC_H
