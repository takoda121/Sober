#include "jni.hpp"
#include "trampoline.hpp"
#include <cassert>
#include <exception>
#include <iostream>
#include <ostream>
#include <type_traits>
#include <vector>

namespace JNI {

thread_local JNIThreadData thread_data = {};

Class *string_class, *list_class;

static JNIEnv *attach_thread(JavaVM *vm) {
    if (thread_data.initialized) {
        return &thread_data.env;
    }

    thread_data.env.native_interface_functions = vm->native_interface_functions;
    thread_data.env.vm = vm;

    thread_data.initialized = true;

    return &thread_data.env;
}

static std::vector<void*> jni_env_functions;
static std::vector<void*> jni_invocation_functions;
static void **invocation_interface = nullptr;
static void **jni_env = nullptr;
static bool did_init_funcs = false;
JavaVM *java_vm_new() {
    JavaVM *vm = new JavaVM();
    if (!did_init_funcs) {
        for (const auto &sym : jni_env_syms) {
            void *trampoline = sym.arg_types.size() ? trampoline_create(sym.name, sym.value, sym.arg_types, sym.is_ellipsis_function, sym.has_execution_side_effects).guest_trampoline : sym.value;
            jni_env_functions.push_back(trampoline);
        }
        for (const auto &sym : jni_invocation_interface_syms) {
            void *trampoline = sym.arg_types.size() ? trampoline_create(sym.name, sym.value, sym.arg_types, sym.is_ellipsis_function, sym.has_execution_side_effects).guest_trampoline : sym.value;
            jni_invocation_functions.push_back(trampoline);
        }
        jni_env = jni_env_functions.data();
        invocation_interface = jni_invocation_functions.data();
        did_init_funcs = true;
    }

    vm->invocation = invocation_interface;
    vm->native_interface_functions = jni_env;

    attach_thread(vm);

    string_class = create_class(vm, "java/lang/String");
    string_class->method("getBytes", [](Object *object, va_list) -> ValueVariant {
        std::string &str = reinterpret_cast<String*>(object)->string;
        auto bytes = new ByteArray();
        std::copy(str.begin(), str.end(), std::back_inserter(bytes->array));
        return bytes;
    });

    // TODO: this is probably not correct lol
    list_class = create_class(vm, "java/util/List");
    list_class->method("get", [](Object *list, va_list args) -> ValueVariant {
        jint index = va_arg(args, jint);
        return reinterpret_cast<ObjectArray*>(list)->array.at(index);
    });
    list_class->method("size", [](Object *list, va_list) -> ValueVariant {
        return reinterpret_cast<Array*>(list)->array_size;
    });

    return vm;
}

JNIEnv *get_jni_env() {
    if (!thread_data.initialized) {
        return nullptr;
    }

    return &thread_data.env;
}

Class *create_class(JavaVM *vm, const char *name) {
    auto p = vm->name_to_class.insert({ name, {name} });
    return &p.first->second;
}

String *create_string(const char *data) {
    String *string = new String();
    string->string = data;
    return string;
}

String *create_string(const std::string &str) {
    return create_string(str.c_str());
}

Method *Class::static_method(const char *name, const static_method_function &func) {
    return &methods.insert({ name, { name, this, func } }).first->second;
}

Method *Class::method(const char *name, const method_function &func) {
    return &methods.insert({ name, { name, this, func } }).first->second;
}

jfieldID Class::static_field(const char *name, ValueVariant value) {
    return &fields.insert({ name, Field(value) }).first->first;
}

jfieldID Class::field(const char *name) {
    return &fields.insert({ name, Field(nullptr) }).first->first;
}

jint GetVersion(JNIEnv *) { return 0x10006; }

jint GetEnv(JavaVM *, void **env, jint version) {
    assert(version == 0x10006);

    if (!thread_data.initialized) {
        *env = nullptr;
        return JNI_EDETACHED;
    }

    *env = &thread_data.env;

    return JNI_OK;
}

void DeleteGlobalRef(JNIEnv *, jobject) {
    // no-op
}

void DeleteLocalRef(JNIEnv *, jobject) {
    // no-op
}

jclass FindClass(JNIEnv *env, const char *name) {
    auto it = env->vm->name_to_class.find(name);
    if (it != env->vm->name_to_class.end()) {
#if JNI_LOGGING
        std::cout << "class " << name << std::endl;
#endif
        return &((*it).second);
    } else {
        std::cerr << "!! CLASS NOT FOUND IN FindClass: " << name << std::endl;
        std::terminate();
        env->has_exception = true;
        return reinterpret_cast<jclass>(JNI_EEXIST);
    }
}

static jmethodID find_method_generic(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    auto it = clazz->methods.find(name);
    if (it != clazz->methods.end()) {
        return &((*it).second);
    } else {
        std::cerr << "!! METHOD NOT FOUND: " << name << ", " << sig << " in " << clazz->full_name << std::endl;
        std::terminate();
        env->has_exception = true;
        return reinterpret_cast<jmethodID>(JNI_EEXIST);
    }
}

jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
#if JNI_LOGGING
    std::cout << "static method " << name << " with sig " << sig << " in " << clazz->full_name << std::endl;
#endif
    return find_method_generic(env, clazz, name, sig);
}

jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
#if JNI_LOGGING
    std::cout << "method " << name << " with sig " << sig << " in " << clazz->full_name << std::endl;
#endif
    return find_method_generic(env, clazz, name, sig);
}

// TODO: hack just to get this working for now
jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
#if JNI_LOGGING
    std::cout << "field " << name << " with sig " << sig << " in " << clazz->full_name << std::endl;
#endif
    auto it = clazz->fields.find(name);
    if (it != clazz->fields.end()) {
        return &((*it).first);
    } else {
        std::cerr << "!! FIELD NOT FOUND: " << name << ", " << sig << " in " << clazz->full_name << std::endl;
        std::terminate();
        env->has_exception = true;
        return nullptr;
    }
}

jfieldID GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
#if JNI_LOGGING
    std::cout << "static "; // no newline
#endif
    return GetFieldID(env, clazz, name, sig);
}

jboolean ExceptionCheck(JNIEnv *env) {
    return env->has_exception;
}

jthrowable ExceptionOccurred(JNIEnv *env) {
    if (env->has_exception) {
        assert(!"ExceptionOccurred was called while an exception was thrown");
        return nullptr;
    }
    return nullptr;
}

void ExceptionClear(JNIEnv *env) {
    env->has_exception = false;
}

void ExceptionDescribe(JNIEnv *) {
    std::cerr << "ExceptionDescribe called" << std::endl;
}

jsize GetStringUTFLength(JNIEnv *, jstring string) {
    return string->string.length();
}

const char *GetStringUTFChars(JNIEnv *, jstring string, jboolean *is_copy) {
    if (is_copy)
        *is_copy = false;
    return string->string.data();
}
void ReleaseStringUTFChars(JNIEnv *, jstring, char *) {
    // no-op
}

jint AttachCurrentThread(JavaVM *vm, void **env, void *) {
    attach_thread(vm);
    *env = &thread_data.env;
    return JNI_OK;
}

jint DetachCurrentThread(JavaVM *) {
    return JNI_OK; // no-op
}

jobject NewGlobalRef(JNIEnv *, jobject obj) {
    return obj;
}

jclass GetObjectClass(JNIEnv *, jobject o) {
    return o->super;
}

jstring NewStringUTF(JNIEnv *, const char *string) {
    return create_string(string);
}

jweak NewWeakGlobalRef(JNIEnv *, jobject object) {
    return object; // no-op
}
void DeleteWeakGlobalRef(JNIEnv *, jweak) {
    // no-op
}

template<typename T>
static T method_call_generic(jmethodID method, jclass clazz, jobject object, va_list args) {
    if (method->type == Method::Type::Static) {
        assert(clazz != nullptr);
#if JNI_LOGGING
        std::cout << "JNI calling static method " << method->name << " in " << clazz->full_name << std::endl;
#endif
        if constexpr (std::is_void<T>::value) {
            method->static_method(clazz, args);
            return;
        } else {
            return std::get<T>(method->static_method(clazz, args));
        }
    } else {
        assert(method->type == Method::Type::Member);
        assert(object != nullptr);
#if JNI_LOGGING
        std::cout << "JNI calling method " << method->name << " in " << object->super->full_name << std::endl;
#endif
        if constexpr (std::is_void<T>::value) {
            method->method(object, args);
            return;
        } else {
            return std::get<T>(method->method(object, args));
        }
    }
}

