/*
 * File: libshim_base.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-10
 * Purpose: Shim library providing legacy android::base::Basename(const std::string&)
 *          symbol for legacy MediaTek vendor blobs (libnvram.so, libsysenv.so).
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

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

}
