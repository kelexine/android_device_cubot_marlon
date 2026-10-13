/*
 * File: libshim_cxx.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: Legacy C++ ABI exception handling stubs for 32-bit VoLTE and RIL daemons.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>

extern "C" {

/*
 * Legacy C++ ABI Exception Handling for 32-bit VoLTE / RIL Daemons:
 * bip, volte_imcb, volte_stack, volte_ua, volte_imsm_93, wfca
 */
bool __cxa_type_match(void* /*throw_type*/, void* /*catch_type*/, bool /*is_reference*/, void** /*matched_object*/) {
    return false;
}

void __cxa_begin_cleanup(void* /*unwind_exception*/) {
}

void __cxa_call_unexpected(void* /*unwind_exception*/) {
}

} // extern "C"
