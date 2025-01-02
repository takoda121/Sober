#include <cassert>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>
#include <EGL/egl.h>
#include "vendor/glad/glad.h"
#include "trampoline.hpp"

static std::mutex gl_lock;
static bool gl_initialized = false;
static std::vector<std::pair<deferred_symbol, usize>> symbols_with_trampoline_ids;
static std::unordered_map<std::string, void*> proc_to_guest_routine;

static const std::vector<deferred_symbol> extra_gl_symbols = {
    { "glBindVertexArray", &glBindVertexArray },
    { "glDeleteVertexArrays", &glDeleteVertexArrays },
    { "glGenVertexArrays", &glGenVertexArrays },
    { "glMapBuffer", &glMapBufferRange },
    { "glUnmapBuffer", &glUnmapBuffer },
    { "glMapBufferRange", &glMapBufferRange },
    { "glTexImage3D", &glTexImage3D },
    { "glTexSubImage3D", &glTexSubImage3D },
    { "glCompressedTexImage3D", &glCompressedTexImage3D },
    { "glCompressedTexSubImage3D", &glCompressedTexSubImage3D },
    { "glCopyImageSubData", &glCopyImageSubData },
    { "glTexStorage2D", &glTexStorage2D },
    { "glTexStorage3D", &glTexStorage3D },
    { "glFenceSync", &glFenceSync },
    { "glDeleteSync", &glDeleteSync },
    { "glClientWaitSync", &glClientWaitSync },
    { "glWaitSync", &glWaitSync },
    { "glBlitFramebuffer", &glBlitFramebuffer },
    { "glRenderbufferStorageMultisample", &glRenderbufferStorageMultisample },
    { "glInvalidateFramebuffer", &glInvalidateFramebuffer },
    { "glDrawBuffers", &glDrawBuffers },
    { "glClearBufferiv", &glClearBufferiv },
    { "glClearBufferfv", &glClearBufferfv },
    { "glClearBufferfi", &glClearBufferfi },
    { "glUniformBlockBinding", &glUniformBlockBinding },
    { "glBindBufferBase", &glBindBufferBase },
    { "glBindBufferRange", &glBindBufferRange },
    { "glGetUniformBlockIndex", &glGetUniformBlockIndex },
    { "glDrawElementsInstanced", &glDrawElementsInstanced },
    { "glDrawArraysInstanced", &glDrawArraysInstanced },
    { "glPushGroupMarker", &glPushGroupMarkerEXT },
    { "glPopGroupMarker", &glPopGroupMarkerEXT },
    { "glGenQueries", &glGenQueries },
    { "glDeleteQueries", &glDeleteQueries },
    { "glBeginQuery", &glBeginQuery },
    { "glEndQuery", &glEndQuery },
    { "glQueryCounter", &glQueryCounterEXT },
    { "glGetQueryiv", &glGetQueryiv },
    { "glGetQueryObjectiv", &glGetQueryObjectivEXT },
    { "glGetQueryObjectui64v", &glGetQueryObjectui64vEXT },
    { "glGetInteger64v", &glGetInteger64v },
};

extern const std::vector<dynamic_symbol> egl_symbols = {
    { "eglChooseConfig", eglChooseConfig },
    { "eglCreateContext", eglCreateContext },
    { "eglCreatePbufferSurface", eglCreatePbufferSurface },
    { "eglCreateWindowSurface", eglCreateWindowSurface },
    { "eglDestroyContext", eglDestroyContext },
    { "eglDestroySurface", eglDestroySurface },
    { "eglGetConfigAttrib", eglGetConfigAttrib },
    { "eglGetCurrentContext", eglGetCurrentContext },
    { "eglGetDisplay", eglGetDisplay },
    { "eglGetError", eglGetError },
    { "eglGetProcAddress", +[](const char *name) -> void* {
        std::scoped_lock<std::mutex> lock(gl_lock);
        if (!gl_initialized) {
            gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);

            for (auto &[symbol, trampoline_id] : symbols_with_trampoline_ids) {
                auto &trampoline = trampoline_get_by_id(trampoline_id + trampoline_id_offset);
                trampoline.native_func = *symbol.value;
                proc_to_guest_routine.emplace(symbol.name, trampoline.guest_trampoline);
            }
        }
        auto it = proc_to_guest_routine.find(name);
        if (it == proc_to_guest_routine.end()) {
            std::cerr << "eglGetProcAddress: unknown proc " << name << std::endl;
            assert(!"unknown proc in eglGetProcAddress");
        } else {
            // std::cout << "eglGetProcAddress: " << name << " -> " << it->second << std::endl;
            return it->second;
        }
    } },
    { "eglInitialize", eglInitialize },
    { "eglMakeCurrent", eglMakeCurrent },
    { "eglQuerySurface", eglQuerySurface },
    { "eglSwapBuffers", eglSwapBuffers },
    { "eglSwapInterval", eglSwapInterval },
    { "eglTerminate", eglTerminate }
};

void create_extra_gl_trampolines() {
    for (auto &sym : extra_gl_symbols) {
        assert(!sym.arg_types.empty());
        symbols_with_trampoline_ids.emplace_back(sym, trampoline_create(sym.name, nullptr, sym.arg_types, false, false).id);
    }
}
