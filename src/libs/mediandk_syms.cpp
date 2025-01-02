#include "function_traits.hpp"
#include <sys/types.h>

enum media_status_t {
    AMEDIA_OK = 0,
    AMEDIACODEC_ERROR_INSUFFICIENT_RESOURCE = 1100,
    AMEDIACODEC_ERROR_RECLAIMED = 1101,
    AMEDIA_ERROR_BASE = -10000,
    AMEDIA_ERROR_UNKNOWN = AMEDIA_ERROR_BASE,
    AMEDIA_ERROR_MALFORMED = AMEDIA_ERROR_BASE - 1,
    AMEDIA_ERROR_UNSUPPORTED = AMEDIA_ERROR_BASE - 2,
    AMEDIA_ERROR_INVALID_OBJECT = AMEDIA_ERROR_BASE - 3,
    AMEDIA_ERROR_INVALID_PARAMETER = AMEDIA_ERROR_BASE - 4,
    AMEDIA_ERROR_INVALID_OPERATION = AMEDIA_ERROR_BASE - 5,
    AMEDIA_ERROR_END_OF_STREAM = AMEDIA_ERROR_BASE - 6,
    AMEDIA_ERROR_IO = AMEDIA_ERROR_BASE - 7,
    AMEDIA_ERROR_WOULD_BLOCK = AMEDIA_ERROR_BASE - 8,
    AMEDIA_DRM_ERROR_BASE = -20000,
    AMEDIA_DRM_NOT_PROVISIONED = AMEDIA_DRM_ERROR_BASE - 1,
    AMEDIA_DRM_RESOURCE_BUSY = AMEDIA_DRM_ERROR_BASE - 2,
    AMEDIA_DRM_DEVICE_REVOKED = AMEDIA_DRM_ERROR_BASE - 3,
    AMEDIA_DRM_SHORT_BUFFER = AMEDIA_DRM_ERROR_BASE - 4,
    AMEDIA_DRM_SESSION_NOT_OPENED = AMEDIA_DRM_ERROR_BASE - 5,
    AMEDIA_DRM_TAMPER_DETECTED = AMEDIA_DRM_ERROR_BASE - 6,
    AMEDIA_DRM_VERIFY_FAILED = AMEDIA_DRM_ERROR_BASE - 7,
    AMEDIA_DRM_NEED_KEY = AMEDIA_DRM_ERROR_BASE - 8,
    AMEDIA_DRM_LICENSE_EXPIRED = AMEDIA_DRM_ERROR_BASE - 9,
    AMEDIA_IMGREADER_ERROR_BASE = -30000,
    AMEDIA_IMGREADER_NO_BUFFER_AVAILABLE = AMEDIA_IMGREADER_ERROR_BASE - 1,
    AMEDIA_IMGREADER_MAX_IMAGES_ACQUIRED = AMEDIA_IMGREADER_ERROR_BASE - 2,
    AMEDIA_IMGREADER_CANNOT_LOCK_IMAGE = AMEDIA_IMGREADER_ERROR_BASE - 3,
    AMEDIA_IMGREADER_CANNOT_UNLOCK_IMAGE = AMEDIA_IMGREADER_ERROR_BASE - 4,
    AMEDIA_IMGREADER_IMAGE_NOT_LOCKED = AMEDIA_IMGREADER_ERROR_BASE - 5
};

const char *AMEDIAFORMAT_KEY_CHANNEL_COUNT = nullptr;
const char *AMEDIAFORMAT_KEY_COLOR_FORMAT = nullptr;
const char *AMEDIAFORMAT_KEY_HEIGHT = nullptr;
const char *AMEDIAFORMAT_KEY_MIME = nullptr;
const char *AMEDIAFORMAT_KEY_SAMPLE_RATE = nullptr;
const char *AMEDIAFORMAT_KEY_STRIDE = nullptr;
const char *AMEDIAFORMAT_KEY_WIDTH = nullptr;

