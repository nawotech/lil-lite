#ifndef WIFI_MODE_H
#define WIFI_MODE_H

#include "patman.h"

class WifiMode : public Pattern
{
public:
    WifiMode(Patman *Pat);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color;
    uint16_t _speed;
    Patman *_Pat;
};

#endif