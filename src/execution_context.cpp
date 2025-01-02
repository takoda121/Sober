#include "execution_context.hpp"
#include "types.hpp"
#include "trampoline.hpp"
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cstring>
#include <dynarmic/interface/A64/a64.h>
#include <dynarmic/interface/A64/config.h>
#include <dynarmic/interface/exclusive_monitor.h>
#include <dyncall.h>
#include <exception>
#include <iostream>
#include <mutex>
#include <optional>
#include <ratio>
#include <sys/mman.h>

thread_local ExecutionContext execution_context = {};

#if TRAMPOLINE_DEBUG_CALLS
static std::vector<ExecutionContext*> execution_contexts;
ExecutionContext **execution_contexts_buffer = nullptr;
usize num_execution_contexts = 0;
#endif

Dynarmic::A64::Jit *get_jit() { return execution_context.jit; }

class Environment : public Dynarmic::A64::UserCallbacks {
public:
    static constexpr u64 CNT_FREQUENCY = 19'200'000; // CNTPCT Frequency is 19.2 MHz
    using CntRatio = std::ratio<CNT_FREQUENCY, std::nano::den>;
    u32 processor_id = 0;

    explicit Environment() {}

    std::optional<SupervisorFastCall> GetSupervisorFastCall(u32 swi) override {
        Trampoline *trampoline = trampoline_maybe_get_by_id(swi);
        if (!trampoline || !trampoline->native_func) return std::nullopt;

        u32 int_args = 0;
        u32 float_args = 0;

        if (trampoline->has_execution_side_effects) {
            // This function manipulates the execution context (such as by creating a new execution fiber)
            // as such, we don't support making a fast path for it, as it would require a proper halting
            // mechanisn as we have in CallSVC.
            return std::nullopt;
        }

        if (!VARIADIC_TRAMPOLINE_FAST_PATH && trampoline->is_ellipsis_function) {
            // We don't support fast paths for ellipsis functions
            return std::nullopt;
        }

        for (usize i = 1; i < trampoline->types.size(); i++) {
            switch (trampoline->types[i]) {
                case FunctionArgumentType::unsigned_byte: /* through */
                case FunctionArgumentType::signed_byte: /* through */
                case FunctionArgumentType::unsigned_halfword: /* through */
                case FunctionArgumentType::signed_halfword: /* through */
                case FunctionArgumentType::unsigned_word: /* through */
                case FunctionArgumentType::signed_word: /* through */
                case FunctionArgumentType::unsigned_doubleword: /* through */
                case FunctionArgumentType::signed_doubleword: /* through */
                case FunctionArgumentType::pointer: {
                    int_args++;
                } break;
            
                case FunctionArgumentType::single_precision_fp: /* through */
                case FunctionArgumentType::double_precision_fp: {
                    float_args++;
                } break;

                case FunctionArgumentType::variadic_list: {
                    // We don't support fast paths for functions that take va_args
                    return std::nullopt;
                } break;

                case FunctionArgumentType::none: /* through */
                case FunctionArgumentType::unsupported: /* through */
                default: {
                    std::cerr << "GetSupervisorFastCall: Unsupported function argument type" << std::endl;
                    return std::nullopt;
                }
            }
        }

        // Linux x86_64 ABI limitations
        if (int_args > 6 || float_args > 8) {
            return std::nullopt;
        }

        // Pass as many register arguments as we can for ellipsis functions
        // in the CallSVC, we support up to 8 int args instead of the 6 here.
        // We don't support stack-spilled variadic arguments in either.
        // This *should* be fine.
        if (VARIADIC_TRAMPOLINE_FAST_PATH && trampoline->is_ellipsis_function) {
            int_args = 6;
            float_args = 8;
        }

        SupervisorFastCall::ReturnType return_type;

        switch (trampoline->types[0]) {
            case FunctionArgumentType::unsigned_byte: /* through */
            case FunctionArgumentType::signed_byte: /* through */
            case FunctionArgumentType::unsigned_halfword: /* through */
            case FunctionArgumentType::signed_halfword: /* through */
            case FunctionArgumentType::unsigned_word: /* through */
            case FunctionArgumentType::signed_word: /* through */
            case FunctionArgumentType::unsigned_doubleword: /* through */
            case FunctionArgumentType::signed_doubleword: /* through */
            case FunctionArgumentType::pointer: {
                return_type = SupervisorFastCall::ReturnType::Integer;
            } break;

            case FunctionArgumentType::single_precision_fp: /* through */
            case FunctionArgumentType::double_precision_fp: {
                return_type = SupervisorFastCall::ReturnType::Float;
            } break;

            case FunctionArgumentType::none: {
                return_type = SupervisorFastCall::ReturnType::Void;
            } break;

            case FunctionArgumentType::unsupported: /* through */
            default: {
                std::cerr << "GetSupervisorFastCall: Unsupported function return type" << std::endl;
                return std::nullopt;
            }
        }

        return SupervisorFastCall(reinterpret_cast<void(*)()>(trampoline->native_func), int_args, float_args, return_type);
    }

