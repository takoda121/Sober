#include "libc_impl.hpp"
#include "execution_context.hpp"


int global_argc;
char **global_argv;

void linker_func_caller(void *addr, bool is_ctor) {
    ScopedExecutionFiber fiber(FiberKind::Callee);

    if (is_ctor)
        fiber.run(addr, global_argc, global_argv, environ);
    else
        fiber.run(addr);
}

int linker_phdr_cb_caller(void *addr, dl_phdr_info* info, size_t size, void* data) {
    ScopedExecutionFiber fiber(FiberKind::Callee);
    fiber.run(addr, info, size, data);
    return fiber.get_register(0);
}

static void atexit_routine(void *arg) {
    ScopedExecutionFiber fiber(FiberKind::Callee);

    auto *wrapper = reinterpret_cast<GuestRoutineWrapper*>(arg);
    fiber.run(wrapper->guest_routine, wrapper->value);
    delete wrapper;
}

extern "C" int __cxa_atexit(void (*)(void *), void*, void*);

int atexit_impl(void (*guest_routine)(void *), void *arg, void *dso_handle) {
    return __cxa_atexit(atexit_routine, new GuestRoutineWrapper(guest_routine, arg), dso_handle);
}

thread_local __compar_fn_t current_qsort_compar = nullptr;
thread_local __compar_fn_t current_bsearch_compar = nullptr;

void *bsearch_impl(void *key, void *base, size_t nmemb, size_t size, __compar_fn_t __compar) {
    current_bsearch_compar = __compar;
    return bsearch(key, base, nmemb, size, [] (const void *a, const void *b) -> int {
        ScopedExecutionFiber fiber(FiberKind::Callee);
        fiber.run(current_bsearch_compar, a, b);
        return fiber.get_register(0);
    });
}

void qsort_impl(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar) {
    current_qsort_compar = __compar;
    qsort(__base, __nmemb, __size, [] (const void *a, const void *b) -> int {
        ScopedExecutionFiber fiber(FiberKind::Callee);
        fiber.run(current_qsort_compar, a, b);
        return fiber.get_register(0);
    });
}

int setjmp_impl(u64 *ctx) {
    // ctx is an array of 32 u64's.
    SavedRegisters *regs = reinterpret_cast<SavedRegisters*>(ctx);

    ScopedExecutionFiber fiber(FiberKind::Callee);

    *regs = fiber.saved_registers;

    return 0;
}

void longjmp_impl(u64 *ctx, int value) {
    // ctx is an array of 32 u64's.
    SavedRegisters *regs = reinterpret_cast<SavedRegisters*>(ctx);

    ScopedExecutionFiber fiber(FiberKind::Callee);
    fiber.saved_registers = *regs;
    fiber.set_register(0, value);
}
