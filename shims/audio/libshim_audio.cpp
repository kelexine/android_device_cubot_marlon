/*
 * File: libshim_audio.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: Audio HAL compatibility shim providing TypeConverter::mTable,
 *          audio_utils_fifo constructor, and deviceFromString.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdbool.h>
#include <system/audio.h>

#define MAKE_STRING_FROM_ENUM(enumval) { #enumval, enumval }
#define TERMINATOR { .literal = nullptr }

namespace android {

template <typename T>
struct DefaultTraits {
    typedef T Type;
    struct Table {
        const char *literal;
        Type value;
    };
    static const Table mTable[];
};

template <>
const DefaultTraits<audio_format_t>::Table DefaultTraits<audio_format_t>::mTable[] = {
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_PCM_16_BIT),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_PCM_8_BIT),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_PCM_32_BIT),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_PCM_8_24_BIT),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_PCM_FLOAT),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_PCM_24_BIT_PACKED),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_MP3),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AMR_NB),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AMR_WB),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_MAIN),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_LC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_SSR),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_LTP),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_HE_V1),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_SCALABLE),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ERLC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_LD),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_HE_V2),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ELD),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_XHE),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_MAIN),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_LC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_SSR),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_LTP),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_HE_V1),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_SCALABLE),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_ERLC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_LD),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_HE_V2),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_ELD),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS_XHE),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_VORBIS),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_HE_AAC_V1),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_HE_AAC_V2),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_OPUS),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AC3),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_E_AC3),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_DTS),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_DTS_HD),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_IEC61937),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_DOLBY_TRUEHD),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_EVRC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_EVRCB),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_EVRCWB),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_EVRCNW),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADIF),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_WMA),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_WMA_PRO),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AMR_WB_PLUS),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_MP2),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_QCELP),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_DSD),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_FLAC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_ALAC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_APE),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_ADTS),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_SBC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_APTX),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_APTX_HD),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AC4),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_LDAC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_MAT),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_E_AC3_JOC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_MAT_1_0),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_MAT_2_0),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_MAT_2_1),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_LATM),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_LATM_LC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_LATM_HE_V1),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_AAC_LATM_HE_V2),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_CELT),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_APTX_ADAPTIVE),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_LHDC),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_LHDC_LL),
    MAKE_STRING_FROM_ENUM(AUDIO_FORMAT_APTX_TWSP),
    TERMINATOR
};

template <typename Traits>
class TypeConverter {
public:
    static const typename Traits::Table mTable[];
};

template <typename Traits>
const typename Traits::Table TypeConverter<Traits>::mTable[] = {
    TERMINATOR
};

template class TypeConverter<DefaultTraits<audio_format_t>>;

} // namespace android

extern "C" {

/*
 * Modern 5-argument constructor in libaudioutils:
 * audio_utils_fifo(frameCount, frameSize, buffer, throttle, sync)
 */
extern void _ZN16audio_utils_fifoC1EjjPvb21audio_utils_fifo_sync(void* this_ptr, unsigned int frameCount, unsigned int frameSize, void* buffer, bool throttle, int sync);

/*
 * Legacy 4-argument constructor:
 * audio_utils_fifo::audio_utils_fifo(unsigned int, unsigned int, void*, bool)
 */
void _ZN16audio_utils_fifoC1EjjPvb(void* this_ptr, unsigned int frameCount, unsigned int frameSize, void* buffer, bool throttle) {
    _ZN16audio_utils_fifoC1EjjPvb21audio_utils_fifo_sync(this_ptr, frameCount, frameSize, buffer, throttle, 0);
}

void _ZN16audio_utils_fifoC2EjjPvb(void* this_ptr, unsigned int frameCount, unsigned int frameSize, void* buffer, bool throttle) {
    _ZN16audio_utils_fifoC1EjjPvb(this_ptr, frameCount, frameSize, buffer, throttle);
}

/*
 * Audio effects config:
 * android::deviceFromString(const std::string&, unsigned int&)
 */
bool _ZN7android16deviceFromStringERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERj(const void* /*str*/, uint32_t* dev) {
    if (dev) *dev = 0;
    return true;
}

} // extern "C"
