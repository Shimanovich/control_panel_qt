// ================================================
// ПРОСТОЕ Qt UDP-КЛИЕНТ ПРИЛОЖЕНИЕ (C++17/Qt 5.15+ или Qt 6)
// ================================================
// Поддерживает:
// • Подключение (bind + указание целевого UDP-сервера)
// • Отправка сообщений через очередь (thread-safe)
// • Приём ответов в отдельном потоке
// • Отправку можно вызывать из ЛЮБЫХ потоков (не только GUI)
// • GUI: лог отправок/приёмов + обработка ответов

// Как использовать:
// 1. Создайте проект Qt Widgets Application в Qt Creator
// 2. Замените содержимое файлов на код ниже
// 3. В .pro файле добавьте: QT += network
// 4. Соберите и запустите

// Файл: udpworker.h
#ifndef UDPWORKER_H
#define UDPWORKER_H

#include <QObject>
#include <QUdpSocket>
#include <QQueue>
#include <QMutex>
#include <QHostAddress>

class UdpWorker : public QObject
{
    Q_OBJECT
public:
    explicit UdpWorker(QObject *parent = nullptr);
    ~UdpWorker();

    // Потокобезопасный метод отправки (можно вызывать из любого потока)
    void enqueueSend(const QByteArray &data);

public slots:
    void init();                                      // запускается в рабочем потоке
    void setTarget(const QHostAddress &address, quint16 port);

    // Внутренний слот обработки очереди (вызывается через invokeMethod)
    void sendPending();

signals:
    void received(const QByteArray &data,
                  const QHostAddress &sender,
                  quint16 senderPort);
    void errorOccurred(const QString &message);

private slots:
    void processReceived();

private:
    QUdpSocket     *m_socket = nullptr;
    QHostAddress    m_targetAddress;
    quint16         m_targetPort = 0;

    QQueue<QByteArray> m_sendQueue;
    QMutex             m_mutex;
};

#endif // UDPWORKER_H
