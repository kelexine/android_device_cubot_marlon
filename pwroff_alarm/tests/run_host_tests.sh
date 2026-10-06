#!/usr/bin/env bash
# Author: kelexine <https://github.com/kelexine>
# SPDX-License-Identifier: Apache-2.0
#
# Builds and runs the pwroff_alarm tests on a plain Linux host (no AOSP tree).
# Needs: a C++20 compiler (clang++ or g++) and gtest (libgtest-dev).
# Mirrors the flags in Android.bp: -Wall -Wextra -Werror.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(dirname "$here")"
out="${OUT_DIR:-$(mktemp -d)}"
cxx="${CXX:-$(command -v clang++ || command -v g++)}"
flags=(-std=c++20 -Wall -Wextra -Werror -Wshadow -I"$root/include" -g)

echo "== compiler: $cxx  out: $out"
for src in "$root"/src/*.cpp; do
    [ "$(basename "$src")" = main.cpp ] && continue
    "$cxx" "${flags[@]}" -c "$src" -o "$out/$(basename "$src").o"
done

echo "== unit + integration tests"
"$cxx" "${flags[@]}" "$here"/unit/*.cpp "$here"/integration/*.cpp "$out"/*.o \
    -lgtest -lgtest_main -lpthread -o "$out/pwroff_alarm_tests"
"$out/pwroff_alarm_tests" --gtest_brief=1

echo "== binary smoke test"
"$cxx" "${flags[@]}" "$root/src/main.cpp" "$out"/*.o -o "$out/pwroff_alarm"
"$here/cli_smoke.sh" "$out/pwroff_alarm"

echo "== all pwroff_alarm checks passed"
