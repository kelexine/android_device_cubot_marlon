// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include "pwroff_alarm/alarm_device.h"

#include <fcntl.h>

#include <cerrno>
#include <ctime>
#include <limits>
#include <utility>

namespace pwroff {

namespace {

// EINTR can hit a slow ioctl during boot; a handful of retries is plenty.
constexpr int kMaxEintrRetries = 5;

// Closes the descriptor on every exit path.
class ScopedFd {
  public:
    ScopedFd(SyscallApi* syscalls, int fd) : syscalls_(syscalls), fd_(fd) {}
    ~ScopedFd() {
        if (fd_ >= 0) {
            syscalls_->Close(fd_);
        }
    }
    ScopedFd(const ScopedFd&) = delete;
    ScopedFd& operator=(const ScopedFd&) = delete;

    int get() const { return fd_; }

  private:
    SyscallApi* syscalls_;
    int fd_;
};

}  // namespace

KernelAlarmDevice::KernelAlarmDevice(SyscallApi* syscalls, std::string path)
    : syscalls_(syscalls), path_(std::move(path)) {}

Status KernelAlarmDevice::Issue(IoctlRequest request, const void* arg, const char* what) {
    // alarm_ioctl() rejects descriptors opened read-only, so O_RDWR is required.
    ScopedFd fd(syscalls_, syscalls_->Open(path_.c_str(), O_RDWR | O_CLOEXEC));
    if (fd.get() < 0) {
        return Status::Errno(errno, "open " + path_);
    }
    for (int attempt = 0; attempt <= kMaxEintrRetries; ++attempt) {
        if (syscalls_->Ioctl(fd.get(), request, arg) == 0) {
            return Status::Ok();
        }
        if (errno != EINTR) {
            return Status::Errno(errno, std::string("ioctl ") + what);
        }
    }
    return Status::Errno(EINTR, std::string("ioctl ") + what);
}

Status KernelAlarmDevice::SetPowerOn(int64_t epoch) {
    if (epoch < 0 || epoch > static_cast<int64_t>(std::numeric_limits<time_t>::max())) {
        return Status::Errno(ERANGE, "power-on epoch");
    }
    struct timespec ts {};
    ts.tv_sec = static_cast<time_t>(epoch);
    ts.tv_nsec = 0;
    return Issue(kAlarmSetPowerOn, &ts, "ANDROID_ALARM_SET(POWER_ON)");
}

Status KernelAlarmDevice::ClearPowerOn() {
    return Issue(kAlarmClearPowerOn, nullptr, "ANDROID_ALARM_CLEAR(POWER_ON)");
}

}  // namespace pwroff
