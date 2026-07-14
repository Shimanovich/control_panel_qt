#ifndef SBGCPROTOCOL_H
#define SBGCPROTOCOL_H

#include "sbgc.hpp"
#include <QByteArray>
#include <functional>
#include <cstdint>

/**
 * @class SbgcProtocol
 * @brief Qt-адаптированный класс для формирования и отправки команд SimpleBGC Serial API (protocol v2 with CRC16).
 * 
 * Используется совместно с UdpWorker. 
 * Вызывающий код (например gimbalLogic) должен добавлять байт device_address в начало UDP-пакета.
 * 
 * Пример использования:
 *   SbgcProtocol* proto = new SbgcProtocol();
 *   proto->setSendFunction([](const QByteArray& pkt){ udpWorker->enqueueSend(pkt); });
 *   proto->sendCmdCtrl_ext(SBGC_CONTROL_MODE_SPEED, SBGC_CONTROL_MODE_ANGLE_REL_FRAME, az, el);
 */
class SbgcProtocol
{
public:
    explicit SbgcProtocol() = default;
    ~SbgcProtocol() = default;

    /**
     * @brief Устанавливает функцию отправки готового SBGC-пакета (без device address).
     *        В вызывающем коде (gimbalLogic) обернуть: [device_addr] + pkt
     */
    void setSendFunction(std::function<void(const QByteArray& packet)> func);

    // ====================== КОМАНДЫ УПРАВЛЕНИЯ (минимум по запросу) ======================

    /** Отправить базовую позицию (mode RC, все скорости/углы 0) */
    void send_message_base_pos();

    /**
     * @brief Расширенное управление осями (Az/Yaw и El/Pitch)
     * @param modeAz  SBGC_CONTROL_MODE_SPEED или SBGC_CONTROL_MODE_ANGLE_REL_FRAME
     * @param modeEl  SBGC_CONTROL_MODE_SPEED или SBGC_CONTROL_MODE_ANGLE_REL_FRAME
     * @param az      скорость или угол для YAW
     * @param el      скорость или угол для PITCH
     */
    void sendCmdCtrl_ext(uint8_t modeAz, uint8_t modeEl, int16_t az, int16_t el);

    // Дополнительные полезные методы (можно раскомментировать/добавить по необходимости)
    void sendCmdCtrl_pos(int16_t az, int16_t el);
    void sendCmdCtrl_speed(int16_t az, int16_t el);
    void send_message_motor_on();
    void send_message_reset_engine();
    void send_message_periodic(); // запрос REALTIME_DATA_4

private:
    std::function<void(const QByteArray& packet)> m_sendFunction;

    SerialCommand_t cmdout;

    void sendcmd();

    void crc16_update(uint16_t length, uint8_t *data, uint8_t crc[2]);
    void crc16_calculate(uint16_t length, uint8_t *data, uint8_t crc[2]);
};

#endif // SBGCPROTOCOL_H
