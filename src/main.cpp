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
USBCDC USBSerial;
Timer LightTimer;
Button Bttn(BUTTON_PIN, true);
KXTJ3 Accel(0x0E); // Address pin GND
Motion Mot(&Accel);
VoltageMonitor Vbus(VBUS_MONITOR_PIN, 2.96078);
VoltageMonitor Vbat(VBAT_MONITOR_PIN, 2.0);
VoltageMonitor Ichrg(CHARGE_I_PIN, 1.0);
Power Pwr(&Vbus, &Vbat, &Ichrg, CHARGE_STATUS_PIN, 400.0);              // 400mAh battery
LightSensor LightSens(LIGHT_SENSOR_READ_PIN, LIGHT_SENSOR_EN_PIN, 600); // night mV found by testing

RgbColor Red(255, 0, 0);
RgbColor Yellow(235, 25, 0);
RgbColor Green(0, 30, 0);

Flash FlashRed(&Patterns, Red);
Fade FadeAmber(&Patterns, Yellow);
Fade FadeGreen(&Patterns, Green);
Charge ChargeProgress(&Patterns, &Pwr);
PowerOn PoweringOn(&Patterns);
BatteryGauge BattGauge(&Patterns, &Pwr);
PowerOff PoweringOff(&Patterns);

typedef enum
{
  POWERING_ON,
  BATTERY_GAUGE,
  ON,
  CHARGE,
  POWERING_OFF,
  OFF,
  PARKED,
  DAY_RIDING
} light_state_t;

light_state_t state;

bool did_button_wake();
void sleep(bool motion_wake);

void setup()
{
  analog_setup();

  LightSens.begin();
  Bttn.begin();
  Pwr.begin();

  pinMode(SW_EN_PIN, OUTPUT);
  digitalWrite(SW_EN_PIN, 0);

  delay(100);

  Pwr.set_battery_level_mAh(bat_level_mAh); // restore battery capacity from last saved value
  Pwr.set_battery_load_current(CURRENT_IDLE_MA);

  digitalWrite(SW_EN_PIN, 1);

  Wire.setPins(ACCEL_I2C_SDA_PIN, ACCEL_I2C_SCL_PIN); // accel library uses Wire, for ESP32 set pins
  Mot.begin();
  Patterns.begin();
  USBSerial.begin(115200);
  USB.begin();

  Patterns.set_pattern(&PoweringOn);

  state = POWERING_ON;
}

void loop()
{
  Pwr.update();
  Bttn.update();

  light_state_t new_state = state;

  button_state_t bttn_state = Bttn.get_state();

  if (bttn_state == BUTTON_LONG_HOLD_START)
  {
    Patterns.set_pattern(&PoweringOff);
    state = POWERING_OFF;
    new_state = POWERING_OFF;
  }

  Patterns.update();

  // HANDLE CURRENT STATE
  switch (state)
  {
  case POWERING_ON:
  {
    if (!Patterns.is_running())
    {
      if (Pwr.get_state() == BATTERY_POWER)
      {
        new_state = BATTERY_GAUGE;
      }
      else
      {
        new_state = ON;
      }
    }
    break;
  }

  case ON:
  {
    if (bttn_state == BUTTON_SHORT_PRESS)
    {
      new_state = BATTERY_GAUGE;
      break;
    }
    if (Pwr.get_state() == CHARGING || Pwr.get_state() == USB_POWER)
    {
      new_state = CHARGE;
    }
    else
    {
      Mot.update();
      motion_state_t mot_state = Mot.get_state();
      switch (mot_state)
      {
      case MOTION_STOPPED:
        Patterns.set_pattern(&FadeAmber);
        Pwr.set_battery_load_current(CURRENT_IDLE_MA + 25);
        break;

      case MOTION_START_MOVING:
        break;

      case MOTION_MOVING:
        Patterns.set_pattern(&FlashRed);
        Pwr.set_battery_load_current(CURRENT_IDLE_MA + 16);
        break;

      case MOTION_BRAKING:
        break;

      case MOTION_PARKED:
        new_state = PARKED;
        break;
      }
    }
    break;
  }

  case OFF:
  {
    sleep(false); // do not wake up on motion, only charging or button
    // if sleep returns, it failed, go back ON
    new_state = ON;
    break;
  }

  case PARKED:
  {
    sleep(true); // wake up on motion when parked
    // if sleep returns, it failed, go back ON
    new_state = ON;
    break;
  }

  case CHARGE:
  {
    if (Pwr.get_state() == BATTERY_POWER)
    {
      new_state = OFF;
    }
    else if (Pwr.get_state() == USB_POWER)
    {
      Patterns.set_pattern(&FadeGreen);
    }
    else if (Pwr.get_state() == CHARGING)
    {
      Patterns.set_pattern(&ChargeProgress);
    }
    break;
  }

  case BATTERY_GAUGE:
  {
    if (!Patterns.is_running())
    {
      new_state = ON;
    }
    break;
  }

  case POWERING_OFF:
  {
    if (!Patterns.is_running())
    {
      new_state = OFF;
    }
    if (!Bttn.is_pressed())
    {
      new_state = ON;
    }
    break;
  }
  }

  // HANDLE CHANGE STATE
  if (new_state != state)
  {
    if (new_state == BATTERY_GAUGE)
    {
      Patterns.set_pattern(&BattGauge);
    }

    state = new_state;
  }
}

bool did_button_wake()
{
  uint64_t button_mask = 1 << BUTTON_PIN;
  return (esp_sleep_get_ext1_wakeup_status() & button_mask);
}

void sleep(bool motion_wake)
{
  Patterns.blank_leds();
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