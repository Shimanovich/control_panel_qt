#include "cameralogic.h"
#include "Settings.h"
#include <QDebug>

cameraLogic::cameraLogic()
{
}

cameraLogic::~cameraLogic()
{
    if (m_ownsWorker && m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait(3000);
    }
    if (m_ownsWorker) {
        delete m_worker;
    }
}

int cameraLogic::loadCameraSettings(Settings* settings, UdpWorker* sharedWorker)
{
    if (!settings) return 0;

    this->cameraZoomPosList = settings->getZoomPositions();
    camNetInfo = settings->getTargetControl();

    auto devAddrs = settings->getDeviceAddresses();
    m_deviceAddr = devAddrs.camera;

    cameraProtocol = new MC108M3Camera();

    if (sharedWorker) {
        m_worker = sharedWorker;
        m_ownsWorker = false;
        connect(m_worker, &UdpWorker::received, this, &cameraLogic::onReceived, Qt::UniqueConnection);
    } else {
        m_ownsWorker = true;
        m_worker = new UdpWorker();
        m_worker->setTarget(QHostAddress(camNetInfo.ip), camNetInfo.port);

        m_thread = new QThread(this);
        m_worker->moveToThread(m_thread);

        connect(m_thread, &QThread::started, m_worker, &UdpWorker::init);
        connect(m_worker, &UdpWorker::received, this, &cameraLogic::onReceived);

        m_thread->start();
    }

    cameraProtocol->setSendFunction([this](const QByteArray &viscaPayload) {
        sendWithAddr(viscaPayload);
    });

    return 1;
}

void cameraLogic::sendWithAddr(const QByteArray &payload)
{
    if (m_worker) {
        QByteArray fullMessage;
        fullMessage.append(static_cast<char>(m_deviceAddr));
        fullMessage.append(payload);
        m_worker->enqueueSend(fullMessage);
    }
}

void cameraLogic::setServer(const QHostAddress &addr, quint16 port)
{
    if (addr.isNull()) {
        emit logMessage("❌ Ошибка: Неверный IP-адрес!");
        return;
    }

    QMetaObject::invokeMethod(m_worker, "setTarget",
                              Qt::QueuedConnection,
                              Q_ARG(QHostAddress, addr),
                              Q_ARG(quint16, port));

    emit logMessage(QString("🎯 Сервер установлен: %1:%2")
                        .arg(addr.toString()).arg(port));
}

void cameraLogic::onReceived(const QByteArray &data,
                             const QHostAddress &sender,
                             quint16 port)
{
    if (data.isEmpty()) return;

    quint8 firstByte = static_cast<quint8>(data[0]);
    if (firstByte != m_deviceAddr) {
        return;
    }

    QByteArray payload = data.mid(1);

    QString hex = payload.toHex(' ').toUpper();
    QString logText = QString("📥 [CAMERA addr=0x%1] от %2:%3 → %4 байт: %5")
                          .arg(m_deviceAddr, 2, 16, QChar('0'))
                          .arg(sender.toString())
                          .arg(port)
                          .arg(payload.size())
                          .arg(hex);

    emit logMessage(logText);
}

void cameraLogic::zoom_wide()
{
    if (zoom_index > 0 ){ zoom_index--; }

    cameraProtocol->sendZoomDirect(cameraZoomPosList[zoom_index]);
    qDebug() << "Zoom Wide, index = "<< zoom_index;
}

void cameraLogic::zoom_tele()
{
    if (zoom_index < cameraZoomPosList.count() - 1){ zoom_index++;}

    cameraProtocol->sendZoomDirect(cameraZoomPosList[zoom_index]);
    qDebug() << "Zoom Tele, index =" << zoom_index;
}

void cameraLogic::fokus_auto()
{
    cameraProtocol->sendFocusAuto();
    qDebug() << "Cmd Autofokus";
}

void cameraLogic::fokus_inf()
{
    cameraProtocol->sendFocusInfinity();
    qDebug() << "Cmd Infyfokus";
}

void  cameraLogic::bright_minus()
{
    cameraProtocol->sendBrightnessDown();
}

void cameraLogic::bright_plus()
{
    cameraProtocol->sendBrightnessUp();
}
