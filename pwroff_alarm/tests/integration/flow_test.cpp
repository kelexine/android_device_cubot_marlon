// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0
//
// End-to-end through RunCli -> policy -> KernelAlarmDevice -> syscall seam.
// Only the kernel boundary is faked, so these cover the exact bytes a real
// /dev/alarm would receive.

#include <gtest/gtest.h>

#include <string>

#include "../fakes.h"
#include "pwroff_alarm/cli.h"

namespace pwroff {
namespace {

using testing::FakeClock;
using testing::FakeProperties;
using testing::FakeSyscalls;

constexpr int64_t kNow = 1791188000;

TEST(Flow, AlarmLifecycleSetMoveClear) {
    FakeSyscalls sys;
    FakeClock clock(kNow);
    FakeProperties props;
    const Deps deps{sys, clock, props};

    // Clock app sets 07:00, then the user moves it, then deletes it.
    props.Set(kEpochProperty, std::to_string(kNow + 3600));
    ASSERT_EQ(RunCli({"sync"}, deps), ExitCode::kOk);
    EXPECT_EQ(sys.ArmedEpoch(), kNow + 3600);

    props.Set(kEpochProperty, std::to_string(kNow + 7200));
    ASSERT_EQ(RunCli({"sync"}, deps), ExitCode::kOk);
    EXPECT_EQ(sys.ArmedEpoch(), kNow + 7200);

    props.Set(kEpochProperty, "0");
    ASSERT_EQ(RunCli({"sync"}, deps), ExitCode::kOk);
    EXPECT_EQ(sys.ArmedEpoch(), 0);
}

TEST(Flow, EveryOperationOpensAndClosesExactlyOnce) {
    FakeSyscalls sys;
    FakeClock clock(kNow);
    FakeProperties props;
    const Deps deps{sys, clock, props};

    ASSERT_EQ(RunCli({"set", std::to_string(kNow + 600)}, deps), ExitCode::kOk);
    ASSERT_EQ(RunCli({"clear"}, deps), ExitCode::kOk);
    EXPECT_EQ(sys.opened.size(), 2u);
    EXPECT_EQ(sys.closed.size(), 2u);
}

TEST(Flow, AlarmBootConsumedThenRearmedForTheNextOne) {
    FakeSyscalls sys;
    FakeClock before(kNow + 100);
    FakeProperties props;

    // The armed alarm is already due after the alarm boot: the app republishes
    // the *next* alarm and the stale one must not be re-armed.
    props.Set(kEpochProperty, std::to_string(kNow));  // due: expired
    ASSERT_EQ(RunCli({"sync"}, Deps{sys, before, props}), ExitCode::kOk);
    EXPECT_EQ(sys.ArmedEpoch(), 0);

    props.Set(kEpochProperty, std::to_string(kNow + 86400));  // tomorrow
    ASSERT_EQ(RunCli({"sync"}, Deps{sys, before, props}), ExitCode::kOk);
    EXPECT_EQ(sys.ArmedEpoch(), kNow + 86400);
}

TEST(Flow, FailedDeviceDoesNotLoseTheNextAttempt) {
    FakeSyscalls sys;
    FakeClock clock(kNow);
    FakeProperties props;
    const Deps deps{sys, clock, props};
    props.Set(kEpochProperty, std::to_string(kNow + 600));

    sys.open_errno = EBUSY;  // e.g. another process holds /dev/alarm
    EXPECT_EQ(RunCli({"sync"}, deps), ExitCode::kDeviceError);
    EXPECT_EQ(sys.ArmedEpoch(), -1);

    sys.open_errno = 0;  // the next property write retries cleanly
    EXPECT_EQ(RunCli({"sync"}, deps), ExitCode::kOk);
    EXPECT_EQ(sys.ArmedEpoch(), kNow + 600);
}

TEST(Flow, InvalidWriteNeverDisturbsAnArmedAlarm) {
    FakeSyscalls sys;
    FakeClock clock(kNow);
    FakeProperties props;
    const Deps deps{sys, clock, props};

    props.Set(kEpochProperty, std::to_string(kNow + 600));
    ASSERT_EQ(RunCli({"sync"}, deps), ExitCode::kOk);
    const size_t ioctls_before = sys.ioctls.size();

    props.Set(kEpochProperty, "garbage");
    EXPECT_EQ(RunCli({"sync"}, deps), ExitCode::kInvalidEpoch);
    props.Set(kEpochProperty, "99999999999999999999");
    EXPECT_EQ(RunCli({"sync"}, deps), ExitCode::kInvalidEpoch);

    EXPECT_EQ(sys.ioctls.size(), ioctls_before);
    EXPECT_EQ(sys.ArmedEpoch(), kNow + 600);
}

}  // namespace
}  // namespace pwroff
