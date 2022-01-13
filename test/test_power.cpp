#include <ArduinoFake.h>
#include <unity.h>

#include "motion.h"
// #include "power.h"
#include "voltage_monitor.h"
#include "power.h"

using namespace fakeit;

void simulate_vbus_connected()
{
    const uint8_t vbus_pin = 2;
    When(Method(ArduinoFake(), digitalRead).Using(vbus_pin)).AlwaysReturn(1);
}

void simulate_vbus_not_connected()
{
    const uint8_t vbus_pin = 2;
    When(Method(ArduinoFake(), digitalRead).Using(vbus_pin)).AlwaysReturn(0);
}

void simulate_charge_stat_high()
{
    const uint8_t charge_stat_pin = 3;
    When(Method(ArduinoFake(), digitalRead).Using(charge_stat_pin)).AlwaysReturn(1);
}

void simulate_charge_stat_low()
{
    const uint8_t charge_stat_pin = 3;
    When(Method(ArduinoFake(), digitalRead).Using(charge_stat_pin)).AlwaysReturn(0);
}

void test_states(void)
{

    VoltageMonitor Vbat;
    VoltageMonitor Ichrg;
    const uint8_t vbus_pin = 2;
    const uint8_t charge_stat_pin = 3;

    Power Pwr(&Vbat, &Ichrg, charge_stat_pin, vbus_pin, 400, 10.0);

    simulate_vbus_connected();
    simulate_charge_stat_low();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus = 1 and charge stat = 0");

    simulate_vbus_connected();
    simulate_charge_stat_high();
    Pwr._Tmr_VBUS.MOCK_time_passed(50);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus = 1 for less than 100ms and charge stat = 1");

    simulate_vbus_connected();
    simulate_charge_stat_high();
    Pwr._Tmr_VBUS.MOCK_time_passed(150);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(USB_POWER, Pwr.get_state(), "state = USB_POWER when vbus = 1 for greater than 100ms and charge stat = 1");

    simulate_vbus_connected();
    simulate_charge_stat_low();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus = 1 and charge stat = 0");

    simulate_vbus_connected();
    simulate_charge_stat_high();
    Pwr._Tmr_VBUS.MOCK_time_passed(50);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus = 1 for less than 100ms and charge stat = 1");

    simulate_vbus_not_connected();
    simulate_charge_stat_high();
    Vbat.MOCK_get_mV(3750);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");

    simulate_vbus_not_connected();
    simulate_charge_stat_high();
    Vbat.MOCK_get_mV(3000);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(LOW_BATTERY, Pwr.get_state(), "state = LOW_BATTERY when vbus disconnected and battery low");
}

