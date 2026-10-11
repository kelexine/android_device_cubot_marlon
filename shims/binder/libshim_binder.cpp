/*
 * File: libshim_binder.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: VNDK Binder compatibility and vndservicemanager stubs.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdbool.h>

extern "C" {

/*
 * VNDK Binder compatibility:
 * android::BpBinder::handle() const
 */
int32_t _ZNK7android8BpBinder6handleEv(void* /*this_ptr*/) {
    return 0;
}

/*
 * Legacy vndservicemanager Binder ProcessState / IPCThreadState stubs
 */
bool _ZN7android12ProcessState20becomeContextManagerEPFbRKNS_8String16ERKNS_2spINS_7IBinderEEEPvES9_(void* /*this_ptr*/, void* /*cb*/, void* /*userData*/) {
    return true;
}

int32_t _ZN7android12ProcessState32getStrongRefCountForNodeByHandleEi(void* /*this_ptr*/, int32_t /*handle*/) {
    return 1;
}

void _ZN7android14IPCThreadState19setTheContextObjectENS_2spINS_7BBinderEEE(void* /*this_ptr*/, void* /*obj*/) {
}

} // extern "C"
