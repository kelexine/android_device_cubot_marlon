// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>

namespace pwroff {

// Wall-clock source, injectable so policy code can be tested without sleeping.
class Clock {
  public:
    virtual ~Clock() = default;
    virtual int64_t NowSeconds() const = 0;
};

// CLOCK_REALTIME, the same time base the RTC is synchronised from.
class SystemClock final : public Clock {
  public:
    int64_t NowSeconds() const override;
};

}  // namespace pwroff
