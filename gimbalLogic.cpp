#include "gimbalLogic.h"
#include <QDebug>
#include "Settings.h"
#include <QByteArray>

// For reinterpret_cast safety
#include <cstring>

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

void gimbalLogic::onReceived(const QByteArray &rawData, const QHostAddress &sender, quint16 port)
{
    QByteArray data = rawData;

    // Skip possible device address byte (first byte) from central router
    if (data.size() > 1 && (data[0] < 0x10 || data[0] > 0x7F)) { // rough heuristic: small values likely addr
        data = data.mid(1);
    }

    if (data.size() < 4 || data[0] != SBGC_CMD_START_BYTE) {
        QString hex = data.toHex(' ').toUpper();
        emit logMessage(QString("📥 [GIMBAL] Raw data from %1:%2 (%3 bytes): %4")
                            .arg(sender.toString()).arg(port).arg(rawData.size()).arg(hex));
        return;
    }

    // Parse SBGC packet
    uint8_t cmdId = data[1];
    uint8_t payloadSize = data[2];
    uint8_t hcs = data[3];

    if (cmdId == SBGC_CMD_REALTIME_DATA_4 && payloadSize >= sizeof(SBGC_cmd_realtime_data_t)) {
        SBGC_cmd_realtime_data_t rtd;
        std::memcpy(&rtd, data.constData() + 4, sizeof(rtd));

        // Nice formatted log of key fields
        QString log = QString("📊 [GIMBAL REALTIME_DATA_4]");
        log += QString(" Angles (IMU): R%1 P%2 Y%3°")
                  .arg(rtd.imu_angle[0]*360.0/16384, 0, 'f', 1)
                  .arg(rtd.imu_angle[1]*360.0/16384, 0, 'f', 1)
                  .arg(rtd.imu_angle[2]*360.0/16384, 0, 'f', 1);
        log += QString(" | Frame: R%1 P%2 Y%3°")
                  .arg(rtd.frame_imu_angle[0]*360.0/16384, 0, 'f', 1)
                  .arg(rtd.frame_imu_angle[1]*360.0/16384, 0, 'f', 1)
                  .arg(rtd.frame_imu_angle[2]*360.0/16384, 0, 'f', 1);
        log += QString(" | Target: R%1 P%2 Y%3°")
                  .arg(rtd.target_angle[0]*360.0/16384, 0, 'f', 1)
                  .arg(rtd.target_angle[1]*360.0/16384, 0, 'f', 1)
                  .arg(rtd.target_angle[2]*360.0/16384, 0, 'f', 1);
        log += QString(" | Battery: %1V | Current: %2mA")
                  .arg(rtd.battery_voltage / 100.0, 0, 'f', 2)
                  .arg(rtd.current);
        log += QString(" | Motor power: R%1% P%2% Y%3%")
                  .arg(rtd.motor_power[0]).arg(rtd.motor_power[1]).arg(rtd.motor_power[2]);
        log += QString(" | Temps: IMU %1°C Frame %2°C")
                  .arg(rtd.imu_temp_celcius).arg(rtd.frame_imu_temp_celcius);

        emit logMessage(log);

        // Future: emit structured signal
        // emit realtimeDataReceived(rtd);

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
