#ifndef FLASH_H
#define FLASH_H

#include "patman.h"

class Flash : public Pattern
{
public:
    Flash(Patman *Pat, RgbColor color);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color;
    void re_start();
    uint16_t _count = 0;
    Patman *_Pat;
};

#endif