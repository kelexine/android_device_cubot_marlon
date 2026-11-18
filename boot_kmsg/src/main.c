/*
 * Script/File: boot_kmsg/src/main.c
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-09 (booted-phase continuation added 2026-11-18)
 * Purpose: Kernel message (kmsg) + logcat diagnostic collector for Cubot P50
 *          (marlon). Streams from early boot; when init sets
 *          vendor.boot_kmsg.phase=booted the same process keeps running and
 *          moves its output to a separate directory (default
 *          /cache/booted_logs) instead of stopping or restarting.
 *
 * Usage: boot_kmsg [boot-kmsg-path] [timeout-secs] [booted-dir]
 *        timeout-secs: total run time, 0 = until SIGTERM.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cutils/properties.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "fsutil.h"
#include "logcat_proc.h"
#include "phase.h"
#include "rotating_file.h"

typedef struct {
    rotating_file_t kmsg;
    logcat_proc_t logcat;
    char booted_dir[RF_PATH_MAX];
    boot_phase_t phase;
    long next_phase_check_ms;
    long next_supervise_ms;
    long retry_switch_ms;
    int switch_failures;
    long last_sync_ms;
} collector_t;

static volatile sig_atomic_t g_running = 1;

static void handle_signal(int sig) {
    (void)sig;
    g_running = 0;
}

static long milliseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static int read_prop(const char *key, char *value) {
    return property_get(key, value, "");
}

static void install_signals(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal; /* no SA_RESTART: poll() must wake up */
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);
}

