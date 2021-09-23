#include <ArduinoFake.h>
#include <unity.h>

#include "test_motion.h"
#include "test_power.h"

void setUp(void)
{
    ArduinoFakeReset();
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    test_motion();
    test_power();

    UNITY_END();

    return 0;
}