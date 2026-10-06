// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include "pwroff_alarm/clock.h"

#include <ctime>

namespace pwroff {

int64_t SystemClock::NowSeconds() const {
    struct timespec ts {};
    // CLOCK_REALTIME with a valid pointer cannot fail on Linux/bionic.
    clock_gettime(CLOCK_REALTIME, &ts);
    return static_cast<int64_t>(ts.tv_sec);
}

}  // namespace pwroff
