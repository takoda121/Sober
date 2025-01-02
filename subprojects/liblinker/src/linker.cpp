#include "linker.hpp"

#include "../external/bionic/linker/linker.h"
#include "../external/bionic/linker/linker_soinfo.h"
#include "../external/bionic/linker/linker_debug.h"

#include <cstdlib>

soinfo* soinfo_from_handle(void* handle);
void solist_init(void);
void soinfo_init(linker::linker_function_caller_t new_caller);
void linker_init(linker::linker_dl_iterate_phdr_cb_caller new_caller);

void linker::init(linker::linker_function_caller_t new_func_caller, linker::linker_dl_iterate_phdr_cb_caller new_phdr_cb_caller) {
    solist_init();
    soinfo_init(new_func_caller);
    linker_init(new_phdr_cb_caller);
    g_ld_debug_verbosity = 0;
}

void *linker::soinfo_create_empty(const char *name) {
    return create_empty_soinfo(name);
}

void *linker::soinfo_add_custom_symbol(void *info, const char *name, void *addr) {
    soinfo *si = reinterpret_cast<soinfo*>(info);
    if (!si) return nullptr;

    return (void*)(si->add_custom_symbol(name, reinterpret_cast<ElfW(Addr)>(addr)));
}

size_t linker::get_library_base(void *handle) {
    return soinfo_from_handle(handle)->base;
}

void linker::get_library_code_region(void *handle, size_t &base, size_t &size) {
    auto s = soinfo_from_handle(handle);
    for (size_t i = 0; i < s->phnum; i++) {
        if (s->phdr[i].p_type == PT_LOAD && s->phdr[i].p_flags & PF_X) {
            base = s->base + s->phdr[i].p_vaddr;
            size = s->phdr[i].p_memsz;
        }
    }
}

extern "C" void __loader_assert(const char* file, int line, const char* msg) {
    fprintf(stderr, "linker assert failed at %s:%i: %s\n", file, line, msg);
    abort();
}

extern int do_dlclose(void* handle);

int linker::dlclose_unlocked(void* handle) {
    return do_dlclose(handle);
}
