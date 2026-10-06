// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../fakes.h"
#include "pwroff_alarm/cli.h"

namespace pwroff {
namespace {

using testing::FakeClock;
using testing::FakeProperties;
using testing::FakeSyscalls;

constexpr int64_t kNow = 1791188000;
constexpr int64_t kFuture = kNow + 3600;

struct Rig {
    FakeSyscalls sys;
    FakeClock clock{kNow};
    FakeProperties props;

    ExitCode Run(std::vector<std::string> args) {
        const Deps deps{sys, clock, props};
        return RunCli(args, deps);
    }
};

TEST(CliUsage, NoArgsIsUsageAndTouchesNothing) {
    Rig r;
    EXPECT_EQ(r.Run({}), ExitCode::kUsage);
    EXPECT_TRUE(r.sys.opened.empty());
}

TEST(CliUsage, UnknownCommandAndWrongArityAreUsage) {
    for (const auto& args : std::vector<std::vector<std::string>>{
                 {"frobnicate"},
                 {"set"},
                 {"set", "1", "2"},
                 {"clear", "extra"},
                 {"sync", "extra"},
                 {"--device"},
                 {"--device", "/x"},
         }) {
        Rig r;
        EXPECT_EQ(r.Run(args), ExitCode::kUsage);
        EXPECT_TRUE(r.sys.opened.empty());
    }
}

TEST(CliSet, ArmsFutureEpoch) {
    Rig r;
    EXPECT_EQ(r.Run({"set", std::to_string(kFuture)}), ExitCode::kOk);
    EXPECT_EQ(r.sys.ArmedEpoch(), kFuture);
}

TEST(CliSet, ZeroClears) {
    Rig r;
    EXPECT_EQ(r.Run({"set", "0"}), ExitCode::kOk);
    EXPECT_EQ(r.sys.ArmedEpoch(), 0);
}

TEST(CliSet, GarbageIsRejectedBeforeTheDevice) {
    Rig r;
    EXPECT_EQ(r.Run({"set", "tomorrow"}), ExitCode::kInvalidEpoch);
    EXPECT_EQ(r.Run({"set", "-5"}), ExitCode::kInvalidEpoch);
    EXPECT_TRUE(r.sys.opened.empty());
}

TEST(CliSet, ExpiredEpochClearsInsteadOfArming) {
    Rig r;
    EXPECT_EQ(r.Run({"set", std::to_string(kNow - 60)}), ExitCode::kOk);
    EXPECT_EQ(r.sys.ArmedEpoch(), 0);
}

TEST(CliSet, BeyondRtcRangeIsRejectedWithoutTouchingTheAlarm) {
    Rig r;
    EXPECT_EQ(r.Run({"set", "9999999999999"}), ExitCode::kInvalidEpoch);
    EXPECT_TRUE(r.sys.opened.empty());
}

TEST(CliSet, DeviceFailureMapsToDeviceError) {
    Rig r;
    r.sys.ioctl_errno = EPERM;
    EXPECT_EQ(r.Run({"set", std::to_string(kFuture)}), ExitCode::kDeviceError);
    r.sys.ioctl_errno = 0;
    r.sys.open_errno = ENOENT;
    EXPECT_EQ(r.Run({"clear"}), ExitCode::kDeviceError);
}

TEST(CliDevice, OverrideIsHonoured) {
    Rig r;
    EXPECT_EQ(r.Run({"--device", "/tmp/fake", "clear"}), ExitCode::kOk);
    EXPECT_EQ(r.sys.opened.at(0), "/tmp/fake");
}

TEST(CliSync, UnsetPropertyIsANoOpNotADisarm) {
    Rig r;
    EXPECT_EQ(r.Run({"sync"}), ExitCode::kOk);
    EXPECT_TRUE(r.sys.opened.empty());
}

TEST(CliSync, AppliesPropertyValue) {
    Rig r;
    r.props.Set(kEpochProperty, std::to_string(kFuture));
    EXPECT_EQ(r.Run({"sync"}), ExitCode::kOk);
    EXPECT_EQ(r.sys.ArmedEpoch(), kFuture);
    EXPECT_EQ(r.sys.ioctls.size(), 1u);  // stable value is applied exactly once
}

TEST(CliSync, ZeroDisarms) {
    Rig r;
    r.props.Set(kEpochProperty, "0");
    EXPECT_EQ(r.Run({"sync"}), ExitCode::kOk);
    EXPECT_EQ(r.sys.ArmedEpoch(), 0);
}

TEST(CliSync, InvalidPropertyLeavesTheArmedAlarmAlone) {
    Rig r;
    r.props.Set(kEpochProperty, "12abc");
    EXPECT_EQ(r.Run({"sync"}), ExitCode::kInvalidEpoch);
    EXPECT_TRUE(r.sys.opened.empty());
}

TEST(CliSync, ConvergesWhenThePropertyChangesMidRun) {
    Rig r;
    const std::string first = std::to_string(kFuture);
    const std::string second = std::to_string(kFuture + 600);
    r.props.Set(kEpochProperty, first);
    // A newer framework write lands right after the first read.
    r.props.on_read = [&](int count) {
        if (count == 1) {
            r.props.Set(kEpochProperty, second);
        }
    };
    EXPECT_EQ(r.Run({"sync"}), ExitCode::kOk);
    EXPECT_EQ(r.sys.ArmedEpoch(), kFuture + 600);  // newest value wins
    EXPECT_EQ(r.sys.ioctls.size(), 2u);
}

TEST(CliSync, BoundedWhenThePropertyNeverSettles) {
    Rig r;
    int64_t v = kFuture;
    r.props.Set(kEpochProperty, std::to_string(v));
    r.props.on_read = [&](int) { r.props.Set(kEpochProperty, std::to_string(++v)); };
    EXPECT_EQ(r.Run({"sync"}), ExitCode::kOk);
    EXPECT_LE(r.props.reads(), 5);
    EXPECT_LE(r.sys.ioctls.size(), 5u);
}

TEST(CliSync, HostileValuesAreSanitisedAndRejected) {
    Rig r;
    r.props.Set(kEpochProperty, std::string("17\n9\x01\x02") + std::string(200, 'x'));
    EXPECT_EQ(r.Run({"sync"}), ExitCode::kInvalidEpoch);
    EXPECT_TRUE(r.sys.opened.empty());
}

}  // namespace
}  // namespace pwroff
