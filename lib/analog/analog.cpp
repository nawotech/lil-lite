#include "analog.h"

#include <Arduino.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

esp_adc_cal_characteristics_t adc_chars;

void analog_setup()
{
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_13, 1100, &adc_chars);
}

uint16_t analog_get_mV(uint8_t pin, uint8_t n_samples)
{
    uint32_t sum = 0;
    for (int i = 0; i < n_samples; i++)
    {
        sum += analogRead(pin);
    }

    sum /= n_samples;
    uint32_t voltage = esp_adc_cal_raw_to_voltage(sum, &adc_chars);

    return voltage;
}