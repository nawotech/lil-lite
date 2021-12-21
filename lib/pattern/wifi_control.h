#ifndef WIFI_CONTROL_H
#define WIFI_CONTROL_H

#include "patman.h"

class WifiControl : public Pattern
{
public:
    WifiControl(Patman *Pat);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color;
    uint16_t _speed;
    Patman *_Pat;
};

#endif