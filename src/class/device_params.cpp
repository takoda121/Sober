#include "classes.hpp"
#include "app_state.hpp"

JNI::jclass device_params_class = nullptr;

static JNI::jfieldID app_build_variant_field = nullptr;
static JNI::jfieldID app_version_field = nullptr;
static JNI::jfieldID country_field = nullptr;
static JNI::jfieldID device_name_field = nullptr;
static JNI::jfieldID device_total_memory_mb_field = nullptr;
static JNI::jfieldID display_resolution_field = nullptr;
static JNI::jfieldID is_chrome_field = nullptr;
static JNI::jfieldID low_memory_killer_background_app_threshold_field = nullptr;
static JNI::jfieldID low_memory_killer_foreground_app_threshold_field = nullptr;
static JNI::jfieldID manufacturer_field = nullptr;
static JNI::jfieldID network_type_field = nullptr;
static JNI::jfieldID os_version_field = nullptr;

static JNI::jobject device_params = nullptr;

void create_device_params_class(JNI::JavaVM *vm) {
    device_params_class = JNI::create_class(vm, "com/roblox/engine/jni/model/DeviceParams");
    app_build_variant_field = device_params_class->field("appBuildVariant");
    app_version_field = device_params_class->field("appVersion");
    country_field = device_params_class->field("country");
    device_name_field = device_params_class->field("deviceName");
    device_total_memory_mb_field = device_params_class->field("deviceTotalMemoryMB");
    display_resolution_field = device_params_class->field("displayResolution");
    is_chrome_field = device_params_class->field("isChrome");
    low_memory_killer_background_app_threshold_field = device_params_class->field("lowMemoryKillerBackgroundAppThreshold");
    low_memory_killer_foreground_app_threshold_field = device_params_class->field("lowMemoryKillerForegroundAppThreshold");
    manufacturer_field = device_params_class->field("manufacturer");
    network_type_field = device_params_class->field("networkType");
    os_version_field = device_params_class->field("osVersion");
}

JNI::jobject get_device_params() {
    if (device_params)
        return device_params;

    device_params = new JNI::Object(device_params_class);
    device_params->fields.emplace(app_build_variant_field, JNI::create_string("googleProdRelease"));
    device_params->fields.emplace(app_version_field, JNI::create_string(roblox_version));
    device_params->fields.emplace(country_field, JNI::create_string(""));
    device_params->fields.emplace(device_name_field, JNI::create_string(""));
    device_params->fields.emplace(device_total_memory_mb_field, static_cast<JNI::jint>(device_memory_mb));
    device_params->fields.emplace(display_resolution_field, JNI::create_string(std::to_string(display_width) + "x" + std::to_string(display_height)));
    device_params->fields.emplace(is_chrome_field, static_cast<JNI::jboolean>(true));
    device_params->fields.emplace(low_memory_killer_background_app_threshold_field, static_cast<JNI::jlong>(10000));
    device_params->fields.emplace(low_memory_killer_foreground_app_threshold_field, static_cast<JNI::jlong>(10000));
    device_params->fields.emplace(manufacturer_field, JNI::create_string(""));
    device_params->fields.emplace(network_type_field, JNI::create_string("WiFi"));
    device_params->fields.emplace(os_version_field, JNI::create_string(std::to_string(ANDROID_SDK_VERSION)));
    return device_params;
}
