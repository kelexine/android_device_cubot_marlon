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
    size_t length() const;
    char* lockBuffer(size_t size);
    void unlockBuffer();

    void toUpper();
    void toUpper(size_t start, size_t num);
};

void String8::toUpper() {
    toUpper(0, length());
}

void String8::toUpper(size_t start, size_t num) {
    const size_t len = length();
    if (start >= len) {
        return;
    }
    if (num > len - start) {
        num = len - start;
    }
    char* buf = lockBuffer(len);
    if (!buf) {
        return;
    }
    for (size_t i = start; i < start + num; i++) {
        buf[i] = static_cast<char>(toupper(static_cast<unsigned char>(buf[i])));
    }
    unlockBuffer();
}

} // namespace android
