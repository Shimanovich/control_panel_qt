#include "cameralogic.h"
#include "gimbalLogic.h"
#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <Settings.h>

#include <QCoreApplication>
#include <QDebug>

MainWindow * w;
cameraLogic * cam;


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // Устанавливаем название компании и приложения
    QCoreApplication::setOrganizationName("MyCompany");
    QCoreApplication::setApplicationName("MyAwesomeApp");

    Settings* settings = Settings::instance();

    cam  = new cameraLogic();
    cam->loadCameraSettings(settings);

    gimbalLogic *gim = new gimbalLogic();           // ← добавить
    gim->loadGimbalSettings(settings);              // ← если метод есть (аналогично камере)


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

    w = new MainWindow(cam, gim);

    // w.setCamPrt(cam);

    w->show();
    int ret = a.exec();

    // Cleanup
    delete w;
    delete cam;
    delete gim;
    // Singleton not deleted (intentional for now)

    return ret;
}