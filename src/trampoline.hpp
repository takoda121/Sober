#pragma once

#include "types.hpp"
#include "function_traits.hpp"
#include <vector>

#define TRAMPOLINE_DEBUG_COUNT_CALLS 0
#define TRAMPOLINE_DEBUG_CALLS 0

#if TRAMPOLINE_DEBUG_COUNT_CALLS
#include <atomic>
#endif

constexpr u32 trampoline_id_offset = 0x10;

struct Trampoline {
    const char *name;
    u32 id = 0;
    void *native_func = nullptr;
    std::vector<FunctionArgumentType> types;
    bool is_ellipsis_function = false;
    bool has_execution_side_effects = false;
    void *guest_trampoline = nullptr;

#if TRAMPOLINE_DEBUG_COUNT_CALLS
    std::atomic<u64> count = 0;
#endif

    Trampoline(const char *name, u32 id, void *native_func, std::vector<FunctionArgumentType> types, bool is_ellipsis_function, bool has_execution_side_effects);
    Trampoline(Trampoline &&other);
};

int trampoline_init();
Trampoline &trampoline_create(const char *name, void *native_func, std::vector<FunctionArgumentType> types, bool is_ellipsis_function, bool has_execution_side_effects);
Trampoline &trampoline_get_by_id(u32 id);
Trampoline *trampoline_maybe_get_by_id(u32 id);
u32 *trampoline_get_guest_exit();

#if TRAMPOLINE_DEBUG_COUNT_CALLS
void trampoline_print_counts();
#endif
