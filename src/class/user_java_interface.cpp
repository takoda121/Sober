#include "classes.hpp"
#include "app_state.hpp"

JNI::jclass user_java_interface_class = nullptr;

void create_user_java_interface_class(JNI::JavaVM *vm) {
    user_java_interface_class = JNI::create_class(vm, "com/roblox/engine/jni/user/NativeUserJavaInterface");
    user_java_interface_class->static_method("getUserId", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jlong>(user_state.user_id);
    });
    user_java_interface_class->static_method("getIsUnder13", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jboolean>(user_state.under_13);
    });
    user_java_interface_class->static_method("getUsername", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string(user_state.username);
    });
    user_java_interface_class->static_method("getDisplayName", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string(user_state.display_name);
    });
    user_java_interface_class->static_method("getAlternateName", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    user_java_interface_class->static_method("getPlatformName", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    user_java_interface_class->static_method("getMembershipType", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jint>(user_state.membership_type);
    });
    user_java_interface_class->static_method("getTheme", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string(user_state.theme);
    });
}
