// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "pwroff_alarm/alarm_abi.h"

namespace pwroff {

// The three syscalls the helper needs, behind an interface so the ioctl numbers
// and payload bytes can be verified without a real /dev/alarm.
// All methods follow libc conventions: -1 with errno set on failure.
class SyscallApi {
  public:
    virtual ~SyscallApi() = default;
    virtual int Open(const char* path, int flags) = 0;
    virtual int Ioctl(int fd, IoctlRequest request, const void* arg) = 0;
    virtual void Close(int fd) = 0;
};

class RealSyscalls final : public SyscallApi {
  public:
    int Open(const char* path, int flags) override;
    int Ioctl(int fd, IoctlRequest request, const void* arg) override;
    void Close(int fd) override;
};

}  // namespace pwroff
