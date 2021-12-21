#include <Arduino.h>
#include "USB.h"
#include "analog.h"
#include "voltage_monitor.h"
#include "pins.h"
#include "button.h"
#include "timer.h"
#include "NeoPixelBus.h"
#include "NeoPixelAnimator.h"
#include "motion.h"
#include "Wire.h"
#include "kxtj3-1057.h"
#include <ArduinoJSON.h>
#include "light_sensor.h"

#define ACCEL_SAMPLE_RATE 3.125 // HZ - Samples per second - 0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600Hz
#define ACCEL_RANGE 2           // Accelerometer range = 2, 4, 8, 16g

const uint8_t NUM_LEDS = 6;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixels(NUM_LEDS, LED_DATA_PIN);
RgbColor Red(30, 0, 0);
RgbColor Green(0, 30, 0);
RgbColor Blue(0, 0, 30);

USBCDC USBSerial;

Timer TmrAutosend;

VoltageMonitor Vbat(VBAT_MONITOR_PIN, 2.0);
VoltageMonitor Imon(CHARGE_I_PIN, 1.0);

Button Bttn(BUTTON_PIN, true);

KXTJ3 Accel(0x0E); // Address pin GND

LightSensor LightSens(LIGHT_SENSOR_READ_PIN, LIGHT_SENSOR_EN_PIN, 600); // night mV found by testing

void esp_sleep();

void configure_accel_int(bool polarity)
{
  Accel.intConf(20, 1, 3, polarity, true); // set accelerometer to cause interrupt on motion
}

void setup()
{
  pinMode(BUTTON_PIN, INPUT);
  pinMode(VBUS_PIN, INPUT);
  pinMode(VBAT_MONITOR_PIN, INPUT);
  pinMode(CHARGE_STATUS_PIN, INPUT);
  pinMode(LIGHT_SENSOR_EN_PIN, OUTPUT);
  pinMode(LIGHT_SENSOR_READ_PIN, INPUT);
  pinMode(ACCEL_INTERRUPT_PIN, INPUT);
  pinMode(SW_EN_PIN, OUTPUT);
  analog_setup();
  digitalWrite(SW_EN_PIN, 1);
  digitalWrite(LIGHT_SENSOR_EN_PIN, 1);

  Wire.setPins(ACCEL_I2C_SDA_PIN, ACCEL_I2C_SCL_PIN); // accel library uses Wire, for ESP32 set pins
  Accel.begin(ACCEL_SAMPLE_RATE, ACCEL_RANGE);
  configure_accel_int(true);

  Pixels.Begin();
  Pixels.Show();

  USBSerial.begin(115200);
  USB.begin();

  Pixels.SetPixelColor(0, Blue);
  Pixels.Show();

  delay(2000);
}

void send_all_readings()
{
  StaticJsonDocument<200> Readings;

  Readings["vbat_v"] = Vbat.get_mV() / 1000.0;
  Readings["button_pressed"] = (int)Bttn.is_pressed();
  Readings["charge_stat"] = digitalRead(CHARGE_STATUS_PIN);
  Readings["vbus"] = digitalRead(VBUS_PIN);
  Readings["charge_i_mA"] = Imon.get_mV() / 10.0;
  Readings["light_sensor_V"] = LightSens.read_mV() / 1000.0;
  Readings["accel_int"] = digitalRead(ACCEL_INTERRUPT_PIN);
  int intr = (int)Accel.isMotionInt();
  Readings["i2c_int"] = intr;

  Readings["accel_x"] = Accel.axisAccel(X);
  Readings["accel_y"] = Accel.axisAccel(Y);
  Readings["accel_z"] = Accel.axisAccel(Z);

  serializeJson(Readings, USBSerial);
  USBSerial.println();
}

bool autosend = false;

void loop()
{
  Bttn.update();

  if (USBSerial.available() > 0)
  {
    char byte = USBSerial.read();
    if (byte == 'R')
    {
      send_all_readings();
    }
    else if (byte == 'L')
    {
      int led_num = USBSerial.parseInt();
      int r = USBSerial.parseInt();
      int g = USBSerial.parseInt();
      int b = USBSerial.parseInt();
      RgbColor color(r, g, b);
      Pixels.SetPixelColor(led_num, color);
      Pixels.Show();
    }
    else if (byte == 'S')
    {
      esp_sleep();
    }
    else if (byte == 'E')
    {
      int on = USBSerial.parseInt();
      if (on == 1)
      {
        digitalWrite(SW_EN_PIN, 1);
      }
      else if (on == 0)
      {
        digitalWrite(SW_EN_PIN, 0);
      }
    }
    else if (byte == 'T')
    {
      int on = USBSerial.parseInt();
      if (on == 1)
      {
        Accel.setSelfTest(1);
      }
      else if (on == 0)
      {
        Accel.setSelfTest(0);
      }
    }
    else if (byte == 'P')
    {
      int on = USBSerial.parseInt();
      if (on == 1)
      {
        configure_accel_int(1);
      }
      else if (on == 0)
      {
        configure_accel_int(0);
      }
    }
    else if (byte == 'A')
    {
      autosend = true;
    }
    else if (byte == 'B')
    {
      button_state_t button_state = Bttn.get_state();
      switch (button_state)
      {
      case BUTTON_NONE:
        USBSerial.println("NONE");
        break;

      case BUTTON_SHORT_PRESS:
        USBSerial.println("SHORT_PRESS");
        break;

      case BUTTON_LONG_HOLD_START:
        USBSerial.println("LONG_HOLD_START");
        break;

      case BUTTON_LONG_HOLD_END:
        USBSerial.println("LONG_HOLD_END");
        break;

      case BUTTON_DOUPLE_PRESS:
        USBSerial.println("DOUBLE_PRESS");
        break;

      case BUTTON_TRIPLE_PRESS:
        USBSerial.println("TRIPLE_PRESS");
        break;
      }
    }
  }

  if (autosend)
  {
    if (TmrAutosend.time_passed(100))
    {
      send_all_readings();
    }
  }
}

void esp_sleep()
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
  uint64_t wake_pins = (1 << VBUS_PIN | 1 << BUTTON_PIN);
  esp_sleep_enable_ext1_wakeup(wake_pins, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
}