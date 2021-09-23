#include <ArduinoFake.h>
#include <unity.h>

#include "kxtj3-1057.h"
#include "motion.h"

void test_begin(void)
{
    // create fake accel for test
    KXTJ3 MockAccel;
    // init motion object to test
    Motion TestMotion(&MockAccel);

    TestMotion.begin();

    // check the initial state is stopped
    TEST_ASSERT(TestMotion.get_state() == MOTION_STOPPED);
}

void test_movement(void)
{
    // create fake accel for test
    KXTJ3 MockAccel;
    // init motion object to test
    Motion TestMotion(&MockAccel);

    TestMotion.begin();

    // check the initial state is stopped
    TEST_ASSERT(TestMotion.get_state() == MOTION_STOPPED);

    TestMotion.update();

    // check still stopped
    TEST_ASSERT(TestMotion.get_state() == MOTION_STOPPED);

    // simulate motion
    MockAccel.MOCK_isMotionInt(true);
    TestMotion.update();

    // check motion starts
    TEST_ASSERT(TestMotion.get_state() == MOTION_MOVING);

    TestMotion.update();

    TEST_ASSERT(TestMotion.get_state() == MOTION_MOVING);

    // simulate motion stopped
    MockAccel.MOCK_isMotionInt(false);
    TestMotion.update();

    TEST_ASSERT(TestMotion.get_state() == MOTION_STOPPED);
}

void test_motion()
{
    RUN_TEST(test_begin);
    RUN_TEST(test_movement);
}