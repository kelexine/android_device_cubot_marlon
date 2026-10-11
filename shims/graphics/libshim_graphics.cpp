/*
 * File: libshim_graphics.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: GraphicBufferMapper lock/unlock legacy compatibility shim for MediaTek OMX.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

struct native_handle;
typedef struct native_handle native_handle_t;
typedef int32_t status_t;

namespace android {

class Rect;

class GraphicBufferMapper {
public:
    status_t lock(const native_handle_t* handle, uint32_t usage, const Rect& bounds, void** vaddr);
    status_t unlock(const native_handle_t* handle);
};

extern "C" {

/*
 * Legacy GraphicBufferMapper::lock:
 * status_t lock(native_handle_t const* handle, uint32_t usage, const Rect& bounds, void** vaddr, int32_t* outBytesPerPixel, int32_t* outBytesPerStride)
 */
status_t _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPvPiS9_(
        void* this_ptr,
        const native_handle_t* handle,
        uint32_t usage,
        const Rect& bounds,
        void** vaddr,
        int32_t* outBytesPerPixel,
        int32_t* outBytesPerStride) {
    GraphicBufferMapper* mapper = static_cast<GraphicBufferMapper*>(this_ptr);
    if (outBytesPerPixel) {
        *outBytesPerPixel = -1;
    }
    if (outBytesPerStride) {
        *outBytesPerStride = -1;
    }
    return mapper->lock(handle, usage, bounds, vaddr);
}

/*
 * Legacy GraphicBufferMapper::unlock:
 * status_t unlock(native_handle_t const* handle)
 */
status_t _ZN7android19GraphicBufferMapper6unlockEPK13native_handle(
        void* this_ptr,
        const native_handle_t* handle) {
    GraphicBufferMapper* mapper = static_cast<GraphicBufferMapper*>(this_ptr);
    return mapper->unlock(handle);
}

} // extern "C"
} // namespace android

