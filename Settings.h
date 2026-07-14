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
        //QString protocol = "tcp";
        //int     timeoutMs = 5000;
        //bool    enabled = true;
    };


public:
    static Settings* instance();

    /*load zoom positions from ini file */
    QList<int> getZoomPositions();

    //get target ip and port
    TargetControlInfo getTargetControl();

    // Основные настройки
    void sync();

signals:
    void settingsChanged();

private:
    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    void loadDefaults();

    QSettings m_settings;

    static Settings* s_instance;
};
