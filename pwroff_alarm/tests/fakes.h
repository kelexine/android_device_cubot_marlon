// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0
//
// Test doubles shared by the unit and integration suites.

#pragma once

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "pwroff_alarm/alarm_abi.h"
#include "pwroff_alarm/alarm_device.h"
#include "pwroff_alarm/clock.h"
#include "pwroff_alarm/property_source.h"
#include "pwroff_alarm/syscalls.h"

namespace pwroff::testing {

class FakeClock final : public Clock {
  public:
    explicit FakeClock(int64_t now) : now_(now) {}
    int64_t NowSeconds() const override { return now_; }

  private:
    int64_t now_;
};

// Property source whose value can change between reads.
class FakeProperties final : public PropertySource {
  public:
    // Called after every Get(); lets a test mutate the property mid-sync.
    std::function<void(int read_count)> on_read;

    void Set(const std::string& name, const std::string& value) { values_[name] = value; }
    int reads() const { return reads_; }

    std::string Get(const std::string& name) const override {
        const auto it = values_.find(name);
        const std::string value = it == values_.end() ? std::string() : it->second;
        ++reads_;
        if (on_read) {
            const_cast<FakeProperties*>(this)->on_read(reads_);
        }
        return value;
    }

  private:
    std::map<std::string, std::string> values_;
    mutable int reads_ = 0;
};

struct IoctlCall {
    IoctlRequest request;
    bool has_payload;
    int64_t tv_sec;
    int64_t tv_nsec;
};

// Records what reaches the kernel boundary. Failure injection is per call.
class FakeSyscalls final : public SyscallApi {
  public:
    int open_errno = 0;                 // non-zero: Open() fails with this errno
    int ioctl_errno = 0;                // non-zero: Ioctl() fails with this errno
    int eintr_failures_before_ok = 0;   // Ioctl() returns EINTR this many times first

    std::vector<std::string> opened;
    std::vector<int> closed;
    std::vector<IoctlCall> ioctls;
    std::vector<int> open_flags;

    int Open(const char* path, int flags) override {
        opened.emplace_back(path);
        open_flags.push_back(flags);
        if (open_errno != 0) {
            errno = open_errno;
            return -1;
        }
        return next_fd_++;
    }

    int Ioctl(int /*fd*/, IoctlRequest request, const void* arg) override {
        IoctlCall call{request, arg != nullptr, 0, 0};
        if (arg != nullptr) {
            struct timespec ts {};
            std::memcpy(&ts, arg, sizeof(ts));
            call.tv_sec = static_cast<int64_t>(ts.tv_sec);
            call.tv_nsec = static_cast<int64_t>(ts.tv_nsec);
        }
        ioctls.push_back(call);
        if (eintr_failures_before_ok > 0) {
            --eintr_failures_before_ok;
            errno = EINTR;
            return -1;
        }
        if (ioctl_errno != 0) {
            errno = ioctl_errno;
            return -1;
        }
        return 0;
    }

    void Close(int fd) override { closed.push_back(fd); }

    // Last effect on the "hardware": +epoch armed, 0 cleared, -1 never touched.
    int64_t ArmedEpoch() const {
        int64_t state = -1;
        for (const IoctlCall& call : ioctls) {
            if (call.request == kAlarmSetPowerOn) {
                state = call.tv_sec;
            } else if (call.request == kAlarmClearPowerOn) {
                state = 0;
            }
        }
        return state;
    }

  private:
    int next_fd_ = 100;
};

}  // namespace pwroff::testing
