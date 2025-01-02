#include "SDL_events.h"
#include "SDL_mouse.h"
#include "SDL_scancode.h"
#include "SDL_video.h"
#include "app_methods.hpp"
#include "window.hpp"
#include <cstddef>
#include <exception>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <utility>
#if TRAMPOLINE_DEBUG_CALLS
#include <csignal>
#endif

#include <SDL.h>
#include <curl/curl.h>

#include "vendor/nlohmann/json.hpp"

#include "types.hpp"
#include "execution_context.hpp"
#include "jni.hpp"
#include "trampoline.hpp"
#include "linker.hpp"
#include "libc_shim.h"
#include "libs/pthread_impl.hpp"
#include "libs/libc_impl.hpp"
#include "class/classes.hpp"
#include "app_state.hpp"
#include "keyboard.hpp"
#include "libs/syms.hpp"


JNI::jclass cookie_handler_class = nullptr;
JNI::JavaVM *create_java_vm() {
    JNI::JavaVM *vm = JNI::java_vm_new();

    JNI::create_class(vm, "com/roblox/engine/jni/model/ApplicationExitInfoCpp");
    JNI::create_class(vm, "com/roblox/audio/AppRtcDeviceWrapper");
    JNI::create_class(vm, "org/fmod/MediaCodec");
    JNI::create_class(vm, "com/roblox/engine/jni/memstorage/Connection");
    JNI::create_class(vm, "org/webrtc/voiceengine/BuildInfo");
    JNI::create_class(vm, "org/webrtc/voiceengine/WebRtcAudioManager");
    JNI::create_class(vm, "org/webrtc/voiceengine/WebRtcAudioRecord");
    JNI::create_class(vm, "org/webrtc/voiceengine/WebRtcAudioTrack");

    auto codec_info_utils = JNI::create_class(vm, "com/roblox/engine/jni/video/MediaCodecInfoUtils");
    codec_info_utils->static_method("getHardwareVideoDecoders", [](JNI::Class *, va_list) -> JNI::ValueVariant { assert(!"Stub"); });

    auto network_utils = JNI::create_class(vm, "com/roblox/engine/jni/util/NetworkUtils");
    network_utils->static_method("getPublicIPv4Addresseses", [](JNI::Class *, va_list) -> JNI::ValueVariant {
        return JNI::create_string("");
    });

    auto messagebus_connection = JNI::create_class(vm, "com/roblox/universalapp/messagebus/Connection");
    messagebus_connection->method("<init>", [](JNI::Object*, va_list) -> JNI::ValueVariant {
        return 0;
    });

    cookie_handler_class = JNI::create_class(vm, "com/roblox/universalapp/cookie/OnSetCookieHandlerImpl");
    cookie_handler_class->method("onSetCookie", [](JNI::Object*, va_list args) -> JNI::ValueVariant {
        auto arr = va_arg(args, JNI::jobjectArray);
        for (auto &obj : arr->array) {
            // TODO: ignores domain, expiration and case-insensivity
            std::string header = reinterpret_cast<JNI::jstring>(obj)->string;
            std::string cookie = header.substr(0, header.find(";"));
            std::string::size_type pos = cookie.find("=");
            std::string key = cookie.substr(0, pos);
            std::string value = cookie.substr(pos+1);

            cookies.insert_or_assign(key, value);
        }
        std::ofstream cookie_file(get_cookies_path(), std::ios::trunc);
        for (auto i = cookies.begin();;) {
            cookie_file << i->first << "=" << i->second;
            if (++i != cookies.end())
                cookie_file << "; ";
            else
                break;
        }
        return 0;
    });

    auto native_text_box_info = JNI::create_class(vm, "com/roblox/engine/jni/model/NativeTextBoxInfo");
    native_text_box_info->method("<init>", [](JNI::Object*, va_list) -> JNI::ValueVariant {
        return 0;
    });

    return vm;
}

#if TRAMPOLINE_DEBUG_CALLS
static void signal_print(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
}
#endif

template<typename... Args>
void run(void *routine, Args ...args) {
    ScopedExecutionFiber fiber(FiberKind::Callee);
    fiber.run(routine, args...);
}

