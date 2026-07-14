#include "gimbalLogic.h"
#include <QDebug>

#include "Settings.h"

gimbalLogic::gimbalLogic()
{
}

gimbalLogic::~gimbalLogic()
{
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait(3000);
    }
    delete m_worker;
    // gimbalProtocol will be deleted by parent or manually if needed
}

int gimbalLogic::loadGimbalSettings(Settings* settings)
{
    if (!settings) return 0;

    gimbalNetInfo = settings->getTargetControl();
    auto devAddrs = settings->getDeviceAddresses();
    quint8 gimbalAddr = devAddrs.gimbal;

    gimbalProtocol = new SbgcProtocol();

    m_worker = new UdpWorker();
    m_worker->setTarget(QHostAddress(gimbalNetInfo.ip), gimbalNetInfo.port);

    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &UdpWorker::init);
    connect(m_worker, &UdpWorker::received, this, &gimbalLogic::onReceived);

    m_thread->start();

    gimbalProtocol->setSendFunction([this, gimbalAddr](const QByteArray &sbgcPacket) {
        if (m_worker) {
            QByteArray fullMsg;
            fullMsg.append(static_cast<char>(gimbalAddr)); // device address byte (0th byte of UDP)
            fullMsg.append(sbgcPacket);
            m_worker->enqueueSend(fullMsg);
        }
    });

    return 1;
}

void gimbalLogic::setServer(const QHostAddress &addr, quint16 port)
{
    if (addr.isNull()) {
        emit logMessage("❌ Ошибка: Неверный IP-адрес для gimbal!");
        return;
    }

    QMetaObject::invokeMethod(m_worker, "setTarget",
                              Qt::QueuedConnection,
                              Q_ARG(QHostAddress, addr),
                              Q_ARG(quint16, port));

    emit logMessage(QString("🎯 Gimbal сервер установлен: %1:%2")
                        .arg(addr.toString()).arg(port));
}

void gimbalLogic::onReceived(const QByteArray &data, const QHostAddress &sender, quint16 port)
{
    // Для начала просто логируем (позже можно добавить парсер REALTIME_DATA)
    QString hexData = data.toHex(' ').toUpper();
    QString logText = QString("📥 [GIMBAL] Получено от %1:%2 → %3 байт: %4")
                          .arg(sender.toString())
                          .arg(port)
                          .arg(data.size())
                          .arg(hexData);

    emit logMessage(logText);
}

// ====================== Control wrappers ======================

void gimbalLogic::sendBasePos()
{
    if (gimbalProtocol) gimbalProtocol->send_message_base_pos();
}

void gimbalLogic::sendCtrlExt(uint8_t modeAz, uint8_t modeEl, int16_t az, int16_t el)
{
    if (gimbalProtocol) gimbalProtocol->sendCmdCtrl_ext(modeAz, modeEl, az, el);
}

void gimbalLogic::sendMotorOn()
{
    if (gimbalProtocol) gimbalProtocol->send_message_motor_on();
}

void gimbalLogic::sendReset()
{
    if (gimbalProtocol) gimbalProtocol->send_message_reset_engine();
}

void gimbalLogic::requestRealtimeData()
{
    if (gimbalProtocol) gimbalProtocol->send_message_periodic();
}
