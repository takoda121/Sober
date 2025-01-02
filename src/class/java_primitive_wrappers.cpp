// This file used for battery state only, does not contain full functionality.
// Contains wrappers for integer, boolean, float, and long

#include "classes.hpp"

JNI::jclass java_lang_integer_class = nullptr;
JNI::jclass java_lang_boolean_class = nullptr;
JNI::jclass java_lang_float_class = nullptr;
JNI::jclass java_lang_long_class = nullptr;

static JNI::jfieldID java_lang_integer_value = nullptr;
static JNI::jfieldID java_lang_boolean_value = nullptr;
static JNI::jfieldID java_lang_float_value = nullptr;
static JNI::jfieldID java_lang_long_value = nullptr;

void create_java_primitive_classes(JNI::JavaVM *vm) {
    java_lang_integer_class = JNI::create_class(vm, "java/lang/Integer");
    java_lang_integer_value = java_lang_integer_class->field("value");
    java_lang_integer_class->method("intValue", +[](JNI::Object *object, va_list) -> JNI::ValueVariant {
        return std::get<JNI::jint>(object->fields.at(java_lang_integer_value).value);
    });
    java_lang_integer_class->method("floatValue", +[](JNI::Object *object, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jfloat>(std::get<JNI::jint>(object->fields.at(java_lang_integer_value).value));
    });

    java_lang_boolean_class = JNI::create_class(vm, "java/lang/Boolean");
    java_lang_boolean_value = java_lang_boolean_class->field("value");
    java_lang_boolean_class->method("booleanValue", +[](JNI::Object *object, va_list) -> JNI::ValueVariant {
        return std::get<JNI::jboolean>(object->fields.at(java_lang_boolean_value).value);
    });

    java_lang_float_class = JNI::create_class(vm, "java/lang/Float");
    java_lang_float_value = java_lang_float_class->field("value");
    java_lang_float_class->method("floatValue", +[](JNI::Object *object, va_list) -> JNI::ValueVariant {
        return std::get<JNI::jfloat>(object->fields.at(java_lang_float_value).value);
    });

    java_lang_long_class = JNI::create_class(vm, "java/lang/Long");
    java_lang_long_value = java_lang_long_class->field("value");
    java_lang_long_class->method("longValue", +[](JNI::Object *object, va_list) -> JNI::ValueVariant {
        return std::get<JNI::jlong>(object->fields.at(java_lang_long_value).value);
    });
}

JNI::jobject create_java_lang_integer(int value) {
    JNI::jobject java_lang_integer = new JNI::Object(java_lang_integer_class);
    java_lang_integer->fields.emplace(java_lang_integer_value, static_cast<JNI::jint>(value));
    return java_lang_integer;
}

JNI::jobject create_java_lang_boolean(bool value) {
    JNI::jobject java_lang_boolean = new JNI::Object(java_lang_boolean_class);
    java_lang_boolean->fields.emplace(java_lang_boolean_value, static_cast<JNI::jboolean>(value));
    return java_lang_boolean;
}

JNI::jobject create_java_lang_float(float value) {
    JNI::jobject java_lang_float = new JNI::Object(java_lang_float_class);
    java_lang_float->fields.emplace(java_lang_float_value, static_cast<JNI::jfloat>(value));
    return java_lang_float;
}

JNI::jobject create_java_lang_long(long value) {
    JNI::jobject java_lang_long = new JNI::Object(java_lang_long_class);
    java_lang_long->fields.emplace(java_lang_long_value, static_cast<JNI::jlong>(value));
    return java_lang_long;
}
