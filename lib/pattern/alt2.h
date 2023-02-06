#ifndef ALT2_H
#define ALT2_H

#include "patman.h"

class Alt2 : public Pattern
{
public:
    Alt2(Patman *Pat, RgbColor color1, RgbColor color2);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color1;
    RgbColor _color2;
    void re_start();
    Patman *_Pat;
};

#endif