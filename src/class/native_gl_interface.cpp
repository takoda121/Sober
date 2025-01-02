#include "classes.hpp"
#include <cassert>
#include <iostream>
#include <SDL2/SDL.h>
#include "jni.hpp"
#include "window.hpp"


JNI::jclass native_gl_interface_class = nullptr;

JNI::jclass settings_interface_class = nullptr;
JNI::jclass app_bridge_interface_class = nullptr;
JNI::jclass cookie_protocol_class = nullptr;
JNI::jclass input_interface_class;


static JNI::ValueVariant generic_stub(JNI::Class*, va_list) {
    assert(!"JNI stub reached");
}


void create_native_gl_interface_class(JNI::JavaVM *vm) {
    settings_interface_class = JNI::create_class(vm, "com/roblox/engine/jni/NativeSettingsInterface");
    app_bridge_interface_class = JNI::create_class(vm, "com/roblox/engine/jni/NativeAppBridgeInterface");
    cookie_protocol_class = JNI::create_class(vm, "com/roblox/universalapp/cookie/JNICookieProtocol");
    input_interface_class = JNI::create_class(vm, "com/roblox/engine/jni/NativeInputInterface");

    native_gl_interface_class = JNI::create_class(vm, "com/roblox/engine/jni/NativeGLJavaInterface");
    native_gl_interface_class->static_method("promptNativePurchase", generic_stub);
    native_gl_interface_class->static_method("promptNativePurchaseWithPayload", generic_stub);
    native_gl_interface_class->static_method("exitGameWithError", generic_stub);
    native_gl_interface_class->static_method("gameDidLeave", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        std::cout << "gameDidLeave is a stub" << std::endl;
        return 0;
    });
    native_gl_interface_class->static_method("onAppShellReloadNeeded", generic_stub);
    native_gl_interface_class->static_method("listenToMotionEvents", generic_stub);
    native_gl_interface_class->static_method("screenOrientationChanged", [](JNI::Class *, va_list) -> JNI::ValueVariant { return 0; });
    native_gl_interface_class->static_method("openNativeOverlay", generic_stub);
    native_gl_interface_class->static_method("onDataModelNotificationCallback", [](JNI::Class *, va_list args) -> JNI::ValueVariant {
        auto type = va_arg(args, JNI::jstring)->string;
        auto info = va_arg(args, JNI::jstring)->string;
#if JNI_LOGGING
        std::cout << "onDataModelNotificationCallback: " << type << ", " << info << std::endl;
#endif

        if (type == "APP_READY" && info == "Startup") {
            SDL_Event event;
            SDL_zero(event);
            event.type = get_sdl_custom_event();
            event.user.code = CUSTOM_EVENT_APP_STARTUP;
            SDL_PushEvent(&event);
        }

        return 0;
    });
    native_gl_interface_class->static_method("gameLoadedCallback", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return 0;
    });
    native_gl_interface_class->static_method("onLuaTextBoxChangedCallback", [](JNI::Class *, va_list args ) -> JNI::ValueVariant {
        std::cout << "onLuaTextBoxChangedCallback stub called with str " << va_arg(args, JNI::jstring)->string << std::endl;
        return 0;
    });
    native_gl_interface_class->static_method("onLuaTextBoxPropertyChangedCallback", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        std::cout << "onLuaTextBoxPropertyChangedCallback is a stub" << std::endl;
        return 0;
    });
    native_gl_interface_class->static_method("onAppBridgeNotification", [](JNI::Class *, [[maybe_unused]] va_list args) -> JNI::ValueVariant {
#if JNI_LOGGING
        auto type = va_arg(args, JNI::jstring)->string;
        auto info = va_arg(args, JNI::jstring)->string;
        std::cout << "onAppBridgeNotification: " << type << ", " << info << std::endl;
#endif
        return 0;
    });
    native_gl_interface_class->static_method("onExtendedAnalyticsRecvCallback", generic_stub);
    native_gl_interface_class->static_method("saveImageToAlbum", generic_stub);
    native_gl_interface_class->static_method("onVrSessionStateUpdate", generic_stub);
}
