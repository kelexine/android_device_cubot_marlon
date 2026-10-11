/*
 * File: libshim_unwindstack.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: Unwindstack compatibility shim for Elf::GetRelPc and Regs::Read.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>

extern "C" {

/*
 * unwindstack::Elf::GetRelPc definitions
 */
uint64_t _ZN11unwindstack3Elf8GetRelPcEmPNS_7MapInfoE(uint64_t step_pc, void* /*map_info*/) {
    return step_pc;
}

uint64_t _ZN11unwindstack3Elf8GetRelPcEmPKNS_7MapInfoE(uint64_t step_pc, const void* /*map_info*/) {
    return step_pc;
}

uint64_t _ZN11unwindstack3Elf8GetRelPcEyPNS_7MapInfoE(uint64_t step_pc, void* /*map_info*/) {
    return step_pc;
}

uint64_t _ZN11unwindstack3Elf8GetRelPcEyPKNS_7MapInfoE(uint64_t step_pc, const void* /*map_info*/) {
    return step_pc;
}

/*
 * unwindstack::RegsArm::Read and RegsArm64::Read definitions
 */
bool _ZN11unwindstack7RegsArm4ReadEPKv(void* /*this_ptr*/, const void* /*data*/) {
    return false;
}

bool _ZN11unwindstack7RegsArm4ReadEPv(void* /*this_ptr*/, void* /*data*/) {
    return false;
}

bool _ZN11unwindstack9RegsArm644ReadEPKv(void* /*this_ptr*/, const void* /*data*/) {
    return false;
}

bool _ZN11unwindstack9RegsArm644ReadEPv(void* /*this_ptr*/, void* /*data*/) {
    return false;
}

} // extern "C"
