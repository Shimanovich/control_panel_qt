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
    if (m_ownsWorker && m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait(3000);
    }
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

        m_thread = new QThread(this);
        m_worker->moveToThread(m_thread);

        connect(m_thread, &QThread::started, m_worker, &UdpWorker::init);
        connect(m_worker, &UdpWorker::received, this, &gimbalLogic::onReceived);

        m_thread->start();
    }

    gimbalProtocol->setSendFunction([this](const QByteArray &sbgcPacket) {
        if (m_worker) {
            QByteArray fullMsg;
            fullMsg.append(static_cast<char>(m_deviceAddr));
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
    if (data.isEmpty()) return;

    quint8 firstByte = static_cast<quint8>(data[0]);
    if (firstByte != m_deviceAddr) {
        return; // не для gimbal
    }

    QByteArray payload = data.mid(1);

    if (payload.size() < 4 || payload[0] != SBGC_CMD_START_BYTE) {
        QString hex = payload.toHex(' ').toUpper();
        emit logMessage(QString("📥 [GIMBAL] Raw from %1:%2 (%3 bytes): %4")
                            .arg(sender.toString()).arg(port).arg(payload.size()).arg(hex));
        return;
    }

    uint8_t cmdId = payload[1];
    uint8_t payloadSize = payload[2];

    if (cmdId == SBGC_CMD_REALTIME_DATA_4 && payloadSize >= sizeof(SBGC_cmd_realtime_data_t)) {
        SBGC_cmd_realtime_data_t rtd;
        std::memcpy(&rtd, payload.constData() + 4, sizeof(rtd));

        QString log = QString("📊 [GIMBAL REALTIME_DATA_4 addr=0x%1]")
                        .arg(m_deviceAddr, 2, 16, QChar('0'));
        log += QString(" Angles (IMU): R%1 P%2 Y%3°")
                  .arg(rtd.imu_angle[0]*360.0/16384, 0, 'f', 1)
                  .arg(rtd.imu_angle[1]*360.0/16384, 0, 'f', 1)
                  .arg(rtd.imu_angle[2]*360.0/16384, 0, 'f', 1);
        log += QString(" | Battery: %1V | Current: %2mA | MotorPwr: R%3 P%4 Y%5")
                  .arg(rtd.battery_voltage / 100.0, 0, 'f', 2)
                  .arg(rtd.current)
                  .arg(rtd.motor_power[0]).arg(rtd.motor_power[1]).arg(rtd.motor_power[2]);
        log += QString(" | Temps: IMU %1°C Frame %2°C")
                  .arg(rtd.imu_temp_celcius).arg(rtd.frame_imu_temp_celcius);

        emit logMessage(log);

    } else {
        QString logText = QString("📥 [GIMBAL CMD 0x%1] Size %2 from %3:%4")
                              .arg(cmdId, 2, 16, QChar('0'))
                              .arg(payloadSize)
                              .arg(sender.toString())
                              .arg(port);
        emit logMessage(logText);
    }
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
