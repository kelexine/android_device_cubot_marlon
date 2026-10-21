/*
 * File: libshim_binder.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: VNDK Binder compatibility and vndservicemanager functional wrappers.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdbool.h>

#include <binder/BpBinder.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

extern "C" {

/*
 * VNDK Binder compatibility:
 * android::BpBinder::handle() const -> forwards to BpBinder::getDebugBinderHandle()
 */
int32_t _ZNK7android8BpBinder6handleEv(const android::BpBinder* this_ptr) {
    if (!this_ptr) {
        return 0;
    }
    return this_ptr->getDebugBinderHandle().value_or(0);
}

/*
 * Legacy vndservicemanager Binder ProcessState::becomeContextManager(cb, userData)
 * Forwards to modern 0-arg ProcessState::becomeContextManager() so /dev/vndbinder
 * actually registers the context manager via BINDER_SET_CONTEXT_MGR_EXT ioctl.
 */
bool _ZN7android12ProcessState20becomeContextManagerEPFbRKNS_8String16ERKNS_2spINS_7IBinderEEEPvES9_(
        android::ProcessState* this_ptr,
        void* /*cb*/,
        void* /*userData*/) {
    if (this_ptr) {
        return this_ptr->becomeContextManager();
    }
    return android::ProcessState::self()->becomeContextManager();
}

int32_t _ZN7android12ProcessState32getStrongRefCountForNodeByHandleEi(
        android::ProcessState* /*this_ptr*/,
        int32_t /*handle*/) {
    return 1;
}

/*
 * Legacy IPCThreadState::setTheContextObject(sp<BBinder> obj)
 * Forwards to modern IPCThreadState::setTheContextObject(const sp<BBinder>& obj).
 */
void _ZN7android14IPCThreadState19setTheContextObjectENS_2spINS_7BBinderEEE(
        android::IPCThreadState* this_ptr,
        const android::sp<android::BBinder>& obj) {
    if (this_ptr) {
        this_ptr->setTheContextObject(obj);
    } else {
        android::IPCThreadState::self()->setTheContextObject(obj);
    }
}

} // extern "C"
