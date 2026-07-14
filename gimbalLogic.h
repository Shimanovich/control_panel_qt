#ifndef GIMBALLOGIC_H
#define GIMBALLOGIC_H

#include "Settings.h"
#include "sbgcprotocol.hpp"
#include "udpworker.h"
#include <QThread>
#include <QObject>
#include <QHostAddress>

class gimbalLogic : public QObject
{
    Q_OBJECT

private:
    UdpWorker     *m_worker = nullptr;
    QThread       *m_thread = nullptr;
    SbgcProtocol  *gimbalProtocol = nullptr;
    bool           m_ownsWorker = true;
    quint8         m_deviceAddr = 0x02;

public:
    Settings::TargetControlInfo gimbalNetInfo;

public:
    gimbalLogic();
    ~gimbalLogic();

    int loadGimbalSettings(Settings* settings, UdpWorker* sharedWorker = nullptr);

    // Метод для установки сервера (вызывается из MainWindow)
    void setServer(const QHostAddress &addr, quint16 port);

    // Control methods
    void sendBasePos();
    void sendCtrlExt(uint8_t modeAz, uint8_t modeEl, int16_t az, int16_t el);
    void sendMotorOn();
    void sendReset();
    void requestRealtimeData();

public slots:
    void onReceived(const QByteArray &data, const QHostAddress &sender, quint16 port);

signals:
    void logMessage(const QString &message);
};

#endif // GIMBALLOGIC_H
