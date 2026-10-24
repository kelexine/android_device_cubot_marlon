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

#define MAKE_STRING_FROM_ENUM(enumval) { #enumval, enumval }
#define TERMINATOR { .literal = nullptr, .value = AUDIO_FORMAT_DEFAULT }

enum audio_format_t : uint32_t {
    AUDIO_FORMAT_INVALID = 0xFFFFFFFFu,
    AUDIO_FORMAT_DEFAULT = 0,
    AUDIO_FORMAT_PCM_16_BIT = 0x1u,
    AUDIO_FORMAT_PCM_8_BIT = 0x2u,
    AUDIO_FORMAT_PCM_32_BIT = 0x3u,
    AUDIO_FORMAT_PCM_8_24_BIT = 0x4u,
    AUDIO_FORMAT_PCM_FLOAT = 0x5u,
    AUDIO_FORMAT_PCM_24_BIT_PACKED = 0x6u,
    AUDIO_FORMAT_MP3 = 0x01000000u,
    AUDIO_FORMAT_AMR_NB = 0x02000000u,
    AUDIO_FORMAT_AMR_WB = 0x03000000u,
    AUDIO_FORMAT_AAC = 0x04000000u,
    AUDIO_FORMAT_AAC_MAIN = 0x04000001u,
    AUDIO_FORMAT_AAC_LC = 0x04000002u,
    AUDIO_FORMAT_AAC_SSR = 0x04000004u,
    AUDIO_FORMAT_AAC_LTP = 0x04000008u,
    AUDIO_FORMAT_AAC_HE_V1 = 0x04000010u,
    AUDIO_FORMAT_AAC_SCALABLE = 0x04000020u,
    AUDIO_FORMAT_AAC_ERLC = 0x04000040u,
    AUDIO_FORMAT_AAC_LD = 0x04000080u,
    AUDIO_FORMAT_AAC_HE_V2 = 0x04000100u,
    AUDIO_FORMAT_AAC_ELD = 0x04000200u,
    AUDIO_FORMAT_AAC_XHE = 0x04000300u,
    AUDIO_FORMAT_AAC_ADTS = 0x1e000000u,
    AUDIO_FORMAT_AAC_ADTS_MAIN = 0x1e000001u,
    AUDIO_FORMAT_AAC_ADTS_LC = 0x1e000002u,
    AUDIO_FORMAT_AAC_ADTS_SSR = 0x1e000004u,
    AUDIO_FORMAT_AAC_ADTS_LTP = 0x1e000008u,
    AUDIO_FORMAT_AAC_ADTS_HE_V1 = 0x1e000010u,
    AUDIO_FORMAT_AAC_ADTS_SCALABLE = 0x1e000020u,
    AUDIO_FORMAT_AAC_ADTS_ERLC = 0x1e000040u,
    AUDIO_FORMAT_AAC_ADTS_LD = 0x1e000080u,
    AUDIO_FORMAT_AAC_ADTS_HE_V2 = 0x1e000100u,
    AUDIO_FORMAT_AAC_ADTS_ELD = 0x1e000200u,
    AUDIO_FORMAT_AAC_ADTS_XHE = 0x1e000300u,
    AUDIO_FORMAT_VORBIS = 0x05000000u,
    AUDIO_FORMAT_HE_AAC_V1 = 0x05000001u,
    AUDIO_FORMAT_HE_AAC_V2 = 0x05000002u,
    AUDIO_FORMAT_OPUS = 0x08000000u,
    AUDIO_FORMAT_AC3 = 0x09000000u,
    AUDIO_FORMAT_E_AC3 = 0x0a000000u,
    AUDIO_FORMAT_DTS = 0x0b000000u,
    AUDIO_FORMAT_DTS_HD = 0x0c000000u,
    AUDIO_FORMAT_IEC61937 = 0x0d000000u,
    AUDIO_FORMAT_DOLBY_TRUEHD = 0x0e000000u,
    AUDIO_FORMAT_EVRC = 0x10000000u,
    AUDIO_FORMAT_EVRCB = 0x11000000u,
    AUDIO_FORMAT_EVRCWB = 0x12000000u,
    AUDIO_FORMAT_EVRCNW = 0x13000000u,
    AUDIO_FORMAT_AAC_ADIF = 0x14000000u,
    AUDIO_FORMAT_WMA = 0x15000000u,
    AUDIO_FORMAT_WMA_PRO = 0x16000000u,
    AUDIO_FORMAT_AMR_WB_PLUS = 0x17000000u,
    AUDIO_FORMAT_MP2 = 0x18000000u,
    AUDIO_FORMAT_QCELP = 0x19000000u,
    AUDIO_FORMAT_DSD = 0x1c000000u,
    AUDIO_FORMAT_FLAC = 0x1d000000u,
    AUDIO_FORMAT_ALAC = 0x1f000000u,
    AUDIO_FORMAT_APE = 0x20000000u,
    AUDIO_FORMAT_SBC = 0x21000000u,
    AUDIO_FORMAT_APTX = 0x22000000u,
    AUDIO_FORMAT_APTX_HD = 0x23000000u,
    AUDIO_FORMAT_AC4 = 0x24000000u,
    AUDIO_FORMAT_LDAC = 0x25000000u,
    AUDIO_FORMAT_MAT = 0x26000000u,
    AUDIO_FORMAT_E_AC3_JOC = 0x27000000u,
    AUDIO_FORMAT_MAT_1_0 = 0x26000001u,
    AUDIO_FORMAT_MAT_2_0 = 0x26000002u,
    AUDIO_FORMAT_MAT_2_1 = 0x26000003u,
    AUDIO_FORMAT_AAC_LATM = 0x28000000u,
    AUDIO_FORMAT_AAC_LATM_LC = 0x28000002u,
    AUDIO_FORMAT_AAC_LATM_HE_V1 = 0x28000010u,
    AUDIO_FORMAT_AAC_LATM_HE_V2 = 0x28000100u,
    AUDIO_FORMAT_CELT = 0x29000000u,
    AUDIO_FORMAT_APTX_ADAPTIVE = 0x2a000000u,
    AUDIO_FORMAT_LHDC = 0x2b000000u,
    AUDIO_FORMAT_LHDC_LL = 0x2c000000u,
    AUDIO_FORMAT_APTX_TWSP = 0x2d000000u
};

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

