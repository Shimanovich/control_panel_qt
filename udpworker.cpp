// Файл: udpworker.cpp
#include "udpworker.h"
#include <QMetaObject>

UdpWorker::UdpWorker(QObject *parent)
    : QObject(parent)
{
}

UdpWorker::~UdpWorker()
{
    // socket удалится автоматически, т.к. является child-ом this
}

void UdpWorker::init()
{
    m_socket = new QUdpSocket(this);

    connect(m_socket, &QUdpSocket::readyRead,
            this, &UdpWorker::processReceived);

    // Привязываем сокет к любому локальному адресу (порт выбирает ОС)
    if (!m_socket->bind(QHostAddress::AnyIPv4, 0)) {
        emit errorOccurred("Не удалось привязать сокет: " + m_socket->errorString());
    } else {
        emit errorOccurred(QString("Сокет привязан к локальному порту %1")
                               .arg(m_socket->localPort()));
    }
}

void UdpWorker::setTarget(const QHostAddress &address, quint16 port)
{
    m_targetAddress = address;
    m_targetPort = port;
}

void UdpWorker::enqueueSend(const QByteArray &data)
{
    if (data.isEmpty()) return;

    {
        QMutexLocker locker(&m_mutex);
        m_sendQueue.enqueue(data);
    }

    // Передаём управление в поток worker-а
    QMetaObject::invokeMethod(this, "sendPending", Qt::QueuedConnection);
}

void UdpWorker::sendPending()
{
    QList<QByteArray> pending;

    // Быстро забираем всё из очереди под локом
    {
        QMutexLocker locker(&m_mutex);
        while (!m_sendQueue.isEmpty()) {
            pending.append(m_sendQueue.dequeue());
        }
    }

    for (const QByteArray &data : pending) {
        if (!m_socket || m_targetPort == 0) {
            emit errorOccurred("Целевой сервер не настроен!");
            continue;
        }

        qint64 bytes = m_socket->writeDatagram(data, m_targetAddress, m_targetPort);
        if (bytes == -1) {
            emit errorOccurred("Ошибка отправки: " + m_socket->errorString());
        }
    }
}

void UdpWorker::processReceived()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(static_cast<int>(m_socket->pendingDatagramSize()));

        QHostAddress sender;
        quint16 senderPort = 0;

        m_socket->readDatagram(data.data(), data.size(), &sender, &senderPort);

        emit received(data, sender, senderPort);
    }
}
