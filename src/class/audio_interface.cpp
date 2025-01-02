#include "jni.hpp"
#include <SDL_audio.h>
#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>

JNI::jclass fmod_class;
JNI::jclass audio_device_class;


static SDL_AudioDeviceID audio_device = 0;
static u8 audio_buffer[2048] = {};
static std::mutex audio_mutex;
static std::condition_variable audio_cond_var;


static void audio_callback(void *, u8 *stream, int len) {
    assert(len == 2048);
    {
        std::scoped_lock lock(audio_mutex);
        memcpy(stream, audio_buffer, len);
    }
    audio_cond_var.notify_one();
}


int audio_interface_init() {
    SDL_AudioSpec desired_spec, obtained_spec;
    SDL_zero(desired_spec);
    desired_spec.freq = 48000;
    desired_spec.format = AUDIO_S16;
    desired_spec.channels = 2;
    desired_spec.samples = 512;
    desired_spec.callback = audio_callback;
    audio_device = SDL_OpenAudioDevice(nullptr, 0, &desired_spec, &obtained_spec, 0);
    if (audio_device == 0) {
        std::cerr << "SDL_OpenAudioDevice failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    return 0;
}

void audio_interface_close() {
    if (!audio_device) {
        return;
    }

    SDL_CloseAudioDevice(audio_device);
}

void create_audio_interface_classes(JNI::JavaVM *vm) {
    fmod_class = JNI::create_class(vm, "org/fmod/FMOD");
    fmod_class->static_method("checkInit", [](JNI::Class *, va_list) -> JNI::ValueVariant { return static_cast<JNI::jboolean>(false); });
    fmod_class->static_method("supportsLowLatency", [](JNI::Class *, va_list) -> JNI::ValueVariant { return static_cast<JNI::jboolean>(false); });
    fmod_class->static_method("supportsAAudio", [](JNI::Class *, va_list) -> JNI::ValueVariant { return static_cast<JNI::jboolean>(false); });

    audio_device_class = JNI::create_class(vm, "org/fmod/AudioDevice");
    audio_device_class->method("<init>", [](JNI::Object *, va_list) -> JNI::ValueVariant { return 0; });
    audio_device_class->method("init", [](JNI::Object *, va_list) -> JNI::ValueVariant {
        SDL_PauseAudioDevice(audio_device, 0);
        return static_cast<JNI::jboolean>(true);
    });
    audio_device_class->method("close", [](JNI::Object *, va_list) -> JNI::ValueVariant { return 0; });
    audio_device_class->method("write", [](JNI::Object *, va_list args) -> JNI::ValueVariant {
        auto audio_data = va_arg(args, JNI::jbyteArray);
        auto size = va_arg(args, JNI::jint);
        assert(size == 2048);

        std::unique_lock lock(audio_mutex);
        memcpy(audio_buffer, audio_data->array.data(), size);
        audio_cond_var.wait(lock);

        return 0;
    });
}