template <typename R, typename... Args>
R run_with_return(void *routine, Args... args) {
    ScopedExecutionFiber fiber(FiberKind::Callee);
    return fiber.run_with_return<R>(routine, args...);
}

int main(int argc, char **argv) {
    if (argc == 2) {
        set_path_prefix(argv[1]);
    }

    if (!std::filesystem::is_directory(get_assets_dir())) {
        std::cerr << "Expected assets directory " << get_assets_dir() << " does not exist" << std::endl;
        return -1;
    }
    if (!std::filesystem::exists(get_core_lib_path())) {
        std::cerr << "Expected library file " << get_core_lib_path() << " does not exist" << std::endl;
        return -1;
    }

    auto cert_path = get_assets_dir() / "ssl" / "cacert.pem";
    if (!std::filesystem::is_regular_file(cert_path)) {
        std::cerr << "Expected asset file " << cert_path << " is not a regular file or does not exist" << std::endl;
        return -1;
    }


    std::filesystem::create_directory(get_cache_dir());
    std::filesystem::create_directory(get_files_dir());

    auto cert_dir = get_files_dir() / "exe";
    std::filesystem::create_directory(cert_dir);
    std::filesystem::copy(cert_path, cert_dir / "cacert.pem", std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::skip_symlinks);


    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_DisplayMode display_mode;
    if (SDL_GetDesktopDisplayMode(0, &display_mode) != 0) {
        std::cerr << "SDL_GetDesktopDisplayMode failed: " << SDL_GetError() << std::endl;
        std::terminate();
    }

    display_width = display_mode.w;
    display_height = display_mode.h;


    global_argc = argc;
    global_argv = argv;
    device_memory_mb = (sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE)) / (1024 * 1024);

    global_window_init();

    execution_context_init();

    trampoline_init();

    shim::libc_shim_common_init(atexit_impl, qsort_impl, bsearch_impl, setjmp_impl, longjmp_impl);
    shim::libc_shim_pthread_init(pthread_caller, pthread_create_impl, pthread_key_create_impl, pthread_key_delete_impl, pthread_setspecific_impl, pthread_getspecific_impl);
    linker::init(linker_func_caller, linker_phdr_cb_caller);

    syms_init();


#if TRAMPOLINE_DEBUG_CALLS
    auto signal_handler = +[] (int signum) {
        signal_print("Received fatal signal, printing last calls of all execution contexts:\n");
        for (usize context_i = 0; context_i < num_execution_contexts; context_i++) {
            auto *context = execution_contexts_buffer[context_i];
            usize i = context->last_calls_buffer_write_index;
            do {
                signal_print(" - ");
                signal_print(context->last_calls_buffer[i]);
                signal_print("\n");
                i = (i + 1) % ExecutionContext::LAST_CALLS_BUFFER_SIZE;
            } while(i != context->last_calls_buffer_write_index);
            if (context_i != num_execution_contexts - 1)
                signal_print("Next execution context:\n");
        }
        signal(signum, SIG_DFL);
        signal(SIGABRT, SIG_DFL);
        std::abort();
    };
    signal(SIGABRT, signal_handler);
    signal(SIGSEGV, signal_handler);
