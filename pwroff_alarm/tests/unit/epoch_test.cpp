// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "pwroff_alarm/epoch.h"

namespace pwroff {

TEST(ParseEpoch, AcceptsPlainDecimal) {
    int64_t v = -1;
    EXPECT_EQ(ParseEpoch("1791188633", &v), ParseResult::kOk);
    EXPECT_EQ(v, 1791188633);
}

TEST(ParseEpoch, ZeroIsValid) {
    int64_t v = -1;
    EXPECT_EQ(ParseEpoch("0", &v), ParseResult::kOk);
    EXPECT_EQ(v, 0);
}

TEST(ParseEpoch, LeadingZerosAreDecimalNotOctal) {
    int64_t v = -1;
    EXPECT_EQ(ParseEpoch("0010", &v), ParseResult::kOk);
    EXPECT_EQ(v, 10);
}

TEST(ParseEpoch, EmptyIsReportedSeparately) {
    int64_t v = 7;
    EXPECT_EQ(ParseEpoch("", &v), ParseResult::kEmpty);
    EXPECT_EQ(v, 7);  // untouched on failure
}

TEST(ParseEpoch, RejectsSignsWhitespaceAndSuffixes) {
    for (const char* bad : {"-1", "+5", " 5", "5 ", "5s", "0x10", "1e9", "12.5", "٣"}) {
        int64_t v = 7;
        EXPECT_EQ(ParseEpoch(bad, &v), ParseResult::kNotNumeric) << bad;
        EXPECT_EQ(v, 7) << bad;
    }
}

TEST(ParseEpoch, RejectsNewlineInjection) {
    int64_t v = 7;
    EXPECT_EQ(ParseEpoch("1791188633\n", &v), ParseResult::kNotNumeric);
}

TEST(ParseEpoch, Int64MaxParsesAndOverflowIsRejected) {
    int64_t v = 0;
    EXPECT_EQ(ParseEpoch("9223372036854775807", &v), ParseResult::kOk);
    EXPECT_EQ(v, INT64_MAX);
    EXPECT_EQ(ParseEpoch("9223372036854775808", &v), ParseResult::kOutOfRange);
    EXPECT_EQ(ParseEpoch("99999999999999999999999", &v), ParseResult::kOutOfRange);
}

TEST(ParseEpoch, NonDigitBeatsOverflow) {
    int64_t v = 0;
    EXPECT_EQ(ParseEpoch("99999999999999999999999x", &v), ParseResult::kNotNumeric);
}

TEST(MaxArmableEpoch, NeverExceedsRtcRange) {
    EXPECT_LE(MaxArmableEpoch(), kRtcMaxEpoch);
    EXPECT_GT(MaxArmableEpoch(), 1791188633);  // this decade must be armable
}

TEST(Classify, ZeroClears) {
    EXPECT_EQ(Classify(0, 1000), Action::kClear);
}

TEST(Classify, DueOrPastIsExpired) {
    const int64_t now = 1000000;
    EXPECT_EQ(Classify(now - 3600, now), Action::kExpired);
    EXPECT_EQ(Classify(now, now), Action::kExpired);
    EXPECT_EQ(Classify(1, now), Action::kExpired);
}

TEST(Classify, MinimumLeadBoundary) {
    const int64_t now = 1000000;
    EXPECT_EQ(Classify(now + kMinLeadSeconds, now), Action::kExpired);
    EXPECT_EQ(Classify(now + kMinLeadSeconds + 1, now), Action::kArm);
}

TEST(Classify, FarFutureIsRejectedNotArmed) {
    const int64_t now = 1000000;
    EXPECT_EQ(Classify(MaxArmableEpoch(), now), Action::kArm);
    if (MaxArmableEpoch() < INT64_MAX) {
        EXPECT_EQ(Classify(MaxArmableEpoch() + 1, now), Action::kTooFar);
    }
    EXPECT_EQ(Classify(INT64_MAX, now), Action::kTooFar);
}

TEST(Classify, ExtremeNowDoesNotOverflow) {
    // epoch - lead must not wrap for hostile clock values.
    EXPECT_EQ(Classify(INT64_MIN + 1, 0), Action::kExpired);
    EXPECT_EQ(Classify(10, INT64_MAX), Action::kExpired);
}

}  // namespace pwroff
