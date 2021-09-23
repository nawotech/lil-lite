#include "kxtj3-1057.h"

kxtj3_status_t KXTJ3::begin(float SampleRate, uint8_t accRange)
{
	return IMU_GENERIC_ERROR;
}

kxtj3_status_t KXTJ3::intConf(uint16_t threshold, uint8_t moveDur, uint8_t naDur, bool polarity)
{
	return IMU_GENERIC_ERROR;
}

float KXTJ3::axisAccel(axis_t _axis)
{
	return 0.0;
}

bool KXTJ3::isMotionInt()
{
	return is_motion_int;
}

void KXTJ3::MOCK_isMotionInt(bool return_val)
{
	is_motion_int = return_val;
}