#pragma once

#include "types.hpp"
#include "function_traits.hpp"
#include <cstdarg>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <functional>

#define JNI_LOGGING 0

namespace JNI {

constexpr int JNI_OK = 0;
constexpr int JNI_ERR = -1;
constexpr int JNI_EDETACHED = -2;
constexpr int JNI_EVERSION = -3;
constexpr int JNI_ENOMEM = -4;
constexpr int JNI_EEXIST = -5;
constexpr int JNI_EINVAL = -6;

using jboolean = u8;
using jbyte = i8;
using jchar = u16;
using jshort = i16;
using jint = i32;
using jlong = i64;
using jfloat = float;
using jdouble = double;
using jsize = jint;
using jobject = struct Object *;

union jvalue {
    jboolean z;
    jbyte b;
    jchar c;
    jshort s;
    jint i;
    jlong j;
    jfloat f;
    jdouble d;
    jobject l;
};

using ValueVariant = std::variant<jboolean, jbyte, jchar, jshort, jint, jlong, jfloat, jdouble, jobject>;
using jfieldID = const std::string *; // TODO: hack just to get this working for now


using method_function = std::function<ValueVariant(struct Object *, va_list)>;
using static_method_function = std::function<ValueVariant(struct Class *, va_list)>;

struct JavaVM *java_vm_new();
struct Class *create_class(struct JavaVM *vm, const char *name);
struct String *create_string(const char *data);
struct String *create_string(const std::string &str);
struct JNIEnv *get_jni_env();

struct Method {
    enum class Type {
        Static,
        Member
    };

    Type type;
    const char *name;
    Class *owner_class;
    
    method_function method;
    static_method_function static_method;

    Method(const char *name, Class *owner_class, const method_function &method)
        : type(Type::Member), name(name), owner_class(owner_class), method(method) {}

    Method(const char *name, Class *owner_class, const static_method_function &static_method)
        : type(Type::Static), name(name), owner_class(owner_class), static_method(static_method) {}

    Method(const Method &other) : type(other.type), name(other.name), owner_class(other.owner_class) {
        if (type == Type::Member)
            method = other.method;
        else
            static_method = other.static_method;
    }

    ~Method() {}
};

struct Field {
    Field(ValueVariant value) : value(value) {}

    ValueVariant value;
};

struct Object {
    struct Class *super = nullptr;
    std::unordered_map<jfieldID, Field> fields;

    Object(Class *super = nullptr) : super(super) {}
};

struct Class : public Object {
    std::string full_name;
    std::unordered_map<std::string, Field> fields;
    std::unordered_map<std::string, Method> methods;

    Class(const char *name) : full_name(name) {}

    Method *static_method(const char *name, const static_method_function &func);
    Method *method(const char *name, const method_function &func);
    jfieldID static_field(const char *name, ValueVariant value);
    jfieldID field(const char *name);
};

extern Class *string_class;
extern Class *list_class;

struct Throwable : public Object {};

struct JNINativeMethod {};

struct String : public Object {
    std::string string;

    String() : Object(string_class) {}
};

struct Array : public Class {
    jsize array_size = 0;

