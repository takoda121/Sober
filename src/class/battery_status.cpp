#include "classes.hpp"

JNI::jclass battery_status_class = nullptr;
static JNI::jobject battery_status = nullptr;

static JNI::jfieldID battery_low_field = nullptr;
static JNI::jfieldID battery_percentage_field = nullptr;
static JNI::jfieldID charge_counter_field = nullptr;
static JNI::jfieldID current_average_field = nullptr;
static JNI::jfieldID current_now_field = nullptr;
static JNI::jfieldID energy_counter_field = nullptr;
static JNI::jfieldID health_field = nullptr;
static JNI::jfieldID plugged_field = nullptr;
static JNI::jfieldID power_field = nullptr;
static JNI::jfieldID present_field = nullptr;
static JNI::jfieldID status_field = nullptr;
static JNI::jfieldID temperature_field = nullptr;
static JNI::jfieldID voltage_field = nullptr;

void create_battery_status_class(JNI::JavaVM *vm) {
    battery_status_class = JNI::create_class(vm, "com/roblox/engine/jni/model/BatteryStatus");
    battery_low_field = battery_status_class->field("batteryLow");
    battery_percentage_field = battery_status_class->field("batteryPercentage");
    charge_counter_field = battery_status_class->field("chargeCounter");
    current_average_field = battery_status_class->field("currentAverage");
    current_now_field = battery_status_class->field("currentNow");
    energy_counter_field = battery_status_class->field("energyCounter");
    health_field = battery_status_class->field("health");
    plugged_field = battery_status_class->field("plugged");
    power_field = battery_status_class->field("power");
    present_field = battery_status_class->field("present");
    status_field = battery_status_class->field("status");
    // TECHNOLOGY FIELD NOT IMPLEMENTED
    temperature_field = battery_status_class->field("temperature");
    voltage_field = battery_status_class->field("voltage");
}

JNI::jobject get_battery_status() {
    if (battery_status)
        return battery_status;

    JNI::jobject battery_status = new JNI::Object(battery_status_class);
    battery_status->fields.emplace(battery_low_field, create_java_lang_boolean(false));
    battery_status->fields.emplace(battery_percentage_field, create_java_lang_integer(100));
    battery_status->fields.emplace(charge_counter_field, create_java_lang_integer(3000000));
    battery_status->fields.emplace(current_average_field, create_java_lang_integer(1000000));
    battery_status->fields.emplace(current_now_field, create_java_lang_integer(1000000));
    battery_status->fields.emplace(energy_counter_field, create_java_lang_long(36000000000));
    battery_status->fields.emplace(health_field, create_java_lang_integer(2));
    battery_status->fields.emplace(plugged_field, create_java_lang_integer(1));
    battery_status->fields.emplace(power_field, create_java_lang_integer(12));
    battery_status->fields.emplace(present_field, create_java_lang_boolean(true));
    battery_status->fields.emplace(status_field, create_java_lang_integer(2));
    battery_status->fields.emplace(temperature_field, create_java_lang_integer(20));
    battery_status->fields.emplace(voltage_field, create_java_lang_integer(12));
    return battery_status;
}
