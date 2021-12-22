#ifndef WIFI_CONTROL_H
#define WIFI_CONTROL_H

#include "WiFi.h"
#include <WiFiUdp.h>
#include <coap-simple.h>
#include "kxtj3-1057.h"

class WifiControl
{
public:
    WifiControl(Coap *Cp, KXTJ3 *Accel);
    void begin();
    void update();
    bool is_enabled();

private:
    Coap *_Cp;
    KXTJ3 *_Accel;
    void callback_accel_read(CoapPacket &packet, IPAddress ip, int port);
    bool _enabled = false;
};

#endif