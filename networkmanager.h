#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "udpworker.h"
#include <QObject>
#include <QThread>
#include <QHostAddress>
#include <Settings.h>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    void init(Settings* settings = nullptr);
    void setTarget(const QHostAddress &address, quint16 port);

    void enqueueSend(const QByteArray &data);

    UdpWorker* getWorker() const { return m_worker; }

signals:
    void received(const QByteArray &data, const QHostAddress &sender, quint16 senderPort);
    void errorOccurred(const QString &message);

private:
    UdpWorker *m_worker = nullptr;
    QThread   *m_thread = nullptr;
};

#endif // NETWORKMANAGER_H
