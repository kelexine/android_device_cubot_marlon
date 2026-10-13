// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>
#include <vector>

#include "pwroff_alarm/clock.h"
#include "pwroff_alarm/property_source.h"
#include "pwroff_alarm/status.h"
#include "pwroff_alarm/syscalls.h"

namespace pwroff {

// Property the framework app writes (UTC epoch seconds; "0" = no alarm).
inline constexpr const char* kEpochProperty = "vendor.pwroff_alarm.epoch";

struct Deps {
    SyscallApi& syscalls;
    const Clock& clock;
    const PropertySource& properties;
};

// Usage: pwroff_alarm [--device PATH] set <epoch> | clear | sync
//   set <epoch>  arm the PMIC power-on alarm at <epoch> (0 clears)
//   clear        disarm it
//   sync         apply vendor.pwroff_alarm.epoch, re-reading it until stable so
//                concurrent invocations converge on the newest value
// `args` excludes argv[0].
ExitCode RunCli(const std::vector<std::string>& args, const Deps& deps);

}  // namespace pwroff
