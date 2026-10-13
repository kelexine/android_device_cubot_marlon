// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <sys/ioctl.h>
#include <time.h>

namespace pwroff {

inline constexpr const char* kDefaultAlarmDevice = "/dev/alarm";

// ANDROID_ALARM_POWER_ON: arms the PMIC power-on alarm *without* the boot logo
// flag, which is the variant stock LK turns into boot mode 7 (ALARM_BOOT).
inline constexpr int kAlarmTypePowerOn = 6;

#ifdef __ANDROID__
using IoctlRequest = int;  // bionic: int ioctl(int, int, ...)
#else
using IoctlRequest = unsigned long;  // glibc: int ioctl(int, unsigned long, ...)
#endif

// Mirrors ALARM_IO / ALARM_IOW from the kernel's drivers/staging/android/
// android_alarm.h: _IO[W]('a', nr | (type << 4), ...). The timespec size is the
// userspace one on purpose, so a 32-bit build lands on the compat ioctl numbers.
inline constexpr IoctlRequest kAlarmSetPowerOn =
        _IOW('a', 2 | (kAlarmTypePowerOn << 4), struct timespec);
inline constexpr IoctlRequest kAlarmClearPowerOn = _IO('a', 0 | (kAlarmTypePowerOn << 4));

}  // namespace pwroff
