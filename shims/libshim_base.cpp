/*
 * File: libshim_base.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-10
 * Purpose: Shim library providing legacy android::base::Basename(const std::string&),
 *          unwindstack symbols (GetRelPc, RegsArm::Read, RegsArm64::Read),
 *          and BoringSSL CBS_init for legacy MediaTek vendor blobs.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>

struct CBS {
    const uint8_t *data;
    size_t len;
};

extern "C" {

/* Symbol exported by modern Android libbase: android::base::Basename(std::string_view) */
void _ZN7android4base8BasenameENSt3__117basic_string_viewIcNS1_11char_traitsIcEEEE(void* ret, const char* data, size_t len);

/* Legacy symbol required by MTK vendor blobs: android::base::Basename(const std::string&) */
void _ZN7android4base8BasenameERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void* ret, const void* str) {
    const unsigned char* s = (const unsigned char*)str;
    const char* data;
    size_t len;
    if (s[0] & 1) {
        /* Long string: libc++ layout [cap|1], [size], [data pointer] */
        len = ((const size_t*)str)[1];
        data = ((const char* const*)str)[2];
    } else {
        /* Short string: length is first byte >> 1, data starts at byte 1 */
        len = s[0] >> 1;
        data = (const char*)(s + 1);
    }
    _ZN7android4base8BasenameENSt3__117basic_string_viewIcNS1_11char_traitsIcEEEE(ret, data, len);
}

/*
 * unwindstack::Elf::GetRelPc(unsigned long, unwindstack::MapInfo const*)
 * Modern libunwindstack expects (unsigned long, unwindstack::MapInfo*) without const.
 */
uint64_t _ZN11unwindstack3Elf8GetRelPcEmPNS_7MapInfoE(uint64_t step_pc, void* map_info);

uint64_t _ZN11unwindstack3Elf8GetRelPcEmPKNS_7MapInfoE(uint64_t step_pc, const void* map_info) {
    return _ZN11unwindstack3Elf8GetRelPcEmPNS_7MapInfoE(step_pc, const_cast<void*>(map_info));
}

/*
 * unwindstack::RegsArm::Read(void*)
 * Modern libunwindstack expects Read(void const*).
 */
bool _ZN11unwindstack7RegsArm4ReadEPKv(void* this_ptr, const void* data);

bool _ZN11unwindstack7RegsArm4ReadEPv(void* this_ptr, void* data) {
    return _ZN11unwindstack7RegsArm4ReadEPKv(this_ptr, data);
}

/*
 * unwindstack::RegsArm64::Read(void*)
 * Modern libunwindstack expects Read(void const*).
 */
bool _ZN11unwindstack9RegsArm644ReadEPKv(void* this_ptr, const void* data);

bool _ZN11unwindstack9RegsArm644ReadEPv(void* this_ptr, void* data) {
    return _ZN11unwindstack9RegsArm644ReadEPKv(this_ptr, data);
}

/*
 * BoringSSL CBS_init shim for legacy Widevine DRM (libwvhidl.so).
 */
void CBS_init(struct CBS *cbs, const uint8_t *data, size_t len) {
    if (cbs) {
        cbs->data = data;
        cbs->len = len;
    }
}

}
