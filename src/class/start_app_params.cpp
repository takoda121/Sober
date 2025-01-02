#include "classes.hpp"
#include "app_state.hpp"

JNI::jclass start_app_params_class = nullptr;

void create_start_app_params_class(JNI::JavaVM *vm) {
    start_app_params_class = JNI::create_class(vm, "com/roblox/engine/jni/autovalue/StartAppParams");
    start_app_params_class->method("appStarterPlace", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("rbxasset://places/Mobile.rbxl");
    });
    start_app_params_class->method("appStarterScript", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("LuaAppStarterScript");
    });
    start_app_params_class->method("appUserId", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jlong>(user_state.user_id);
    });
    start_app_params_class->method("isUnder13", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jboolean>(user_state.under_13);
    });
    start_app_params_class->method("membershipType", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jint>(user_state.membership_type);
    });
    start_app_params_class->method("platformParams", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return get_platform_params();
    });
    start_app_params_class->method("selectedTheme", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string(user_state.theme);
    });
    start_app_params_class->method("surface", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return new JNI::Object();
    });
    start_app_params_class->method("username", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string(user_state.username);
    });
    start_app_params_class->method("vrContext", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        return nullptr;
    });
}
