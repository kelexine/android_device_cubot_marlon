// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <fcntl.h>

#include <cerrno>
#include <limits>

#include "../fakes.h"
#include "pwroff_alarm/alarm_device.h"

namespace pwroff {
namespace {

using testing::FakeSyscalls;

TEST(AlarmAbi, IoctlNumbersMatchTheKernelHeader) {
    // Verified live on the P50: dmesg showed "alarm_dev: alarm 6 set ..." for
    // these numbers. 0x40106162 assumes a 16-byte timespec (64-bit userspace).
    if (sizeof(struct timespec) == 16) {
        EXPECT_EQ(static_cast<unsigned long>(kAlarmSetPowerOn), 0x40106162ul);
    } else if (sizeof(struct timespec) == 8) {
        EXPECT_EQ(static_cast<unsigned long>(kAlarmSetPowerOn), 0x40086162ul);  // compat
    }
    EXPECT_EQ(static_cast<unsigned long>(kAlarmClearPowerOn), 0x6160ul);
}

TEST(KernelAlarmDevice, SetSendsExactTimespecAndClosesFd) {
    FakeSyscalls sys;
    KernelAlarmDevice dev(&sys, "/dev/alarm");
    ASSERT_TRUE(dev.SetPowerOn(1791188633).ok());

    ASSERT_EQ(sys.opened.size(), 1u);
    EXPECT_EQ(sys.opened[0], "/dev/alarm");
    EXPECT_EQ(sys.open_flags[0] & O_ACCMODE, O_RDWR);
    EXPECT_NE(sys.open_flags[0] & O_CLOEXEC, 0);
    ASSERT_EQ(sys.ioctls.size(), 1u);
    EXPECT_EQ(sys.ioctls[0].request, kAlarmSetPowerOn);
    EXPECT_TRUE(sys.ioctls[0].has_payload);
    EXPECT_EQ(sys.ioctls[0].tv_sec, 1791188633);
    EXPECT_EQ(sys.ioctls[0].tv_nsec, 0);
    ASSERT_EQ(sys.closed.size(), 1u);
}

TEST(KernelAlarmDevice, ClearSendsNoPayload) {
    FakeSyscalls sys;
    KernelAlarmDevice dev(&sys, "/dev/alarm");
    ASSERT_TRUE(dev.ClearPowerOn().ok());
    ASSERT_EQ(sys.ioctls.size(), 1u);
    EXPECT_EQ(sys.ioctls[0].request, kAlarmClearPowerOn);
    EXPECT_FALSE(sys.ioctls[0].has_payload);
    EXPECT_EQ(sys.closed.size(), 1u);
}

TEST(KernelAlarmDevice, OpenFailureReportsErrnoAndSkipsIoctl) {
    FakeSyscalls sys;
    sys.open_errno = EACCES;
    KernelAlarmDevice dev(&sys, "/dev/alarm");
    const Status s = dev.SetPowerOn(1791188633);
    EXPECT_FALSE(s.ok());
    EXPECT_EQ(s.err(), EACCES);
    EXPECT_NE(s.ToString().find("open /dev/alarm"), std::string::npos);
    EXPECT_TRUE(sys.ioctls.empty());
    EXPECT_TRUE(sys.closed.empty());  // nothing to close
}

TEST(KernelAlarmDevice, IoctlFailureStillClosesFd) {
    FakeSyscalls sys;
    sys.ioctl_errno = EINVAL;
    KernelAlarmDevice dev(&sys, "/dev/alarm");
    const Status s = dev.ClearPowerOn();
    EXPECT_EQ(s.err(), EINVAL);
    EXPECT_EQ(sys.closed.size(), 1u);
}

TEST(KernelAlarmDevice, RetriesEintrThenSucceeds) {
    FakeSyscalls sys;
    sys.eintr_failures_before_ok = 3;
    KernelAlarmDevice dev(&sys, "/dev/alarm");
    EXPECT_TRUE(dev.SetPowerOn(1791188633).ok());
    EXPECT_EQ(sys.ioctls.size(), 4u);
    EXPECT_EQ(sys.opened.size(), 1u);  // one open for all retries
}

TEST(KernelAlarmDevice, GivesUpAfterBoundedEintr) {
    FakeSyscalls sys;
    sys.eintr_failures_before_ok = 1000;
    KernelAlarmDevice dev(&sys, "/dev/alarm");
    const Status s = dev.SetPowerOn(1791188633);
    EXPECT_EQ(s.err(), EINTR);
    EXPECT_EQ(sys.ioctls.size(), 6u);  // first try + 5 retries
    EXPECT_EQ(sys.closed.size(), 1u);
}

TEST(KernelAlarmDevice, RejectsNegativeEpochWithoutOpening) {
    FakeSyscalls sys;
    KernelAlarmDevice dev(&sys, "/dev/alarm");
    EXPECT_EQ(dev.SetPowerOn(-1).err(), ERANGE);
    EXPECT_TRUE(sys.opened.empty());
}

TEST(KernelAlarmDevice, RejectsEpochBeyondTimeT) {
    if (sizeof(time_t) >= sizeof(int64_t)) {
        GTEST_SKIP() << "time_t is 64-bit; every int64 epoch fits";
    }
    FakeSyscalls sys;
    KernelAlarmDevice dev(&sys, "/dev/alarm");
    // Only reachable with a 32-bit time_t, where INT32_MAX + 1 no longer fits.
    const int64_t too_big = static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1;
    EXPECT_EQ(dev.SetPowerOn(too_big).err(), ERANGE);
    EXPECT_TRUE(sys.opened.empty());
}

TEST(KernelAlarmDevice, UsesTheConfiguredPath) {
    FakeSyscalls sys;
    KernelAlarmDevice dev(&sys, "/tmp/fake-alarm");
    ASSERT_TRUE(dev.ClearPowerOn().ok());
    EXPECT_EQ(sys.opened.at(0), "/tmp/fake-alarm");
}

}  // namespace
}  // namespace pwroff
