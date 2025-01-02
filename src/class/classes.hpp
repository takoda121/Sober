#pragma once

#include "jni.hpp"

extern JNI::jclass session_reporter_interface_class;
extern JNI::jclass native_gl_interface_class;
extern JNI::jclass settings_interface_class;
extern JNI::jclass app_bridge_interface_class;
extern JNI::jclass cookie_protocol_class;
extern JNI::jclass user_java_interface_class;
extern JNI::jclass platform_params_class;
extern JNI::jclass device_params_class;
extern JNI::jclass init_params_class;
extern JNI::jclass start_app_params_class;
extern JNI::jclass start_game_params_class;
extern JNI::jclass java_lang_integer_class;
extern JNI::jclass java_lang_boolean_class;
extern JNI::jclass java_lang_float_class;
extern JNI::jclass java_lang_long_class;
extern JNI::jclass battery_status_class;
extern JNI::jclass native_locale_java_interface_class;
extern JNI::jclass fmod_class;
extern JNI::jclass audio_device_class;
extern JNI::jclass surface_class;
extern JNI::jclass input_interface_class;

int audio_interface_init();
void audio_interface_close();
void init_classes(JNI::JavaVM *);

JNI::jobject create_java_lang_integer(int value);
JNI::jobject create_java_lang_boolean(bool value);
JNI::jobject create_java_lang_float(float value);
JNI::jobject create_java_lang_long(long value);

JNI::jobject get_platform_params();
JNI::jobject get_device_params();

void set_start_game_params(long place_id, long user_id, int join_request_type, const std::string &join_attempt_id, const std::string &join_attempt_origin, const std::string &referral_page);

JNI::jobject get_battery_status();