    Array() : Class("<array>") {}
};

template<typename T>
struct ArrayBase : public Array {
    std::vector<T> array;
};

using BooleanArray = ArrayBase<jboolean>;
using ByteArray = ArrayBase<jbyte>;
using CharArray = ArrayBase<jchar>;
using ShortArray = ArrayBase<jshort>;
using IntArray = ArrayBase<jint>;
using LongArray = ArrayBase<jlong>;
using FloatArray = ArrayBase<jfloat>;
using DoubleArray = ArrayBase<jdouble>;
using ObjectArray = ArrayBase<jobject>;


using JNINativeInterface = void **;
using JNIInvocationInterface = void **;

struct JNIEnv {
    JNINativeInterface native_interface_functions = nullptr;
    struct JavaVM *vm = nullptr;
    bool has_exception = false; // TODO: hack
};

struct JavaVM {
    JNIInvocationInterface invocation = nullptr;
    JNINativeInterface native_interface_functions = nullptr;
    std::unordered_map<std::string, Class> name_to_class;
};

struct JNIThreadData {
    bool initialized = false;
    JNIEnv env;
};

extern thread_local JNIThreadData thread_data;

using jmethodID = Method *;
using jclass = Class *;
using jthrowable = Throwable *;
using jweak = jobject;

using jstring = String *;
using jarray = Array *;
using jbooleanArray = BooleanArray *;
using jbyteArray = ByteArray *;
using jcharArray = CharArray *;
using jshortArray = ShortArray *;
using jintArray = IntArray *;
using jlongArray = LongArray *;
using jfloatArray = FloatArray *;
using jdoubleArray = DoubleArray *;
using jobjectArray = ObjectArray *;


/* Invocation interface */
/* Read "5. The Invocation API" */
jint GetEnv(JavaVM *, void **, jint);
jint DestroyJavaVM(JavaVM *);
jint AttachCurrentThread(JavaVM *, void **, void *);
jint DetachCurrentThread(JavaVM *);
jint AttachCurrentAsDaemon(JavaVM *, void **, void *);

/* JNIEnv functions */
jint GetVersion(JNIEnv *);

jclass DefineClass(JNIEnv *, const char *, jobject, const jbyte *, jsize);
jclass FindClass(JNIEnv *, const char *);

jmethodID FromReflectedMethod(JNIEnv *, jobject);
jfieldID FromReflectedField(JNIEnv *, jobject);
/* spec doesn't show jboolean parameter */
jobject ToReflectedMethod(JNIEnv *, jclass, jmethodID, jboolean);

jclass GetSuperclass(JNIEnv *, jclass);
jboolean IsAssignableFrom(JNIEnv *, jclass, jclass);

/* spec doesn't show jboolean parameter */
jobject ToReflectedField(JNIEnv *, jclass, jfieldID, jboolean);

jint Throw(JNIEnv *, jthrowable);
jint ThrowNew(JNIEnv *, jclass, const char *);
jthrowable ExceptionOccurred(JNIEnv *);
void ExceptionDescribe(JNIEnv *);
void ExceptionClear(JNIEnv *);
void FatalError(JNIEnv *, const char *);

jint PushLocalFrame(JNIEnv *, jint);
jobject PopLocalFrame(JNIEnv *, jobject);

jobject NewGlobalRef(JNIEnv *, jobject);
void DeleteGlobalRef(JNIEnv *, jobject);
void DeleteLocalRef(JNIEnv *, jobject);
jboolean IsSameObject(JNIEnv *, jobject, jobject);

jobject NewLocalRef(JNIEnv *, jobject);
jint EnsureLocalCapacity(JNIEnv *, jint);

jobject AllocObject(JNIEnv *, jclass);
jobject NewObject(JNIEnv *, jclass, jmethodID, ...);
jobject NewObjectV(JNIEnv *, jclass, jmethodID, va_list);
jobject NewObjectA(JNIEnv *, jclass, jmethodID, const jvalue *);

jclass GetObjectClass(JNIEnv *, jobject);
jboolean IsInstanceOf(JNIEnv *, jobject, jclass);
jmethodID GetMethodID(JNIEnv *, jclass, const char *, const char *);

jobject CallObjectMethod(JNIEnv *, jobject, jmethodID, ...);
jobject CallObjectMethodV(JNIEnv *, jobject, jmethodID, va_list);
jobject CallObjectMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
jboolean CallBooleanMethod(JNIEnv *, jobject, jmethodID, ...);
jboolean CallBooleanMethodV(JNIEnv *, jobject, jmethodID, va_list);
jboolean CallBooleanMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
jbyte CallByteMethod(JNIEnv *, jobject, jmethodID, ...);
jbyte CallByteMethodV(JNIEnv *, jobject, jmethodID, va_list);
jbyte CallByteMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
jchar CallCharMethod(JNIEnv *, jobject, jmethodID, ...);
jchar CallCharMethodV(JNIEnv *, jobject, jmethodID, va_list);
jchar CallCharMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
jshort CallShortMethod(JNIEnv *, jobject, jmethodID, ...);
jshort CallShortMethodV(JNIEnv *, jobject, jmethodID, va_list);
jshort CallShortMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
jint CallIntMethod(JNIEnv *, jobject, jmethodID, ...);
jint CallIntMethodV(JNIEnv *, jobject, jmethodID, va_list);
jint CallIntMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
jlong CallLongMethod(JNIEnv *, jobject, jmethodID, ...);
jlong CallLongMethodV(JNIEnv *, jobject, jmethodID, va_list);
jlong CallLongMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
jfloat CallFloatMethod(JNIEnv *, jobject, jmethodID, ...);
jfloat CallFloatMethodV(JNIEnv *, jobject, jmethodID, va_list);
jfloat CallFloatMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
jdouble CallDoubleMethod(JNIEnv *, jobject, jmethodID, ...);
jdouble CallDoubleMethodV(JNIEnv *, jobject, jmethodID, va_list);
jdouble CallDoubleMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);
void CallVoidMethod(JNIEnv *, jobject, jmethodID, ...);
void CallVoidMethodV(JNIEnv *, jobject, jmethodID, va_list);
void CallVoidMethodA(JNIEnv *, jobject, jmethodID, const jvalue *);

jobject CallNonvirtualObjectMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jobject CallNonvirtualObjectMethodV(JNIEnv *, jobject, jclass, jmethodID,
                                    va_list);
jobject CallNonvirtualObjectMethodA(JNIEnv *, jobject, jclass, jmethodID,
                                    const jvalue *);
jboolean CallNonvirtualBooleanMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jboolean CallNonvirtualBooleanMethodV(JNIEnv *, jobject, jclass, jmethodID,
                                      va_list);
jboolean CallNonvirtualBooleanMethodA(JNIEnv *, jobject, jclass, jmethodID,
                                      const jvalue *);
jbyte CallNonvirtualByteMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jbyte CallNonvirtualByteMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list);
jbyte CallNonvirtualByteMethodA(JNIEnv *, jobject, jclass, jmethodID,
                                const jvalue *);
jchar CallNonvirtualCharMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jchar CallNonvirtualCharMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list);
jchar CallNonvirtualCharMethodA(JNIEnv *, jobject, jclass, jmethodID,
                                const jvalue *);
jshort CallNonvirtualShortMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jshort CallNonvirtualShortMethodV(JNIEnv *, jobject, jclass, jmethodID,
                                  va_list);
jshort CallNonvirtualShortMethodA(JNIEnv *, jobject, jclass, jmethodID,
                                  const jvalue *);
jint CallNonvirtualIntMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jint CallNonvirtualIntMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list);
jint CallNonvirtualIntMethodA(JNIEnv *, jobject, jclass, jmethodID,
                              const jvalue *);
jlong CallNonvirtualLongMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jlong CallNonvirtualLongMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list);
jlong CallNonvirtualLongMethodA(JNIEnv *, jobject, jclass, jmethodID,
                                const jvalue *);
jfloat CallNonvirtualFloatMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jfloat CallNonvirtualFloatMethodV(JNIEnv *, jobject, jclass, jmethodID,
                                  va_list);
jfloat CallNonvirtualFloatMethodA(JNIEnv *, jobject, jclass, jmethodID,
                                  const jvalue *);
jdouble CallNonvirtualDoubleMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
jdouble CallNonvirtualDoubleMethodV(JNIEnv *, jobject, jclass, jmethodID,
                                    va_list);
jdouble CallNonvirtualDoubleMethodA(JNIEnv *, jobject, jclass, jmethodID,
                                    const jvalue *);
void CallNonvirtualVoidMethod(JNIEnv *, jobject, jclass, jmethodID, ...);
void CallNonvirtualVoidMethodV(JNIEnv *, jobject, jclass, jmethodID, va_list);
void CallNonvirtualVoidMethodA(JNIEnv *, jobject, jclass, jmethodID,
                               const jvalue *);

jfieldID GetFieldID(JNIEnv *, jclass, const char *, const char *);

jobject GetObjectField(JNIEnv *, jobject, jfieldID);
jboolean GetBooleanField(JNIEnv *, jobject, jfieldID);
jbyte GetByteField(JNIEnv *, jobject, jfieldID);
jchar GetCharField(JNIEnv *, jobject, jfieldID);
jshort GetShortField(JNIEnv *, jobject, jfieldID);
jint GetIntField(JNIEnv *, jobject, jfieldID);
jlong GetLongField(JNIEnv *, jobject, jfieldID);
jfloat GetFloatField(JNIEnv *, jobject, jfieldID);
jdouble GetDoubleField(JNIEnv *, jobject, jfieldID);

void SetObjectField(JNIEnv *, jobject, jfieldID, jobject);
void SetBooleanField(JNIEnv *, jobject, jfieldID, jboolean);
void SetByteField(JNIEnv *, jobject, jfieldID, jbyte);
void SetCharField(JNIEnv *, jobject, jfieldID, jchar);
void SetShortField(JNIEnv *, jobject, jfieldID, jshort);
void SetIntField(JNIEnv *, jobject, jfieldID, jint);
void SetLongField(JNIEnv *, jobject, jfieldID, jlong);
void SetFloatField(JNIEnv *, jobject, jfieldID, jfloat);
void SetDoubleField(JNIEnv *, jobject, jfieldID, jdouble);

jmethodID GetStaticMethodID(JNIEnv *, jclass, const char *, const char *);

