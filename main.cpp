#define SDL_MAIN_HANDLED

#include "cameralogic.h"
#include "gimbalLogic.h"
#include "mainwindow.h"
#include "networkmanager.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <Settings.h>

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

MainWindow * w;
cameraLogic * cam;
gimbalLogic * gimbal;
NetworkManager * networkManager;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // Устанавливаем название компании и приложения
    QCoreApplication::setOrganizationName("MyCompany");
    QCoreApplication::setApplicationName("MyAwesomeApp");

    Settings* settings = Settings::instance();

    // === ЦЕНТРАЛИЗОВАННЫЙ NETWORK MANAGER ===
    networkManager = new NetworkManager();
    networkManager->init(settings);

    cam  = new cameraLogic();
    cam->loadCameraSettings(settings, networkManager->getWorker());

    gimbal = new gimbalLogic();
    gimbal->loadGimbalSettings(settings, networkManager->getWorker());

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "controlPanel_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    w = new MainWindow(cam, gimbal);

    w->show();
    return a.exec();
}
