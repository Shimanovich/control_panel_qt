#include "cameralogic.h"
#include "gimbalLogic.h"
#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <Settings.h>

#include <QCoreApplication>
#include <QDebug>

MainWindow * w = nullptr;
cameraLogic * cam = nullptr;
gimbalLogic * gim = nullptr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("MyCompany");
    QCoreApplication::setApplicationName("MyAwesomeApp");

    Settings* settings = Settings::instance();

    cam = new cameraLogic();
    cam->loadCameraSettings(settings);

    gim = new gimbalLogic();
    gim->loadGimbalSettings(settings);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "controlPanel_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    w = new MainWindow(cam, gim);

    w->show();
    int ret = a.exec();
    w->hide();

<<<<<<< HEAD
    delete w;
    cam->deleteLater();
    gim->deleteLater();
=======
    if (w) {
        delete w;
        w = nullptr;
    }

    if (cam) {
        cam->deleteLater();
        cam = nullptr;
    }
    if (gim) {
        gim->deleteLater();
        gim = nullptr;
    }

    // Settings singleton lives until app exit
>>>>>>> remotes/github/master

    return ret;
}