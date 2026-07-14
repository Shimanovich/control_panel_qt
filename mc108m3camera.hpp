// ========================================================
//  mc108m3camera.hpp
//  Управление камерой MC-108-M3 (VISCA)
//  Заголовочный файл
// ========================================================

#pragma once

#include <QByteArray>
#include <functional>

/*
 * @class MC108M3Camera
 * @brief Класс для управления камерой MC-108-M3 по протоколу VISCA
 *
 */


class MC108M3Camera
{
public:
    explicit MC108M3Camera() = default;
    ~MC108M3Camera() = default;

    // ====================== НАСТРОЙКА ======================

    /**
     * @brief Устанавливает функцию отправки данных в камеру (serial, TCP и т.д.)
     * @param func Функция, принимающая готовый VISCA-пакет
     */
    void setSendFunction(std::function<void(const QByteArray& packet)> func);

    /**
     * @brief Устанавливает callback для получения любых сырых данных от камеры
     * @param func Callback, вызываемый при поступлении данных
     */
    void setResponseCallback(std::function<void(const QByteArray& data)> func);

    /**
     * @brief Устанавливает callback при получении ACK (команда принята)
     */
    void setAckCallback(std::function<void()> func);

    /**
     * @brief Устанавливает callback при получении Completion (команда выполнена)
     */
    void setCompletionCallback(std::function<void()> func);

    /**
     * @brief Устанавливает callback при ошибке VISCA
     * @param func Callback, получающий код ошибки
     */
    void setErrorCallback(std::function<void(int errorCode)> func);

    // ====================== ИСПОЛЬЗОВАНИЕ ======================

    /**
     * @brief Должен вызываться каждый раз, когда приходят данные от камеры
     * @param data Полученные байты от камеры
     */
    void handleIncomingData(const QByteArray& data);

    // ====================== КОМАНДЫ ======================

    /** Прямая установка положения зума (0x0000 – 0x7FFF) */
    void sendZoomDirect(uint16_t zoomPosition);

    void sendDZoomOn();
    void sendDZoomOff();

    void sendFocusAuto();
    void sendFocusInfinity();

    void sendBrightnessUp();
    void sendBrightnessDown();

    void sendDefogOn();
    void sendDefogOff();

    /**
     * @brief Управление ICR (Night mode)
     * @param on true — Night (ICR On), false — Day (ICR Off)
     */
    void sendICR(bool on);

private:
    std::function<void(const QByteArray& packet)> m_sendFunction;
    std::function<void(const QByteArray& data)>   m_responseCallback;
    std::function<void()>                         m_ackCallback;
    std::function<void()>                         m_completionCallback;
    std::function<void(int)>                      m_errorCallback;

    void sendViscaCommand(const QByteArray& payload);
    void parseResponse(const QByteArray& data);
};
