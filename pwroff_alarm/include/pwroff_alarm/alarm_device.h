// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <string>

#include "pwroff_alarm/status.h"
#include "pwroff_alarm/syscalls.h"

namespace pwroff {

// The PMIC power-on alarm as seen by policy code.
class AlarmDevice {
  public:
    virtual ~AlarmDevice() = default;
    // Arms a one-shot power-on alarm at `epoch` (UTC seconds). The caller has
    // already range-checked it; the device still refuses values time_t can't hold.
    virtual Status SetPowerOn(int64_t epoch) = 0;
    // Disarms the power-on alarm. Safe to call when nothing is armed.
    virtual Status ClearPowerOn() = 0;
};

// Talks to the kernel's /dev/alarm (drivers/staging/android/alarm-dev.c). The
// node is opened per operation and closed immediately: the kernel keeps the
// power-on time in PMIC registers, and a long-lived fd would only pin
// alarm_opened for no benefit.
class KernelAlarmDevice final : public AlarmDevice {
  public:
    KernelAlarmDevice(SyscallApi* syscalls, std::string path);

    Status SetPowerOn(int64_t epoch) override;
    Status ClearPowerOn() override;

  private:
    Status Issue(IoctlRequest request, const void* arg, const char* what);

    SyscallApi* syscalls_;
    std::string path_;
};

}  // namespace pwroff
