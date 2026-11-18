/*
 * Script/File: boot_kmsg/src/phase.h
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Boot phase detection. init flips vendor.boot_kmsg.phase to
 *          "booted" on sys.boot_completed=1; the collector only reads it.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BOOT_KMSG_PHASE_H
#define BOOT_KMSG_PHASE_H

/* Must match init.target.rc and sepolicy/vendor/property_contexts. */
#define BOOT_PHASE_PROP "vendor.boot_kmsg.phase"
#define BOOT_PHASE_BOOTED "booted"
#define PHASE_VALUE_MAX 92 /* PROP_VALUE_MAX */

typedef enum {
    PHASE_BOOTING = 0,
    PHASE_BOOTED = 1,
} boot_phase_t;

/* Fills `value` (PHASE_VALUE_MAX bytes) for `key`; returns its length. */
typedef int (*prop_reader_fn)(const char *key, char *value);

/* PHASE_BOOTED only when the property is exactly "booted". A missing
 * property, a NULL reader or any other value means PHASE_BOOTING. */
boot_phase_t phase_read(prop_reader_fn reader);

#endif /* BOOT_KMSG_PHASE_H */
