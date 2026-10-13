#!/usr/bin/env bash
# Author: kelexine <https://github.com/kelexine>
# SPDX-License-Identifier: Apache-2.0
#
# Smoke-tests the real pwroff_alarm binary: exit codes and no-device behaviour.
# Usage: cli_smoke.sh <path-to-pwroff_alarm>
set -u

bin="${1:?usage: cli_smoke.sh <pwroff_alarm binary>}"
fail=0

expect() {  # expect <want-exit> <description> <cmd...>
    local want="$1" desc="$2"
    shift 2
    "$@" >/dev/null 2>&1
    local got=$?
    if [ "$got" -ne "$want" ]; then
        echo "FAIL: $desc (want exit $want, got $got)"
        fail=1
    else
        echo "ok:   $desc"
    fi
}

expect 64 "no arguments"                       "$bin"
expect 64 "unknown command"                    "$bin" frobnicate
expect 64 "set without epoch"                  "$bin" set
expect 65 "set with garbage epoch"             "$bin" set tomorrow
expect 65 "set beyond RTC range"               "$bin" set 99999999999999999
expect 74 "clear on a missing device"          "$bin" --device /nonexistent/alarm clear
expect 74 "future set on a missing device"     "$bin" --device /nonexistent/alarm set 3900000000
expect 0  "sync with property unset"           env -u PWROFF_PROP_VENDOR_PWROFF_ALARM_EPOCH "$bin" sync
expect 65 "sync with garbage property"         env PWROFF_PROP_VENDOR_PWROFF_ALARM_EPOCH=12abc "$bin" sync
expect 74 "sync arms via missing device"       env PWROFF_PROP_VENDOR_PWROFF_ALARM_EPOCH=3900000000 "$bin" --device /nonexistent/alarm sync

exit "$fail"
