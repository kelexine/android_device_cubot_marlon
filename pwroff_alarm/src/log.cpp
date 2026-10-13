// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include "pwroff_alarm/log.h"

#include <cstdarg>
#include <cstdio>

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace pwroff {

namespace {
constexpr const char* kTag = "pwroff_alarm";
}  // namespace

void Log(LogLevel level, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
#ifdef __ANDROID__
    int prio = ANDROID_LOG_INFO;
    if (level == LogLevel::kWarn) {
        prio = ANDROID_LOG_WARN;
    } else if (level == LogLevel::kError) {
        prio = ANDROID_LOG_ERROR;
    }
    __android_log_vprint(prio, kTag, fmt, ap);
#else
    const char* label = "I";
    if (level == LogLevel::kWarn) {
        label = "W";
    } else if (level == LogLevel::kError) {
        label = "E";
    }
    std::fprintf(stderr, "%s/%s: ", label, kTag);
    std::vfprintf(stderr, fmt, ap);
    std::fputc('\n', stderr);
#endif
    va_end(ap);
}

}  // namespace pwroff