    template<typename T>
    inline T read(u64 vaddr) {
        // std::cout << processor_id << ": Reading @ " << std::hex << vaddr << std::endl;
        T value;
        std::memcpy(&value, reinterpret_cast<void*>(vaddr), sizeof(T));
        return value;
    }

    template<typename T>
    inline void write(u64 vaddr, const T& value) {
        // std::cout << "Writing @ " << std::hex << vaddr << std::endl;
        std::memcpy((void*)vaddr, &value, sizeof(T));
    }

    std::optional<u32> MemoryReadCode(u64 vaddr) override {
        //std::cout << processor_id << ": Reading code @ " << std::hex << vaddr << std::endl;
        u32 a = read<u32>(vaddr);
        return a;
    }

    u8 MemoryRead8(u64 vaddr) override { return read<u8>(vaddr); }
    u16 MemoryRead16(u64 vaddr) override { return read<u16>(vaddr); }
    u32 MemoryRead32(u64 vaddr) override { return read<u32>(vaddr); }
    u64 MemoryRead64(u64 vaddr) override { return read<u64>(vaddr); }
    Dynarmic::A64::Vector MemoryRead128(u64 vaddr) override { return read<Dynarmic::A64::Vector>(vaddr); }

    void MemoryWrite8(u64 vaddr, u8 value) override { write(vaddr, value); }
    void MemoryWrite16(u64 vaddr, u16 value) override { write(vaddr, value); }
    void MemoryWrite32(u64 vaddr, u32 value) override { write(vaddr, value); }
    void MemoryWrite64(u64 vaddr, u64 value) override { write(vaddr, value); }
    void MemoryWrite128(u64 vaddr, Dynarmic::A64::Vector value) override { write(vaddr, value); }

    template <typename T>
    bool write_exclusive(T *ptr, const T value, const T expected) {
        return __sync_bool_compare_and_swap(ptr, expected, value);
    }

    bool MemoryWriteExclusive8(u64 vaddr, u8 value, [[maybe_unused]] u8 expected) override {
        return write_exclusive(std::bit_cast<u8*>(vaddr), value, expected);
    }

    bool MemoryWriteExclusive16(u64 vaddr, u16 value, [[maybe_unused]] u16 expected) override {
        return write_exclusive(std::bit_cast<u16*>(vaddr), value, expected);
    }

    bool MemoryWriteExclusive32(u64 vaddr, u32 value, [[maybe_unused]] u32 expected) override {
        return write_exclusive(std::bit_cast<u32*>(vaddr), value, expected);
    }

    bool MemoryWriteExclusive64(u64 vaddr, u64 value, [[maybe_unused]] u64 expected) override {
        return write_exclusive(std::bit_cast<u64*>(vaddr), value, expected);
    }

    bool MemoryWriteExclusive128(u64 vaddr, Dynarmic::A64::Vector value, [[maybe_unused]] Dynarmic::A64::Vector expected) override {
        std::cerr << "STUB: MemoryWriteExclusive128" << std::endl;
        MemoryWrite128(vaddr, value);
        return true;
    }

    void AddTicks(u64) override {
        assert(!"Cycle counting is expected to be disabled");
    }

    u64 GetTicksRemaining() override {
        assert(!"Cycle counting is expected to be disabled");
        return 0;
    }

