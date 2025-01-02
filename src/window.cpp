#include "window.hpp"
#include "SDL_video.h"
#include "class/classes.hpp"
#include "SDL_syswm.h"
#include "SDL_events.h"
#include "types.hpp"
#include <exception>
#include <iostream>


constexpr int initial_window_width = 800, initial_window_height = 600;

static u32 custom_event = static_cast<u32>(-1);
static SDL_Window *global_window = nullptr;
static EGLNativeWindowType egl_window = 0;


void global_window_init() {
    custom_event = SDL_RegisterEvents(1);
    if (custom_event == static_cast<u32>(-1)) {
        std::cerr << "Failed to register SDL event" << std::endl;
        std::terminate();
    }

    global_window = SDL_CreateWindow("sober", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, initial_window_width, initial_window_height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!global_window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        std::terminate();
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(get_window(), &wmInfo);
    switch (wmInfo.subsystem) {
    case SDL_SYSWM_X11:
        egl_window = wmInfo.info.x11.window;
        break;
    case SDL_SYSWM_WAYLAND:
        egl_window = reinterpret_cast<EGLNativeWindowType>(wmInfo.info.wl.egl_window);
        break;
    default:
        std::cerr << "Unknown SDL subsystem" << std::endl;
        std::terminate();
    }    

    SDL_ShowCursor(SDL_DISABLE);

    if (audio_interface_init() < 0) {
        // TODO: make this non-fatal in the future
        std::cerr << "Failed to initialize audio interface" << std::endl;
        std::terminate();
    }
}

SDL_Window *get_window() {
    if (!global_window) {
        std::cerr << "global_window is null" << std::endl;
        std::terminate();
        return nullptr;
    }

    return global_window;
}

u32 get_sdl_custom_event() {
    if (custom_event == static_cast<u32>(-1)) {
        std::cerr << "custom_event is undefined" << std::endl;
        std::terminate();
        return -1;
    }

    return custom_event;
}

EGLNativeWindowType get_egl_window() {
    if (!egl_window) {
        std::cerr << "egl_window is null" << std::endl;
        std::terminate();
        return 0;
    }

    return egl_window;
}

void global_window_destroy() {
    if (global_window) {
        audio_interface_close();
        SDL_DestroyWindow(global_window);
    }
}
