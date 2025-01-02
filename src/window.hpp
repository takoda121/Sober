#pragma once

#include "SDL_video.h"
#include "types.hpp"
#include <EGL/eglplatform.h>


constexpr i32 CUSTOM_EVENT_APP_STARTUP = 1;
constexpr i32 CUSTOM_EVENT_START_GAME = 2;
constexpr i32 CUSTOM_EVENT_REPORT_BATTERY_STATUS = 3;


void global_window_init();
SDL_Window *get_window();
u32 get_sdl_custom_event();
EGLNativeWindowType get_egl_window();
void global_window_destroy();
