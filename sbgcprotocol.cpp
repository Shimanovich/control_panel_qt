#include "sbgcprotocol.hpp"
#include <QDebug>

void SbgcProtocol::setSendFunction(std::function<void(const QByteArray& packet)> func)
{
    m_sendFunction = func;
}

void SbgcProtocol::crc16_update(uint16_t length, uint8_t *data, uint8_t crc[2]) {
    uint16_t counter;
    uint16_t polynom = 0x8005;
    uint16_t crc_register = (uint16_t) crc[0] | ((uint16_t) crc[1] << 8);
    uint8_t shift_register;
    uint8_t data_bit, crc_bit;
    for (counter = 0; counter < length; counter++) {
        for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
            data_bit = (data[counter] & shift_register) ? 1 : 0;
            crc_bit = crc_register >> 15;
            crc_register <<= 1;
            if (data_bit != crc_bit)
                crc_register ^= polynom;
        }
    }
    crc[0] = crc_register;
    crc[1] = (crc_register >> 8);
}

void SbgcProtocol::crc16_calculate(uint16_t length, uint8_t *data, uint8_t crc[2]) {
    crc[0] = 0;
    crc[1] = 0;
    crc16_update(length, data, crc);
}

void SbgcProtocol::sendcmd()
{
    if (cmdout.size > (SBGC_CMD_MAX_BYTES - SBGC_CMD_NON_PAYLOAD_BYTES)) {
        qWarning() << "SBGC: wrong cmd size";
        return;
    }

    cmdout.start = SBGC_CMD_START_BYTE;
    cmdout.hcs   = cmdout.id + cmdout.size;

    QByteArray packet;
    packet.append(reinterpret_cast<const char*>(&cmdout.start), 4); // start + id + size + hcs

    // CRC16 over header (from id) + payload
    crc16_calculate(3, reinterpret_cast<uint8_t*>(&cmdout.id), cmdout.CRCsumm);
    if (cmdout.size > 0) {
        crc16_update(cmdout.size, cmdout.payloadData, cmdout.CRCsumm);
    }

    if (cmdout.size > 0) {
        packet.append(reinterpret_cast<const char*>(cmdout.payloadData), cmdout.size);
    }
    packet.append(reinterpret_cast<const char*>(cmdout.CRCsumm), 2);

    if (m_sendFunction) {
        m_sendFunction(packet);
    } else {
        qWarning() << "SBGC: sendFunction not set!";
    }
}

// ====================== РЕАЛИЗАЦИЯ МЕТОДОВ ======================

void SbgcProtocol::send_message_base_pos()
{
    cmdout.id = SBGC_CMD_CONTROL;
    cmdout.cmd_control.mode       = SBGC_CONTROL_MODE_RC;
    cmdout.cmd_control.speedPITCH = 0;
    cmdout.cmd_control.speedYAW   = 0;
    cmdout.cmd_control.speedROLL  = 0;
    cmdout.cmd_control.anglePITCH = 0;
    cmdout.cmd_control.angleROLL  = 0;
    cmdout.cmd_control.angleYAW   = 0;
    cmdout.size = sizeof(SBGC_cmd_control_t);
    sendcmd();
}

void SbgcProtocol::sendCmdCtrl_ext(uint8_t modeAz, uint8_t modeEl, int16_t az, int16_t el)
{
    cmdout.id = SBGC_CMD_CONTROL;

    cmdout.cmd_control_ext.mode[0] = 0;
    cmdout.cmd_control_ext.data[0].angle = 0;
    cmdout.cmd_control_ext.data[0].speed = 0;

    cmdout.cmd_control_ext.mode[1] = modeEl;

    if (modeEl == SBGC_CONTROL_MODE_SPEED) {
        cmdout.cmd_control_ext.data[1].angle = 0;
        cmdout.cmd_control_ext.data[1].speed = el;
    } else if (modeEl == SBGC_CONTROL_MODE_ANGLE_REL_FRAME) {
        cmdout.cmd_control_ext.data[1].angle = el;
        cmdout.cmd_control_ext.data[1].speed = 0;
    } else {
        cmdout.cmd_control_ext.data[1].angle = 0;
        cmdout.cmd_control_ext.data[1].speed = 0;
    }

    cmdout.cmd_control_ext.mode[2] = modeAz;

    if (modeAz == SBGC_CONTROL_MODE_SPEED) {
        cmdout.cmd_control_ext.data[2].angle = 0;
        cmdout.cmd_control_ext.data[2].speed = az;
    } else if (modeAz == SBGC_CONTROL_MODE_ANGLE_REL_FRAME) {
        cmdout.cmd_control_ext.data[2].angle = az;
        cmdout.cmd_control_ext.data[2].speed = 0;
    } else {
        cmdout.cmd_control_ext.data[2].angle = 0;
        cmdout.cmd_control_ext.data[2].speed = 0;
    }

    cmdout.size = sizeof(SBGC_cmd_control_ext_t);
    sendcmd();
}

void SbgcProtocol::sendCmdCtrl_pos(int16_t az, int16_t el)
{
    cmdout.id = SBGC_CMD_CONTROL;
    cmdout.cmd_control.mode = SBGC_CONTROL_MODE_ANGLE_REL_FRAME;
    cmdout.cmd_control.speedPITCH = 0;
    cmdout.cmd_control.speedYAW   = 0;
    cmdout.cmd_control.speedROLL  = 0;
    cmdout.cmd_control.anglePITCH = el;
    cmdout.cmd_control.angleROLL  = 0;
    cmdout.cmd_control.angleYAW   = az;
    cmdout.size = sizeof(SBGC_cmd_control_t);
    sendcmd();
}

void SbgcProtocol::sendCmdCtrl_speed(int16_t az, int16_t el)
{
    cmdout.id = SBGC_CMD_CONTROL;
    cmdout.cmd_control.mode = SBGC_CONTROL_MODE_SPEED;
    cmdout.cmd_control.speedPITCH = el;
    cmdout.cmd_control.speedYAW   = az;
    cmdout.cmd_control.speedROLL  = 0;
    cmdout.cmd_control.anglePITCH = 0;
    cmdout.cmd_control.angleROLL  = 0;
    cmdout.cmd_control.angleYAW   = 0;
    cmdout.size = sizeof(SBGC_cmd_control_t);
    sendcmd();
}

void SbgcProtocol::send_message_motor_on()
{
    cmdout.id = SBGC_CMD_MOTORS_ON;
    cmdout.size = 0;
    sendcmd();
}

void SbgcProtocol::send_message_reset_engine()
{
    cmdout.id   = SBGC_CMD_RESET;
    cmdout.size = 0;
    sendcmd();
}

void SbgcProtocol::send_message_periodic()
{
    cmdout.id   = SBGC_CMD_REALTIME_DATA_4;
    cmdout.size = 0;
    sendcmd();
}
