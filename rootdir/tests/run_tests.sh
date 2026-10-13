#!/usr/bin/env bash
# Author: kelexine <https://github.com/kelexine>
# SPDX-License-Identifier: Apache-2.0
#
# Static rc/sepolicy/Soong contract checks plus their mutation tests.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
python3 "$here/check_init_contracts.py"
python3 "$here/test_check_init_contracts.py"
