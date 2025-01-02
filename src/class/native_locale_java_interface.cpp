#include "jni.hpp"

JNI::jclass native_locale_java_interface_class;

static std::string roblox_locale = "en_us";


static JNI::ValueVariant get_locale(JNI::Class *, va_list) {
    static JNI::jstring locale_string = JNI::create_string(roblox_locale);
    return locale_string;
}

void create_native_locale_java_interface_class(JNI::JavaVM *vm) {
    native_locale_java_interface_class = JNI::create_class(vm, "com/roblox/engine/jni/locale/NativeLocaleJavaInterface");
    native_locale_java_interface_class->static_method("getLocale", get_locale);
    native_locale_java_interface_class->static_method("getRobloxLocale", get_locale);
    native_locale_java_interface_class->static_method("getGameLocale", get_locale);
}
