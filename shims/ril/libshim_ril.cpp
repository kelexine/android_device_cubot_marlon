/*
 * File: libshim_ril.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-12
 * Purpose: CallStack shim for MTK RIL (libmtk-ril.so).
 * SPDX-License-Identifier: Apache-2.0
 */

#include <utils/CallStack.h>
#include <utils/Printer.h>

namespace android {

// Modern Android removed CallStack::print(Printer&)
// Stub or format the callstack using String8 dump
void CallStack::print(Printer& printer) const {
    String8 str = toString();
    printer.printLine(str.c_str());
}

} // namespace android
