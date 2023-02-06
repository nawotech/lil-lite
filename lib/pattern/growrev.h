#ifndef GROWREV_H
#define GROWREV_H

#include "patman.h"

class GrowRev : public Pattern
{
public:
    GrowRev(Patman *Pat, RgbColor color);
    void start();

private:
    void update_function(const AnimationParam &param);
    void update_function_2(const AnimationParam &param);
    RgbColor _color;
    Patman *_Pat;
};

#endif