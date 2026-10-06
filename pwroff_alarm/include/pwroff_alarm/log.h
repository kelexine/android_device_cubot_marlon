// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace pwroff {

enum class LogLevel { kInfo, kWarn, kError };

// printf-style logging: logcat tag "pwroff_alarm" on Android, stderr on host.
void Log(LogLevel level, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

}  // namespace pwroff
