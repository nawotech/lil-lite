#include "wifi_control.h"
#include <ArduinoJson.h>

WifiControl::WifiControl(Coap *Cp, KXTJ3 *Accel, USBCDC *USBSerial)
{
    _Cp = Cp;
    _Accel = Accel;
    _USBSerial = USBSerial;
}

void WifiControl::begin()
{
    _Cp->server(std::bind(&WifiControl::callback_accel_read, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), "accel");
    _Cp->server(std::bind(&WifiControl::callback_accel_settings, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), "accel_settings");
    // _Cp->response(callback_response);
    _Cp->start();
    _enabled = true;
}

void WifiControl::callback_accel_read(CoapPacket &packet, IPAddress ip, int port)
{
    DynamicJsonDocument Readings(100);
    Readings["x"] = _Accel->axisAccel(X);
    Readings["y"] = _Accel->axisAccel(Y);
    Readings["z"] = _Accel->axisAccel(Z);
    Readings["mot"] = (uint8_t)_Accel->isMotionInt();
    Readings["t_ms"] = millis();

    String reading;
    char reading_c[100];
    serializeJson(Readings, reading);
    reading.toCharArray(reading_c, 100);

    _Cp->sendResponse(ip, port, packet.messageid, reading_c);
}

void WifiControl::callback_accel_settings(CoapPacket &packet, IPAddress ip, int port)
{
    // send response
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = NULL;

    DynamicJsonDocument doc(300);
    deserializeJson(doc, p);

    int range = doc["range"];
    float sample_rate = doc["rate"];

    uint16_t threshold = doc["thres"];
    uint8_t moveDur = doc["move"];
    uint8_t naDur = doc["no_move"];
    float sampleRateHz = doc["wakeup_rate"];
    bool xPosEn = doc["x_p"];
    bool xNegEn = doc["x_n"];
    bool yPosEn = doc["y_p"];
    bool yNegEn = doc["y_n"];
    bool zPosEn = doc["z_p"];
    bool zNegEn = doc["z_n"];

    _Accel->begin(sample_rate, range);
    _Accel->intConf(threshold, moveDur, naDur, true, false, sampleRateHz);
    _Accel->intAxisConf(xPosEn, xNegEn, yPosEn, yNegEn, zPosEn, zNegEn);

/*
    _USBSerial->println(threshold);
    _USBSerial->println(moveDur);
    _USBSerial->println(naDur);
    _USBSerial->println(sampleRateHz);
    _USBSerial->println(xPosEn);
    _USBSerial->println(xNegEn);
    _USBSerial->println(yPosEn);
    _USBSerial->println(yNegEn);
    _USBSerial->println(zPosEn);
    _USBSerial->println(zNegEn);
    _USBSerial->println("");
    */
    _USBSerial->println(packet.payloadlen);
}

void WifiControl::update()
{
    _Cp->loop();
}

bool WifiControl::is_enabled()
{
    return _enabled;
}