void test_capacity(void)
{
    VoltageMonitor Vbat;
    VoltageMonitor Ichrg;
    const uint8_t vbus_pin = 2;
    const uint8_t charge_stat_pin = 3;

    Power Pwr(&Vbat, &Ichrg, charge_stat_pin, vbus_pin, 400, 20.0);

    simulate_vbus_connected();
    simulate_charge_stat_low();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus > 5000 and charge stat = 0");

    // A. test battery reset to full when moving from CHARGING state to USB_POWER state (battery fully charged)

    simulate_vbus_connected();
    simulate_charge_stat_high();
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
    simulate_vbus_not_connected();
    Vbat.MOCK_get_mV(3750);
    simulate_charge_stat_high();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");

    const uint32_t ms_to_min_mult = 1000 * 60;
    Pwr.set_battery_load_current(0.0); // just the base current of 20mA
    Pwr._Tmr_load.MOCK_time_passed(45 * ms_to_min_mult);
    Pwr.set_battery_load_current(80.0);
    Pwr._Tmr_load.MOCK_time_passed(5 * ms_to_min_mult);
    Pwr.set_battery_load_current(0.0);
    Pwr.update();
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(94, Pwr.get_battery_percent(), "battery percent is 94 after example loads set when in USB_POWER state");

    // D. change load current and then switch state to CHARGING, check that battery capacity is updated
    Pwr.set_battery_load_current(80.0);
    Pwr._Tmr_load.MOCK_time_passed(5 * ms_to_min_mult);

    simulate_vbus_connected();
    simulate_charge_stat_low();
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
    simulate_vbus_not_connected();
    Vbat.MOCK_get_mV(3750);
    simulate_charge_stat_high();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");

    simulate_vbus_not_connected();
    Vbat.MOCK_get_mV(3000);
    simulate_charge_stat_high();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(LOW_BATTERY, Pwr.get_state(), "state = LOW_BATTERY when vbus disconnected and battery low");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, Pwr.get_battery_percent(), "battery percent is 0 on entering LOW_BATTERY state");

    simulate_vbus_connected();
    simulate_charge_stat_low();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus > 5000 and charge stat = 0");

    // I. Check that battery cannot be discharged to below 0%

    simulate_vbus_connected();
    simulate_charge_stat_high();
    Pwr.update();

    Pwr._Tmr_VBUS.MOCK_time_passed(150);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(USB_POWER, Pwr.get_state(), "state = USB_POWER when vbus > 5000 for greater than 100ms and charge stat = 1");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(100, Pwr.get_battery_percent(), "battery percent is 100 after charge complete");

    simulate_vbus_not_connected();
    Vbat.MOCK_get_mV(3750);
    simulate_charge_stat_high();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");
    Pwr._Tmr_load.MOCK_time_passed(0);
    Pwr.set_battery_load_current(380.0);
    Pwr._Tmr_load.MOCK_time_passed(120 * ms_to_min_mult);
    Pwr.update();
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, Pwr.get_battery_percent(), "battery percent is 0 after discharging more than capacity");
}

void test_unplug(void)
{
    VoltageMonitor Vbat;
    VoltageMonitor Ichrg;
    const uint8_t vbus_pin = 2;
    const uint8_t charge_stat_pin = 3;

    Power Pwr(&Vbat, &Ichrg, charge_stat_pin, vbus_pin, 400, 10.0);

    simulate_vbus_not_connected();
    simulate_charge_stat_high();
    Vbat.MOCK_get_mV(3750);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");

    simulate_vbus_connected();
    simulate_charge_stat_low();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus = 1 and charge stat = 0");

    Ichrg.MOCK_get_mV(900);
    Pwr._Tmr_load.MOCK_time_passed(1000000);
    Pwr.update();
    Pwr._Tmr_load.MOCK_time_passed(1000000);
    Pwr.update();

    simulate_vbus_connected();
    simulate_charge_stat_high();
    Pwr.update();
    Pwr._Tmr_VBUS.MOCK_time_passed(150);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(USB_POWER, Pwr.get_state(), "state = USB_POWER when vbus = 1 for greater than 100ms and charge stat = 1");

    float bat_cap = Pwr.get_battery_level_mAh();
    TEST_ASSERT_EQUAL(400.0, bat_cap);

    Pwr.set_battery_load_current(100.0);
    Pwr._Tmr_load.MOCK_time_passed(1000 * 60 * 60);
    Pwr.update();

    bat_cap = Pwr.get_battery_level_mAh();
    TEST_ASSERT_EQUAL_MESSAGE(400.0, bat_cap, "No charge gets removed while plugged in");

    simulate_vbus_not_connected();
    simulate_charge_stat_high();
    Vbat.MOCK_get_mV(3750);
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(BATTERY_POWER, Pwr.get_state(), "state = BATTERY_POWER when vbus disconnected and battery not low");

    simulate_vbus_connected();
    simulate_charge_stat_low();
    Pwr.update();
    TEST_ASSERT_EQUAL_INT_MESSAGE(CHARGING, Pwr.get_state(), "state = CHARGING when vbus = 1 and charge stat = 0");

    bat_cap = Pwr.get_battery_level_mAh();
    TEST_ASSERT_EQUAL(400.0, bat_cap);
}

void test_power()
{
    RUN_TEST(test_states);
    RUN_TEST(test_capacity);
    RUN_TEST(test_unplug);
}