#ifndef PINS_H
#define PINS_H
#include <Arduino.h>

// define all microcontroller pins

// power
#define SW_EN_PIN 38

// LEDs
#define LED_DATA_PIN 11

// acceleromter
#define ACCEL_INTERRUPT_PIN 5
#define ACCEL_I2C_SCL_PIN 34
#define ACCEL_I2C_SDA_PIN 33

// light sensor
#define LIGHT_SENSOR_EN_PIN 37
#define LIGHT_SENSOR_READ_PIN 8

// button
#define BUTTON_PIN 17

// power
#define CHARGE_STATUS_PIN 9
#define CHARGE_I_PIN 10
#define VBAT_MONITOR_PIN 7
#define VBUS_PIN 6

// USB data lines
#define USB_D_POS_PIN 20
#define USB_D_NEG_PIN 19

#endif