    u64 GetCNTPCT() override {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() * CntRatio::num / CntRatio::den;
    }

    void InterpreterFallback(u64 pc, usize num_instructions) override {
        u32 instruction = *(std::bit_cast<u32*>(pc));
        if (num_instructions == 1 && instruction == 0xd53be040) { // mrs x0, cntvct_el0
            // std::cout << "cntpct: " << std::dec << GetCNTPCT() << std::endl;

            assert(execution_context.initialized);
            auto jit = get_jit();

            jit->SetRegister(0, GetCNTPCT());
            jit->SetPC(pc + 4);
        } else {
            std::cerr << "Unimplemented instruction at " << std::hex << pc << "[" << std::hex << instruction << "] (" << num_instructions << " instructions)" << std::endl;
            assert(!"Unimplemented instruction");
        }
    }

    void ExceptionRaised([[maybe_unused]] u64 pc, Dynarmic::A64::Exception exception) override {
        switch (exception) {
            case Dynarmic::A64::Exception::Yield: {
                assert(execution_context.initialized);
                get_jit()->HaltExecution(SuccessfulExit);
            } break;

            default: {
                assert(!"exception raised");
            } break;
        }
    }

    void CallSVC(u32 swi) override {
        assert(execution_context.initialized);
        execution_context.svc_number = swi;

        get_jit()->HaltExecution(SupervisorCall);
    }
};

Dynarmic::ExclusiveMonitor exclusive_monitor {MAX_CPUS};

struct x86_64VaArgsData {
    u32 gp_offset;
    u32 fp_offset;
    u8 *overflow_arg_area;
    u8 *reg_save_area;
};

struct Aarch64VaArgsData {
    u8 *stack;
    u8 *gr_top;
    u8 *vr_top;
    i32 gr_offs;
    i32 vr_offs;
};

static bool call_logging_enabled = false;

