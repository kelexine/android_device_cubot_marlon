// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <string_view>

namespace pwroff {

// Last second the PMIC power-on registers can represent. The kernel rejects
// tm_year > 195 (year 2095) and the RTC year field is 7 bits wide from 1968.
inline constexpr int64_t kRtcMaxEpoch = 3976214399;  // 2095-12-31T23:59:59Z

// Alarms closer than this are pointless: the device is awake, and the LK only
// accepts a power-on wake inside [T-1s, T+4s], which a tiny lead can never meet.
inline constexpr int64_t kMinLeadSeconds = 5;

enum class ParseResult { kOk, kEmpty, kNotNumeric, kOutOfRange };

// Parses unsigned decimal seconds ("0" is valid). No sign, whitespace or suffix.
ParseResult ParseEpoch(std::string_view text, int64_t* out);

// Largest epoch this build can hand to the kernel: kRtcMaxEpoch capped by time_t.
int64_t MaxArmableEpoch();

enum class Action { kArm, kClear, kExpired, kTooFar };

// Decides what to do with a requested power-on time:
//   0                    -> kClear
//   < 0 or <= now + lead -> kExpired  (caller clears, so no stale alarm can fire)
//   > MaxArmableEpoch()  -> kTooFar   (caller must not touch the device)
//   otherwise            -> kArm
Action Classify(int64_t epoch, int64_t now);

}  // namespace pwroff