jobject CallStaticObjectMethod(JNIEnv *, jclass, jmethodID, ...);
jobject CallStaticObjectMethodV(JNIEnv *, jclass, jmethodID, va_list);
jobject CallStaticObjectMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
jboolean CallStaticBooleanMethod(JNIEnv *, jclass, jmethodID, ...);
jboolean CallStaticBooleanMethodV(JNIEnv *, jclass, jmethodID, va_list);
jboolean CallStaticBooleanMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
jbyte CallStaticByteMethod(JNIEnv *, jclass, jmethodID, ...);
jbyte CallStaticByteMethodV(JNIEnv *, jclass, jmethodID, va_list);
jbyte CallStaticByteMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
jchar CallStaticCharMethod(JNIEnv *, jclass, jmethodID, ...);
jchar CallStaticCharMethodV(JNIEnv *, jclass, jmethodID, va_list);
jchar CallStaticCharMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
jshort CallStaticShortMethod(JNIEnv *, jclass, jmethodID, ...);
jshort CallStaticShortMethodV(JNIEnv *, jclass, jmethodID, va_list);
jshort CallStaticShortMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
jint CallStaticIntMethod(JNIEnv *, jclass, jmethodID, ...);
jint CallStaticIntMethodV(JNIEnv *, jclass, jmethodID, va_list);
jint CallStaticIntMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
jlong CallStaticLongMethod(JNIEnv *, jclass, jmethodID, ...);
jlong CallStaticLongMethodV(JNIEnv *, jclass, jmethodID, va_list);
jlong CallStaticLongMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
jfloat CallStaticFloatMethod(JNIEnv *, jclass, jmethodID, ...);
jfloat CallStaticFloatMethodV(JNIEnv *, jclass, jmethodID, va_list);
jfloat CallStaticFloatMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
jdouble CallStaticDoubleMethod(JNIEnv *, jclass, jmethodID, ...);
jdouble CallStaticDoubleMethodV(JNIEnv *, jclass, jmethodID, va_list);
jdouble CallStaticDoubleMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);
void CallStaticVoidMethod(JNIEnv *, jclass, jmethodID, ...);
void CallStaticVoidMethodV(JNIEnv *, jclass, jmethodID, va_list);
void CallStaticVoidMethodA(JNIEnv *, jclass, jmethodID, const jvalue *);

jfieldID GetStaticFieldID(JNIEnv *, jclass, const char *, const char *);

jobject GetStaticObjectField(JNIEnv *, jclass, jfieldID);
jboolean GetStaticBooleanField(JNIEnv *, jclass, jfieldID);
jbyte GetStaticByteField(JNIEnv *, jclass, jfieldID);
jchar GetStaticCharField(JNIEnv *, jclass, jfieldID);
jshort GetStaticShortField(JNIEnv *, jclass, jfieldID);
jint GetStaticIntField(JNIEnv *, jclass, jfieldID);
jlong GetStaticLongField(JNIEnv *, jclass, jfieldID);
jfloat GetStaticFloatField(JNIEnv *, jclass, jfieldID);
jdouble GetStaticDoubleField(JNIEnv *, jclass, jfieldID);

void SetStaticObjectField(JNIEnv *, jclass, jfieldID, jobject);
void SetStaticBooleanField(JNIEnv *, jclass, jfieldID, jboolean);
void SetStaticByteField(JNIEnv *, jclass, jfieldID, jbyte);
void SetStaticCharField(JNIEnv *, jclass, jfieldID, jchar);
void SetStaticShortField(JNIEnv *, jclass, jfieldID, jshort);
void SetStaticIntField(JNIEnv *, jclass, jfieldID, jint);
void SetStaticLongField(JNIEnv *, jclass, jfieldID, jlong);
void SetStaticFloatField(JNIEnv *, jclass, jfieldID, jfloat);
void SetStaticDoubleField(JNIEnv *, jclass, jfieldID, jdouble);

jstring NewString(JNIEnv *, const jchar *, jsize);
jsize GetStringLength(JNIEnv *, jstring);
const jchar *GetStringChars(JNIEnv *, jstring, jboolean *);
void ReleaseStringChars(JNIEnv *, jstring, const jchar *);
jstring NewStringUTF(JNIEnv *, const char *);
jsize GetStringUTFLength(JNIEnv *, jstring);
/* JNI spec says this returns const jbyte*, but that's inconsistent */
const char *GetStringUTFChars(JNIEnv *, jstring, jboolean *);
void ReleaseStringUTFChars(JNIEnv *, jstring, char *);
jsize GetArrayLength(JNIEnv *, jarray);
jobjectArray NewObjectArray(JNIEnv *, jsize, jclass, jobject);
jobject GetObjectArrayElement(JNIEnv *, jobjectArray, jsize);
void SetObjectArrayElement(JNIEnv *, jobjectArray, jsize, jobject);

jbooleanArray NewBooleanArray(JNIEnv *, jsize);
jbyteArray NewByteArray(JNIEnv *, jsize);
jcharArray NewCharArray(JNIEnv *, jsize);
jshortArray NewShortArray(JNIEnv *, jsize);
jintArray NewIntArray(JNIEnv *, jsize);
jlongArray NewLongArray(JNIEnv *, jsize);
jfloatArray NewFloatArray(JNIEnv *, jsize);
jdoubleArray NewDoubleArray(JNIEnv *, jsize);

