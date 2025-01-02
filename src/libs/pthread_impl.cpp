#include "../execution_context.hpp"

#include <iostream>
#include <unordered_map>
#include <deque>
#include <algorithm>

void pthread_caller(void *addr, bool pass_ptr_arg, void *ptr) {
    ScopedExecutionFiber fiber(FiberKind::Callee);

    if (pass_ptr_arg)
        fiber.set_register(0, std::bit_cast<u64>(ptr));

    fiber.run(addr);
}

static void *thread_routine(void *arg) {
    auto *wrapper = reinterpret_cast<GuestRoutineWrapper*>(arg);

    execution_context_init();

    u64 out = 0;
    {
        ScopedExecutionFiber fiber(FiberKind::Callee);
        fiber.run(wrapper->guest_routine, wrapper->value);
        out = fiber.get_register(0);
    }

    if (_execution_context_sub_ref() == 0)
        execution_context_free();

    delete wrapper;
    return reinterpret_cast<void*>(out);
}

int pthread_create_impl(pthread_t *thread, const pthread_attr_t *attr, void *(*guest_routine)(void *), void *arg) {
    return pthread_create(thread, attr, thread_routine, new GuestRoutineWrapper(guest_routine, arg));
}

struct MapValue {
    void (*guest_destructor)(void*);
    std::deque<GuestRoutineWrapper*> wrappers;

    MapValue(void (*destructor)(void*)) : guest_destructor(destructor) {}
};

static std::unordered_map<pthread_key_t, MapValue> key_destructor_map;

int pthread_key_create_impl(pthread_key_t *key, void (*guest_destructor)(void *)) {
    int ret = pthread_key_create(key, [] (void *arg) {
        auto *wrapper = reinterpret_cast<GuestRoutineWrapper*>(arg);

        {
            ScopedExecutionFiber fiber(FiberKind::Callee);
            fiber.run(wrapper->guest_routine, wrapper->value);
        }

        delete wrapper;
        if (_execution_context_sub_ref() == 0)
            execution_context_free();
    });

    if (ret)
        return ret;
    
    key_destructor_map.emplace(*key, guest_destructor);

    return 0;
}

int pthread_key_delete_impl(pthread_key_t key) {
    std::cout << "pthread_key_delete called (its a bit broken)\n";

    for (auto *wrapper : key_destructor_map.at(key).wrappers) {
        delete wrapper;
        _execution_context_sub_ref();
    }

    return pthread_key_delete(key);
}

int pthread_setspecific_impl(pthread_key_t key, void *value) {
    auto *wrapper = reinterpret_cast<GuestRoutineWrapper*>(pthread_getspecific(key));

    if (value == nullptr) {
        if (wrapper) {
            auto &wrappers = key_destructor_map.at(key).wrappers;
            wrappers.erase(std::remove(wrappers.begin(), wrappers.end(), wrapper), wrappers.end());
            delete wrapper;
            _execution_context_sub_ref();
            pthread_setspecific(key, nullptr); // guaranteed to not fail
        }
        return 0;
    }

    if (wrapper == nullptr) {
        if (key_destructor_map.contains(key)) {
            MapValue &mapped = key_destructor_map.at(key);
            wrapper = new GuestRoutineWrapper(mapped.guest_destructor, value);
            auto ret = pthread_setspecific(key, wrapper);
            if (ret != 0) {
                delete wrapper;
                return ret;
            }
            mapped.wrappers.push_back(wrapper);
            _execution_context_add_ref();
            return 0;
        } else
            return EINVAL;
    }

    wrapper->value = value;
    return 0;
}

void* pthread_getspecific_impl(pthread_key_t key) {
    auto *wrapper = reinterpret_cast<GuestRoutineWrapper*>(pthread_getspecific(key));
    if (wrapper) [[likely]]
        return wrapper->value;
    else
        return nullptr;
}
