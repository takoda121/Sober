#pragma once
// Values from android ndk linux headers

#if defined(__i386__)
#define FAKE_SYS_gettid 224
#define FAKE_SYS_getrandom 355
#elif defined(__arm__)
#define FAKE_SYS_gettid 224
#define FAKE_SYS_getrandom 384
#elif defined(__aarch64__) || 1 /* TODO: we force aarch64 */
#define FAKE_SYS_gettid 178
#define FAKE_SYS_getrandom 278
#define FAKE_SYS_futex 98
#elif defined(__x86_64__)
#define FAKE_SYS_gettid 186
#define FAKE_SYS_getrandom 318
#endif
