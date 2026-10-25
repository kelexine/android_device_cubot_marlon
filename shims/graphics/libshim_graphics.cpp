/*
 * File: libshim_graphics.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: GraphicBufferMapper lock/unlock/lockYCbCr, GraphicBuffer, and Fence compatibility shims for MediaTek OMX.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string>
#include <sync/sync.h>
#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>
#include <utils/Errors.h>

using android::Rect;
using android::status_t;

extern "C" {

/*
 * Legacy GraphicBufferMapper::lock:
 * status_t lock(native_handle_t const* handle, uint32_t usage, const Rect& bounds,
 *               void** vaddr, int32_t* outBytesPerPixel, int32_t* outBytesPerStride)
 */
status_t _ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPvPiS9_(
        void* thisptr, buffer_handle_t handle, uint32_t usage, const Rect& bounds, void** vaddr,
        int32_t* outBytesPerPixel, int32_t* outBytesPerStride) {
    auto* gpm = static_cast<android::GraphicBufferMapper*>(thisptr);
    return gpm->lock(handle, static_cast<uint64_t>(usage), bounds, vaddr,
                     outBytesPerPixel, outBytesPerStride);
}

/*
 * Legacy GraphicBufferMapper::lockYCbCr:
 * status_t lockYCbCr(native_handle_t const* handle, uint32_t usage, const Rect& bounds,
 *                    android_ycbcr* ycbcr)
 */
status_t _ZN7android19GraphicBufferMapper9lockYCbCrEPK13native_handlejRKNS_4RectEP13android_ycbcr(
        void* thisptr, buffer_handle_t handle, uint32_t usage, const Rect& bounds,
        android_ycbcr* ycbcr) {
    auto* gpm = static_cast<android::GraphicBufferMapper*>(thisptr);
    return gpm->lockYCbCr(handle, static_cast<uint64_t>(usage), bounds, ycbcr);
}

/*
 * Legacy GraphicBufferMapper::unlock:
 * status_t unlock(native_handle_t const* handle)
 */
status_t _ZN7android19GraphicBufferMapper6unlockEPK13native_handle(
        void* thisptr, buffer_handle_t handle) {
    android::base::unique_fd outFence;
    auto* gpm = static_cast<android::GraphicBufferMapper*>(thisptr);
    status_t status = gpm->unlock(handle, &outFence);
    if (status == android::OK && outFence.get() >= 0) {
        sync_wait(outFence.get(), -1);
        outFence.reset();
    }
    return status;
}

/*
 * Legacy 5-argument GraphicBuffer constructor:
 * GraphicBuffer::GraphicBuffer(uint32_t inWidth, uint32_t inHeight, PixelFormat inFormat,
 *                              uint32_t inUsage, std::string requestorName)
 */
void _ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(
        void* thisptr, uint32_t inWidth, uint32_t inHeight, int32_t inFormat,
        uint32_t inUsage, std::string requestorName) {
    new (thisptr) android::GraphicBuffer(inWidth, inHeight, inFormat, 1,
            static_cast<uint64_t>(inUsage), std::move(requestorName));
}

/*
 * Legacy Fence constructor:
 * Fence::Fence(int fenceFd)
 */
void _ZN7android5FenceC1Ei(void* thisptr, int fenceFd) {
    new (thisptr) android::Fence(fenceFd);
}

/*
 * Legacy Fence::wait:
 * status_t Fence::wait(int timeout)
 */
status_t _ZN7android5Fence4waitEi(void* thisptr, int timeout) {
    auto* fence = static_cast<android::Fence*>(thisptr);
    return fence->wait(timeout);
}

/*
 * Fence destructor (no-op, = default in newer AOSP)
 */
void _ZN7android5FenceD1Ev() {
}

} // extern "C"


