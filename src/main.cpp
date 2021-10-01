#include <Arduino.h>
#include "USB.h"
#include "analog.h"
#include "voltage_monitor.h"
#include "pins.h"
#include "button.h"
#include "timer.h"
#include "NeoPixelBus.h"
#include "motion.h"
#include "Wire.h"
#include "kxtj3-1057.h"
#include "power.h"
#include "patman.h"
#include "light_sensor.h"
#include "light.h"

#include "flash.h"
#include "fade.h"
#include "charge.h"
#include "power_on.h"
#include "battery_gauge.h"
#include "power_off.h"

#define CURRENT_IDLE_MA 27

const uint8_t NUM_LEDS = 6;
RTC_DATA_ATTR float bat_level_mAh = 300.0;

Patman Patterns(NUM_LEDS, LED_DATA_PIN);

Button Bttn(BUTTON_PIN, true);
KXTJ3 Accel(0x0E); // Address pin GND
Motion Mot(&Accel);
VoltageMonitor Vbus(VBUS_MONITOR_PIN, 2.96078);
VoltageMonitor Vbat(VBAT_MONITOR_PIN, 2.0);
VoltageMonitor Ichrg(CHARGE_I_PIN, 1.0);
Power Pwr(&Vbus, &Vbat, &Ichrg, CHARGE_STATUS_PIN, 400.0);              // 400mAh battery
LightSensor LightSens(LIGHT_SENSOR_READ_PIN, LIGHT_SENSOR_EN_PIN, 600); // night mV found by testing

USBCDC USBSerial;

// Colors
RgbColor Red(255, 0, 0);
RgbColor Yellow(235, 25, 0);
RgbColor Green(0, 30, 0);
// Patterns
Flash FlashRed(&Patterns, Red);
Fade FadeAmber(&Patterns, Yellow);
Fade ChargeDone(&Patterns, Green);
Charge ChargeProgress(&Patterns, &Pwr);
PowerOn PoweringOn(&Patterns);
BatteryGauge BattGauge(&Patterns, &Pwr);
PowerOff PoweringOff(&Patterns);

void sleep_cb();

Light LilLite(&LightSens,
              &Patterns,
              &Mot,
              &Bttn,
              &Pwr,
              SW_EN_PIN,
              &PoweringOn,
              &PoweringOff,
              &ChargeProgress,
              &ChargeDone,
              &BattGauge,
              &FlashRed,
              &FadeAmber);

void sleep(bool motion_wake);

void setup()
{
  analog_setup();
  Wire.setPins(ACCEL_I2C_SDA_PIN, ACCEL_I2C_SCL_PIN); // accel library uses Wire, for ESP32 set pins

  Pwr.set_battery_level_mAh(bat_level_mAh); // restore battery capacity from last saved value
  // Pwr.set_battery_load_current(CURRENT_IDLE_MA);

  LilLite.begin();

  USBSerial.begin(115200);
  USB.begin();

  Serial.begin(9600);
}

light_state_t old_state = POWERING_ON;

void loop()
{
  LilLite.update();

  light_state_t state = LilLite.get_state();

  if (state == OFF)
  {
    sleep(false);
  }
  else if (state == PARKED)
  {
    sleep(true);
  }

  if (state != old_state)
  {
    Serial.println(state);
    old_state = state;
  }
}

void sleep(bool motion_wake)
{
  int i;
  for (i = 0; i < 100; i++) // wait for VBUS and button to go low before sleeping
  {
    if (Vbus.get_mV() < 1000 && !Bttn.is_pressed())
    {
      break;
    }
    delay(50);
  }
  if (i > 99)
  {
    return; // if vbus fails to go low, do not sleep
  }
  bat_level_mAh = Pwr.get_battery_level_mAh(); // backup battery level
  uint64_t wake_pins = (1 << VBUS_MONITOR_PIN | 1 << BUTTON_PIN);
  if (motion_wake)
  {
    wake_pins |= 1 << ACCEL_INTERRUPT_PIN;
  }
  esp_sleep_enable_ext1_wakeup(wake_pins, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
}