template<typename T>
static T field_get_generic(jobject object, jfieldID field) {
#if JNI_LOGGING
    std::cout << "JNI getting field " << *field << " in " << object->super->full_name << std::endl;
#endif
    auto it = object->fields.find(field);
    if (it != object->fields.end()) {
        return std::get<T>(((*it).second).value);
    } else {
        assert(!"Invalid jfieldID");
    }
}

template<typename T>
static T static_field_get_generic(jclass clazz, jfieldID field) {
#if JNI_LOGGING
    std::cout << "JNI getting static field " << *field << " in " << clazz->full_name << std::endl;
#endif
    auto it = clazz->fields.find(*field);
    if (it != clazz->fields.end()) {
        return std::get<T>(((*it).second).value);
    } else {
        assert(!"Invalid jfieldID");
    }
}

#define DEFINE_METHOD_CALL_FUNCTIONS(M_type_name, M_type) \
    M_type CallStatic ## M_type_name ## Method(JNIEnv *, jclass, jmethodID, ...) { assert(!"Not implemented"); } \
    M_type CallStatic ## M_type_name ## MethodA(JNIEnv *, jclass, jmethodID, const jvalue *) { assert(!"Not implemented"); } \
    M_type CallStatic ## M_type_name ## MethodV(JNIEnv *, jclass clazz, jmethodID methodID, va_list args) { \
        return method_call_generic<M_type>(methodID, clazz, nullptr, args); \
    } \
    M_type Call ## M_type_name ## Method(JNIEnv *, jobject, jmethodID, ...) { assert(!"Not implemented"); } \
    M_type Call ## M_type_name ## MethodA(JNIEnv *, jobject, jmethodID, const jvalue *) { assert(!"Not implemented"); } \
    M_type Call ## M_type_name ## MethodV(JNIEnv *, jobject obj, jmethodID methodID, va_list args){ \
        return method_call_generic<M_type>(methodID, nullptr, obj, args); \
    } \
    M_type CallNonvirtual ## M_type_name ## Method(JNIEnv *, jobject, jclass, jmethodID, ...) { assert(!"Not implemented"); } \
    M_type CallNonvirtual ## M_type_name ## MethodA(JNIEnv *, jobject, jclass, jmethodID, const jvalue *) { assert(!"Not implemented"); } \
    M_type CallNonvirtual ## M_type_name ## MethodV(JNIEnv *, jobject obj, jclass clazz, jmethodID methodID, va_list args) { \
        return method_call_generic<M_type>(methodID, clazz, obj, args); \
    } \

DEFINE_METHOD_CALL_FUNCTIONS(Void, void)
DEFINE_METHOD_CALL_FUNCTIONS(Object, jobject)
DEFINE_METHOD_CALL_FUNCTIONS(Boolean, jboolean)
DEFINE_METHOD_CALL_FUNCTIONS(Byte, jbyte)
DEFINE_METHOD_CALL_FUNCTIONS(Char, jchar)
DEFINE_METHOD_CALL_FUNCTIONS(Short, jshort)
DEFINE_METHOD_CALL_FUNCTIONS(Int, jint)
DEFINE_METHOD_CALL_FUNCTIONS(Long, jlong)
DEFINE_METHOD_CALL_FUNCTIONS(Float, jfloat)
DEFINE_METHOD_CALL_FUNCTIONS(Double, jdouble)


#define DEFINE_FIELD_GET_FUNCTIONS(M_type_name, M_type) \
    M_type Get ## M_type_name ## Field(JNIEnv *, jobject object, jfieldID field) { \
        return field_get_generic<M_type>(object, field); \
    } \
    M_type GetStatic ## M_type_name ## Field(JNIEnv *, jclass clazz, jfieldID field) { \
        return static_field_get_generic<M_type>(clazz, field); \
    }

DEFINE_FIELD_GET_FUNCTIONS(Object, jobject)
DEFINE_FIELD_GET_FUNCTIONS(Boolean, jboolean)
DEFINE_FIELD_GET_FUNCTIONS(Byte, jbyte)
DEFINE_FIELD_GET_FUNCTIONS(Char, jchar)
DEFINE_FIELD_GET_FUNCTIONS(Short, jshort)
DEFINE_FIELD_GET_FUNCTIONS(Int, jint)
DEFINE_FIELD_GET_FUNCTIONS(Long, jlong)
DEFINE_FIELD_GET_FUNCTIONS(Float, jfloat)
DEFINE_FIELD_GET_FUNCTIONS(Double, jdouble)

