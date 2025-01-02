#include "app_methods.hpp"
#include "linker.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>


static void *find_symbol(void *handle, const char *symbol) {
    void *sym = linker::dlsym(handle, symbol);
    if (!sym) {
        std::cerr << "try_get_symbol: could not find symbol: " << symbol << std::endl;
        throw std::runtime_error("try_get_symbol could not locate required symbol");
        return nullptr;
    }

    return sym;
}

int AppMethods::init(void *l) {
    assert(l);

    try {
        JNI_OnLoad = find_symbol(l, "JNI_OnLoad");

        // NativeSettingsInterface
        nativeSetDefaultAppPolicyFile = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetDefaultAppPolicyFile");
        nativeSetExceptionReasonFilename = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetExceptionReasonFilename");
        nativeSetBaseUrl = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetBaseUrl");
        nativeSetRobloxChannel = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetRobloxChannel");
        nativeSetCacheDirectory = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetCacheDirectory");
        nativeSetFilesDirectory = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetFilesDirectory");
        nativeInitFastLog = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeInitFastLog");
        nativeSetRobloxVersion = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetRobloxVersion");
        nativeSetPlatformHeadersWithIdfa = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetPlatformHeadersWithIdfa");
        nativeSetCookiesForDomain = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetCookiesForDomain");
        nativeSetHttpClientProxy = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetHttpClientProxy");
        nativeSetPreferencesFile = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetPreferencesFile");
        nativeSetDeviceInfo = find_symbol(l, "Java_com_roblox_engine_jni_NativeSettingsInterface_nativeSetDeviceInfo");
        // NativeGLInterface
        nativeGameGlobalInit = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeGameGlobalInit");
        nativeUpdateAdapterInit = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeUpdateAdapterInit");
        nativeInitClientSettings = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeInitClientSettings");
        nativePostClientSettingsLoadedInitialization3 = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativePostClientSettingsLoadedInitialization3");
        nativeAppBridgeV2InitWithParams = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeAppBridgeV2InitWithParams");
        nativeAppBridgeV2StartAppWithParams = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeAppBridgeV2StartAppWithParams");
        nativeAppBridgeV2StartGameWithParam = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeAppBridgeV2StartGameWithParam");
        nativeAppBridgeV2UpdateSurfaceApp = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeAppBridgeV2UpdateSurfaceApp");
        nativeAppBridgeStartLuaAppDM = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeAppBridgeStartLuaAppDM");
        nativePassKeyEvent = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativePassKeyEvent");
        reportBatteryStateChanged = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_reportBatteryStateChanged");
        reportBatteryStatus = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_reportBatteryStatus");
        reportThermalStateChanged = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_reportThermalStateChanged");
        syncTextboxTextAndCursorPosition2 = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_syncTextboxTextAndCursorPosition2");
        nativePassText = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativePassText");
        nativeReleaseFocus = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeReleaseFocus");
        nativeReturnPressedFromOnScreenKeyboard = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeReturnPressedFromOnScreenKeyboard");
        nativeAppBridgeV2DestroyApp = find_symbol(l, "Java_com_roblox_engine_jni_NativeGLInterface_nativeAppBridgeV2DestroyApp");
        // NativeAppBridgeInterface
        nativeAppBridgeAppStart = find_symbol(l, "Java_com_roblox_engine_jni_NativeAppBridgeInterface_nativeAppBridgeAppStart__Ljava_lang_String_2Ljava_lang_String_2ZLjava_lang_String_2Ljava_lang_String_2");
        // JNICookieProtocol
        updateOnSetCookieHandler = find_symbol(l, "Java_com_roblox_universalapp_cookie_JNICookieProtocol_updateOnSetCookieHandler");
        // JNIExperienceProtocol
        getLaunchId = find_symbol(l, "Java_com_roblox_universalapp_experience_JNIExperienceProtocol_getLaunchId");
        // NativeInputInterface
        nativePassMouseMove = find_symbol(l, "Java_com_roblox_engine_jni_NativeInputInterface_nativePassMouseMove");
        nativePassMouseButton = find_symbol(l, "Java_com_roblox_engine_jni_NativeInputInterface_nativePassMouseButton");
        nativePassMouseWheel = find_symbol(l, "Java_com_roblox_engine_jni_NativeInputInterface_nativePassMouseWheel");
        // MessageBus
        doSubscribeRaw = find_symbol(l, "Java_com_roblox_universalapp_messagebus_MessageBus_doSubscribeRaw");
    } catch(const std::runtime_error& error) {
        std::cerr << "AppMethods: one or more required symbols could not be located" << std::endl;
        return -1;
    }

    return 0;
}
