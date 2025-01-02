#include "classes.hpp"
#include "app_state.hpp"

JNI::jclass platform_params_class = nullptr;

static JNI::jfieldID asset_folder_path_field = nullptr;
static JNI::jfieldID dpi_scale_field = nullptr;
static JNI::jfieldID is_keyboard_device_field = nullptr;
static JNI::jfieldID is_mouse_device_field = nullptr;
static JNI::jfieldID is_touch_device_field = nullptr;

static JNI::jobject platform_params = nullptr;

void create_platform_params_class(JNI::JavaVM *vm) {
    platform_params_class = JNI::create_class(vm, "com/roblox/engine/jni/model/PlatformParams");
    asset_folder_path_field = platform_params_class->field("assetFolderPath");
    dpi_scale_field = platform_params_class->field("dpiScale");
    is_keyboard_device_field = platform_params_class->field("isKeyboardDevice");
    is_mouse_device_field = platform_params_class->field("isMouseDevice");
    is_touch_device_field = platform_params_class->field("isTouchDevice");
}

JNI::jobject get_platform_params() {
    if (platform_params)
        return platform_params;

    platform_params = new JNI::Object(platform_params_class);
    platform_params->fields.emplace(asset_folder_path_field, JNI::create_string(get_assets_dir() / "content/"));
    platform_params->fields.emplace(dpi_scale_field, 1.0f);
    platform_params->fields.emplace(is_keyboard_device_field, true);
    platform_params->fields.emplace(is_mouse_device_field, true);
    platform_params->fields.emplace(is_touch_device_field, false);
    return platform_params;
}
