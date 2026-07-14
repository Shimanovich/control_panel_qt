#include "Settings.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

Settings* Settings::s_instance = nullptr;

Settings* Settings::instance()
{
    if (!s_instance)
        s_instance = new Settings();
    return s_instance;
}

Settings::Settings(QObject *parent)
    : QObject(parent),
    m_settings(QCoreApplication::applicationDirPath() + "/config.ini",
               QSettings::IniFormat)
{
    qDebug() << "Файл настроек:" << m_settings.fileName();
    qDebug() << "Существует:" << QFile::exists(m_settings.fileName());
}

Settings::~Settings()
{
    sync();
}

// add your settings here
QList<int> Settings::getZoomPositions()
{
    QList<int> posList;
    int size = m_settings.beginReadArray("zoomPos");
    for (int i = 0; i < size; ++i)
    {
        m_settings.setArrayIndex(i);
        posList.append(m_settings.value("pos").toInt());
    }
    m_settings.endArray();
    qDebug() << "Загружено значений зума :" << posList.size() << posList;
    return posList;
}

Settings::TargetControlInfo Settings::getTargetControl()
{
    TargetControlInfo info;

    m_settings.beginGroup("updParam");
    info.ip   = m_settings.value("targetControlip",   "127.0.0.1").toString();
    info.port = m_settings.value("targetControlPort", 1020).toInt();
    m_settings.endGroup();

    qDebug() << "Загружено Настройки камеры ip:" << info.ip <<" Port:"<<info.port;
    return info;
}

Settings::DeviceAddresses Settings::getDeviceAddresses()
{
    DeviceAddresses addrs;
    m_settings.beginGroup("deviceAddresses");
    addrs.camera = m_settings.value("camera", 0x01).toUInt() & 0xFF;
    addrs.gimbal = m_settings.value("gimbal", 0x02).toUInt() & 0xFF;
    addrs.laser  = m_settings.value("laser",  0x03).toUInt() & 0xFF;
    m_settings.endGroup();
    return addrs;
}

quint8 Settings::getGimbalDeviceAddress()
{
    return getDeviceAddresses().gimbal;
}

quint8 Settings::getCameraDeviceAddress()
{
    return getDeviceAddresses().camera;
}

void Settings::sync()
{
    m_settings.sync();
}
