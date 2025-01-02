#include "classes.hpp"
#include "app_state.hpp"

JNI::jclass session_reporter_interface_class = nullptr;

void create_session_reporter_interface_class(JNI::JavaVM *vm) {
    session_reporter_interface_class = JNI::create_class(vm, "com/roblox/engine/jni/reporter/SessionReporterJavaInterface");
    session_reporter_interface_class->static_method("getFilesDir", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string(get_files_dir());
    });
    session_reporter_interface_class->static_method("setEventTrackingGoogleAnalytics", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return 0;
    });
    session_reporter_interface_class->static_method("getAppVersion", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string(roblox_version);
    });
    session_reporter_interface_class->static_method("getLastLoggedInUser", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    session_reporter_interface_class->static_method("getLastLoggedInUserId", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });
    session_reporter_interface_class->static_method("sendSessionReport", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return 0;
    });
}