// STUBS
media_status_t AMediaCodec_configure(...) { return AMEDIA_ERROR_UNKNOWN; }
void *AMediaCodec_createDecoderByType(...) { return nullptr; }
media_status_t AMediaCodec_delete(...) { return AMEDIA_ERROR_UNKNOWN; }
ssize_t AMediaCodec_dequeueInputBuffer(...) { return 0; }
ssize_t AMediaCodec_dequeueOutputBuffer(...) { return 0; }
media_status_t AMediaCodec_flush(...) { return AMEDIA_ERROR_UNKNOWN; }
void *AMediaCodec_getInputBuffer(...) { return nullptr; }
void *AMediaCodec_getOutputBuffer(...) { return nullptr; }
void *AMediaCodec_getOutputFormat(...) { return nullptr; }
media_status_t AMediaCodec_queueInputBuffer(...) { return AMEDIA_ERROR_UNKNOWN; }
media_status_t AMediaCodec_releaseOutputBuffer(...) { return AMEDIA_ERROR_UNKNOWN; }
media_status_t AMediaCodec_start(...) { return AMEDIA_ERROR_UNKNOWN; }
media_status_t AMediaCodec_stop(...) { return AMEDIA_ERROR_UNKNOWN; }
media_status_t AMediaFormat_delete(...) { return AMEDIA_ERROR_UNKNOWN; }
bool AMediaFormat_getInt32(...) { return false; }
void *AMediaFormat_new(...) { return nullptr; }
void AMediaFormat_setBuffer(...) { return; }
void AMediaFormat_setInt32(...) { return; }
void AMediaFormat_setString(...) { return; }
const char *AMediaFormat_toString(...) { return nullptr; }


extern const std::vector<dynamic_symbol> libmediandk_symbols = {
    { "AMEDIAFORMAT_KEY_CHANNEL_COUNT", &AMEDIAFORMAT_KEY_CHANNEL_COUNT },
    { "AMEDIAFORMAT_KEY_COLOR_FORMAT", &AMEDIAFORMAT_KEY_COLOR_FORMAT },
    { "AMEDIAFORMAT_KEY_HEIGHT", &AMEDIAFORMAT_KEY_HEIGHT },
    { "AMEDIAFORMAT_KEY_MIME", &AMEDIAFORMAT_KEY_MIME },
    { "AMEDIAFORMAT_KEY_SAMPLE_RATE", &AMEDIAFORMAT_KEY_SAMPLE_RATE },
    { "AMEDIAFORMAT_KEY_STRIDE", &AMEDIAFORMAT_KEY_STRIDE },
    { "AMEDIAFORMAT_KEY_WIDTH", &AMEDIAFORMAT_KEY_WIDTH },

    { "AMediaCodec_configure", AMediaCodec_configure },
    { "AMediaCodec_createDecoderByType", AMediaCodec_createDecoderByType },
    { "AMediaCodec_delete", AMediaCodec_delete },
    { "AMediaCodec_dequeueInputBuffer", AMediaCodec_dequeueInputBuffer },
    { "AMediaCodec_dequeueOutputBuffer", AMediaCodec_dequeueOutputBuffer },
    { "AMediaCodec_flush", AMediaCodec_flush },
    { "AMediaCodec_getInputBuffer", AMediaCodec_getInputBuffer },
    { "AMediaCodec_getOutputBuffer", AMediaCodec_getOutputBuffer },
    { "AMediaCodec_getOutputFormat", AMediaCodec_getOutputFormat },
    { "AMediaCodec_queueInputBuffer", AMediaCodec_queueInputBuffer },
    { "AMediaCodec_releaseOutputBuffer", AMediaCodec_releaseOutputBuffer },
    { "AMediaCodec_start", AMediaCodec_start },
    { "AMediaCodec_stop", AMediaCodec_stop },
    { "AMediaFormat_delete", AMediaFormat_delete },
    { "AMediaFormat_getInt32", AMediaFormat_getInt32 },
    { "AMediaFormat_new", AMediaFormat_new },
    { "AMediaFormat_setBuffer", AMediaFormat_setBuffer },
    { "AMediaFormat_setInt32", AMediaFormat_setInt32 },
    { "AMediaFormat_setString", AMediaFormat_setString },
    { "AMediaFormat_toString", AMediaFormat_toString },
};
