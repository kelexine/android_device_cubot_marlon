/*
 * Script/File: boot_kmsg/src/logcat_proc.h
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Supervised `logcat -f` child. Can hand over to a new output path
 *          without a gap, and respawns itself (resuming from the last time it
 *          was seen alive) if logcat or logd dies.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BOOT_KMSG_LOGCAT_PROC_H
#define BOOT_KMSG_LOGCAT_PROC_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <time.h>

#define LC_PATH_MAX 512

typedef struct {
    const char *bin;           /* logcat executable */
    pid_t pid;                 /* -1 when not running */
    bool enabled;              /* false after logcat_stop(): never respawn */
    char path[LC_PATH_MAX];    /* active -f target */
    int rotate_kib;            /* -r */
    int rotate_files;          /* -n */
    long last_spawn_ms;        /* monotonic, for respawn backoff */
    struct timespec last_alive; /* CLOCK_REALTIME, last time seen running */
} logcat_proc_t;

/* "/x/boot-kmsg.log" -> "/x/boot-logcat.log"; paths without "kmsg.log" get
 * ".logcat" appended. 0 on success, -1 if `out` is too small. */
int logcat_derive_path(const char *kmsg_path, char *out, size_t n);

/* Formats a logcat -T argument ("sec.mmm", epoch) for `ts` minus margin. */
void logcat_format_since(char *out, size_t n, const struct timespec *ts, int margin_secs);

void logcat_init(logcat_proc_t *lp, const char *bin);

/* Starts logcat streaming the full buffer to `path`. 0 / -1. */
int logcat_start(logcat_proc_t *lp, const char *path, int rotate_kib, int rotate_files,
                 long now_ms);

/* Starts a new logcat at `new_path` (from now - margin) BEFORE stopping the
 * old one, so no line is lost; a few lines near the cut may appear in both.
 * On failure the old logcat keeps running and -1 is returned. */
int logcat_switch(logcat_proc_t *lp, const char *new_path, int rotate_kib,
                  int rotate_files, long now_ms);

/* Call periodically: notes liveness and respawns a dead logcat (backoff). */
void logcat_supervise(logcat_proc_t *lp, long now_ms);

/* SIGTERM, short grace, then SIGKILL; reaps the child. */
void logcat_stop(logcat_proc_t *lp);

#endif /* BOOT_KMSG_LOGCAT_PROC_H */
