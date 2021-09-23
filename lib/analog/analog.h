#ifndef ANALOG_H
#define ANALOG_H

#include <stdint.h>

// calibrate ADC, call in setup function
void analog_setup();

// reads analog pin
// averages n_samples
// returns voltage in mV
uint16_t analog_get_mV(uint8_t pin, uint8_t n_samples = 50);

#endif