struct OutputDeviceTraits {
    typedef uint32_t Type;
    struct Table {
        const char *literal;
        Type value;
    };
    static const Table mTable[];
};

struct InputDeviceTraits {
    typedef uint32_t Type;
    struct Table {
        const char *literal;
        Type value;
    };
    static const Table mTable[];
};

template class TypeConverter<DefaultTraits<audio_format_t>>;
template class TypeConverter<OutputDeviceTraits>;
template class TypeConverter<InputDeviceTraits>;

} // namespace android

extern "C" {

/*
 * Modern 5-argument constructor in libaudioutils (weak link / forward):
 * audio_utils_fifo(frameCount, frameSize, buffer, throttle, sync)
 */
__attribute__((weak))
void _ZN16audio_utils_fifoC1EjjPvb21audio_utils_fifo_sync(void* this_ptr, unsigned int frameCount, unsigned int frameSize, void* buffer, bool throttle, int sync);

/*
 * Legacy 4-argument constructor:
 * audio_utils_fifo::audio_utils_fifo(unsigned int, unsigned int, void*, bool)
 */
void _ZN16audio_utils_fifoC1EjjPvb(void* this_ptr, unsigned int frameCount, unsigned int frameSize, void* buffer, bool throttle) {
    if (_ZN16audio_utils_fifoC1EjjPvb21audio_utils_fifo_sync) {
        _ZN16audio_utils_fifoC1EjjPvb21audio_utils_fifo_sync(this_ptr, frameCount, frameSize, buffer, throttle, 0);
    }
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

/*
 * Modern 3-argument function in android.hardware.audio.common@6.0-util.so:
 * status_t HidlUtils::audioConfigFromHal(const audio_config& halConfig, bool isInput, AudioConfig* config)
 */
__attribute__((weak))
int32_t _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils18audioConfigFromHalERK12audio_configbPNS3_11AudioConfigE(
    const void* halConfig, bool isInput, void* config);

/*
 * Legacy 2-argument function expected by proprietary MTK audio HAL:
 * status_t HidlUtils::audioConfigFromHal(const audio_config& halConfig, AudioConfig* config)
 */
int32_t _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils18audioConfigFromHalERK12audio_configPNS3_11AudioConfigE(
    const void* halConfig, void* config)
{
    if (_ZN7android8hardware5audio6common4V6_014implementation9HidlUtils18audioConfigFromHalERK12audio_configbPNS3_11AudioConfigE) {
        return _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils18audioConfigFromHalERK12audio_configbPNS3_11AudioConfigE(
            halConfig, false /* isInput */, config);
    }
    return 0; // OK
}

/*
 * Modern 2-argument function in android.hardware.audio.common@6.0-util.so:
 * status_t HidlUtils::audioPortConfigsToHal(const hidl_vec<AudioPortConfig>& configs, std::unique_ptr<audio_port_config[]>* halConfigs)
 */
__attribute__((weak))
int32_t _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils21audioPortConfigsToHalERKNS0_8hidl_vecINS3_15AudioPortConfigEEEPNSt3__110unique_ptrIA_17audio_port_configNSB_14default_deleteISE_EEEE(
    const void* configs, void* halConfigs);

/*
 * Legacy function expected by proprietary MTK audio HAL:
 * std::unique_ptr<audio_port_config[]> HidlUtils::audioPortConfigsToHal(const hidl_vec<AudioPortConfig>& configs)
 */
#if defined(__aarch64__)
__attribute__((naked))
void _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils21audioPortConfigsToHalERKNS0_8hidl_vecINS3_15AudioPortConfigEEE() {
    __asm__ volatile(
        "mov x1, x8\n"
        "b _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils21audioPortConfigsToHalERKNS0_8hidl_vecINS3_15AudioPortConfigEEEPNSt3__110unique_ptrIA_17audio_port_configNSB_14default_deleteISE_EEEE\n"
    );
}
#elif defined(__arm__)
__attribute__((naked))
void _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils21audioPortConfigsToHalERKNS0_8hidl_vecINS3_15AudioPortConfigEEE() {
    __asm__ volatile(
        "push {r0, lr}\n"
        "mov r2, r0\n"
        "mov r0, r1\n"
        "mov r1, r2\n"
        "bl _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils21audioPortConfigsToHalERKNS0_8hidl_vecINS3_15AudioPortConfigEEEPNSt3__110unique_ptrIA_17audio_port_configNSB_14default_deleteISE_EEEE\n"
        "pop {r0, pc}\n"
    );
}
#endif

/*
 * Modern functions in UuidUtils (android.hardware.audio.common@6.0-util.so):
 * void UuidUtils::uuidFromHal(const audio_uuid_s&, Uuid*)
 * void UuidUtils::uuidToHal(const Uuid&, audio_uuid_s*)
 */
__attribute__((weak))
void _ZN7android8hardware5audio6common4V6_014implementation9UuidUtils11uuidFromHalERK12audio_uuid_sPNS3_4UuidE(
    const void* halUuid, void* uuid);

__attribute__((weak))
void _ZN7android8hardware5audio6common4V6_014implementation9UuidUtils9uuidToHalERKNS3_4UuidEP12audio_uuid_s(
    const void* uuid, void* halUuid);

/*
 * Legacy functions in HidlUtils (expected by android.hardware.audio.effect@6.0-impl.so):
 * void HidlUtils::uuidFromHal(const audio_uuid_s&, Uuid*)
 * void HidlUtils::uuidToHal(const Uuid&, audio_uuid_s*)
 */
void _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils11uuidFromHalERK12audio_uuid_sPNS3_4UuidE(
    const void* halUuid, void* uuid)
{
    if (_ZN7android8hardware5audio6common4V6_014implementation9UuidUtils11uuidFromHalERK12audio_uuid_sPNS3_4UuidE) {
        _ZN7android8hardware5audio6common4V6_014implementation9UuidUtils11uuidFromHalERK12audio_uuid_sPNS3_4UuidE(halUuid, uuid);
    }
}

void _ZN7android8hardware5audio6common4V6_014implementation9HidlUtils9uuidToHalERKNS3_4UuidEP12audio_uuid_s(
    const void* uuid, void* halUuid)
{
    if (_ZN7android8hardware5audio6common4V6_014implementation9UuidUtils9uuidToHalERKNS3_4UuidEP12audio_uuid_s) {
        _ZN7android8hardware5audio6common4V6_014implementation9UuidUtils9uuidToHalERKNS3_4UuidEP12audio_uuid_s(uuid, halUuid);
    }
}

} // extern "C"
