// ========================================================
//  mc108m3camera.cpp
//  Управление камерой MC-108-M3 (VISCA)
// ========================================================

#include "mc108m3camera.hpp"

#include <QDebug>

// ====================== НАСТРОЙКА CALLBACK'ОВ ======================

void MC108M3Camera::setSendFunction(std::function<void(const QByteArray& packet)> func)
{
    m_sendFunction = std::move(func);
}

void MC108M3Camera::setResponseCallback(std::function<void(const QByteArray& data)> func)
{
    m_responseCallback = std::move(func);
}

void MC108M3Camera::setAckCallback(std::function<void()> func)
{
    m_ackCallback = std::move(func);
}

void MC108M3Camera::setCompletionCallback(std::function<void()> func)
{
    m_completionCallback = std::move(func);
}

void MC108M3Camera::setErrorCallback(std::function<void(int errorCode)> func)
{
    m_errorCallback = std::move(func);
}

// ====================== РАБОТА С ДАННЫМИ ======================

void MC108M3Camera::handleIncomingData(const QByteArray& data)
{
    if (data.isEmpty()) return;

    qDebug() << "← VISCA получено:" << data.toHex(' ').toUpper();

    parseResponse(data);

    if (m_responseCallback)
        m_responseCallback(data);
}

void MC108M3Camera::sendViscaCommand(const QByteArray& payload)
{
    if (!m_sendFunction) {
        qWarning() << "Send function не задана! Вызовите setSendFunction() перед отправкой команд.";
        return;
    }

    QByteArray packet;
    packet.reserve(payload.size() + 2);
    packet.append(0x81);      // VISCA Header (адрес 1)
    packet.append(payload);
    packet.append(0xFF);      // Terminator

    m_sendFunction(packet);

    qDebug() << "→ VISCA отправлено:" << packet.toHex(' ').toUpper();
}

// ====================== КОМАНДЫ ======================

void MC108M3Camera::sendZoomDirect(uint16_t zoomPosition)
{
    QByteArray p = QByteArray::fromHex("010447");
    p.append((zoomPosition >> 12) & 0x0F);
    p.append((zoomPosition >> 8)  & 0x0F);
    p.append((zoomPosition >> 4)  & 0x0F);
    p.append(zoomPosition & 0x0F);
    sendViscaCommand(p);
}

void MC108M3Camera::sendDZoomOn()
{
    sendViscaCommand(QByteArray::fromHex("01040602"));
}

void MC108M3Camera::sendDZoomOff()
{
    sendViscaCommand(QByteArray::fromHex("01040603"));
}

void MC108M3Camera::sendFocusAuto()
{
    sendViscaCommand(QByteArray::fromHex("01043802"));
}

void MC108M3Camera::sendFocusInfinity()
{
    sendViscaCommand(QByteArray::fromHex("01041802"));
}

void MC108M3Camera::sendBrightnessUp()
{
    sendViscaCommand(QByteArray::fromHex("01040D02"));
}

void MC108M3Camera::sendBrightnessDown()
{
    sendViscaCommand(QByteArray::fromHex("01040D03"));
}

void MC108M3Camera::sendDefogOn()
{
    sendViscaCommand(QByteArray::fromHex("01046502"));
}

void MC108M3Camera::sendDefogOff()
{
    sendViscaCommand(QByteArray::fromHex("01046503"));
}

void MC108M3Camera::sendICR(bool on)
{
    sendViscaCommand(QByteArray::fromHex(on ? "01041302" : "01041301"));
}

// ====================== ПАРСИНГ ОТВЕТОВ ======================

void MC108M3Camera::parseResponse(const QByteArray& data)
{
    if (data.size() < 3 || data.back() != char(0xFF) || !data.startsWith("\x90"))
        return;

    const uint8_t second = static_cast<uint8_t>(data[1]);

    if ((second & 0xF0) == 0x40) {           // ACK
        if (m_ackCallback) m_ackCallback();
    }
    else if ((second & 0xF0) == 0x50) {      // Completion
        if (m_completionCallback) m_completionCallback();
    }
    else if ((second & 0xF0) == 0x60) {      // Error
        int errCode = second & 0x0F;
        if (m_errorCallback) m_errorCallback(errCode);
        qWarning() << "VISCA Error code:" << errCode;
    }
}
