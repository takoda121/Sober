#pragma once

#include <vector>
#include <string>
#include "../../../src/function_traits.hpp"

namespace shim {
    using shimmed_symbol = dynamic_symbol; 

    typedef void (*pthread_function_caller_t)(void *addr, bool pass_ptr_arg, void *ptr);
    typedef int (*pthread_create_impl_t)(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
    typedef int (*pthread_key_create_impl_t)(pthread_key_t *key, void (*destructor)(void *));
    typedef int (*pthread_key_delete_impl_t)(pthread_key_t key);
    typedef int (*pthread_setspecific_impl_t)(pthread_key_t key, void *value);
    typedef void* (*pthread_getspecific_impl_t)(pthread_key_t key);
    void libc_shim_pthread_init(pthread_function_caller_t new_pthread_caller_func, pthread_create_impl_t new_pthread_create_impl, 
        pthread_key_create_impl_t new_pthread_key_create_impl, pthread_key_delete_impl_t new_pthread_key_delete_impl,
        pthread_setspecific_impl_t new_pthread_setspecific_impl, pthread_getspecific_impl_t new_pthread_getspecific_impl);

    typedef int (*atexit_impl_t)(void (*func)(void *), void *arg, void *dso_handle);
    typedef void (*qsort_impl_t)(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
    typedef void *(*bsearch_impl_t)(void *key, void *base, size_t nmemb, size_t size, __compar_fn_t __compar);
    typedef int(*setjmp_impl_t)(u64 *ctx);
    typedef void(*longjmp_impl_t)(u64 *ctx, int value);
    void libc_shim_common_init(atexit_impl_t atexit_impl, qsort_impl_t qsort_impl, bsearch_impl_t bsearch_impl, setjmp_impl_t setjmp_impl, longjmp_impl_t longjmp_impl);

    std::vector<shimmed_symbol> get_shimmed_symbols();

    // Rewrite filesystem access
    extern std::vector<std::pair<std::string, std::string>> rewrite_filesystem_access;
}