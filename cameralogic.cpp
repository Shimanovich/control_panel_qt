#include "cameralogic.h"
#include "Settings.h"
#include <QDebug>

cameraLogic::cameraLogic()
{
}

cameraLogic::~cameraLogic()
{
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait(3000);
    }
    delete m_worker;
}

int cameraLogic::loadCameraSettings(Settings* settings)
{
    if (!settings) return 0;

    this->cameraZoomPosList = settings->getZoomPositions();
    camNetInfo = settings->getTargetControl();

    auto devAddrs = settings->getDeviceAddresses();
    quint8 camAddr = devAddrs.camera;

    cameraProtocol = new MC108M3Camera();



    m_worker = new UdpWorker();
    m_worker->setTarget(QHostAddress(camNetInfo.ip), camNetInfo.port);

    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &UdpWorker::init);
    connect(m_worker, &UdpWorker::received, this, &cameraLogic::onReceived);

    m_thread->start();


    cameraProtocol->setSendFunction([this, camAddr](const QByteArray &viscaPayload) {
        if (m_worker) {
            QByteArray fullMessage;
            fullMessage.append(static_cast<char>(camAddr));
            fullMessage.append(viscaPayload);
            m_worker->enqueueSend(fullMessage);
        }
    });

    return 1;
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



/*
 * Обработка принятых ответов
 */
void cameraLogic::onReceived(const QByteArray &data,
                             const QHostAddress &sender,
                             quint16 port)
{
    QString text = QString::fromUtf8(data);

    QString logText = QString("📥 Получено от %1:%2 → %3")
                          .arg(sender.toString())
                          .arg(port)
                          .arg(text);

    emit logMessage(logText);
}

void cameraLogic::zoom_wide()
{
    if (zoom_index > 0 ){ zoom_index--; }

    cameraProtocol->sendZoomDirect(cameraZoomPosList[zoom_index]);
    qDebug() << "Zoom Wide, index = "<< zoom_index;
    //emit zoomWideRequested();
}

void cameraLogic::zoom_tele()
{
    if (zoom_index < cameraZoomPosList.count() - 1){ zoom_index++;}

    cameraProtocol->sendZoomDirect(cameraZoomPosList[zoom_index]);
    qDebug() << "Zoom Tele, index =" << zoom_index;
    //emit zoomTeleRequested();
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
