/*
 * File: libshim_crypto.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: BoringSSL crypto shim providing CBS_init for legacy Widevine DRM.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>

extern "C" {

struct CBS {
    const uint8_t *data;
    size_t len;
};

/*
 * BoringSSL CBS_init shim for legacy Widevine DRM (libwvhidl.so / libwvdrmengine.so).
 */
void CBS_init(struct CBS *cbs, const uint8_t *data, size_t len) {
    if (cbs) {
        cbs->data = data;
        cbs->len = len;
    }
}

} // extern "C"
