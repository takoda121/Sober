#include "syms.hpp"
#include "function_traits.hpp"
#include "libc_shim.h"
#include "linker.hpp"
#include "trampoline.hpp"
#include <vector>

extern const std::vector<dynamic_symbol> zlib_symbols;
extern const std::vector<dynamic_symbol> vulkan_symbols;
extern const std::vector<dynamic_symbol> libopensles_symbols;
extern const std::vector<dynamic_symbol> libmediandk_symbols;
extern const std::vector<dynamic_symbol> log_symbols;
extern const std::vector<dynamic_symbol> libm_symbols;
extern const std::vector<dynamic_symbol> libglesv2_symbols;
extern const std::vector<dynamic_symbol> egl_symbols;
extern const std::vector<dynamic_symbol> android_symbols;

void create_extra_gl_trampolines(); // egl syms
void create_vulkan_trampolines(); // vulkan syms


static void add_library(const char *name, const std::vector<dynamic_symbol> &symbols) {
    void *soinfo = linker::soinfo_create_empty(name);
    for (auto &shim : symbols) {
        void *addr = shim.arg_types.empty() ? shim.value : trampoline_create(shim.name, shim.value, shim.arg_types, shim.is_ellipsis_function, shim.has_execution_side_effects).guest_trampoline;
        linker::soinfo_add_custom_symbol(soinfo, shim.name, addr);
    }
}

void syms_init() {
    add_library("libdl.so", get_libdl_symbols());
    add_library("libc.so", shim::get_shimmed_symbols());
    add_library("libm.so", libm_symbols);
    add_library("libGLESv2.so", libglesv2_symbols);
    add_library("libOpenSLES.so", libopensles_symbols);
    add_library("libmediandk.so", libmediandk_symbols);
    add_library("liblog.so", log_symbols);
    add_library("libandroid.so", android_symbols);
    add_library("libz.so", zlib_symbols);
    add_library("libEGL.so", egl_symbols);
    add_library("libvulkan.so.1", vulkan_symbols);
    linker::soinfo_create_empty("libOpenMAXAL.so");
    
    create_extra_gl_trampolines();
    create_vulkan_trampolines();
}