#endif

    AppMethods m;
    auto lib_path = get_core_lib_path();
    void *main_library = linker::dlopen(lib_path.c_str(), 0);
    if (!main_library) {
        std::cerr << "failed to load main shared object" << std::endl;
        std::terminate();
        return 1;
    }
    if (m.init(main_library) < 0) {
        std::cerr << "failed to load main shared object symbols" << std::endl;
        std::terminate();
        return 1;
    }

    auto vm = create_java_vm();
    auto env = JNI::get_jni_env();

    init_classes(vm);

    auto cookie_handler_instance = JNI::Object();
    cookie_handler_instance.super = cookie_handler_class;

    auto cookie_protocol_instance = JNI::Object();
    cookie_protocol_instance.super = cookie_protocol_class;

    auto surface = JNI::Object();
    surface.super = surface_class;

    long current_text_box_id = 0;
    
    native_gl_interface_class->static_method("showKeyboard", [&](JNI::Class *, va_list args) -> JNI::ValueVariant {
        current_text_box_id = va_arg(args, JNI::jlong);
        [[maybe_unused]] bool unknown_boolean = va_arg(args, int);
        JNI::jbyteArray raw_text = va_arg(args, JNI::jbyteArray);
        [[maybe_unused]] JNI::jobject text_box_info = va_arg(args, JNI::jobject);

        std::string text((const char *)raw_text->array.data(), raw_text->array.size());

        run(m.syncTextboxTextAndCursorPosition2, env, native_gl_interface_class, JNI::create_string(""), 0);
        run(m.nativePassText, env, native_gl_interface_class, current_text_box_id, JNI::create_string(""), false, 0);
        run(m.syncTextboxTextAndCursorPosition2, env, native_gl_interface_class, JNI::create_string("hello"), 5);
        run(m.nativePassText, env, native_gl_interface_class, current_text_box_id, JNI::create_string("hello"), false, 5);
        run(m.nativeReturnPressedFromOnScreenKeyboard, env, native_gl_interface_class, current_text_box_id);
        run(m.nativeReleaseFocus, env, native_gl_interface_class, current_text_box_id);

        return 0;
    });

    native_gl_interface_class->static_method("hideKeyboard", [&](JNI::Class *, va_list) -> JNI::ValueVariant {
        run(m.nativeReleaseFocus, env, native_gl_interface_class, current_text_box_id);
        current_text_box_id = 0;
        return 0;
    });

    run(m.JNI_OnLoad, vm, 0);

    run(m.nativeSetDefaultAppPolicyFile, env, settings_interface_class, JNI::create_string(get_assets_dir() / "content/guac/defaultConfigs/GuacDefaultPolicy-GlobalDist.json"));
    run(m.nativeSetExceptionReasonFilename, env, settings_interface_class, JNI::create_string("exception_reason.txt"));
    run(m.nativeSetBaseUrl, env, settings_interface_class, JNI::create_string("https://www.roblox.com/"), JNI::create_string("https://api.roblox.com/"));
    run(m.nativeSetRobloxChannel, env, settings_interface_class, JNI::create_string(""));
    run(m.nativeSetCacheDirectory, env, settings_interface_class, JNI::create_string(get_cache_dir()));
    run(m.nativeSetFilesDirectory, env, settings_interface_class, JNI::create_string(get_files_dir()));
    run(m.nativeInitFastLog, env, settings_interface_class);
    run(m.nativeSetRobloxVersion, env, settings_interface_class, JNI::create_string(roblox_version));
    run(m.nativeSetPlatformHeadersWithIdfa, env, settings_interface_class, JNI::create_string(""), JNI::create_string("googleplay"), JNI::create_string(""));

    std::ifstream cookie_file(get_cookies_path());
    std::stringstream cookie_buffer;
    cookie_buffer << cookie_file.rdbuf();
    cookie_file.close();
    for (std::string cookie; std::getline(cookie_buffer, cookie, ';');) {
        std::string::size_type pos = cookie.find("=");
        std::string key = cookie.substr(0, pos);
        std::string value = cookie.substr(pos + 1);
        cookies.insert_or_assign(key, value);
    }
    run(m.nativeSetCookiesForDomain, env, settings_interface_class, JNI::create_string(".roblox.com"), JNI::create_string(cookie_buffer.str().c_str()));

    run(m.nativeGameGlobalInit, env, native_gl_interface_class);

    run(m.nativeSetHttpClientProxy, env, settings_interface_class, JNI::create_string(""), 0);
    run(m.nativeUpdateAdapterInit, env, native_gl_interface_class);
    run(m.updateOnSetCookieHandler, env, &cookie_protocol_instance, &cookie_handler_instance);

    {
        auto clientsettings_url = get_clientsettings_url();
        CURL *curl = curl_easy_init();
        std::string buffer;

        curl_easy_setopt(curl, CURLOPT_URL, clientsettings_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char *in, size_t size, size_t nmemb, std::string *out) -> std::size_t {
            std::size_t total_size = size * nmemb;
            if (total_size) {
                out->append(in, total_size);
                return total_size;
            }
            return 0;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        auto res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "fetch clientsettings: curl_easy_perform failed: " << curl_easy_strerror(res) << std::endl;
            return -1;
        }
        curl_easy_cleanup(curl);

        run(m.nativeInitClientSettings, env, native_gl_interface_class, JNI::create_string(buffer), nullptr, JNI::create_string("GoogleAndroidApp"));
    }

    auto empty_list = JNI::ObjectArray();
    empty_list.super = JNI::list_class;
    run(m.nativePostClientSettingsLoadedInitialization3, env, native_gl_interface_class, &empty_list);
    
    run(m.nativeSetPreferencesFile, env, settings_interface_class, JNI::create_string("prefs"));

    run(m.nativeSetDeviceInfo, env, settings_interface_class, get_device_params());
    run(m.nativeAppBridgeAppStart, env, app_bridge_interface_class, JNI::create_string("https://www.roblox.com/"), JNI::create_string(user_agent), false, JNI::create_string(""), JNI::create_string("AppAndroidV"));

    run(m.nativeAppBridgeV2InitWithParams, env, native_gl_interface_class, new JNI::Object(init_params_class));
    run(m.nativeAppBridgeStartLuaAppDM, env, native_gl_interface_class);


    SDL_Event event;
    
    bool is_fullscreen = false;
    bool is_running = true;
    bool window_presented = false;
    bool cursor_grabbed = false;
    bool cursor_locked = false;

    bool startup_task = false;
    bool game_start_task = false;
    bool resize_task = false;

    i32 mouse_x = -1;
    i32 mouse_y = -1;

    while (is_running) {
        i32 frame_del_x = 0;
        i32 frame_del_y = 0;
        bool mouse_motion_task = false;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                is_running = false;
            } break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode == SDL_SCANCODE_F11) {
                    auto window = get_window();
                    is_fullscreen = !is_fullscreen;
                    if (is_fullscreen) {
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                        SDL_SetWindowDisplayMode(window, &display_mode);

                        cursor_grabbed = true;
                        SDL_SetRelativeMouseMode(SDL_TRUE);
                    } else {
                        SDL_SetWindowFullscreen(window, 0);
                    }
                }
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    cursor_grabbed = false;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }
    #if TRAMPOLINE_DEBUG_COUNT_CALLS
                if (event.key.keysym.scancode == SDL_SCANCODE_KP_9)
                    trampoline_print_counts();
    #endif
                /* through */
            case SDL_KEYUP: {
                assert(event.key.keysym.scancode < sdl_scancode_to_linux_keycode_table_length);
                int linux_keycode = sdl_scancode_to_linux_keycode_table[event.key.keysym.scancode];
                int android_keycode = sdl_scancode_to_android_keycode(event.key.keysym.scancode);
                run(m.nativePassKeyEvent, env, native_gl_interface_class, event.type == SDL_KEYDOWN, linux_keycode, android_keycode, event.key.repeat > 0);
            } break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                int button_id = 0;
                if (event.button.button == SDL_BUTTON_LEFT) button_id = 1;
                else if (event.button.button == SDL_BUTTON_RIGHT) button_id = 2;
                else if (event.button.button == SDL_BUTTON_MIDDLE) button_id = 4;

                run(m.nativePassMouseButton, env, input_interface_class, (float)mouse_x, (float)mouse_y, event.button.state, button_id - 1);

                if (!cursor_grabbed) {
                    cursor_grabbed = true;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }

                if (event.button.button == SDL_BUTTON_RIGHT) {
                    // Holding right click locks the cursor in place to pan the camera
                    cursor_locked = event.button.state;
                }
            } break;
            case SDL_MOUSEMOTION: {
                frame_del_x += event.motion.xrel;
                frame_del_y += event.motion.yrel;
                if (cursor_locked) {
                    SDL_WarpMouseInWindow(get_window(), mouse_x, mouse_y);
                } else {
                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                }
                mouse_motion_task = true;
            } break;
            case SDL_MOUSEWHEEL: {
                run(m.nativePassMouseWheel, env, input_interface_class, (float)mouse_x, (float)mouse_y, (float)event.wheel.preciseY);
            } break;
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    resize_task = true;
                } break;
                case SDL_WINDOWEVENT_SHOWN: {
                    window_presented = true;
                } break;
                }
            } break;
            default: {
                if (event.type != get_sdl_custom_event())
                    break;

                switch (event.user.code) {
                case CUSTOM_EVENT_APP_STARTUP: {
                    startup_task = true;
                } break;
                case CUSTOM_EVENT_START_GAME: {
                    game_start_task = true;
                } break;
                case CUSTOM_EVENT_REPORT_BATTERY_STATUS: {
                    run(m.reportBatteryStatus, env, native_gl_interface_class, get_battery_status());
                } break;
                }
            } break;
            }
        }


        if (resize_task) {
            resize_task = false;

            run(m.nativeAppBridgeV2UpdateSurfaceApp, env, native_gl_interface_class, &surface, 1.0f);
        }

        if (startup_task && window_presented) {
            startup_task = false;
            
            run(m.nativeAppBridgeV2StartAppWithParams, env, native_gl_interface_class, new JNI::Object(start_app_params_class));

            JNI::jclass experience_protocol_class = JNI::create_class(vm, "com/roblox/universalapp/experience/JNIExperienceProtocol");
            JNI::jclass message_bus_class = JNI::create_class(vm, "com/roblox/universalapp/messagebus/MessageBus");

            JNI::jclass callback_class = JNI::create_class(vm, "MessageBusCallback");
            callback_class->method("run", [&](JNI::Object*, va_list args) -> JNI::ValueVariant {
                auto str = va_arg(args, JNI::jstring);
                int join_request_type = 0;

                auto json = nlohmann::json::parse(str->string);

                auto place_id = json.value("placeId", (long)0);
                auto user_id = json.value("userId", (long)-1);
                auto join_attempt_id = json.value("joinAttemptId", "");
                auto join_attempt_origin = json.value("joinAttemptOrigin", "");
                auto referral_page = json.value("referralPage", "");
                
                if (place_id != 0 && user_id == -1) {
                    join_request_type = 0;
                } else if (user_id != -1) {
                    join_request_type = 1;
                } else {
                    std::cerr << "MessageBusCallback: don't know how to handle join request" << std::endl;
                    return 0;
                }

                set_start_game_params(place_id, user_id, join_request_type, join_attempt_id, join_attempt_origin, referral_page);

                SDL_Event event;
                SDL_zero(event);
                event.type = get_sdl_custom_event();
                event.user.code = CUSTOM_EVENT_START_GAME;
                SDL_PushEvent(&event);

                return 0;
            });

            JNI::jobject callback_object = new JNI::Object(callback_class);
            JNI::jstring launch_id = run_with_return<JNI::jstring>(m.getLaunchId, env, experience_protocol_class);
            run(m.doSubscribeRaw, env, message_bus_class, launch_id, callback_object, false);

            // disable battery throttling part 1
            SDL_AddTimer(30'000, [] (u32 interval, void *custom_event) -> u32 {
                SDL_Event event;
                SDL_zero(event);
                event.type = (u32)(u64)custom_event;
                event.user.code = CUSTOM_EVENT_REPORT_BATTERY_STATUS;
                SDL_PushEvent(&event);
                return interval;
            }, (void*)(u64)get_sdl_custom_event());
        }

        if (game_start_task) {
            game_start_task = false;

            run(m.nativeAppBridgeV2StartGameWithParam, env, native_gl_interface_class, new JNI::Object(start_game_params_class));

            run(m.reportBatteryStateChanged, env, native_gl_interface_class, 100, 5); // disable battery throttling part 2
            run(m.reportThermalStateChanged, env, native_gl_interface_class, 0); // disable thermal throttling

            // send a dummy key press so that roblox understands we have a keyboard
            SDL_Scancode dummy_scancode = SDL_SCANCODE_H;
            int linux_keycode = sdl_scancode_to_linux_keycode_table[dummy_scancode];
            int android_keycode = sdl_scancode_to_android_keycode(dummy_scancode);
            usleep(10000);
            run(m.nativePassKeyEvent, env, native_gl_interface_class, true, linux_keycode, android_keycode, false);
            usleep(10000);
            run(m.nativePassKeyEvent, env, native_gl_interface_class, false, linux_keycode, android_keycode, false);
        }

        if (mouse_motion_task) {
            mouse_motion_task = false;

            run(m.nativePassMouseMove, env, input_interface_class, (float)mouse_x, (float)mouse_y, (float)frame_del_x, (float)frame_del_y);
        }
    }

    run(m.nativeAppBridgeV2DestroyApp);
    global_window_destroy();
    SDL_Quit();

    return 0;
}
