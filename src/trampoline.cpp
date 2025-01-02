#include "trampoline.hpp"
#include "types.hpp"
#include "function_traits.hpp"
#include <iostream>
// #include <mutex>
// #include <shared_mutex>
#include <oaknut/oaknut.hpp>
#include <stdexcept>
#include <sys/mman.h>
#include <vector>

std::vector<Trampoline> trampolines;

constexpr usize trampoline_code_size = sizeof(u32) * 2 * 8192;
u32 *trampoline_code = nullptr;
usize trampoline_code_index = 0;

std::vector<u32> temp_code_gen_buf;
oaknut::VectorCodeGenerator temp_code_generator{temp_code_gen_buf};

u32 *guest_exit_trampoline = nullptr;

// static std::shared_mutex trampoline_mutex;

Trampoline::Trampoline(const char *name, u32 id, void *native_func, std::vector<FunctionArgumentType> types, bool is_ellipsis_function, bool has_execution_side_effects)
    : name(name)
    , id(id)
    , native_func(native_func)
    , types(types)
    , is_ellipsis_function(is_ellipsis_function)
    , has_execution_side_effects(has_execution_side_effects) {}

Trampoline::Trampoline(Trampoline &&other)
    : name(other.name)
    , id(other.id)
    , native_func(other.native_func)
    , types(std::move(other.types))
    , is_ellipsis_function(other.is_ellipsis_function)
    , has_execution_side_effects(other.has_execution_side_effects)
    , guest_trampoline(other.guest_trampoline) {}


u32 *trampoline_get_guest_exit() {
    return guest_exit_trampoline;
}

int trampoline_init() {
    trampoline_code = (u32*)mmap(nullptr, trampoline_code_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (trampoline_code == MAP_FAILED) {
        std::cerr << "Failed to allocate trampolines buffer\n";
        return -1;
    }

    temp_code_gen_buf.reserve(2);

    guest_exit_trampoline = &trampoline_code[trampoline_code_index];
    temp_code_generator.YIELD();
    trampoline_code[trampoline_code_index++] = temp_code_gen_buf[0];
    temp_code_gen_buf.clear();

    return 0;
}

Trampoline &trampoline_create(const char *name, void *native_func, std::vector<FunctionArgumentType> types, bool is_ellipsis_function, bool has_execution_side_effects) {
    // std::unique_lock lock(trampoline_mutex);

    if (!trampoline_code) {
        throw std::runtime_error("Trampoline buffer not allocated");
    }

    if (trampoline_code + trampoline_code[trampoline_code_index + 2] >= trampoline_code + trampoline_code_size) {
        throw std::runtime_error("Trampoline buffer exhausted");
    }

    u16 id = trampolines.size();
    Trampoline &sym = trampolines.emplace_back(name, id, native_func, types, is_ellipsis_function, has_execution_side_effects);

    u32 *addr = &trampoline_code[trampoline_code_index];

    temp_code_generator.SVC(id + trampoline_id_offset);
    temp_code_generator.RET();

    trampoline_code[trampoline_code_index++] = temp_code_gen_buf[0];
    trampoline_code[trampoline_code_index++] = temp_code_gen_buf[1];

    temp_code_gen_buf.clear();

    sym.guest_trampoline = addr;

    return sym;
}

Trampoline &trampoline_get_by_id(u32 id) {
    // std::shared_lock lock(trampoline_mutex);
    return trampolines[id - trampoline_id_offset];
}

Trampoline *trampoline_maybe_get_by_id(u32 id) {
    if (id < trampoline_id_offset) return nullptr;
    u32 computed_id = id - trampoline_id_offset;
    if (computed_id >= trampolines.size()) {
        return nullptr;
    }

    return &trampolines[computed_id];
}

#if TRAMPOLINE_DEBUG_COUNT_CALLS
void trampoline_print_counts() {
    std::cout << "Trampoline counts:" << std::endl;
    for (auto &trampoline : trampolines)
        std::cout << trampoline.name << ": " << std::dec << trampoline.count.load() << std::endl;
}
#endif