jobject NewObjectV(JNIEnv *, jclass clazz, jmethodID constructor, va_list args) {
    jobject object = new Object(clazz);
    method_call_generic<void>(constructor, clazz, object, args);
    return object;
}

jsize GetArrayLength(JNIEnv *, jarray array) {
    return array->array_size;
}

template<typename T>
ArrayBase<T>* new_array_generic(jsize length) {
    auto array = new ArrayBase<T>();
    array->array_size = length;
    array->array.reserve(length);
    for (jsize i = 0; i < length; i++)
        array->array.push_back(T());
    return array;
}

template<typename T>
static T* get_array_elements_generic(ArrayBase<T> *array, jboolean *is_copy) {
    if (is_copy)
        *is_copy = false;
    return array->array.data();
}

template<typename T>
static void get_array_region_generic(ArrayBase<T> *array, jsize start, jsize len, T *buf) {
    for (jsize i = start; i < len; i++)
        buf[i] = array->array[i];
}

template<typename T>
static void set_array_region_generic(ArrayBase<T> *array, jsize start, jsize len, const T *buf) {
    for (jsize i = start; i < len; i++)
        array->array[i] = buf[i];
}

#define DEFINE_ARRAY_FUNCTIONS(M_type_name, M_type) \
    M_type ## Array New ## M_type_name ## Array(JNIEnv *, jsize length) { \
        return new_array_generic<M_type>(length); \
    } \
    M_type* Get ## M_type_name ## ArrayElements(JNIEnv *, M_type ## Array array, jboolean *isCopy) { \
        return get_array_elements_generic<M_type>(array, isCopy); \
    } \
    void Release ## M_type_name ## ArrayElements(JNIEnv *, M_type ## Array, M_type *, jint) {} \
    void Get ## M_type_name ## ArrayRegion(JNIEnv *, M_type ## Array array, jsize start, jsize len, M_type *buf) { \
        return get_array_region_generic<M_type>(array, start, len, buf); \
    } \
    void Set ## M_type_name ## ArrayRegion(JNIEnv *, M_type ## Array array, jsize start, jsize len, const M_type *buf) { \
        return set_array_region_generic<M_type>(array, start, len, buf); \
    }

DEFINE_ARRAY_FUNCTIONS(Boolean, jboolean)
DEFINE_ARRAY_FUNCTIONS(Byte, jbyte)
DEFINE_ARRAY_FUNCTIONS(Char, jchar)
DEFINE_ARRAY_FUNCTIONS(Short, jshort)
DEFINE_ARRAY_FUNCTIONS(Int, jint)
DEFINE_ARRAY_FUNCTIONS(Long, jlong)
DEFINE_ARRAY_FUNCTIONS(Float, jfloat)
DEFINE_ARRAY_FUNCTIONS(Double, jdouble)

jobjectArray NewObjectArray(JNIEnv *, jsize length, jclass, jobject initial_element) {
    jobjectArray array = new ObjectArray();
    array->array_size = length;
    array->array.reserve(length);
    for (jsize i = 0; i < length; i++)
        array->array.push_back(initial_element);
    return array;
}

jobject GetObjectArrayElement(JNIEnv *, jobjectArray array, jsize index) {
    return array->array[index];
}

void SetObjectArrayElement(JNIEnv *, jobjectArray array, jsize index, jobject value) {
    array->array[index] = value;
}

jint DestroyJavaVM(JavaVM *) {
    assert("UNIMPLEMENTED DestroyJavaVM" == nullptr);
}

jint AttachCurrentAsDaemon(JavaVM *, void **, void *) {
    assert("UNIMPLEMENTED AttachCurrentAsDaemon" == nullptr);
}

jclass DefineClass(JNIEnv *, const char *, jobject, const jbyte *, jsize) {
    assert("UNIMPLEMENTED DefineClass" == nullptr);
}

jmethodID FromReflectedMethod(JNIEnv *, jobject) {
    assert("UNIMPLEMENTED FromReflectedMethod" == nullptr);
}
jfieldID FromReflectedField(JNIEnv *, jobject) {
    assert("UNIMPLEMENTED FromReflectedField" == nullptr);
}

