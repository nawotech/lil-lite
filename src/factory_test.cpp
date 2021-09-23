
/*#include <Arduino.h>
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

#define ACCEL_SAMPLE_RATE 3.125 // HZ - Samples per second - 0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600Hz
#define ACCEL_RANGE 2           // Accelerometer range = 2, 4, 8, 16g

const uint8_t NUM_LEDS = 6;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixels(NUM_LEDS, LED_DATA_PIN);
RgbColor Red(30, 0, 0);
RgbColor Green(0, 30, 0);
RgbColor Blue(0, 0, 30);

USBCDC USBSerial;

Timer DebugTimer;

VoltageMonitor Vbus(VBUS_MONITOR_PIN, 2.96078);
VoltageMonitor Vbat(VBAT_MONITOR_PIN, 2.0);

Button Bttn(BUTTON_PIN, true);

KXTJ3 Accel(0x0E); // Address pin GND
Motion Move(&Accel);

static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == ARDUINO_USB_EVENTS)
  {
    arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
    switch (event_id)
    {
    case ARDUINO_USB_STARTED_EVENT:
      Serial.println("USB PLUGGED");
      break;
    case ARDUINO_USB_STOPPED_EVENT:
      Serial.println("USB UNPLUGGED");
      break;
    case ARDUINO_USB_SUSPEND_EVENT:
      Serial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
      break;
    case ARDUINO_USB_RESUME_EVENT:
      Serial.println("USB RESUMED");
      break;

    default:
      break;
    }
  }
  else if (event_base == ARDUINO_USB_CDC_EVENTS)
  {
    arduino_usb_cdc_event_data_t *data = (arduino_usb_cdc_event_data_t *)event_data;
    switch (event_id)
    {
    case ARDUINO_USB_CDC_CONNECTED_EVENT:
      Serial.println("CDC CONNECTED");
      break;
    case ARDUINO_USB_CDC_DISCONNECTED_EVENT:
      Serial.println("CDC DISCONNECTED");
      break;
    case ARDUINO_USB_CDC_LINE_STATE_EVENT:
      Serial.printf("CDC LINE STATE: dtr: %u, rts: %u\n", data->line_state.dtr, data->line_state.rts);
      break;
    case ARDUINO_USB_CDC_LINE_CODING_EVENT:
      Serial.printf("CDC LINE CODING: bit_rate: %u, data_bits: %u, stop_bits: %u, parity: %u\n", data->line_coding.bit_rate, data->line_coding.data_bits, data->line_coding.stop_bits, data->line_coding.parity);
      break;
    case ARDUINO_USB_CDC_RX_EVENT:
      Serial.printf("CDC RX: %u\n", data->rx.len);
      {
        uint8_t buf[data->rx.len];
        size_t len = USBSerial.read(buf, data->rx.len);
        Serial.write(buf, len);
      }
      break;

    default:
      break;
    }
  }
}

void setup()
{
  pinMode(BUTTON_PIN, INPUT);
  pinMode(VBUS_MONITOR_PIN, INPUT);
  pinMode(VBAT_MONITOR_PIN, INPUT);
  pinMode(CHARGE_STATUS_PIN, INPUT);
  pinMode(LIGHT_SENSOR_EN_PIN, OUTPUT);
  pinMode(LIGHT_SENSOR_READ_PIN, INPUT);
  pinMode(SW_EN_PIN, OUTPUT);
  analog_setup();
  digitalWrite(SW_EN_PIN, 1);
  digitalWrite(LIGHT_SENSOR_EN_PIN, 1);

  Wire.setPins(ACCEL_I2C_SDA_PIN, ACCEL_I2C_SCL_PIN); // accel library uses Wire, for ESP32 set pins
  Accel.begin(ACCEL_SAMPLE_RATE, ACCEL_RANGE);
  Accel.intConf(20, 1, 2, true); // set accelerometer to cause interrupt on motion

  Pixels.Begin();
  Pixels.Show();

  USB.onEvent(usbEventCallback);
  USBSerial.onEvent(usbEventCallback);
  USBSerial.begin(115200);
  USB.begin();

  Pixels.SetPixelColor(0, Blue);
  Pixels.Show();

  delay(2000);
}

void loop()
{
start:

  USBSerial.println("FACTORY TEST");
  USBSerial.println("---------------------------------------------");

  USBSerial.println("PRESS BUTTON TO START");
  while (1)
  {
    // if USB is unplugged, sleep
    if (Vbus.get_mV() < 4000)
    {
      uint64_t wake_pins = (1 << VBUS_MONITOR_PIN);
      esp_sleep_enable_ext1_wakeup(wake_pins, ESP_EXT1_WAKEUP_ANY_HIGH);
      esp_deep_sleep_start();
    }
    if (Bttn.is_pressed())
    {
      break;
    }
    delay(100);
  }

  while (1)
  {
    if (!Bttn.is_pressed())
    {
      break;
    }
  }

  delay(1000);

  USBSerial.println("1. VBUS MONITOR");
  uint16_t vbus = Vbus.get_mV();
  if (vbus >= 5000 && vbus <= 5400)
  {
    USBSerial.println("PASS");
  }
  else
  {
    USBSerial.println("FAIL");
    goto start;
  }
  USBSerial.println(vbus);
  USBSerial.println("");

  delay(1000);

  USBSerial.println("2. BUTTON, NOT PRESSED");
  bool button = Bttn.is_pressed();

  if (!button)
  {
    USBSerial.println("PASS");
  }
  else
  {
    USBSerial.println("FAIL");
    goto start;
  }
  USBSerial.println("");

  delay(1000);

  USBSerial.println("3. BUTTON, PRESSED, PRESS WITHIN THE NEXT 10 SEC");
  DebugTimer.reset();
  while (1)
  {
    if (DebugTimer.time_passed(10000))
    {
      USBSerial.println("FAIL");
      goto start;
    }

    if (Bttn.is_pressed())
    {
      USBSerial.println("PASS");
      break;
    }
  }
  USBSerial.println("");

  delay(1000);

  USBSerial.println("4. ACCEL, MOTION, SHAKE BOARD WITHIN 10 SEC");

  delay(1000);

  while (Accel.isMotionInt())
  {
  }

  while (1)
  {
    if (DebugTimer.time_passed(10000))
    {
      USBSerial.println("FAIL");
      goto start;
    }

    if (Accel.isMotionInt())
    {
      USBSerial.println("PASS");
      break;
    }
  }
  USBSerial.println("");

  delay(1000);

  USBSerial.println("4. ACCEL, NO MOTION, PLACE BOARD ON TABLE");

  while (1)
  {
    if (DebugTimer.time_passed(20000))
    {
      USBSerial.println("FAIL");
      goto start;
    }

    if (!Accel.isMotionInt())
    {
      USBSerial.println("PASS");
      break;
    }
  }
  USBSerial.println("");

  delay(1000);

  USBSerial.println("5. LEDS, PRESS BUTTON IF ALL LEDS TURN RED, GREEN, BLUE");
  USBSerial.println("");

  int color = 0;

  while (1)
  {
    if (Bttn.is_pressed())
    {
      Pixels.ClearTo(0);
      Pixels.SetPixelColor(0, Blue);
      Pixels.Show();
      break;
    }
    if (DebugTimer.time_passed(700))
    {
      if (color == 0)
      {
        Pixels.ClearTo(Red);
      }
      else if (color == 1)
      {
        Pixels.ClearTo(Green);
      }
      else if (color == 2)
      {
        Pixels.ClearTo(Blue);
      }
      Pixels.Show();
      color++;
      if (color > 2)
      {
        color = 0;
      }
    }
  }

  delay(1000);

  USBSerial.println("6. BATTERY MONITOR VOLTAGE");
  uint16_t vbat = Vbat.get_mV();
  USBSerial.println(vbat);
  if (vbat >= 3500 && vbat <= 4210)
  {
    USBSerial.println("PASS");
  }
  else
  {
    USBSerial.println("FAIL");
    goto start;
  }
  USBSerial.println("");

  delay(1000);

  USBSerial.println("7. CHARGE STATUS PIN = LOW (CHARGING)");
  if (!digitalRead(CHARGE_STATUS_PIN))
  {
    USBSerial.println("PASS");
  }
  else
  {
    USBSerial.println("FAIL");
    goto start;
  }
  USBSerial.println("");

  delay(1000);

  USBSerial.println("8. PRESS BUTTON IF LIGHT SENSOR VALUES CHANGE FROM LIGHT TO DARK");

  while (1)
  {
    if (DebugTimer.time_passed(1000))
    {
      USBSerial.println(analog_get_mV(LIGHT_SENSOR_READ_PIN));
    }

    if (Bttn.is_pressed())
    {
      break;
    }
  }

  USBSerial.println("TEST ALL PASSED! :)");

  delay(1000);
}
*/