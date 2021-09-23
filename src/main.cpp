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
#include "power.h"

const uint8_t NUM_LEDS = 6;

RgbColor Red(255, 0, 0);
RgbColor Purple(255, 0, 255);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> Pixels(NUM_LEDS, LED_DATA_PIN);
NeoPixelAnimator Anim(1);
USBCDC USBSerial;
Timer DebugTimer;
VoltageMonitor Vbus(VBUS_MONITOR_PIN, 2.96078);
VoltageMonitor Vbat(VBAT_MONITOR_PIN, 2.0);
VoltageMonitor Ichrg(CHARGE_I_PIN, 1.0);
Button Bttn(BUTTON_PIN, true);
KXTJ3 Accel(0x0E); // Address pin GND
Motion Move(&Accel);
Power Pwr(&Vbus, &Vbat, &Ichrg, CHARGE_STATUS_PIN, 400.0); // 400mAh battery

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
  pinMode(CHARGE_I_PIN, INPUT);
  pinMode(LIGHT_SENSOR_EN_PIN, OUTPUT);
  pinMode(LIGHT_SENSOR_READ_PIN, INPUT);
  pinMode(SW_EN_PIN, OUTPUT);

  analog_setup();
  digitalWrite(SW_EN_PIN, 1);
  digitalWrite(LIGHT_SENSOR_EN_PIN, 1);

  Wire.setPins(ACCEL_I2C_SDA_PIN, ACCEL_I2C_SCL_PIN); // accel library uses Wire, for ESP32 set pins
  Move.begin();

  Pixels.Begin();
  Pixels.Show();

  USB.onEvent(usbEventCallback);
  USBSerial.onEvent(usbEventCallback);
  USBSerial.begin(115200);
  USB.begin();
}

void anim_fade_colors(const AnimationParam &param)
{
  float progress = NeoEase::CubicInOut(param.progress);

  RgbColor color = RgbColor::LinearBlend(Red, Purple, progress);

  Pixels.ClearTo(color);
}

bool forward = true;

void anim_move(const AnimationParam &param)
{
  float progress = NeoEase::CircularInOut(param.progress);

  uint8_t led_n;

  if (forward)
  {
    led_n = progress * NUM_LEDS;
  }
  else
  {
    led_n = (1.0 - progress) * NUM_LEDS;
  }

  Pixels.ClearTo(0);
  Pixels.SetPixelColor(led_n, Red);
}

void loop()
{
  if (DebugTimer.time_passed(1000))
  {
    // USBSerial.println(Vbus.get_mV());

    Move.update();

    // USBSerial.println(Move.get_state());

    Pwr.update();

    power_state_t pwr_state = Pwr.get_state();
    switch (pwr_state)
    {
    case USB_POWER:
      USBSerial.println("USB POWER");
      break;

    case CHARGING:
      USBSerial.println("CHARGING");
      USBSerial.println(Pwr.get_battery_percent());
      break;

    case BATTERY_POWER:
      USBSerial.println("BATTERY_POWER");
      USBSerial.println(Pwr.get_battery_percent());
      break;

    case LOW_BATTERY:
      USBSerial.println("LOW_BATTERY");
      break;
    }
  }

  button_state_t but_state = Bttn.get_state();
  if (but_state == BUTTON_SHORT_PRESS)
  {
    USBSerial.println("Short press");
  }
  else if (but_state == BUTTON_LONG_HOLD_START)
  {
    USBSerial.println("Long hold start");
  }
  else if (but_state == BUTTON_LONG_HOLD_END)
  {
    USBSerial.println("Long hold end");
  }

  Bttn.update();

  if (Anim.IsAnimating())
  {
    Anim.UpdateAnimations();
    Pixels.Show();
  }
  else
  {
    if (Pwr.get_state() == CHARGING)
    {
      uint8_t bat_lvl = Pwr.get_battery_percent();
      Pixels.ClearTo(0);
      Pixels.SetPixelColor(0, Purple);
      if (bat_lvl > 20)
      {
        Pixels.SetPixelColor(1, Purple);
      }
      if (bat_lvl > 40)
      {
        Pixels.SetPixelColor(2, Purple);
      }
      if (bat_lvl > 60)
      {
        Pixels.SetPixelColor(3, Purple);
      }
      if (bat_lvl > 80)
      {
        Pixels.SetPixelColor(4, Purple);
      }
      if (bat_lvl > 95)
      {
        Pixels.SetPixelColor(5, Purple);
      }
      Pixels.Show();
    }
    else if (Move.get_state() == MOTION_MOVING)
    {
      forward = !forward;
      Anim.StartAnimation(0, 300, anim_fade_colors);
    }
  }
}