#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cameralogic.h"
#include <QMainWindow>
#include <QMessageBox>

#include <QTimer>
#include "SDL2/SDL.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(cameraLogic *camPtr, QWidget *parent = nullptr);
    ~MainWindow();

    void setCamPrt(cameraLogic *camPtr);

private slots:
    void onSetServerClicked();
    void onError(const QString &message);

    void on_ZoomWide_clicked();
    void on_zoomTele_clicked();

    void on_radioAutofokus_clicked();

    void on_radioInfFokus_clicked();

    void on_pushButton_Bright_minus_clicked();

    void on_pushButton_Bright_plus_clicked();

private:


    Ui::MainWindow *ui = nullptr;
    cameraLogic    *m_cam = nullptr;

private:
    QTimer *m_joystickTimer = nullptr;
    SDL_Joystick *m_joystick = nullptr;
    SDL_JoystickID m_joystickID = -1;

    void initJoystick();
    void processJoystick();
    void updateJoystickStatus(const QString &status);
};

#endif // MAINWINDOW_H