jboolean *GetBooleanArrayElements(JNIEnv *, jbooleanArray, jboolean *);
jbyte *GetByteArrayElements(JNIEnv *, jbyteArray, jboolean *);
jchar *GetCharArrayElements(JNIEnv *, jcharArray, jboolean *);
jshort *GetShortArrayElements(JNIEnv *, jshortArray, jboolean *);
jint *GetIntArrayElements(JNIEnv *, jintArray, jboolean *);
jlong *GetLongArrayElements(JNIEnv *, jlongArray, jboolean *);
jfloat *GetFloatArrayElements(JNIEnv *, jfloatArray, jboolean *);
jdouble *GetDoubleArrayElements(JNIEnv *, jdoubleArray, jboolean *);

void ReleaseBooleanArrayElements(JNIEnv *, jbooleanArray, jboolean *, jint);
void ReleaseByteArrayElements(JNIEnv *, jbyteArray, jbyte *, jint);
void ReleaseCharArrayElements(JNIEnv *, jcharArray, jchar *, jint);
void ReleaseShortArrayElements(JNIEnv *, jshortArray, jshort *, jint);
void ReleaseIntArrayElements(JNIEnv *, jintArray, jint *, jint);
void ReleaseLongArrayElements(JNIEnv *, jlongArray, jlong *, jint);
void ReleaseFloatArrayElements(JNIEnv *, jfloatArray, jfloat *, jint);
void ReleaseDoubleArrayElements(JNIEnv *, jdoubleArray, jdouble *, jint);

void GetBooleanArrayRegion(JNIEnv *, jbooleanArray, jsize, jsize, jboolean *);
void GetByteArrayRegion(JNIEnv *, jbyteArray, jsize, jsize, jbyte *);
void GetCharArrayRegion(JNIEnv *, jcharArray, jsize, jsize, jchar *);
void GetShortArrayRegion(JNIEnv *, jshortArray, jsize, jsize, jshort *);
void GetIntArrayRegion(JNIEnv *, jintArray, jsize, jsize, jint *);
void GetLongArrayRegion(JNIEnv *, jlongArray, jsize, jsize, jlong *);
void GetFloatArrayRegion(JNIEnv *, jfloatArray, jsize, jsize, jfloat *);
void GetDoubleArrayRegion(JNIEnv *, jdoubleArray, jsize, jsize, jdouble *);

void SetBooleanArrayRegion(JNIEnv *, jbooleanArray, jsize, jsize,
                           const jboolean *);
void SetByteArrayRegion(JNIEnv *, jbyteArray, jsize, jsize, const jbyte *);
void SetCharArrayRegion(JNIEnv *, jcharArray, jsize, jsize, const jchar *);
void SetShortArrayRegion(JNIEnv *, jshortArray, jsize, jsize, const jshort *);
void SetIntArrayRegion(JNIEnv *, jintArray, jsize, jsize, const jint *);
void SetLongArrayRegion(JNIEnv *, jlongArray, jsize, jsize, const jlong *);
void SetFloatArrayRegion(JNIEnv *, jfloatArray, jsize, jsize, const jfloat *);
void SetDoubleArrayRegion(JNIEnv *, jdoubleArray, jsize, jsize,
                          const jdouble *);

jint RegisterNatives(JNIEnv *, jclass, const JNINativeMethod *, jint);
jint UnregisterNatives(JNIEnv *, jclass);
jint MonitorEnter(JNIEnv *, jobject);
jint MonitorExit(JNIEnv *, jobject);
jint GetJavaVM(JNIEnv *, JavaVM **);

void GetStringRegion(JNIEnv *, jstring, jsize, jsize, jchar *);
void GetStringUTFRegion(JNIEnv *, jstring, jsize, jsize, char *);

void *GetPrimitiveArrayCritical(JNIEnv *, jarray, jboolean *);
void ReleasePrimitiveArrayCritical(JNIEnv *, jarray, void *, jint);

const jchar *GetStringCritical(JNIEnv *, jstring, jboolean *);
void ReleaseStringCritical(JNIEnv *, jstring, const jchar *);

jweak NewWeakGlobalRef(JNIEnv *, jobject);
void DeleteWeakGlobalRef(JNIEnv *, jweak);

jboolean ExceptionCheck(JNIEnv *);

jobject NewDirectByteBuffer(JNIEnv *, void *, jlong);
void *GetDirectBufferAddress(JNIEnv *, jobject);
jlong GetDirectBufferCapacity(JNIEnv *, jobject);

/* added in JNI 1.6 */
// jobjectRefType  GetObjectRefType(JNIEnv*, jobject);

