#pragma once
#include <QObject>
#include <QSettings>
#include <QSize>
#include <QColor>
#include <QStringList>

class Settings : public QObject
{
    Q_OBJECT

public:
    struct TargetControlInfo
    {
        QString ip;
        int     port = 1020;
    };

    struct DeviceAddresses {
        quint8 camera = 0x01;
        quint8 gimbal = 0x02;
        quint8 laser  = 0x03;
    };

public:
    static Settings* instance();

    QList<int> getZoomPositions();

    TargetControlInfo getTargetControl();

    DeviceAddresses getDeviceAddresses();
    quint8 getGimbalDeviceAddress();
    quint8 getCameraDeviceAddress();

    void sync();

signals:
    void settingsChanged();

private:
    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    void loadDefaults();

    QSettings m_settings;

    static Settings* s_instance;
    bool m_destroyed = false;  // guard
};

#endif // SETTINGS_H