jobject ToReflectedMethod(JNIEnv *, jclass, jmethodID, jboolean) {
    assert("UNIMPLEMENTED ToReflectedMethod" == nullptr);
}

jclass GetSuperclass(JNIEnv *, jclass) {
    assert("UNIMPLEMENTED GetSuperclass" == nullptr);
}
jboolean IsAssignableFrom(JNIEnv *, jclass, jclass) {
    assert("UNIMPLEMENTED IsAssignableFrom" == nullptr);
}

jobject ToReflectedField(JNIEnv *, jclass, jfieldID, jboolean) {
    assert("UNIMPLEMENTED ToReflectedField" == nullptr);
}

jint Throw(JNIEnv *, jthrowable) { assert("UNIMPLEMENTED Throw" == nullptr); }
jint ThrowNew(JNIEnv *, jclass, const char *) {
    assert("UNIMPLEMENTED ThrowNew" == nullptr);
}
void FatalError(JNIEnv *, const char *) {
    assert("UNIMPLEMENTED FatalError" == nullptr);
}

jint PushLocalFrame(JNIEnv *, jint) {
    assert("UNIMPLEMENTED PushLocalFrame" == nullptr);
}
jobject PopLocalFrame(JNIEnv *, jobject) {
    assert("UNIMPLEMENTED PopLocalFrame" == nullptr);
}

jboolean IsSameObject(JNIEnv *, jobject, jobject) {
    assert("UNIMPLEMENTED IsSameObject" == nullptr);
}

jobject NewLocalRef(JNIEnv *, jobject) {
    assert("UNIMPLEMENTED NewLocalRef" == nullptr);
}
jint EnsureLocalCapacity(JNIEnv *, jint) {
    assert("UNIMPLEMENTED EnsureLocalCapacity" == nullptr);
}

jobject AllocObject(JNIEnv *, jclass) {
    assert("UNIMPLEMENTED AllocObject" == nullptr);
}
jobject NewObject(JNIEnv *, jclass, jmethodID, ...) {
    assert("UNIMPLEMENTED NewObject" == nullptr);
}
jobject NewObjectA(JNIEnv *, jclass, jmethodID, const jvalue *) {
    assert("UNIMPLEMENTED NewObjectA" == nullptr);
}

jboolean IsInstanceOf(JNIEnv *, jobject, jclass) {
    assert("UNIMPLEMENTED IsInstanceOf" == nullptr);
}


void SetObjectField(JNIEnv *, jobject, jfieldID, jobject) {
    assert("UNIMPLEMENTED SetObjectField" == nullptr);
}
void SetBooleanField(JNIEnv *, jobject, jfieldID, jboolean) {
    assert("UNIMPLEMENTED SetBooleanField" == nullptr);
}
void SetByteField(JNIEnv *, jobject, jfieldID, jbyte) {
    assert("UNIMPLEMENTED SetByteField" == nullptr);
}
void SetCharField(JNIEnv *, jobject, jfieldID, jchar) {
    assert("UNIMPLEMENTED SetCharField" == nullptr);
}
void SetShortField(JNIEnv *, jobject, jfieldID, jshort) {
    assert("UNIMPLEMENTED SetShortField" == nullptr);
}
void SetIntField(JNIEnv *, jobject, jfieldID, jint) {
    assert("UNIMPLEMENTED SetIntField" == nullptr);
}
void SetLongField(JNIEnv *, jobject, jfieldID, jlong) {
    assert("UNIMPLEMENTED SetLongField" == nullptr);
}
void SetFloatField(JNIEnv *, jobject, jfieldID, jfloat) {
    assert("UNIMPLEMENTED SetFloatField" == nullptr);
}
void SetDoubleField(JNIEnv *, jobject, jfieldID, jdouble) {
    assert("UNIMPLEMENTED SetDoubleField" == nullptr);
}


