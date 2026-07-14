#define SDL_MAIN_HANDLED

#include "cameralogic.h"
#include "mainwindow.h"
#include "udpworker.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <Settings.h>

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

MainWindow * w;
cameraLogic * cam;
UdpWorker * sharedUdpWorker;
QThread * udpThread;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // Устанавливаем название компании и приложения
    QCoreApplication::setOrganizationName("MyCompany");
    QCoreApplication::setApplicationName("MyAwesomeApp");

    Settings* settings = Settings::instance();

    // === ЦЕНТРАЛИЗОВАННЫЙ UDP (один IP:port для всех устройств) ===
    sharedUdpWorker = new UdpWorker();
    udpThread = new QThread();
    sharedUdpWorker->moveToThread(udpThread);

    // Устанавливаем target из настроек (один для всех)
    auto targetInfo = settings->getTargetControl();
    sharedUdpWorker->setTarget(QHostAddress(targetInfo.ip), targetInfo.port);

    MainWindow::connect(udpThread, &QThread::started, sharedUdpWorker, &UdpWorker::init);
    udpThread->start();

    cam  = new cameraLogic();
    cam->loadCameraSettings(settings, sharedUdpWorker);  // передаём shared worker


    // qDebug() << "Пользователь:" << settings->getUsername();
    // qDebug() << "Тёмная тема:" << settings->isDarkMode();
    // qDebug() << "Размер окна:" << settings->getWindowSize();

    // // Изменяем настройки
    // settings->setDarkMode(false);
    // settings->setAccentColor(QColor("#FF5722"));
    // settings->addRecentFile("C:/projects/myapp/main.cpp");


    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "controlPanel_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    w = new MainWindow(cam);

   // w.setCamPrt(cam);

    w->show();
    return a.exec();
}
