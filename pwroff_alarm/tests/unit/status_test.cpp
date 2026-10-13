// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <cerrno>

#include "pwroff_alarm/status.h"

namespace pwroff {

TEST(Status, OkIsOk) {
    const Status s = Status::Ok();
    EXPECT_TRUE(s.ok());
    EXPECT_EQ(s.err(), 0);
    EXPECT_EQ(s.ToString(), "ok");
}

TEST(Status, ErrnoCarriesContextAndMessage) {
    const Status s = Status::Errno(ENOENT, "open /dev/alarm");
    EXPECT_FALSE(s.ok());
    EXPECT_EQ(s.err(), ENOENT);
    EXPECT_EQ(s.context(), "open /dev/alarm");
    EXPECT_NE(s.ToString().find("open /dev/alarm: "), std::string::npos);
}

TEST(Status, EmptyContextFallsBackToError) {
    EXPECT_EQ(Status::Errno(EPERM, "").ToString().rfind("error: ", 0), 0u);
}

TEST(ExitCode, ValuesAreStableForInitAndScripts) {
    EXPECT_EQ(static_cast<int>(ExitCode::kOk), 0);
    EXPECT_EQ(static_cast<int>(ExitCode::kUsage), 64);
    EXPECT_EQ(static_cast<int>(ExitCode::kInvalidEpoch), 65);
    EXPECT_EQ(static_cast<int>(ExitCode::kDeviceError), 74);
}

}  // namespace pwroff