/* Marker line in the kmsg stream (phase changes, failures). */
static void note(collector_t *c, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void note(collector_t *c, const char *fmt, ...) {
    char line[RF_PATH_MAX + 96];
    int n;
    va_list ap;

    va_start(ap, fmt);
    n = vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    if (n > 0) {
        size_t len = (size_t)n < sizeof(line) ? (size_t)n : sizeof(line) - 1;
        rf_write(&c->kmsg, line, len);
    }
}

/* Opens the first candidate that works; returns it or NULL. */
static const char *open_first(rotating_file_t *rf, const char *const *candidates, size_t n,
                              bool truncate) {
    for (size_t i = 0; i < n; ++i) {
        struct storage_limits lim = limits_for_path(candidates[i]);

        (void)fs_ensure_parent(candidates[i]);
        if (rf_open(rf, candidates[i], lim.kmsg_max_bytes, lim.files, truncate) == 0) {
            return candidates[i];
        }
    }
    return NULL;
}

static int start_logcat(collector_t *c, long now_ms) {
    char logcat_path[RF_PATH_MAX];
    struct storage_limits lim = limits_for_path(c->kmsg.path);

    if (logcat_derive_path(c->kmsg.path, logcat_path, sizeof(logcat_path)) < 0) {
        return -1;
    }
    return logcat_start(&c->logcat, logcat_path, lim.logcat_kib, lim.files, now_ms);
}

static void write_header(collector_t *c, const char *title) {
    note(c,
         "========================================\n"
         " Marlon kmsg & logcat collector\n"
         " Device: Cubot P50 (marlon / MT6765)\n"
         " Phase: %s\n"
         "========================================\n",
         title);
    rf_sync(&c->kmsg);
}

/* Moves kmsg and logcat output to the booted directory without stopping the
 * kmsg stream (same fd, same cursor) and with a gap-free logcat handover. */
static int switch_to_booted(collector_t *c, long now_ms) {
    char kmsg_path[RF_PATH_MAX];
    char logcat_path[RF_PATH_MAX];
    struct storage_limits lim;
    int n;

    if (fs_ensure_dir(c->booted_dir) < 0) {
        return -1;
    }
    n = snprintf(kmsg_path, sizeof(kmsg_path), "%s/%s", c->booted_dir, BOOTED_KMSG_NAME);
    if (n < 0 || (size_t)n >= sizeof(kmsg_path) ||
        logcat_derive_path(kmsg_path, logcat_path, sizeof(logcat_path)) < 0) {
        errno = ENAMETOOLONG;
        return -1;
    }
    lim = limits_for_path(kmsg_path);

    note(c, "\n==== phase: booted -- output continues in %s ====\n", c->booted_dir);
    rf_sync(&c->kmsg);
    if (rf_redirect(&c->kmsg, kmsg_path, false, lim.kmsg_max_bytes, lim.files) < 0) {
        return -1;
    }
    write_header(c, "booted (continued from boot log, kmsg stream not interrupted)");

    /* A failed logcat handover keeps the old logcat alive: logging goes on. */
    if (logcat_switch(&c->logcat, logcat_path, lim.logcat_kib, lim.files, now_ms) < 0) {
        note(c, "[boot_kmsg] logcat handover failed (errno %d); still logging to old path\n",
             errno);
    }
    return 0;
}

static void tick(collector_t *c, long now_ms) {
    if (c->phase == PHASE_BOOTING && now_ms >= c->next_phase_check_ms) {
        c->next_phase_check_ms = now_ms + PHASE_CHECK_INTERVAL_MS;
        if (phase_read(read_prop) == PHASE_BOOTED && now_ms >= c->retry_switch_ms) {
            if (switch_to_booted(c, now_ms) == 0) {
                c->phase = PHASE_BOOTED;
            } else {
                int err = errno;

                c->retry_switch_ms = now_ms + SWITCH_RETRY_MS;
                if (c->switch_failures++ == 0) {
                    note(c, "[boot_kmsg] cannot switch to %s (errno %d); will retry\n",
                         c->booted_dir, err);
                }
            }
        }
    }
    if (now_ms >= c->next_supervise_ms) {
        c->next_supervise_ms = now_ms + SUPERVISE_INTERVAL_MS;
        logcat_supervise(&c->logcat, now_ms);
    }
}

static bool is_critical(const char *buf) {
    return strstr(buf, "init:") != NULL || strstr(buf, "FATAL") != NULL ||
           strstr(buf, "panic") != NULL || strstr(buf, "BUG:") != NULL ||
           strstr(buf, "Call trace:") != NULL;
}

int main(int argc, char **argv) {
    const char *target_path = DEFAULT_LOG_PATH;
    const char *booted_dir = DEFAULT_BOOTED_DIR;
    long timeout_secs = DEFAULT_TIMEOUT_SECS;
    collector_t c;
    const char *candidates[3];
    char booted_kmsg[RF_PATH_MAX];
    size_t ncand = 0;
    char buffer[BUFFER_SIZE];
    long end_time;
    int kmsg;
    int rc = 0;

    if (argc >= 2 && argv[1][0] != '\0') {
        target_path = argv[1];
    }
    if (argc >= 3) {
        timeout_secs = strtol(argv[2], NULL, 10);
    }
    if (argc >= 4 && argv[3][0] != '\0') {
        booted_dir = argv[3];
    }
    if (strlen(booted_dir) + strlen(BOOTED_KMSG_NAME) + 2 >= RF_PATH_MAX ||
        strlen(target_path) >= RF_PATH_MAX - 16) {
        return 3;
    }

    memset(&c, 0, sizeof(c));
    memcpy(c.booted_dir, booted_dir, strlen(booted_dir) + 1);
    logcat_init(&c.logcat, cfg_logcat_bin());
    install_signals();

    /* Open /dev/kmsg non-blocking and rewind to the start of the ring buffer. */
    kmsg = open(cfg_kmsg_device(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (kmsg < 0) {
        return 1;
    }
    lseek(kmsg, 0, SEEK_SET);

    /*
     * Fresh boot: start in the boot log (truncated, as before). A restart
     * after boot completed (crash) resumes straight in the booted directory
     * in append mode so earlier output is not wiped.
     */
    c.phase = phase_read(read_prop);
    if (c.phase == PHASE_BOOTED && fs_ensure_dir(c.booted_dir) == 0) {
        snprintf(booted_kmsg, sizeof(booted_kmsg), "%s/%s", c.booted_dir, BOOTED_KMSG_NAME);
        candidates[ncand++] = booted_kmsg;
    }
    candidates[ncand++] = target_path;
    candidates[ncand++] = FALLBACK_LOG_PATH;
    if (open_first(&c.kmsg, candidates, ncand, c.phase == PHASE_BOOTING) == NULL) {
        close(kmsg);
        return 2;
    }

    c.last_sync_ms = milliseconds();
    (void)start_logcat(&c, c.last_sync_ms);
    write_header(&c, c.phase == PHASE_BOOTED ? "booted (collector restarted)" : "boot");

    end_time = (timeout_secs > 0) ? (c.last_sync_ms + timeout_secs * 1000L) : 0;

    while (g_running && (end_time == 0 || milliseconds() < end_time)) {
        struct pollfd pfd = {.fd = kmsg, .events = POLLIN};
        long sync_interval;
        long now = milliseconds();
        int ret;
        ssize_t count;

        tick(&c, now);

        ret = poll(&pfd, 1, 100);
        if (ret < 0 && errno != EINTR) {
            rc = 4;
            break;
        }
        if (ret <= 0) {
            continue;
        }

        count = read(kmsg, buffer, sizeof(buffer) - 1);
        if (count > 0) {
            buffer[count] = '\0';
            /* A failed write drops the record (counted); we never exit for it. */
            rf_write(&c.kmsg, buffer, (size_t)count);

            now = milliseconds();
            sync_interval = (c.phase == PHASE_BOOTED) ? SYNC_INTERVAL_BOOTED_MS
                                                      : SYNC_INTERVAL_BOOTING_MS;
            /* Immediate sync on kernel panic / init failure messages. */
            if (now - c.last_sync_ms >= sync_interval || is_critical(buffer)) {
                rf_sync(&c.kmsg);
                c.last_sync_ms = now;
            }
        } else if (count == 0) {
            /* EOF (not seen on real /dev/kmsg): avoid spinning on it. */
            struct timespec nap = {0, 50L * 1000000L};
            nanosleep(&nap, NULL);
        } else if (errno == EPIPE) {
            continue; /* ring buffer overrun: next read resumes at the oldest record */
        } else if (errno != EAGAIN && errno != EINTR) {
            rc = 5;
            break;
        }
    }

    logcat_stop(&c.logcat);
    rf_close(&c.kmsg);
    close(kmsg);
    return rc;
}
