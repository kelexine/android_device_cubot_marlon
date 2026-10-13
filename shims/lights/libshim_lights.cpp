/*
 * File: libshim_lights.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: AIDL Lights HAL fromBinder compatibility stub for /vendor/bin/factory.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

extern "C" {

/*
 * aidl::android::hardware::light::ILights::fromBinder stub for /vendor/bin/factory
 */
void _ZN4aidl7android8hardware5light7ILights10fromBinderERKN3ndk10SpAIBinderE(void* ret, const void* /*binder*/) {
    if (ret) {
        memset(ret, 0, sizeof(void*) * 2);
    }
}

} // extern "C"
