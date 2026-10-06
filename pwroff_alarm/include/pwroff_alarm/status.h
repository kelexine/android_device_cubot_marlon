// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>
#include <utility>

namespace pwroff {

// Process exit codes, sysexits.h flavoured so init/logcat output stays greppable.
enum class ExitCode : int {
    kOk = 0,
    kUsage = 64,         // EX_USAGE: bad command line
    kInvalidEpoch = 65,  // EX_DATAERR: epoch text or value rejected
    kDeviceError = 74,   // EX_IOERR: /dev/alarm open or ioctl failed
};

// Outcome of an operation that can fail with an errno value.
class Status {
  public:
    static Status Ok() { return Status(0, std::string()); }
    static Status Errno(int err, std::string context) { return Status(err, std::move(context)); }

    bool ok() const { return err_ == 0; }
    int err() const { return err_; }
    const std::string& context() const { return context_; }

    // "<context>: <strerror>" for failures, "ok" otherwise.
    std::string ToString() const;

  private:
    Status(int err, std::string context) : err_(err), context_(std::move(context)) {}

    int err_;
    std::string context_;
};

}  // namespace pwroff
