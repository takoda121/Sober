#pragma once

#include <pthread.h>

void pthread_caller(void *addr, bool pass_ptr_arg, void *ptr);

int pthread_create_impl(pthread_t *thread, const pthread_attr_t *attr, void *(*guest_routine)(void *), void *arg);
int pthread_key_create_impl(pthread_key_t *key, void (*guest_destructor)(void *));
int pthread_key_delete_impl(pthread_key_t key);
int pthread_setspecific_impl(pthread_key_t key, void *value);
void* pthread_getspecific_impl(pthread_key_t key);
