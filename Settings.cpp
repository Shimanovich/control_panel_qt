#include "Settings.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

Settings* Settings::s_instance = nullptr;

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

quint8 Settings::getCameraDeviceAddress()
{
    return getDeviceAddresses().camera;
}


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


// QString Settings::fileName() const
// {
//     return m_settings.fileName();
// }

// void Settings::loadDefaults()
// {
//     // // Устанавливаем значения по умолчанию
//     // m_settings.setValue("username",     m_settings.value("username", "Гость").toString());
//     // m_settings.setValue("darkMode",     m_settings.value("darkMode", true).toBool());
//     // m_settings.setValue("accentColor",  m_settings.value("accentColor", "#007AFF").toString());

//     // QSize defaultSize(1280, 720);
//     // m_settings.setValue("window/size",  m_settings.value("window/size", defaultSize).toSize());
// }

// QString Settings::getUsername() const
// {
//     return m_settings.value("username", "Гость").toString();
// }

// void Settings::setUsername(const QString &username)
// {
//     if (getUsername() != username) {
//         m_settings.setValue("username", username);
//         sync();
//         emit settingsChanged();
//     }
// }

// QSize Settings::getWindowSize() const
// {
//     return m_settings.value("window/size", QSize(1280, 720)).toSize();
// }

// void Settings::setWindowSize(const QSize &size)
// {
//     if (getWindowSize() != size) {
//         m_settings.setValue("window/size", size);
//         sync();
//         emit settingsChanged();
//     }
// }

// bool Settings::isDarkMode() const
// {
//     return m_settings.value("darkMode", true).toBool();
// }

// void Settings::setDarkMode(bool enabled)
// {
//     if (isDarkMode() != enabled) {
//         m_settings.setValue("darkMode", enabled);
//         sync();
//         emit settingsChanged();
//     }
// }

// QColor Settings::getAccentColor() const
// {
//     return QColor(m_settings.value("accentColor", "#007AFF").toString());
// }

// void Settings::setAccentColor(const QColor &color)
// {
//     if (getAccentColor() != color) {
//         m_settings.setValue("accentColor", color.name());
//         sync();
//         emit settingsChanged();
//     }
// }

// QStringList Settings::getRecentFiles() const
// {
//     return m_settings.value("recentFiles").toStringList();
// }

// void Settings::addRecentFile(const QString &filePath)
// {
//     if (filePath.isEmpty()) return;

//     QStringList files = getRecentFiles();
//     files.removeAll(filePath);
//     files.prepend(filePath);

//     if (files.size() > 10)
//         files.removeLast();

//     m_settings.setValue("recentFiles", files);
//     sync();
//     emit settingsChanged();
// }

void Settings::sync()
{
    m_settings.sync();
}