static void handle_call_svc(u32 swi) {
    auto jit = get_jit();
    auto call_vm = execution_context.dyncall_vm;

    auto &trampoline = trampoline_get_by_id(swi);

#if TRAMPOLINE_DEBUG_CALLS
    if (call_logging_enabled)
        std::cout << execution_context.processor_id << ": Enter " << trampoline.name << " with id " << swi << std::endl;

    execution_context.last_calls_buffer[execution_context.last_calls_buffer_write_index] = trampoline.name;
    execution_context.last_calls_buffer_write_index = (execution_context.last_calls_buffer_write_index + 1) % ExecutionContext::LAST_CALLS_BUFFER_SIZE;
#endif

#if TRAMPOLINE_DEBUG_COUNT_CALLS
    trampoline.count.fetch_add(1);
#endif

    dcReset(call_vm);

    int x_reg = 0;
    int v_reg = 0;
    u64 *stack_cursor = (u64*)jit->GetSP();

    FunctionArgumentType return_type = trampoline.types[0];
    DCpointer fptr = trampoline.native_func;
    x86_64VaArgsData native_va_args_data;
    alignas(16) u8 gp_arg_buffer[16*sizeof(u64)];

    for (usize i = 1; i < trampoline.types.size(); i++) {
        switch (trampoline.types[i]) {
            case FunctionArgumentType::unsigned_byte: /* through */
            case FunctionArgumentType::signed_byte: /* through */
            case FunctionArgumentType::unsigned_halfword: /* through */
            case FunctionArgumentType::signed_halfword: /* through */
            case FunctionArgumentType::unsigned_word: /* through */
            case FunctionArgumentType::signed_word: /* through */
            case FunctionArgumentType::unsigned_doubleword: /* through */
            case FunctionArgumentType::signed_doubleword: /* through */
            case FunctionArgumentType::pointer: {
                if (x_reg > 7) {
                    dcArgLongLong(call_vm, std::bit_cast<u64>(*stack_cursor++));
                    break;
                }
                dcArgLongLong(call_vm, std::bit_cast<u64>(jit->GetRegister(x_reg++)));
            } break;
        
            case FunctionArgumentType::single_precision_fp: /* through */
            case FunctionArgumentType::double_precision_fp: {
                if (v_reg > 7) {
                    dcArgDouble(call_vm, std::bit_cast<double>(*stack_cursor++));
                    break;
                }
                dcArgDouble(call_vm, std::bit_cast<double>(jit->GetVector(v_reg++)[0]));
            } break;

#ifdef __x86_64__
            case FunctionArgumentType::variadic_list: {
                // TODO: may break if theres more than 8 float arguments in the list
                Aarch64VaArgsData *guest_va_args_data;
                if (x_reg > 7) [[unlikely]] {
                    guest_va_args_data = reinterpret_cast<Aarch64VaArgsData *>(*stack_cursor++);
                } else {
                    guest_va_args_data = reinterpret_cast<Aarch64VaArgsData *>(jit->GetRegister(x_reg++));
                }

                native_va_args_data.gp_offset = 48;
                if (guest_va_args_data->gr_offs >= 0) {
                    native_va_args_data.overflow_arg_area = guest_va_args_data->stack;
                } else {
                    i32 gr_size = -guest_va_args_data->gr_offs;
                    native_va_args_data.overflow_arg_area = gp_arg_buffer;
                    std::memcpy(gp_arg_buffer, guest_va_args_data->gr_top - gr_size, gr_size);
                    if (sizeof(gp_arg_buffer) - gr_size > 0 && guest_va_args_data->stack != nullptr)
                        std::memcpy(gp_arg_buffer + gr_size, guest_va_args_data->stack, sizeof(gp_arg_buffer) - gr_size);
                }
                // the real exhaust value for fp_offset is 176
                native_va_args_data.fp_offset = 176 + guest_va_args_data->vr_offs;
                native_va_args_data.reg_save_area = guest_va_args_data->vr_top - 176;

                dcArgPointer(call_vm, &native_va_args_data);
            } break;
#endif

            case FunctionArgumentType::none: /* through */
            case FunctionArgumentType::unsupported: /* through */
            default: {
                std::cerr << "CallSVC(): Unsupported function argument type" << std::endl;
                return;
            }
        }
    }

    // Hack for ellipsis functions :^)
    // TODO: support stack arguments too
    if (trampoline.is_ellipsis_function) [[unlikely]] {
        for (int i = x_reg; i < 8; i++) {
            dcArgLongLong(call_vm, jit->GetRegister(i));
        }

        for (int i = v_reg; i < 8; i++) {
            dcArgDouble(call_vm, std::bit_cast<double>(jit->GetVector(i)[0]));
        }
    }

    switch (return_type) {
        case FunctionArgumentType::unsigned_byte: /* through */
        case FunctionArgumentType::signed_byte: /* through */
        case FunctionArgumentType::unsigned_halfword: /* through */
        case FunctionArgumentType::signed_halfword: /* through */
        case FunctionArgumentType::unsigned_word: /* through */
        case FunctionArgumentType::signed_word: /* through */
        case FunctionArgumentType::unsigned_doubleword: /* through */
        case FunctionArgumentType::signed_doubleword: /* through */
        case FunctionArgumentType::pointer: {
            jit->SetRegister(0, reinterpret_cast<u64>(dcCallPointer(call_vm, fptr)));
        } break;
    
        case FunctionArgumentType::single_precision_fp: /* through */
        case FunctionArgumentType::double_precision_fp: {
            auto vec = jit->GetVector(0);
            vec[0] = std::bit_cast<u64>(dcCallDouble(call_vm, fptr));
            jit->SetVector(0, vec);
        } break;

        case FunctionArgumentType::none: {
            dcCallVoid(call_vm, fptr);
        } break;

        case FunctionArgumentType::unsupported: /* through */
        default: {
            std::cerr << "CallSVC(): Unsupported function return type" << std::endl;
            return;
        }
    }
}


static std::mutex processor_mutex;
static std::array<bool, MAX_CPUS> used_processors = {};

static usize allocate_processor_id() {
    std::unique_lock lock(processor_mutex);

    static constexpr usize not_found_sentinel = -1;
    usize found_index = not_found_sentinel;

    for (usize i = 0; i < used_processors.size(); i++) {
        if (!used_processors[i]) {
            found_index = i;
            break;
        }
    }

    if (found_index == not_found_sentinel) {
        std::cerr << "Could not allocate processor_id: ran out of ids" << std::endl;
        std::terminate();
        return not_found_sentinel;
    }

    used_processors[found_index] = true;

    return found_index;
}

