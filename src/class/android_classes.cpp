#include "jni.hpp"
#include "app_state.hpp"
#include <iostream>


static JNI::jobject activity_thread = nullptr;
static JNI::jobject application = nullptr;
static JNI::jobject shared_preferences = nullptr;
static JNI::jobject shared_preferences_editor = nullptr;
static JNI::jobject resources = nullptr;
static JNI::jobject display_metrics = nullptr;
JNI::jclass surface_class = nullptr;

void create_android_classes(JNI::JavaVM *vm) {
    surface_class = JNI::create_class(vm, "android/view/Surface");

    auto build_class = JNI::create_class(vm, "android/os/Build");
    build_class->static_field("PRODUCT", JNI::create_string(""));
    build_class->static_field("DEVICE", JNI::create_string(""));
    build_class->static_field("BOARD", JNI::create_string(""));
    build_class->static_field("MANUFACTURER", JNI::create_string(""));
    build_class->static_field("BRAND", JNI::create_string(""));
    build_class->static_field("MODEL", JNI::create_string(""));
    build_class->static_field("BOOTLOADER", JNI::create_string(""));
    build_class->static_field("HARDWARE", JNI::create_string(""));
    build_class->static_field("TYPE", JNI::create_string(""));
    build_class->static_field("TAGS", JNI::create_string(""));
    build_class->static_field("FINGERPRINT", JNI::create_string(""));
    build_class->static_field("USER", JNI::create_string(""));
    build_class->static_field("HOST", JNI::create_string(""));
    
    auto debug_class = JNI::create_class(vm, "android/os/Debug");
    debug_class->static_method("isDebuggerConnected", [](JNI::Class*, va_list) -> JNI::ValueVariant {
        return static_cast<JNI::jboolean>(false);
    });

    auto build_version_class = JNI::create_class(vm, "android/os/Build$VERSION");
    build_version_class->static_field("SDK_INT", ANDROID_SDK_VERSION);

    auto context_class = JNI::create_class(vm, "android/content/Context");
    context_class->method("getSharedPreferences", [](JNI::Object*, va_list) -> JNI::ValueVariant {
        return shared_preferences;
    });

    auto shared_preferences_class = JNI::create_class(vm, "android/content/SharedPreferences");
    shared_preferences_class->method("edit", [](JNI::Object*, va_list) -> JNI::ValueVariant {
        return shared_preferences_editor;
    });

    auto shared_preferences_editor_class = JNI::create_class(vm, "android/content/SharedPreferences$Editor");
    shared_preferences_editor_class->method("putString", [](JNI::Object*, va_list args) -> JNI::ValueVariant {
        if constexpr (JNI_LOGGING) {
            auto key = va_arg(args, JNI::jstring);
            auto value = va_arg(args, JNI::jstring);
            std::cout << "put string " << key->string << ": " << value->string << std::endl;
        }
        return shared_preferences_editor;
    });
    shared_preferences_editor_class->method("apply", [](JNI::Object*, va_list) -> JNI::ValueVariant {
        return 0;
    });

    auto display_metrics_class = JNI::create_class(vm, "android/util/DisplayMetrics");
    auto xdpi_field = display_metrics_class->field("xdpi");
    auto ydpi_field = display_metrics_class->field("ydpi");
    auto width_pixels_field = display_metrics_class->field("widthPixels");
    auto height_pixels_field = display_metrics_class->field("heightPixels");

    auto resources_class = JNI::create_class(vm, "android/content/res/Resources");
    resources_class->method("getDisplayMetrics", [](JNI::Object*, va_list) -> JNI::ValueVariant {
        return display_metrics;
    });

    auto application_class = JNI::create_class(vm, "android/app/Application");
    application_class->method("getResources", [](JNI::Object*, va_list) -> JNI::ValueVariant {
        return resources;
    });

    auto activity_thread_class = JNI::create_class(vm, "android/app/ActivityThread");
    activity_thread_class->static_method("currentActivityThread", [](JNI::Class*, va_list) -> JNI::ValueVariant {
        return activity_thread;
    });
    activity_thread_class->method("getApplication", [](JNI::Object*, va_list) -> JNI::ValueVariant {
        return application;
    });

    shared_preferences_editor = new JNI::Object(shared_preferences_editor_class);
    shared_preferences = new JNI::Object(shared_preferences_class);
    resources = new JNI::Object(resources_class);
    application = new JNI::Object(application_class);
    activity_thread = new JNI::Object(activity_thread_class);

    display_metrics = new JNI::Object(display_metrics_class);
    display_metrics->fields.emplace(xdpi_field, 1.0f);
    display_metrics->fields.emplace(ydpi_field, 1.0f);
    display_metrics->fields.emplace(width_pixels_field, static_cast<JNI::jint>(display_width));
    display_metrics->fields.emplace(height_pixels_field, static_cast<JNI::jint>(display_height));
}
