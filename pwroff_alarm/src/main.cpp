// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0
//
// pwroff_alarm: arms/disarms the PMIC power-on alarm through /dev/alarm so a
// scheduled alarm can wake the Cubot P50 (marlon) from full power-off.

#include <string>
#include <vector>

#include "pwroff_alarm/cli.h"
#include "pwroff_alarm/clock.h"
#include "pwroff_alarm/property_source.h"
#include "pwroff_alarm/syscalls.h"

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    pwroff::RealSyscalls syscalls;
    pwroff::SystemClock clock;
    pwroff::PlatformPropertySource properties;
    const pwroff::Deps deps{syscalls, clock, properties};
    return static_cast<int>(pwroff::RunCli(args, deps));
}
