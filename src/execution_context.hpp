#pragma once

#include "types.hpp"
#include "trampoline.hpp"
#include <dynarmic/interface/A64/a64.h>
#include <dynarmic/interface/halt_reason.h>
#include <dyncall.h>

static constexpr bool USE_UNSAFE_OPTIMIZATIONS = true;
static constexpr bool USE_MORE_UNSAFE_OPTIMIZATIONS = false;
static constexpr bool VARIADIC_TRAMPOLINE_FAST_PATH = true;
static constexpr usize TLS_SIZE = 512;
static constexpr usize STACK_SIZE = 4 * 1024 * 1024;
static constexpr usize MAX_CPUS = 64;

static constexpr Dynarmic::HaltReason SuccessfulExit = Dynarmic::HaltReason::UserDefined1;
static constexpr Dynarmic::HaltReason SupervisorCall = Dynarmic::HaltReason::UserDefined2;


class Environment;

struct SavedRegisters {
    u64 r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, sp, pc, d8, d9, d10, d11, d12, d13, d14, d15;
};

struct ExecutionContext {
    Environment *environment = nullptr;

    Dynarmic::A64::Jit *jit = nullptr;

    DCCallVM *dyncall_vm = nullptr;

    u8 *stack = nullptr;
    usize stack_size = 0;

    bool initialized = false;
    usize processor_id = 0;
    usize ref_count = 1;

    u32 svc_number = 0;

    u64 tls_buffer_addr = 0;
    u64 tls_buffer[TLS_SIZE];

#if TRAMPOLINE_DEBUG_CALLS
    static constexpr usize LAST_CALLS_BUFFER_SIZE = 1;
    const char *last_calls_buffer[LAST_CALLS_BUFFER_SIZE];
    usize last_calls_buffer_write_index = 0;
#endif
};

extern thread_local ExecutionContext execution_context;
extern ExecutionContext **execution_contexts_buffer;
extern usize num_execution_contexts;

struct GuestRoutineWrapper {
    void *guest_routine;
    void *value;

    template<typename Ret, typename ...Args>
    GuestRoutineWrapper(Ret (*func)(Args...), void *value) : guest_routine(reinterpret_cast<void*>(func)), value(value) {}
};

enum class FiberKind {
    Root,
    Callee
};

struct ScopedExecutionFiber {
    explicit ScopedExecutionFiber(FiberKind kind);
    ~ScopedExecutionFiber();

    SavedRegisters saved_registers;

    void run(u64 address);
    void run(void *routine);
    void set_register(size_t index, u64 value);
    void set_vector_first(size_t index, u64 value);
    u64 get_register(size_t index);

    template<typename... Args>
    void run(void *routine, Args ...args) {
        int register_index = 0, vector_index = 0;
        ([&] {
            if constexpr(std::is_integral<Args>()) {
                set_register(register_index++, args);
            } else if constexpr(std::is_pointer<Args>()) {
                set_register(register_index++, reinterpret_cast<u64>(args));
            } else if constexpr(std::is_null_pointer<Args>()) {
                set_register(register_index++, 0);
            } else if constexpr(std::is_same<Args, float>()) {
                set_vector_first(vector_index++, static_cast<u64>(std::bit_cast<u32>(args)));
            } else
                static_assert(!"dont know how to handle arg type");
        } (), ...);
        run(routine);
    }

    template<typename R, typename... Args>
    void run(R (*routine)(Args...), Args ...args) {
        run(reinterpret_cast<void*>(routine), args...);
    }

    template<typename R, typename... Args>
    R run_with_return(void *routine, Args ...args) {
        run(routine, args...);
        return reinterpret_cast<R>(get_register(0));
    }

private:
    FiberKind kind_;
};


void execution_context_init();
void execution_context_free();
void set_call_logging(bool logging_enabled);
usize _execution_context_add_ref();
usize _execution_context_sub_ref();
