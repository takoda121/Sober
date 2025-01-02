#pragma once

#include <cstddef>
#include <cstdlib>
#include "types.hpp"
#include "linker.hpp"


extern int global_argc;
extern char **global_argv;


void linker_func_caller(void *addr, bool is_ctor);
int linker_phdr_cb_caller(void *addr, dl_phdr_info* info, size_t size, void* data);
int atexit_impl(void (*guest_routine)(void *), void *arg, void *dso_handle);
void *bsearch_impl(void *key, void *base, size_t nmemb, size_t size, __compar_fn_t __compar);
void qsort_impl(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar);
int setjmp_impl(u64 *ctx);
void longjmp_impl(u64 *ctx, int value);
