#ifndef __KXTJ3_IMU_H__
#define __KXTJ3_IMU_H__

#include <stdint.h>

// Return values
typedef enum
{
	IMU_SUCCESS,
	IMU_HW_ERROR,
	IMU_NOT_SUPPORTED,
	IMU_GENERIC_ERROR,
	IMU_OUT_OF_BOUNDS,
	IMU_ALL_ONES_WARNING
} kxtj3_status_t;

typedef enum
{
	X = 0,
	Y,
	Z,
} axis_t;

class KXTJ3
{
public:
	kxtj3_status_t begin(float SampleRate, uint8_t accRange);
	kxtj3_status_t intConf(uint16_t threshold, uint8_t moveDur, uint8_t naDur, bool polarity = 1);
	float axisAccel(axis_t _axis);
	bool isMotionInt();

	void MOCK_isMotionInt(bool return_val);

	bool is_motion_int = false;
};

#endif