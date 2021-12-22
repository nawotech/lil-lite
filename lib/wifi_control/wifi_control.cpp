#include "wifi_control.h"
#include <ArduinoJson.h>

WifiControl::WifiControl(Coap *Cp, KXTJ3 *Accel)
{
    _Cp = Cp;
    _Accel = Accel;
}

void WifiControl::begin()
{
    _Cp->server(std::bind(&WifiControl::callback_accel_read, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), "accel");
    // _Cp->response(callback_response);
    _Cp->start();
    _enabled = true;
}

void WifiControl::callback_accel_read(CoapPacket &packet, IPAddress ip, int port)
{
    StaticJsonDocument<50> Readings;
    Readings["x"] = _Accel->axisAccel(X);
    Readings["y"] = _Accel->axisAccel(Y);
    Readings["z"] = _Accel->axisAccel(Z);

    String reading;
    char reading_c[50];
    serializeJson(Readings, reading);
    reading.toCharArray(reading_c, 50);

    _Cp->sendResponse(ip, port, packet.messageid, reading_c);
}

void WifiControl::update()
{
    _Cp->loop();
}

bool WifiControl::is_enabled()
{
    return _enabled;
}