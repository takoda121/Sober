#pragma once

// Thanks https://gracicot.github.io/reflection/2018/04/03/reflection-present.html

#include "types.hpp"
#include <cstdarg>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


template<typename Lambda>
struct function_traits : function_traits<decltype(&Lambda::operator())> {};

template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using result = R;
    using args = std::tuple<Args...>;
    static constexpr size_t nargs = sizeof...(Args);
};

template<typename R, typename... Args>
struct function_traits<R(*)(Args..., ...)> {
    using result = R;
    using args = std::tuple<Args...>;
    static constexpr size_t nargs = sizeof...(Args);
};

template<typename R, typename C, typename... Args>
struct function_traits<R(C::*)(Args...) const> {
    using result = R;
    using parameters = std::tuple<Args...>;
    static constexpr size_t nargs = sizeof...(Args);
};

template<typename F>
using function_arguments_t = typename function_traits<F>::args;

template<typename F>
using function_result_t = typename function_traits<F>::result;


enum class FunctionArgumentType : u8 {
    unsupported = 0,
    none,

    signed_byte,
    signed_halfword,
    signed_word,
    signed_doubleword,
    unsigned_byte,
    unsigned_halfword,
    unsigned_word,
    unsigned_doubleword,

    single_precision_fp,
    double_precision_fp,

    pointer,

    variadic_list
};

static inline bool function_argument_is_float(FunctionArgumentType t) {
    return t == FunctionArgumentType::single_precision_fp || t == FunctionArgumentType::double_precision_fp;
}

template<typename T>
static inline constexpr FunctionArgumentType _get_argument_type() {
    if constexpr(std::is_same<T, unsigned char>()) {
        return FunctionArgumentType::unsigned_byte;
    } else if constexpr(std::is_same<T, bool>()) {
        return FunctionArgumentType::unsigned_byte;
    } else if constexpr(std::is_same<T, signed char>()) {
        return FunctionArgumentType::signed_byte;
    } else if constexpr(std::is_same<T, signed short>()) {
        return FunctionArgumentType::signed_halfword;
    } else if constexpr(std::is_same<T, unsigned short>()) {
        return FunctionArgumentType::unsigned_halfword;
    } else if constexpr(std::is_same<T, signed int>()) {
        return FunctionArgumentType::signed_word;
    } else if constexpr(std::is_same<T, unsigned int>()) {
        return FunctionArgumentType::unsigned_word;
    } else if constexpr(std::is_same<T, signed long>()) {
        return FunctionArgumentType::signed_doubleword;
    } else if constexpr(std::is_same<T, unsigned long>()) {
        return FunctionArgumentType::unsigned_doubleword;
    } else if constexpr(std::is_same<T, signed long long>()) {
        return FunctionArgumentType::signed_doubleword;
    } else if constexpr(std::is_same<T, unsigned long long>()) {
        return FunctionArgumentType::unsigned_doubleword;
    } else if constexpr(std::is_same<T, float>()) {
        return FunctionArgumentType::single_precision_fp;
    } else if constexpr(std::is_same<T, double>()) {
        return FunctionArgumentType::double_precision_fp;
    } else if constexpr(std::is_same<std::decay<va_list>::type, T>()) {
        return FunctionArgumentType::variadic_list;
    } else if constexpr(std::is_pointer<T>()) {
        return FunctionArgumentType::pointer;
    } else if constexpr(std::is_same<T, void>()) {
        return FunctionArgumentType::none;
    } else {
        if constexpr(sizeof(T) == 1) {
            return FunctionArgumentType::unsigned_byte;
        } else if constexpr(sizeof(T) == 2) {
            return FunctionArgumentType::unsigned_halfword;
        } else if constexpr(sizeof(T) == 4) {
            return FunctionArgumentType::unsigned_word;
        } else if constexpr(sizeof(T) == 8) {
            return FunctionArgumentType::unsigned_doubleword;
        } else {
            return FunctionArgumentType::unsupported;
        }
    }
}

template <typename Func, typename IndexSeq>
struct _GetFunctionArgumentTypesHelper;

template <typename Func, size_t... Inds>
struct _GetFunctionArgumentTypesHelper<Func, std::integer_sequence<size_t, Inds...>> {
    static inline constexpr std::vector<FunctionArgumentType> do_it() {
        using func_result = function_result_t<Func>;
        return {
            _get_argument_type<func_result>(),
            _get_argument_type<std::tuple_element_t<Inds, function_arguments_t<Func>>>()...
        };
    }
};

template<typename F>
static inline constexpr std::vector<FunctionArgumentType> get_function_argument_types(F f) {
    (void)f;
    constexpr size_t nargs = function_traits<F>::nargs;
    return _GetFunctionArgumentTypesHelper<F, std::make_index_sequence<nargs>>::do_it();
}

// from libc-shim
struct dynamic_symbol {
    enum class Tag {
        None = 0,
        HasExecutionSideEffects
    };

    const char *name;
    void *value;
    std::vector<FunctionArgumentType> arg_types;
    bool is_ellipsis_function = false;
    bool has_execution_side_effects = false;

    static inline void apply_tag(dynamic_symbol *sym, Tag tag) {
        if (tag == Tag::HasExecutionSideEffects) {
            sym->has_execution_side_effects = true;
        }
    }

    dynamic_symbol(const char *name, void *value)
        : name(name), value(value) {}

    template <typename Ret, typename ...Args>
    dynamic_symbol(const char *name, Ret (*ptr)(Args...))
        : name(name), value((void*) ptr), arg_types(get_function_argument_types(ptr)) {}

    template <typename Ret, typename ...Args>
    dynamic_symbol(const char *name, Ret (*ptr)(Args..., ...))
        : name(name), value((void*) ptr), arg_types(get_function_argument_types(ptr)), is_ellipsis_function(true) {}
    
    template<typename R, typename C, typename... Args>
    dynamic_symbol(const char *name, R(C::*ptr)(Args...) const)
        : name(name), value((void*) ptr), arg_types(get_function_argument_types(ptr)) {}


    template <typename Ret, typename ...Args>
    dynamic_symbol(Tag tag, const char *name, Ret (*ptr)(Args...))
        : name(name), value((void*) ptr), arg_types(get_function_argument_types(ptr)) {
        apply_tag(this, tag);
    }

    template <typename Ret, typename ...Args>
    dynamic_symbol(Tag tag, const char *name, Ret (*ptr)(Args..., ...))
        : name(name), value((void*) ptr), arg_types(get_function_argument_types(ptr)), is_ellipsis_function(true) {
        apply_tag(this, tag);
    }

    template<typename R, typename C, typename... Args>
    dynamic_symbol(Tag tag, const char *name, R(C::*ptr)(Args...) const)
        : name(name), value((void*) ptr), arg_types(get_function_argument_types(ptr)) {
        apply_tag(this, tag);
    }
};

struct deferred_symbol {
    const char *name;
    void **value;
    std::vector<FunctionArgumentType> arg_types;

    template <typename Ret, typename ...Args>
    deferred_symbol(const char *name, Ret (**ptr)(Args...))
        : name(name), value((void**)ptr), arg_types(get_function_argument_types(*ptr)) {}
};
