#include "classes.hpp"
#include "jni.hpp"


void create_android_classes(JNI::JavaVM *vm);
void create_session_reporter_interface_class(JNI::JavaVM *vm);
void create_native_gl_interface_class(JNI::JavaVM *vm);
void create_user_java_interface_class(JNI::JavaVM *vm);
void create_platform_params_class(JNI::JavaVM *vm);
void create_device_params_class(JNI::JavaVM *vm);
void create_init_params_class(JNI::JavaVM *vm);
void create_start_app_params_class(JNI::JavaVM *vm);
void create_start_game_params_class(JNI::JavaVM *vm);
void create_native_locale_java_interface_class(JNI::JavaVM *vm);
void create_audio_interface_classes(JNI::JavaVM *vm);
void create_java_primitive_classes(JNI::JavaVM *vm);
void create_battery_status_class(JNI::JavaVM *vm);


void init_classes(JNI::JavaVM *vm) {
    create_android_classes(vm);
    create_session_reporter_interface_class(vm);
    create_native_gl_interface_class(vm);
    create_user_java_interface_class(vm);
    create_platform_params_class(vm);
    create_device_params_class(vm);
    create_init_params_class(vm);
    create_start_app_params_class(vm);
    create_start_game_params_class(vm);
    create_native_locale_java_interface_class(vm);
    create_audio_interface_classes(vm);
    create_java_primitive_classes(vm);
    create_battery_status_class(vm);
}
