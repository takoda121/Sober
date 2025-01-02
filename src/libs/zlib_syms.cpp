#include "function_traits.hpp"
#include "execution_context.hpp"
#include <cassert>
#include <zlib.h>


struct ZlibGuestWrapper {
    void *guest_zalloc = nullptr;
    void *guest_zfree = nullptr;
    void *guest_opaque = nullptr;
    bool needs_cleanup = false;
};

static void *zalloc_caller(void *opaque, unsigned int items, unsigned int size) {
    ZlibGuestWrapper *wrapper = reinterpret_cast<ZlibGuestWrapper*>(opaque);
    assert(wrapper);
    assert(wrapper->guest_zalloc);

    ScopedExecutionFiber fiber(FiberKind::Callee);
    fiber.run(wrapper->guest_zalloc, wrapper->guest_opaque, items, size);
    return reinterpret_cast<void*>(fiber.get_register(0));
}

static void zfree_caller(void *opaque, void *address) {
    ZlibGuestWrapper *wrapper = reinterpret_cast<ZlibGuestWrapper*>(opaque);
    assert(wrapper);
    assert(wrapper->guest_zfree);

    ScopedExecutionFiber fiber(FiberKind::Callee);
    fiber.run(wrapper->guest_zfree, wrapper->guest_opaque, address);
    return;
}

static void patch_z_stream(z_stream *stream) {
    if (!stream->zalloc || !stream->zfree) return;

    void *original_zalloc = reinterpret_cast<void*>(stream->zalloc);
    void *original_zfree = reinterpret_cast<void*>(stream->zfree);
    if (stream->zalloc) {
        stream->zalloc = zalloc_caller;
    }
    if (stream->zfree) {
        stream->zfree = zfree_caller;
    }

    auto wrapper = new ZlibGuestWrapper();
    assert(wrapper);

    wrapper->guest_zalloc = original_zalloc;
    wrapper->guest_zfree = original_zfree;
    wrapper->guest_opaque = stream->opaque;
    wrapper->needs_cleanup = false;

    stream->opaque = wrapper;
}

static void cleanup_z_stream(z_stream *stream) {
    if (!stream || !stream->opaque) return;

    // TODO: here we would free the stream->opaque wrapper, but it seems like the guest zfree might be freeing stream->opaque?
    //       it's weird. we should figure out how to not leak the wrapper.
}


extern const std::vector<dynamic_symbol> zlib_symbols = {
    { dynamic_symbol::Tag::HasExecutionSideEffects, "deflate", deflate },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "deflateEnd", +[](z_streamp strm) -> int {
        cleanup_z_stream(strm);
        return deflateEnd(strm);
    } },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "deflateInit_", +[](z_streamp strm, int level, const char *version, int stream_size) -> int {
        patch_z_stream(strm);
        return deflateInit_(strm, level, version, stream_size);
    } },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "deflateInit2_", +[](z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size) -> int {
        patch_z_stream(strm);
        return deflateInit2_(strm, level, method, windowBits, memLevel, strategy, version, stream_size);
    } },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "deflateReset", deflateReset },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "inflate", inflate },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "inflateEnd", +[](z_streamp strm) -> int {
        cleanup_z_stream(strm);
        return inflateEnd(strm);
    } },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "inflateInit_", +[](z_streamp strm, const char *version, int stream_size) -> int {
        patch_z_stream(strm);
        return inflateInit_(strm, version, stream_size);
    } },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "inflateInit2_", +[](z_streamp strm, int windowBits, const char *version, int stream_size) -> int {
        patch_z_stream(strm);
        return inflateInit2_(strm, windowBits, version, stream_size);
    } },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "inflateReset", inflateReset },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "inflateReset2", inflateReset2 },
    { dynamic_symbol::Tag::HasExecutionSideEffects, "uncompress", uncompress },
    { "adler32", adler32 },
    { "crc32", crc32 },
    { "zError", zError },
    { "zlibVersion", zlibVersion }
};