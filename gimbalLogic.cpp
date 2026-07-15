#include "gimbalLogic.h"
#include <QDebug>
#include "Settings.h"
#include <QByteArray>
#include <cstring>

gimbalLogic::gimbalLogic()
{
}

gimbalLogic::~gimbalLogic()
{
    if (m_ownsWorker) {
        delete m_worker;
    }
}

int gimbalLogic::loadGimbalSettings(Settings* settings, UdpWorker* sharedWorker)
{
    if (!settings) return 0;

    gimbalNetInfo = settings->getTargetControl();
    auto devAddrs = settings->getDeviceAddresses();
    m_deviceAddr = devAddrs.gimbal;

    gimbalProtocol = new SbgcProtocol();

    if (sharedWorker) {
        m_worker = sharedWorker;
        m_ownsWorker = false;
        connect(m_worker, &UdpWorker::received, this, &gimbalLogic::onReceived, Qt::UniqueConnection);
    } else {
        m_ownsWorker = true;
        m_worker = new UdpWorker();
        m_worker->setTarget(QHostAddress(gimbalNetInfo.ip), gimbalNetInfo.port);
    }

    gimbalProtocol->setSendFunction([this](const QByteArray &sbgcPacket) {
        sendWithAddr(sbgcPacket);
    });

    return 1;
}

void gimbalLogic::sendWithAddr(const QByteArray &payload)
{
    if (m_worker) {
        QByteArray fullMsg;
        fullMsg.append(static_cast<char>(m_deviceAddr));
        fullMsg.append(payload);
        m_worker->enqueueSend(fullMsg);
    }
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
    if (data.isEmpty()) return;

    quint8 firstByte = static_cast<quint8>(data[0]);
    if (firstByte != m_deviceAddr) {
        return;
    }

    QByteArray payload = data.mid(1);

    if (payload.size() < 4 || payload[0] != SBGC_CMD_START_BYTE) {
        return;
    }

    uint8_t cmdId = payload[1];
    uint8_t payloadSize = payload[2];

    if (cmdId == SBGC_CMD_REALTIME_DATA_4 && payloadSize >= sizeof(SBGC_cmd_realtime_data_t)) {
        SBGC_cmd_realtime_data_t rtd;
        std::memcpy(&rtd, payload.constData() + 4, sizeof(rtd));

        QString log = QString("📊 [GIMBAL REALTIME_DATA_4 addr=0x%1] Angles IMU: %2° %3° %4° | Battery %5V | Current %6mA")
                          .arg(m_deviceAddr, 2, 16, QChar('0'))
                          .arg(rtd.imu_angle[0]*360.0/16384, 0, 'f', 1)
                          .arg(rtd.imu_angle[1]*360.0/16384, 0, 'f', 1)
                          .arg(rtd.imu_angle[2]*360.0/16384, 0, 'f', 1)
                          .arg(rtd.battery_voltage / 100.0, 0, 'f', 2)
                          .arg(rtd.current);
        emit logMessage(log);

        emit realtimeDataReceived(rtd);
    }
}

void gimbalLogic::sendBasePos() { if (gimbalProtocol) gimbalProtocol->send_message_base_pos(); }
void gimbalLogic::sendCtrlExt(uint8_t ma, uint8_t me, int16_t a, int16_t e) { if (gimbalProtocol) gimbalProtocol->sendCmdCtrl_ext(ma, me, a, e); }
void gimbalLogic::sendMotorOn() { if (gimbalProtocol) gimbalProtocol->send_message_motor_on(); }
void gimbalLogic::sendReset() { if (gimbalProtocol) gimbalProtocol->send_message_reset_engine(); }
void gimbalLogic::requestRealtimeData() { if (gimbalProtocol) gimbalProtocol->send_message_periodic(); }