#include "syms.hpp"
#include "function_traits.hpp"
#include "types.hpp"
#include "window.hpp"
#include <cassert>
#include <EGL/eglplatform.h>
#include <SDL2/SDL_syswm.h>



struct AConfiguration {
    void *_;
};

struct AAssetManager {
    void *_;
};

struct ALooper {
    void *_;
};

using ALooper_callbackFunc = int (*)(int fd, int events, void *data);


void ANativeWindow_acquire([[maybe_unused]] ANativeWindow *window) {}

ANativeWindow* ANativeWindow_fromSurface([[maybe_unused]] void *env, [[maybe_unused]] void *surface) {
    return reinterpret_cast<ANativeWindow*>(get_egl_window());
}

i32 ANativeWindow_getHeight([[maybe_unused]] ANativeWindow *window) {
    i32 height;
    SDL_GetWindowSize(get_window(), nullptr, &height);
    return height;
}

i32 ANativeWindow_getWidth([[maybe_unused]] ANativeWindow *window) {
    i32 width;
    SDL_GetWindowSize(get_window(), &width, nullptr);
    return width;
}

void ANativeWindow_release([[maybe_unused]] ANativeWindow *window) {}

AConfiguration *AConfiguration_new() {
    assert(!"stub");
}

void AConfiguration_fromAssetManager(AConfiguration *, AAssetManager *) {
    assert(!"stub");
}

int32_t AConfiguration_getDensity(AConfiguration *) {
    return 1;
}

void AConfiguration_getCountry(AConfiguration *, char *) {
    assert(!"stub");
}

void AConfiguration_getLanguage(AConfiguration *, char *) {
    assert(!"stub");
}

void AConfiguration_delete(AConfiguration *) {

}

ALooper *ALooper_prepare(int) {
    assert(!"stub");
}

int ALooper_pollAll(int, int *, int *, void **) {
    assert(!"stub");
}

AAssetManager *AAssetManager_fromJava(void *, void *) {
    assert(!"stub");
}

void ALooper_acquire(ALooper *looper) {
    (void)looper;
}

void ALooper_release(ALooper *looper) {
    (void)looper;
}

int ALooper_addFd(ALooper *, int , int, int, ALooper_callbackFunc, void *) {
    assert(!"stub");
}

int ALooper_removeFd(ALooper *, int) {
    assert(!"stub");
}

ALooper *ALooper_forThread() {
    assert(!"stub");
    return nullptr;
}


extern const std::vector<dynamic_symbol> android_symbols = {
    { "ANativeWindow_acquire", ANativeWindow_acquire },
    { "ANativeWindow_fromSurface", ANativeWindow_fromSurface },
    { "ANativeWindow_getHeight", ANativeWindow_getHeight },
    { "ANativeWindow_getWidth", ANativeWindow_getWidth },
    { "ANativeWindow_release", ANativeWindow_release },

    { "AAssetManager_fromJava", AAssetManager_fromJava },

    { "AConfiguration_new", AConfiguration_new },
    { "AConfiguration_fromAssetManager", AConfiguration_fromAssetManager },
    { "AConfiguration_getDensity", AConfiguration_getDensity },
    { "AConfiguration_getCountry", AConfiguration_getCountry },
    { "AConfiguration_getLanguage", AConfiguration_getLanguage },
    { "AConfiguration_delete", AConfiguration_delete },

    { "ALooper_prepare", ALooper_prepare },
    { "ALooper_pollAll", ALooper_pollAll },
    { "ALooper_acquire", ALooper_acquire },
    { "ALooper_addFd", ALooper_addFd },
    { "ALooper_removeFd", ALooper_removeFd },
    { "ALooper_forThread", ALooper_forThread },
    { "ALooper_release", ALooper_release }
};
