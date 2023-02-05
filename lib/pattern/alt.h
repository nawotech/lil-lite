#ifndef ALT_H
#define ALT_H

#include "patman.h"

class Alt : public Pattern
{
public:
    Alt(Patman *Pat, RgbColor color1, RgbColor color2);
    void start();

private:
    void update_function(const AnimationParam &param);
    RgbColor _color1;
    RgbColor _color2;
    void re_start();
    Patman *_Pat;
};

#endif