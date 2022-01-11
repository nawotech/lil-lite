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
    void re_start();
    Patman *_Pat;
};

#endif