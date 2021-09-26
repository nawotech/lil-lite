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
#include "patman.h"

#include "flash.h"
#include "fade.h"
#include "charge.h"

const uint8_t NUM_LEDS = 6;
RTC_DATA_ATTR float bat_level_mAh = 300.0;

Patman Patterns(NUM_LEDS, LED_DATA_PIN);
USBCDC USBSerial;
Timer DebugTimer;
Button Bttn(BUTTON_PIN, true);
KXTJ3 Accel(0x0E); // Address pin GND
Motion Mot(&Accel);
VoltageMonitor Vbus(VBUS_MONITOR_PIN, 2.96078);
VoltageMonitor Vbat(VBAT_MONITOR_PIN, 2.0);
VoltageMonitor Ichrg(CHARGE_I_PIN, 1.0);
Power Pwr(&Vbus, &Vbat, &Ichrg, CHARGE_STATUS_PIN, 400.0); // 400mAh battery

RgbColor Red(255, 0, 0);
RgbColor Yellow(235, 25, 0);

Flash FlashRed(&Patterns, Red);
Fade FadeAmber(&Patterns, Yellow);
Charge ChargeProgress(&Patterns, &Pwr);

typedef enum
{
  USB_CONNECTED,
  CHARGE,
  CHARGE_DONE,
  POWERING_ON,
  ON,
  POWERING_OFF,
  OFF
} light_state_t;

light_state_t state = ON;

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
  Pwr.set_battery_level_mAh(bat_level_mAh); // restore battery capacity from last saved value (before sleeping)

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
  Mot.begin();

  Patterns.begin();

  USB.onEvent(usbEventCallback);
  USBSerial.onEvent(usbEventCallback);
  USBSerial.begin(115200);
  USB.begin();
}

void sleep()
{
  bat_level_mAh = Pwr.get_battery_level_mAh(); // backup battery level
  uint64_t wake_pins = (1 << VBUS_MONITOR_PIN || 1 << BUTTON_PIN);
  esp_sleep_enable_ext1_wakeup(wake_pins, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
}

void pattern()
{
  Mot.update();
  motion_state_t mot_state = Mot.get_state();

  // motion state changed
  switch (mot_state)
  {
  case MOTION_STOPPED:
    Patterns.set_pattern(&FadeAmber);
    break;

  case MOTION_START_MOVING:
    break;

  case MOTION_MOVING:
    Patterns.set_pattern(&FlashRed);
    break;

  case MOTION_BRAKING:
    break;

  case MOTION_PARKED:
    break;
  }
}

void loop()
{
  Patterns.update();
  Pwr.update();
  Bttn.update();

  light_state_t new_state = state;

  if (Bttn.get_state() == BUTTON_LONG_HOLD_END)
  {
    new_state = OFF;
  }

  switch (state)
  {
  case POWERING_ON:
    break;

  case ON:
    if (Pwr.get_state() == CHARGING)
    {
      new_state = CHARGE;
    }
    pattern();
    break;

  case POWERING_OFF:
    break;

  case OFF:
    sleep();
    break;

  case CHARGE:
    if (Pwr.get_state() == BATTERY_POWER)
    {
      new_state = OFF;
    }
    break;
  }

  if (new_state != state)
  {
    if (new_state == CHARGE)
    {
      Patterns.set_pattern(&ChargeProgress);
    }

    state = new_state;
  }
}