static void release_processor_id(usize id) {
    std::unique_lock lock(processor_mutex);

    used_processors[id] = false;
}

void execution_context_init() {
    assert(!execution_context.initialized);

    usize id = allocate_processor_id();

    execution_context.stack_size = STACK_SIZE;
    execution_context.stack = reinterpret_cast<u8*>(mmap(nullptr, execution_context.stack_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    if (execution_context.stack == MAP_FAILED) {
        std::cerr << "execution_context_init(): Failed to allocate stack" << std::endl;
    }
    void *stack_top = (u8*)execution_context.stack + execution_context.stack_size;

    execution_context.tls_buffer_addr = reinterpret_cast<u64>(execution_context.tls_buffer);

    execution_context.environment = new Environment();

    Dynarmic::A64::UserConfig config{execution_context.environment};
    config.global_monitor = &exclusive_monitor;
    config.processor_id = id; // TODO: breaks atomic operations?
    config.tpidr_el0 = &execution_context.tls_buffer_addr;
    config.fastmem_pointer = 0;
    config.fastmem_address_space_bits = 64;
    config.fastmem_exclusive_access = true;
    config.recompile_on_fastmem_failure = false;
    config.recompile_on_exclusive_fastmem_failure = false;
    config.silently_mirror_fastmem = false;
    config.enable_cycle_counting = false;
    config.wall_clock_cntpct = true; // hack?
    config.cntfrq_el0 = execution_context.environment->CNT_FREQUENCY;
    config.code_cache_size = 192 * 1024 * 1024;
    
    if (USE_UNSAFE_OPTIMIZATIONS) {
        config.unsafe_optimizations = true;
        config.optimizations |= Dynarmic::OptimizationFlag::Unsafe_IgnoreGlobalMonitor;
    }

    if (USE_MORE_UNSAFE_OPTIMIZATIONS) {
        config.optimizations |= Dynarmic::OptimizationFlag::Unsafe_ReducedErrorFP;
        config.optimizations |= Dynarmic::OptimizationFlag::Unsafe_InaccurateNaN;
        config.optimizations |= Dynarmic::OptimizationFlag::Unsafe_IgnoreStandardFPCRValue;
    }

    execution_context.environment->processor_id = config.processor_id;
    execution_context.processor_id = config.processor_id;

    Dynarmic::A64::Jit *jit = new Dynarmic::A64::Jit(config);
    jit->SetSP(reinterpret_cast<u64>(stack_top));

    execution_context.jit = jit;

    execution_context.dyncall_vm = dcNewCallVM(2048);
    dcMode(execution_context.dyncall_vm, DC_CALL_C_DEFAULT);

    execution_context.initialized = true;

#if TRAMPOLINE_DEBUG_CALLS
    execution_contexts.push_back(&execution_context);
    execution_contexts_buffer = execution_contexts.data();
    num_execution_contexts = execution_contexts.size();
#endif
}

void execution_context_free() {
#if TRAMPOLINE_DEBUG_CALLS
    execution_contexts.erase(std::remove(execution_contexts.begin(), execution_contexts.end(), &execution_context), execution_contexts.end());
    execution_contexts_buffer = execution_contexts.data();
    num_execution_contexts = execution_contexts.size();
#endif

    if (execution_context.environment) delete execution_context.environment;
    if (execution_context.jit) delete execution_context.jit;
    if (execution_context.stack) munmap(execution_context.stack, execution_context.stack_size);
    if (execution_context.dyncall_vm) dcFree(execution_context.dyncall_vm);

    execution_context.initialized = false;

    release_processor_id(execution_context.processor_id);
}

static SavedRegisters get_saved_registers() {
    auto jit = get_jit();
    
    return {
        .r19 = jit->GetRegister(19),
        .r20 = jit->GetRegister(20),
        .r21 = jit->GetRegister(21),
        .r22 = jit->GetRegister(22),
        .r23 = jit->GetRegister(23),
        .r24 = jit->GetRegister(24),
        .r25 = jit->GetRegister(25),
        .r26 = jit->GetRegister(26),
        .r27 = jit->GetRegister(27),
        .r28 = jit->GetRegister(28),
        .r29 = jit->GetRegister(29),
        .r30 = jit->GetRegister(30),
        .sp = jit->GetSP(),
        .pc = jit->GetPC(),
        .d8 = jit->GetVector(8)[0],
        .d9 = jit->GetVector(9)[0],
        .d10 = jit->GetVector(10)[0],
        .d11 = jit->GetVector(11)[0],
        .d12 = jit->GetVector(12)[0],
        .d13 = jit->GetVector(13)[0],
        .d14 = jit->GetVector(14)[0],
        .d15 = jit->GetVector(15)[0],
    };
}

static void restore_saved_registers(SavedRegisters regs) {
    auto jit = get_jit();

    jit->SetRegister(19, regs.r19);
    jit->SetRegister(20, regs.r20);
    jit->SetRegister(21, regs.r21);
    jit->SetRegister(22, regs.r22);
    jit->SetRegister(23, regs.r23);
    jit->SetRegister(24, regs.r24);
    jit->SetRegister(25, regs.r25);
    jit->SetRegister(26, regs.r26);
    jit->SetRegister(27, regs.r27);
    jit->SetRegister(28, regs.r28);
    jit->SetRegister(29, regs.r29);
    jit->SetRegister(30, regs.r30);
    jit->SetSP(regs.sp);
    jit->SetPC(regs.pc);

    auto set_d_reg = [jit](u64 index, u64 value) {
        auto vector = jit->GetVector(index);
        vector[0] = value;
        jit->SetVector(index, vector);
    };

    set_d_reg(8, regs.d8);
    set_d_reg(9, regs.d9);
    set_d_reg(10, regs.d10);
    set_d_reg(11, regs.d11);
    set_d_reg(12, regs.d12);
    set_d_reg(13, regs.d13);
    set_d_reg(14, regs.d14);
    set_d_reg(15, regs.d15);
}

static void execution_loop() {
    assert(execution_context.initialized);

    auto jit = get_jit();
    bool running = true;

    while (running) {
        jit->ClearExclusiveState(); // TODO?
        auto reason = jit->Run();
        switch (reason) {
            case SupervisorCall: {
                handle_call_svc(execution_context.svc_number);
            } break;

            case SuccessfulExit: {
                running = false;
            } break;

            default: {
                std::cerr << "execution_context_loop: jit halted with unexpected reason: " << static_cast<u32>(reason) << ", terminating..." << std::endl;
                std::terminate();
            } break;
        }
    }
}


ScopedExecutionFiber::ScopedExecutionFiber(FiberKind kind)
    : kind_(kind)
{
    assert(execution_context.initialized);
    if (kind == FiberKind::Callee) {
        saved_registers = get_saved_registers();
    }
}

ScopedExecutionFiber::~ScopedExecutionFiber() {
    assert(execution_context.initialized);
    if (kind_ == FiberKind::Callee) {
        restore_saved_registers(saved_registers);
    }
}

u64 ScopedExecutionFiber::get_register(size_t index) {
    return get_jit()->GetRegister(index);
}

void ScopedExecutionFiber::set_register(size_t index, u64 value) {
    get_jit()->SetRegister(index, value);
}

void ScopedExecutionFiber::set_vector_first(size_t index, u64 value) {
    auto vector = get_jit()->GetVector(index);
    vector[0] = value;
    get_jit()->SetVector(index, vector);
}

void ScopedExecutionFiber::run(u64 address) {
    auto jit = get_jit();
    assert(jit && !jit->IsExecuting());

    jit->SetRegister(30, reinterpret_cast<u64>(trampoline_get_guest_exit()));
    jit->SetPC(address);

    execution_loop();
}

void ScopedExecutionFiber::run(void *routine) {
    run(std::bit_cast<u64>(routine));
}

usize _execution_context_add_ref() {
    return ++execution_context.ref_count;
}

usize _execution_context_sub_ref() {
    return --execution_context.ref_count;
}

void set_call_logging(bool logging_enabled) {
    call_logging_enabled = logging_enabled;
}