static const std::vector<dynamic_symbol> jni_env_syms = {
    {"unused0", nullptr},
    {"unused1", nullptr},
    {"unused2", nullptr},
    {"unused3", nullptr},

    {"GetVersion", GetVersion},

    {"DefineClass", DefineClass},
    {"FindClass", FindClass},

    {"FromReflectedMethod", FromReflectedMethod},
    {"FromReflectedField", FromReflectedField},
    {"ToReflectedMethod", ToReflectedMethod},

    {"GetSuperclass", GetSuperclass},
    {"IsAssignableFrom", IsAssignableFrom},

    {"ToReflectedField", ToReflectedField},

    {"Throw", Throw},
    {"ThrowNew", ThrowNew},
    {"ExceptionOccurred", ExceptionOccurred},
    {"ExceptionDescribe", ExceptionDescribe},
    {"ExceptionClear", ExceptionClear},
    {"FatalError", FatalError},

    {"PushLocalFrame", PushLocalFrame},
    {"PopLocalFrame", PopLocalFrame},

    {"NewGlobalRef", NewGlobalRef},
    {"DeleteGlobalRef", DeleteGlobalRef},
    {"DeleteLocalRef", DeleteLocalRef},
    {"IsSameObject", IsSameObject},
    {"NewLocalRef", NewLocalRef},
    {"EnsureLocalCapacity", EnsureLocalCapacity},

    {"AllocObject", AllocObject},
    {"NewObject", NewObject},
    {"NewObjectV", NewObjectV},
    {"NewObjectA", NewObjectA},

    {"GetObjectClass", GetObjectClass},
    {"IsInstanceOf", IsInstanceOf},

    {"GetMethodID", GetMethodID},

    {"CallObjectMethod", CallObjectMethod},
    {"CallObjectMethodV", CallObjectMethodV},
    {"CallObjectMethodA", CallObjectMethodA},
    {"CallBooleanMethod", CallBooleanMethod},
    {"CallBooleanMethodV", CallBooleanMethodV},
    {"CallBooleanMethodA", CallBooleanMethodA},
    {"CallByteMethod", CallByteMethod},
    {"CallByteMethodV", CallByteMethodV},
    {"CallByteMethodA", CallByteMethodA},
    {"CallCharMethod", CallCharMethod},
    {"CallCharMethodV", CallCharMethodV},
    {"CallCharMethodA", CallCharMethodA},
    {"CallShortMethod", CallShortMethod},
    {"CallShortMethodV", CallShortMethodV},
    {"CallShortMethodA", CallShortMethodA},
    {"CallIntMethod", CallIntMethod},
    {"CallIntMethodV", CallIntMethodV},
    {"CallIntMethodA", CallIntMethodA},
    {"CallLongMethod", CallLongMethod},
    {"CallLongMethodV", CallLongMethodV},
    {"CallLongMethodA", CallLongMethodA},
    {"CallFloatMethod", CallFloatMethod},
    {"CallFloatMethodV", CallFloatMethodV},
    {"CallFloatMethodA", CallFloatMethodA},
    {"CallDoubleMethod", CallDoubleMethod},
    {"CallDoubleMethodV", CallDoubleMethodV},
    {"CallDoubleMethodA", CallDoubleMethodA},
    {"CallVoidMethod", CallVoidMethod},
    {"CallVoidMethodV", CallVoidMethodV},
    {"CallVoidMethodA", CallVoidMethodA},

    {"CallNonvirtualObjectMethod", CallNonvirtualObjectMethod},
    {"CallNonvirtualObjectMethodV", CallNonvirtualObjectMethodV},
    {"CallNonvirtualObjectMethodA", CallNonvirtualObjectMethodA},
    {"CallNonvirtualBooleanMethod", CallNonvirtualBooleanMethod},
    {"CallNonvirtualBooleanMethodV", CallNonvirtualBooleanMethodV},
    {"CallNonvirtualBooleanMethodA", CallNonvirtualBooleanMethodA},
    {"CallNonvirtualByteMethod", CallNonvirtualByteMethod},
    {"CallNonvirtualByteMethodV", CallNonvirtualByteMethodV},
    {"CallNonvirtualByteMethodA", CallNonvirtualByteMethodA},
    {"CallNonvirtualCharMethod", CallNonvirtualCharMethod},
    {"CallNonvirtualCharMethodV", CallNonvirtualCharMethodV},
    {"CallNonvirtualCharMethodA", CallNonvirtualCharMethodA},
    {"CallNonvirtualShortMethod", CallNonvirtualShortMethod},
    {"CallNonvirtualShortMethodV", CallNonvirtualShortMethodV},
    {"CallNonvirtualShortMethodA", CallNonvirtualShortMethodA},
    {"CallNonvirtualIntMethod", CallNonvirtualIntMethod},
    {"CallNonvirtualIntMethodV", CallNonvirtualIntMethodV},
    {"CallNonvirtualIntMethodA", CallNonvirtualIntMethodA},
    {"CallNonvirtualLongMethod", CallNonvirtualLongMethod},
    {"CallNonvirtualLongMethodV", CallNonvirtualLongMethodV},
    {"CallNonvirtualLongMethodA", CallNonvirtualLongMethodA},
    {"CallNonvirtualFloatMethod", CallNonvirtualFloatMethod},
    {"CallNonvirtualFloatMethodV", CallNonvirtualFloatMethodV},
    {"CallNonvirtualFloatMethodA", CallNonvirtualFloatMethodA},
    {"CallNonvirtualDoubleMethod", CallNonvirtualDoubleMethod},
    {"CallNonvirtualDoubleMethodV", CallNonvirtualDoubleMethodV},
    {"CallNonvirtualDoubleMethodA", CallNonvirtualDoubleMethodA},
    {"CallNonvirtualVoidMethod", CallNonvirtualVoidMethod},
    {"CallNonvirtualVoidMethodV", CallNonvirtualVoidMethodV},
    {"CallNonvirtualVoidMethodA", CallNonvirtualVoidMethodA},

    {"GetFieldID", GetFieldID},

    {"GetObjectField", GetObjectField},
    {"GetBooleanField", GetBooleanField},
    {"GetByteField", GetByteField},
    {"GetCharField", GetCharField},
    {"GetShortField", GetShortField},
    {"GetIntField", GetIntField},
    {"GetLongField", GetLongField},
    {"GetFloatField", GetFloatField},
    {"GetDoubleField", GetDoubleField},
    {"SetObjectField", SetObjectField},
    {"SetBooleanField", SetBooleanField},
    {"SetByteField", SetByteField},
    {"SetCharField", SetCharField},
    {"SetShortField", SetShortField},
    {"SetIntField", SetIntField},
    {"SetLongField", SetLongField},
    {"SetFloatField", SetFloatField},
    {"SetDoubleField", SetDoubleField},

    {"GetStaticMethodID", GetStaticMethodID},

    {"CallStaticObjectMethod", CallStaticObjectMethod},
    {"CallStaticObjectMethodV", CallStaticObjectMethodV},
    {"CallStaticObjectMethodA", CallStaticObjectMethodA},
    {"CallStaticBooleanMethod", CallStaticBooleanMethod},
    {"CallStaticBooleanMethodV", CallStaticBooleanMethodV},
    {"CallStaticBooleanMethodA", CallStaticBooleanMethodA},
    {"CallStaticByteMethod", CallStaticByteMethod},
    {"CallStaticByteMethodV", CallStaticByteMethodV},
    {"CallStaticByteMethodA", CallStaticByteMethodA},
    {"CallStaticCharMethod", CallStaticCharMethod},
    {"CallStaticCharMethodV", CallStaticCharMethodV},
    {"CallStaticCharMethodA", CallStaticCharMethodA},
    {"CallStaticShortMethod", CallStaticShortMethod},
    {"CallStaticShortMethodV", CallStaticShortMethodV},
    {"CallStaticShortMethodA", CallStaticShortMethodA},
    {"CallStaticIntMethod", CallStaticIntMethod},
    {"CallStaticIntMethodV", CallStaticIntMethodV},
    {"CallStaticIntMethodA", CallStaticIntMethodA},
    {"CallStaticLongMethod", CallStaticLongMethod},
    {"CallStaticLongMethodV", CallStaticLongMethodV},
    {"CallStaticLongMethodA", CallStaticLongMethodA},
    {"CallStaticFloatMethod", CallStaticFloatMethod},
    {"CallStaticFloatMethodV", CallStaticFloatMethodV},
    {"CallStaticFloatMethodA", CallStaticFloatMethodA},
    {"CallStaticDoubleMethod", CallStaticDoubleMethod},
    {"CallStaticDoubleMethodV", CallStaticDoubleMethodV},
    {"CallStaticDoubleMethodA", CallStaticDoubleMethodA},
    {"CallStaticVoidMethod", CallStaticVoidMethod},
    {"CallStaticVoidMethodV", CallStaticVoidMethodV},
    {"CallStaticVoidMethodA", CallStaticVoidMethodA},

    {"GetStaticFieldID", GetStaticFieldID},

    {"GetStaticObjectField", GetStaticObjectField},
    {"GetStaticBooleanField", GetStaticBooleanField},
    {"GetStaticByteField", GetStaticByteField},
    {"GetStaticCharField", GetStaticCharField},
    {"GetStaticShortField", GetStaticShortField},
    {"GetStaticIntField", GetStaticIntField},
    {"GetStaticLongField", GetStaticLongField},
    {"GetStaticFloatField", GetStaticFloatField},
    {"GetStaticDoubleField", GetStaticDoubleField},

    {"SetStaticObjectField", SetStaticObjectField},
    {"SetStaticBooleanField", SetStaticBooleanField},
    {"SetStaticByteField", SetStaticByteField},
    {"SetStaticCharField", SetStaticCharField},
    {"SetStaticShortField", SetStaticShortField},
    {"SetStaticIntField", SetStaticIntField},
    {"SetStaticLongField", SetStaticLongField},
    {"SetStaticFloatField", SetStaticFloatField},
    {"SetStaticDoubleField", SetStaticDoubleField},

    {"NewString", NewString},

    {"GetStringLength", GetStringLength},
    {"GetStringChars", GetStringChars},
    {"ReleaseStringChars", ReleaseStringChars},

    {"NewStringUTF", NewStringUTF},
    {"GetStringUTFLength", GetStringUTFLength},
    {"GetStringUTFChars", GetStringUTFChars},
    {"ReleaseStringUTFChars", ReleaseStringUTFChars},

    {"GetArrayLength", GetArrayLength},

    {"NewObjectArray", NewObjectArray},
    {"GetObjectArrayElement", GetObjectArrayElement},
    {"SetObjectArrayElement", SetObjectArrayElement},

    {"NewBooleanArray", NewBooleanArray},
    {"NewByteArray", NewByteArray},
    {"NewCharArray", NewCharArray},
    {"NewShortArray", NewShortArray},
    {"NewIntArray", NewIntArray},
    {"NewLongArray", NewLongArray},
    {"NewFloatArray", NewFloatArray},
    {"NewDoubleArray", NewDoubleArray},

    {"GetBooleanArrayElements", GetBooleanArrayElements},
    {"GetByteArrayElements", GetByteArrayElements},
    {"GetCharArrayElements", GetCharArrayElements},
    {"GetShortArrayElements", GetShortArrayElements},
    {"GetIntArrayElements", GetIntArrayElements},
    {"GetLongArrayElements", GetLongArrayElements},
    {"GetFloatArrayElements", GetFloatArrayElements},
    {"GetDoubleArrayElements", GetDoubleArrayElements},

    {"ReleaseBooleanArrayElements", ReleaseBooleanArrayElements},
    {"ReleaseByteArrayElements", ReleaseByteArrayElements},
    {"ReleaseCharArrayElements", ReleaseCharArrayElements},
    {"ReleaseShortArrayElements", ReleaseShortArrayElements},
    {"ReleaseIntArrayElements", ReleaseIntArrayElements},
    {"ReleaseLongArrayElements", ReleaseLongArrayElements},
    {"ReleaseFloatArrayElements", ReleaseFloatArrayElements},
    {"ReleaseDoubleArrayElements", ReleaseDoubleArrayElements},

    {"GetBooleanArrayRegion", GetBooleanArrayRegion},
    {"GetByteArrayRegion", GetByteArrayRegion},
    {"GetCharArrayRegion", GetCharArrayRegion},
    {"GetShortArrayRegion", GetShortArrayRegion},
    {"GetIntArrayRegion", GetIntArrayRegion},
    {"GetLongArrayRegion", GetLongArrayRegion},
    {"GetFloatArrayRegion", GetFloatArrayRegion},
    {"GetDoubleArrayRegion", GetDoubleArrayRegion},
    {"SetBooleanArrayRegion", SetBooleanArrayRegion},
    {"SetByteArrayRegion", SetByteArrayRegion},
    {"SetCharArrayRegion", SetCharArrayRegion},
    {"SetShortArrayRegion", SetShortArrayRegion},
    {"SetIntArrayRegion", SetIntArrayRegion},
    {"SetLongArrayRegion", SetLongArrayRegion},
    {"SetFloatArrayRegion", SetFloatArrayRegion},
    {"SetDoubleArrayRegion", SetDoubleArrayRegion},

    {"RegisterNatives", RegisterNatives},
    {"UnregisterNatives", UnregisterNatives},

    {"MonitorEnter", MonitorEnter},
    {"MonitorExit", MonitorExit},

    {"GetJavaVM", GetJavaVM},

    {"GetStringRegion", GetStringRegion},
    {"GetStringUTFRegion", GetStringUTFRegion},

    {"GetPrimitiveArrayCritical", GetPrimitiveArrayCritical},
    {"ReleasePrimitiveArrayCritical", ReleasePrimitiveArrayCritical},

    {"GetStringCritical", GetStringCritical},
    {"ReleaseStringCritical", ReleaseStringCritical},

    {"NewWeakGlobalRef", NewWeakGlobalRef},
    {"DeleteWeakGlobalRef", DeleteWeakGlobalRef},

    {"ExceptionCheck", ExceptionCheck},

    {"NewDirectByteBuffer", NewDirectByteBuffer},
    {"GetDirectBufferAddress", GetDirectBufferAddress},
    {"GetDirectBufferCapacity", GetDirectBufferCapacity},

    // GetObjectRefType
};

static const std::vector<dynamic_symbol> jni_invocation_interface_syms = {
    {"unused0", nullptr},
    {"unused1", nullptr},
    {"unused2", nullptr},
    {"DestroyJavaVM", DestroyJavaVM},
    {"AttachCurrentThread", AttachCurrentThread},
    {"DetachCurrentThread", DetachCurrentThread},

    {"GetEnv", GetEnv},

    {"AttachCurrentAsDaemon", AttachCurrentAsDaemon}};

} // namespace JNI
