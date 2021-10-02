#include <ArduinoFake.h>
#include <unity.h>

#include "motion.h"
// #include "power.h"
#include "voltage_monitor.h"
#include "power.h"

using namespace fakeit;

void test_states(void)
{

    VoltageMonitor Vbus;
    VoltageMonitor Vbat;
    VoltageMonitor Ichrg;
    const uint8_t charge_stat_pin = 3;

    Power Pwr(&Vbus, &Vbat, &Ichrg, charge_stat_pin, 400);

    Vbus.MOCK_get_mV(5000);
    When(Method(ArduinoFake(), digitalRead)).Return(0);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus > 5000 and charge stat = 0");

    Vbus.MOCK_get_mV(5000);
    Pwr._Tmr_VBUS.MOCK_time_passed(50);
    When(Method(ArduinoFake(), digitalRead)).Return(1);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus > 5000 for less than 100ms and charge stat = 1");

    Vbus.MOCK_get_mV(5000);
    Pwr._Tmr_VBUS.MOCK_time_passed(150);
    When(Method(ArduinoFake(), digitalRead)).Return(1);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(USB_POWER, Pwr.get_state(), "state = USB_POWER when vbus > 5000 for greater than 100ms and charge stat = 1");

    Vbus.MOCK_get_mV(5000);
    When(Method(ArduinoFake(), digitalRead)).Return(0);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus > 5000 and charge stat = 0");

    Vbus.MOCK_get_mV(5000);
    Pwr._Tmr_VBUS.MOCK_time_passed(50);
    When(Method(ArduinoFake(), digitalRead)).Return(1);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus > 5000 for less than 100ms and charge stat = 1");

    Vbus.MOCK_get_mV(100);
    Vbat.MOCK_get_mV(3750);
    When(Method(ArduinoFake(), digitalRead)).Return(1);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");

    Vbus.MOCK_get_mV(100);
    Vbat.MOCK_get_mV(3000);
    When(Method(ArduinoFake(), digitalRead)).Return(1);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(LOW_BATTERY, Pwr.get_state(), "state = LOW_BATTERY when vbus disconnected and battery low");
}

void test_capacity(void)
{
    VoltageMonitor Vbus;
    VoltageMonitor Vbat;
    VoltageMonitor Ichrg;
    const uint8_t charge_stat_pin = 3;

    Power Pwr(&Vbus, &Vbat, &Ichrg, charge_stat_pin, 400);

    Vbus.MOCK_get_mV(5000);
    When(Method(ArduinoFake(), digitalRead)).Return(0);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus > 5000 and charge stat = 0");

    // A. test battery reset to full when moving from CHARGING state to USB_POWER state (battery fully charged)

    Vbus.MOCK_get_mV(5000);
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(1);
    Pwr.update();

    Pwr._Tmr_VBUS.MOCK_time_passed(150);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(USB_POWER, Pwr.get_state(), "state = USB_POWER when vbus > 5000 for greater than 100ms and charge stat = 1");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(100, Pwr.get_battery_percent(), "battery percent is 100 after charge complete");

    // B. test when not being powered by battery (state NOT BATTERY_POWER) the battery level stays constant
    Pwr.set_battery_load_current(100.0);
    Pwr._Tmr_load.MOCK_time_passed(1000000);
    Pwr.set_battery_load_current(200.0);
    Pwr.update();
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(100, Pwr.get_battery_percent(), "battery percent stays at 100 when in USB_POWER state");

    // C. test when powered by battery, setting the load subtracts the appropriate level from battery
    // example- 30mA for 30 min, and then 100mA for 5 min, this is 23.33 mAh, 400 - 23.33 = ~6%
    Vbus.MOCK_get_mV(100);
    Vbat.MOCK_get_mV(3750);
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(1);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");

    const float ms_to_min_mult = 1000 * 60;
    Pwr.set_battery_load_current(30.0);
    Pwr._Tmr_load.MOCK_time_passed(30 * ms_to_min_mult);
    Pwr.set_battery_load_current(100.0);
    Pwr._Tmr_load.MOCK_time_passed(5 * ms_to_min_mult);
    Pwr.set_battery_load_current(0.0);
    Pwr.update();
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(94, Pwr.get_battery_percent(), "battery percent is 94 after example loads set when in USB_POWER state");

    // D. change load current and then switch state to CHARGING, check that battery capacity is updated
    Pwr.set_battery_load_current(100.0);
    Pwr._Tmr_load.MOCK_time_passed(5 * ms_to_min_mult);

    Vbus.MOCK_get_mV(5000);
    When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(0);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus > 5000 and charge stat = 0");

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(92, Pwr.get_battery_percent(), "battery percent is 92 after example load and state changed");

    // E. check that CHARGING state keeps track of charge current into battery
    Ichrg.MOCK_get_mV(1000); // 1000 mV = 100mA charge current
    Pwr._Tmr_load.MOCK_time_passed(30000);
    Pwr.update();
    for (int i = 0; i <= 30; i++)
    {
        Pwr._Tmr_load.MOCK_time_passed(30000);
        Pwr.update();
    }
    // 100mA * 30 sec * 30 = ~ 6% expect new capacity will be 92+5 = 98%
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(98, Pwr.get_battery_percent(), "battery percent is 92 after charge state running");

    // F. check that changing the charge current gives an accurate capacity
    Ichrg.MOCK_get_mV(100); // 100 mV = 10mA charge current
    for (int i = 0; i <= 30; i++)
    {
        Pwr._Tmr_load.MOCK_time_passed(30000);
        Pwr.update();
    }
    // 100mA * 30 sec * 30 = ~ 1.5% expect new capacity will be 99%
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(99, Pwr.get_battery_percent(), "battery percent is 100 after charge current changed");

    // G. make sure capacity cannot go over 100%
    Ichrg.MOCK_get_mV(1000);
    for (int i = 0; i <= 30; i++)
    {
        Pwr._Tmr_load.MOCK_time_passed(30000);
        Pwr.update();
    }
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(100, Pwr.get_battery_percent(), "battery percent does not go above 100%");

    // H. when LOW BATTERY is entered, battery is reset to 0
    Vbus.MOCK_get_mV(100);
    Vbat.MOCK_get_mV(3750);
    When(Method(ArduinoFake(), digitalRead)).Return(1);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");

    Vbus.MOCK_get_mV(100);
    Vbat.MOCK_get_mV(3000);
    When(Method(ArduinoFake(), digitalRead)).Return(1);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(LOW_BATTERY, Pwr.get_state(), "state = LOW_BATTERY when vbus disconnected and battery low");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, Pwr.get_battery_percent(), "battery percent is 0 on entering LOW_BATTERY state");
}

void test_power()
{
    RUN_TEST(test_states);
    RUN_TEST(test_capacity);
}