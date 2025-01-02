#include "function_traits.hpp"
#include "types.hpp"

void* SL_IID_ENGINE = nullptr;
void* SL_IID_ANDROIDSIMPLEBUFFERQUEUE = nullptr;
void* SL_IID_ANDROIDCONFIGURATION = nullptr;
void* SL_IID_RECORD = nullptr;
void* SL_IID_BUFFERQUEUE = nullptr;
void* SL_IID_VOLUME = nullptr;
void* SL_IID_PLAY = nullptr;

// Stub
u32 slCreateEngine(void *, u32, void *, u32, void *, void *) {
    return 0x0000000D; // SL_RESULT_INTERNAL_ERROR
}


extern const std::vector<dynamic_symbol> libopensles_symbols = {
    { "SL_IID_ANDROIDCONFIGURATION", &SL_IID_ANDROIDCONFIGURATION },
    { "SL_IID_ANDROIDSIMPLEBUFFERQUEUE", &SL_IID_ANDROIDSIMPLEBUFFERQUEUE },
    { "SL_IID_BUFFERQUEUE", &SL_IID_BUFFERQUEUE },
    { "SL_IID_ENGINE", &SL_IID_ENGINE },
    { "SL_IID_PLAY", &SL_IID_PLAY },
    { "SL_IID_RECORD", &SL_IID_RECORD },
    { "SL_IID_VOLUME", &SL_IID_VOLUME },
    { "slCreateEngine", slCreateEngine }
};
