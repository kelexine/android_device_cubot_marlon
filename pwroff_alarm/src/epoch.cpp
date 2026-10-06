// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include "pwroff_alarm/epoch.h"

#include <algorithm>
#include <ctime>
#include <limits>

namespace pwroff {

ParseResult ParseEpoch(std::string_view text, int64_t* out) {
    if (text.empty()) {
        return ParseResult::kEmpty;
    }
    // Validate every character first so the result does not depend on where an
    // overflow happens relative to a stray non-digit.
    for (char c : text) {
        if (c < '0' || c > '9') {
            return ParseResult::kNotNumeric;
        }
    }
    int64_t value = 0;
    for (char c : text) {
        const int digit = c - '0';
        if (value > (std::numeric_limits<int64_t>::max() - digit) / 10) {
            return ParseResult::kOutOfRange;
        }
        value = value * 10 + digit;
    }
    *out = value;
    return ParseResult::kOk;
}

int64_t MaxArmableEpoch() {
    return std::min<int64_t>(kRtcMaxEpoch,
                             static_cast<int64_t>(std::numeric_limits<time_t>::max()));
}

Action Classify(int64_t epoch, int64_t now) {
    if (epoch == 0) {
        return Action::kClear;
    }
    // Negative values are never in the future. Handling them first also keeps
    // `epoch - kMinLeadSeconds` below from overflowing near INT64_MIN.
    if (epoch < 0) {
        return Action::kExpired;
    }
    if (epoch - kMinLeadSeconds <= now) {
        return Action::kExpired;
    }
    if (epoch > MaxArmableEpoch()) {
        return Action::kTooFar;
    }
    return Action::kArm;
}

}  // namespace pwroff
