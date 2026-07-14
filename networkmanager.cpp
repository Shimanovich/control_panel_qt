#include "networkmanager.h"
#include "Settings.h"
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
{
}

NetworkManager::~NetworkManager()
{
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait(3000);
    }
    delete m_worker;
}

void NetworkManager::init(Settings* settings)
{
    m_worker = new UdpWorker(this);

    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &UdpWorker::init);
    connect(m_worker, &UdpWorker::received, this, &NetworkManager::received);
    connect(m_worker, &UdpWorker::errorOccurred, this, &NetworkManager::errorOccurred);

    m_thread->start();

    if (settings) {
        auto info = settings->getTargetControl();
        setTarget(QHostAddress(info.ip), info.port);
    }
}

void NetworkManager::setTarget(const QHostAddress &address, quint16 port)
{
    QMetaObject::invokeMethod(m_worker, "setTarget",
                              Qt::QueuedConnection,
                              Q_ARG(QHostAddress, address),
                              Q_ARG(quint16, port));
}

void NetworkManager::enqueueSend(const QByteArray &data)
{
    if (m_worker) {
        m_worker->enqueueSend(data);
    }
}
