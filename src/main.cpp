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
#include "grow.h"
#include "jump.h"
#include "wifi_mode.h"
#include "timer.h"
#include "WiFi.h"
#include <WiFiUdp.h>
#include <coap-simple.h>
#include "wifi_control.h"

const char *ssid = "lil-lite";
const char *password = "letterrip";

const uint8_t NUM_LEDS = 6;

RTC_DATA_ATTR float bat_level_mAh = 300.0;
RTC_DATA_ATTR light_state_t light_state = POWERING_ON;
RTC_DATA_ATTR uint16_t moving_pattern_num = 0;

Button Bttn(BUTTON_PIN, true);
KXTJ3 Accel(0x0E); // Address pin GND
Motion Mot(&Accel);
VoltageMonitor Vbat(VBAT_MONITOR_PIN, 2.0);
VoltageMonitor Ichrg(CHARGE_I_PIN, 1.0);
Power Pwr(&Vbat, &Ichrg, CHARGE_STATUS_PIN, VBUS_PIN, 500.0, 27.0); // 500mAh battery
Patman Patterns(NUM_LEDS, LED_DATA_PIN, &Pwr);
LightSensor LightSens(LIGHT_SENSOR_READ_PIN, LIGHT_SENSOR_EN_PIN, 600); // night mV found by testing

USBCDC USBSerial;

Timer DebugTimer;

WiFiUDP Udp;
Coap Cp(Udp);
WifiControl WifiCont(&Cp, &Accel, &USBSerial);

// Colors
RgbColor Red(255, 0, 0);
RgbColor Yellow(235, 25, 0);
RgbColor Green(0, 30, 0);
RgbColor Purple(90, 66, 245);
RgbColor Pink(245, 66, 182);
// Patterns
Flash FlashRed(&Patterns, Red);
Flash FlashPink(&Patterns, Pink);
Fade FadeAmber(&Patterns, Yellow);
Fade ChargeDone(&Patterns, Green);
Charge ChargeProgress(&Patterns, &Pwr);
PowerOn PoweringOn(&Patterns);
BatteryGauge BattGauge(&Patterns, &Pwr);
PowerOff PoweringOff(&Patterns);
Grow GrowRed(&Patterns, Red);
Jump JumpPurple(&Patterns, Purple);
WifiMode WifiControlling(&Patterns);

const uint16_t num_pats_moving = 4;
Pattern *PatsMoving[10] =
    {
        &FlashRed,
        &FlashPink,
        &GrowRed,
        &JumpPurple};

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
              &FadeAmber,
              &WifiControlling,
              PatsMoving,
              num_pats_moving);

void esp_sleep(bool motion_wake, bool tinmer_wake);

void setup()
{
  FlashRed.current_mA = 33.6;
  FlashPink.current_mA = 65.2;
  GrowRed.current_mA = 63.8;
  JumpPurple.current_mA = 37.0;
  FadeAmber.current_mA = 53.2;

  analog_setup();
  Wire.setPins(ACCEL_I2C_SDA_PIN, ACCEL_I2C_SCL_PIN); // accel library uses Wire, for ESP32 set pins

  Pwr.set_battery_level_mAh(bat_level_mAh);           // restore battery capacity from last saved value
  LilLite.set_moving_pattern_num(moving_pattern_num); // restore pattern number from last saved value

  LilLite.begin(light_state);

  light_state_t start_state = LilLite.get_state();

  if (start_state == DAY_RIDING)
  {
    esp_sleep(false, true);
  }
  else if (start_state == PARKED)
  {
    esp_sleep(true, false);
  }

  USBSerial.begin(115200);
  USB.begin();
}

light_state_t old_state = POWERING_ON;

void loop()
{
  LilLite.update();

  light_state_t state = LilLite.get_state();

  if (state == OFF)
  {
    esp_sleep(false, false);
  }
  else if (state == PARKED)
  {
    esp_sleep(true, false);
  }
  else if (state == WIFI_CONTROL)
  {
    if (!WifiCont.is_enabled())
    {
      WiFi.softAP(ssid, password);
      WifiCont.begin();
      Accel.begin(50, 2);
    }
    WifiCont.update();
  }
}

void esp_sleep(bool motion_wake, bool timer_wake)
{
  int i;
  for (i = 0; i < 100; i++) // wait for VBUS and button to go low before sleeping
  {
    if (digitalRead(VBUS_PIN) == 0 && !Bttn.is_pressed())
    {
      break;
    }
    delay(50);
  }
  if (i > 99)
  {
    return; // if vbus fails to go low, do not sleep
  }
  bat_level_mAh = Pwr.get_battery_level_mAh();           // backup battery level
  moving_pattern_num = LilLite.get_moving_pattern_num(); // backup moving pattern num
  uint64_t wake_pins = (1 << VBUS_PIN | 1 << BUTTON_PIN);
  if (motion_wake)
  {
    wake_pins |= 1 << ACCEL_INTERRUPT_PIN;
  }
  light_state = LilLite.get_state();
  esp_sleep_enable_ext1_wakeup(wake_pins, ESP_EXT1_WAKEUP_ANY_HIGH);
  if (timer_wake)
  {
    esp_sleep_enable_timer_wakeup(1000000 * 15); // 15 seconds
  }
  esp_deep_sleep_start();
}