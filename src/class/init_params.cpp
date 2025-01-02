#include "classes.hpp"
#include "app_state.hpp"


JNI::jclass init_params_class = nullptr;

void create_init_params_class(JNI::JavaVM *vm) {
    init_params_class = JNI::create_class(vm, "com/roblox/engine/jni/autovalue/InitParams");
    init_params_class->method("baseURL", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("https://www.roblox.com/");
    });
    init_params_class->method("buildVariant", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("googleProdRelease");
    });
    init_params_class->method("deviceParams", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return get_device_params();
    });
    init_params_class->method("isTablet", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jboolean>(false);
    });
    init_params_class->method("isVrDevice", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jboolean>(false);
    });
    init_params_class->method("platformParams", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return get_platform_params();
    });
    init_params_class->method("userAgent", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return JNI::create_string(user_agent);
    });
    init_params_class->method("vrContext", +[](JNI::Object *, va_list) -> JNI::ValueVariant {
        return nullptr;
    });
}
