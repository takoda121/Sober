#pragma once


struct AppMethods {
    int init(void *library);


    void *JNI_OnLoad = nullptr;

    // NativeSettingsInterface
    void *nativeSetDefaultAppPolicyFile = nullptr;
    void *nativeSetExceptionReasonFilename = nullptr;
    void *nativeSetBaseUrl = nullptr;
    void *nativeSetRobloxChannel = nullptr;
    void *nativeSetCacheDirectory = nullptr;
    void *nativeSetFilesDirectory = nullptr;
    void *nativeInitFastLog = nullptr;
    void *nativeSetRobloxVersion = nullptr;
    void *nativeSetPlatformHeadersWithIdfa = nullptr;
    void *nativeSetCookiesForDomain = nullptr;
    void *nativeSetHttpClientProxy = nullptr;
    void *nativeSetPreferencesFile = nullptr;
    void *nativeSetDeviceInfo = nullptr;

    // NativeGLInterface
    void *nativeGameGlobalInit = nullptr;
    void *nativeUpdateAdapterInit = nullptr;
    void *nativeInitClientSettings = nullptr;
    void *nativePostClientSettingsLoadedInitialization3 = nullptr;
    void *nativeAppBridgeV2InitWithParams = nullptr;
    void *nativeAppBridgeV2StartAppWithParams = nullptr;
    void *nativeAppBridgeV2StartGameWithParam = nullptr;
    void *nativeAppBridgeV2UpdateSurfaceApp = nullptr;
    void *nativeAppBridgeStartLuaAppDM = nullptr;
    void *nativePassKeyEvent = nullptr;
    void *reportBatteryStateChanged = nullptr;
    void *reportBatteryStatus = nullptr;
    void *reportThermalStateChanged = nullptr;
    void *syncTextboxTextAndCursorPosition2 = nullptr;
    void *nativePassText = nullptr;
    void *nativeReleaseFocus = nullptr;
    void *nativeReturnPressedFromOnScreenKeyboard = nullptr;
    void *nativeAppBridgeV2DestroyApp = nullptr;

    // NativeAppBridgeInterface
    void *nativeAppBridgeAppStart = nullptr;

    // JNICookieProtocol
    void *updateOnSetCookieHandler = nullptr;

    // JNIExperienceProtocol
    void *getLaunchId = nullptr;

    // NativeInputInterface
    void *nativePassMouseMove = nullptr;
    void *nativePassMouseButton = nullptr;
    void *nativePassMouseWheel = nullptr;

    // MessageBus
    void *doSubscribeRaw = nullptr;
};