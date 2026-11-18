/*
 * Script/File: boot_kmsg/src/phase.c
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Boot phase detection from vendor.boot_kmsg.phase.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "phase.h"

#include <string.h>

boot_phase_t phase_read(prop_reader_fn reader) {
    char value[PHASE_VALUE_MAX] = {0};
    int len;

    if (reader == NULL) {
        return PHASE_BOOTING;
    }
    len = reader(BOOT_PHASE_PROP, value);
    value[PHASE_VALUE_MAX - 1] = '\0';
    if (len > 0 && strcmp(value, BOOT_PHASE_BOOTED) == 0) {
        return PHASE_BOOTED;
    }
    return PHASE_BOOTING;
}