void SetStaticObjectField(JNIEnv *, jclass, jfieldID, jobject) {
    assert("UNIMPLEMENTED SetStaticObjectField" == nullptr);
}
void SetStaticBooleanField(JNIEnv *, jclass, jfieldID, jboolean) {
    assert("UNIMPLEMENTED SetStaticBooleanField" == nullptr);
}
void SetStaticByteField(JNIEnv *, jclass, jfieldID, jbyte) {
    assert("UNIMPLEMENTED SetStaticByteField" == nullptr);
}
void SetStaticCharField(JNIEnv *, jclass, jfieldID, jchar) {
    assert("UNIMPLEMENTED SetStaticCharField" == nullptr);
}
void SetStaticShortField(JNIEnv *, jclass, jfieldID, jshort) {
    assert("UNIMPLEMENTED SetStaticShortField" == nullptr);
}
void SetStaticIntField(JNIEnv *, jclass, jfieldID, jint) {
    assert("UNIMPLEMENTED SetStaticIntField" == nullptr);
}
void SetStaticLongField(JNIEnv *, jclass, jfieldID, jlong) {
    assert("UNIMPLEMENTED SetStaticLongField" == nullptr);
}
void SetStaticFloatField(JNIEnv *, jclass, jfieldID, jfloat) {
    assert("UNIMPLEMENTED SetStaticFloatField" == nullptr);
}
void SetStaticDoubleField(JNIEnv *, jclass, jfieldID, jdouble) {
    assert("UNIMPLEMENTED SetStaticDoubleField" == nullptr);
}

jstring NewString(JNIEnv *, const jchar *, jsize) {
    assert("UNIMPLEMENTED NewString" == nullptr);
}
jsize GetStringLength(JNIEnv *, jstring) {
    assert("UNIMPLEMENTED GetStringLength" == nullptr);
}
const jchar *GetStringChars(JNIEnv *, jstring, jboolean *) {
    assert("UNIMPLEMENTED jchar" == nullptr);
}
void ReleaseStringChars(JNIEnv *, jstring, const jchar *) {
    assert("UNIMPLEMENTED ReleaseStringChars" == nullptr);
}

jint RegisterNatives(JNIEnv *, jclass, const JNINativeMethod *, jint) {
    assert("UNIMPLEMENTED RegisterNatives" == nullptr);
}
jint UnregisterNatives(JNIEnv *, jclass) {
    assert("UNIMPLEMENTED UnregisterNatives" == nullptr);
}
jint MonitorEnter(JNIEnv *, jobject) {
    assert("UNIMPLEMENTED MonitorEnter" == nullptr);
}
jint MonitorExit(JNIEnv *, jobject) {
    assert("UNIMPLEMENTED MonitorExit" == nullptr);
}
jint GetJavaVM(JNIEnv *, JavaVM **) {
    assert("UNIMPLEMENTED GetJavaVM" == nullptr);
}

void GetStringRegion(JNIEnv *, jstring, jsize, jsize, jchar *) {
    assert("UNIMPLEMENTED GetStringRegion" == nullptr);
}
void GetStringUTFRegion(JNIEnv *, jstring, jsize, jsize, char *) {
    assert("UNIMPLEMENTED GetStringUTFRegion" == nullptr);
}

void *GetPrimitiveArrayCritical(JNIEnv *, jarray, jboolean *) {
    assert("UNIMPLEMENTED void" == nullptr);
}
void ReleasePrimitiveArrayCritical(JNIEnv *, jarray, void *, jint) {
    assert("UNIMPLEMENTED ReleasePrimitiveArrayCritical" == nullptr);
}

const jchar *GetStringCritical(JNIEnv *, jstring, jboolean *) {
    assert("UNIMPLEMENTED jchar" == nullptr);
}
void ReleaseStringCritical(JNIEnv *, jstring, const jchar *) {
    assert("UNIMPLEMENTED ReleaseStringCritical" == nullptr);
}

jobject NewDirectByteBuffer(JNIEnv *, void *, jlong) {
    assert("UNIMPLEMENTED NewDirectByteBuffer" == nullptr);
}
void *GetDirectBufferAddress(JNIEnv *, jobject) {
    assert("UNIMPLEMENTED void" == nullptr);
}
jlong GetDirectBufferCapacity(JNIEnv *, jobject) {
    assert("UNIMPLEMENTED GetDirectBufferCapacity" == nullptr);
}

// jobjectRefType GetObjectRefType(JNIEnv*, jobject){assert("UNIMPLEMENTED
// GetObjectRefType" == nullptr);}

} // namespace JNI