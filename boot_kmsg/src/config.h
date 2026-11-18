/*
 * Script/File: boot_kmsg/src/config.h
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Compile-time constants, host-test seams and per-volume storage
 *          limits for the marlon boot/booted log collector.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BOOT_KMSG_CONFIG_H
#define BOOT_KMSG_CONFIG_H

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* Boot phase: kmsg + logcat go to the boot log path (argv[1]). */
#define DEFAULT_LOG_PATH "/cache/bootlog/boot-kmsg.log"
#define FALLBACK_LOG_PATH "/metadata/bootlog/boot-kmsg.log"
/* Booted phase: after vendor.boot_kmsg.phase=booted, output moves here. */
#define DEFAULT_BOOTED_DIR "/cache/booted_logs"
#define BOOTED_KMSG_NAME "booted-kmsg.log"

#define DEFAULT_TIMEOUT_SECS 30
#define BUFFER_SIZE 16384

#define PHASE_CHECK_INTERVAL_MS 250
#define SUPERVISE_INTERVAL_MS 1000
#define RESPAWN_BACKOFF_MS 5000
#define SWITCH_RETRY_MS 5000
#define SYNC_INTERVAL_BOOTING_MS 250
#define SYNC_INTERVAL_BOOTED_MS 2000
/* Seconds subtracted from "now" for logcat -T so a handover never has a gap. */
#define SINCE_MARGIN_SECS 2

/*
 * Rolling-log budgets. /cache also stages recovery OTA packages, so stay
 * modest there; /metadata is only 16 MiB and shared with OTA/GSI state.
 */
#define CACHE_KMSG_MAX_BYTES (4L * 1024 * 1024)
#define CACHE_FILES 3
#define CACHE_LOGCAT_KIB 4096
#define METADATA_KMSG_MAX_BYTES (1L * 1024 * 1024)
#define METADATA_FILES 2
#define METADATA_LOGCAT_KIB 1024

struct storage_limits {
    off_t kmsg_max_bytes; /* per kmsg file */
    int files;            /* files kept per stream, active one included */
    int logcat_kib;       /* logcat -r value */
};

static inline struct storage_limits limits_for_path(const char *path) {
    struct storage_limits lim = {CACHE_KMSG_MAX_BYTES, CACHE_FILES, CACHE_LOGCAT_KIB};
    if (path != NULL && strncmp(path, "/metadata/", 10) == 0) {
        lim.kmsg_max_bytes = METADATA_KMSG_MAX_BYTES;
        lim.files = METADATA_FILES;
        lim.logcat_kib = METADATA_LOGCAT_KIB;
    }
    return lim;
}

/* Host-test seams: only honoured when built with -DBOOT_KMSG_TESTING. */
static inline const char *cfg_kmsg_device(void) {
#ifdef BOOT_KMSG_TESTING
    const char *v = getenv("BOOT_KMSG_TEST_KMSG");
    if (v != NULL && v[0] != '\0') {
        return v;
    }
#endif
    return "/dev/kmsg";
}

static inline const char *cfg_logcat_bin(void) {
#ifdef BOOT_KMSG_TESTING
    const char *v = getenv("BOOT_KMSG_TEST_LOGCAT");
    if (v != NULL && v[0] != '\0') {
        return v;
    }
#endif
    return "/system/bin/logcat";
}

#endif /* BOOT_KMSG_CONFIG_H */
