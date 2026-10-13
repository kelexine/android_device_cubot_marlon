/*
 * File: libshim_ril.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-12
 * Purpose: CallStack and String8 shim for MTK RIL (libmtk-ril.so).
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>
#include <ctype.h>

namespace android {

class Printer;

class CallStack {
public:
    CallStack();
    ~CallStack();
    void update(int ignoreDepth = 1, int maxDepth = 31);
    void print(Printer& printer) const;
};

CallStack::CallStack() {}
CallStack::~CallStack() {}
void CallStack::update(int, int) {}
void CallStack::print(Printer&) const {}

class String8 {
public:
    void toUpper();
    void toUpper(size_t start, size_t num);
};

void String8::toUpper() {}
void String8::toUpper(size_t, size_t) {}

} // namespace android
