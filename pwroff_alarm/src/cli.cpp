// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include "pwroff_alarm/cli.h"

#include <cstdint>
#include <string>

#include "pwroff_alarm/alarm_device.h"
#include "pwroff_alarm/epoch.h"
#include "pwroff_alarm/log.h"

namespace pwroff {

namespace {

// init starts one helper per property write, so two may overlap. Each one keeps
// re-reading the property after applying it; five rounds is far more than the
// framework ever produces and still bounds the loop.
constexpr int kMaxSyncRounds = 5;
constexpr size_t kMaxLoggedValue = 32;

// Property values are untrusted text: keep logs single-line and short.
std::string Printable(const std::string& value) {
    std::string out;
    for (char c : value) {
        if (out.size() >= kMaxLoggedValue) {
            out += "...";
            break;
        }
        out += (c >= 0x20 && c < 0x7f) ? c : '?';
    }
    return out;
}

ExitCode Usage() {
    Log(LogLevel::kError, "usage: pwroff_alarm [--device PATH] set <epoch> | clear | sync");
    return ExitCode::kUsage;
}

ExitCode ClearAlarm(AlarmDevice& device, const char* reason) {
    const Status status = device.ClearPowerOn();
    if (!status.ok()) {
        Log(LogLevel::kError, "clear failed: %s", status.ToString().c_str());
        return ExitCode::kDeviceError;
    }
    Log(LogLevel::kInfo, "power-on alarm cleared (%s)", reason);
    return ExitCode::kOk;
}

// Validates `text` as an epoch and applies it. Invalid or out-of-range input
// never reaches the device, so a bad write cannot disturb an armed alarm.
ExitCode ApplyEpoch(const std::string& text, AlarmDevice& device, const Clock& clock) {
    int64_t epoch = 0;
    if (ParseEpoch(text, &epoch) != ParseResult::kOk) {
        Log(LogLevel::kError, "rejected epoch '%s'", Printable(text).c_str());
        return ExitCode::kInvalidEpoch;
    }
    switch (Classify(epoch, clock.NowSeconds())) {
        case Action::kClear:
            return ClearAlarm(device, "requested");
        case Action::kExpired:
            // The requested alarm is due or past; make sure nothing stale stays armed.
            return ClearAlarm(device, "expired");
        case Action::kTooFar:
            Log(LogLevel::kError, "epoch %lld is beyond the RTC range", static_cast<long long>(epoch));
            return ExitCode::kInvalidEpoch;
        case Action::kArm:
            break;
    }
    const Status status = device.SetPowerOn(epoch);
    if (!status.ok()) {
        Log(LogLevel::kError, "arm failed: %s", status.ToString().c_str());
        return ExitCode::kDeviceError;
    }
    Log(LogLevel::kInfo, "power-on alarm armed for %lld", static_cast<long long>(epoch));
    return ExitCode::kOk;
}

ExitCode Sync(AlarmDevice& device, const Deps& deps) {
    std::string applied;
    bool have_applied = false;
    for (int round = 0; round < kMaxSyncRounds; ++round) {
        const std::string value = deps.properties.Get(kEpochProperty);
        if (value.empty()) {
            // Unset is "no request", not "disarm": never drop a live alarm on a
            // transient read.
            Log(LogLevel::kInfo, "%s unset, nothing to do", kEpochProperty);
            return ExitCode::kOk;
        }
        if (have_applied && value == applied) {
            return ExitCode::kOk;  // converged
        }
        const ExitCode rc = ApplyEpoch(value, device, deps.clock);
        if (rc != ExitCode::kOk) {
            return rc;
        }
        applied = value;
        have_applied = true;
    }
    // Still changing: the writes that changed it each start their own helper.
    Log(LogLevel::kWarn, "%s kept changing; last write wins", kEpochProperty);
    return ExitCode::kOk;
}

}  // namespace

ExitCode RunCli(const std::vector<std::string>& args, const Deps& deps) {
    std::string device_path = kDefaultAlarmDevice;
    std::vector<std::string> positional;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--device") {
            if (i + 1 >= args.size()) {
                return Usage();
            }
            device_path = args[++i];
        } else {
            positional.push_back(args[i]);
        }
    }
    if (positional.empty()) {
        return Usage();
    }

    KernelAlarmDevice device(&deps.syscalls, device_path);
    const std::string& command = positional[0];
    if (command == "set" && positional.size() == 2) {
        return ApplyEpoch(positional[1], device, deps.clock);
    }
    if (command == "clear" && positional.size() == 1) {
        return ClearAlarm(device, "requested");
    }
    if (command == "sync" && positional.size() == 1) {
        return Sync(device, deps);
    }
    return Usage();
}

}  // namespace pwroff
