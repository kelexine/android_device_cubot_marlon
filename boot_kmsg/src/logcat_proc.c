/*
 * Script/File: boot_kmsg/src/logcat_proc.c
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Supervised `logcat -f` child with gap-free output handover.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logcat_proc.h"

#include "config.h"
#include "fsutil.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define STOP_POLLS 20
#define STOP_POLL_NS (50L * 1000000L)

int logcat_derive_path(const char *kmsg_path, char *out, size_t n) {
    const char *hit = strstr(kmsg_path, "kmsg.log");
    int written;

    if (hit != NULL) {
        written = snprintf(out, n, "%.*slogcat.log%s", (int)(hit - kmsg_path), kmsg_path,
                           hit + strlen("kmsg.log"));
    } else {
        written = snprintf(out, n, "%s.logcat", kmsg_path);
    }
    return (written < 0 || (size_t)written >= n) ? -1 : 0;
}

void logcat_format_since(char *out, size_t n, const struct timespec *ts, int margin_secs) {
    long long sec = (long long)ts->tv_sec - margin_secs;

    if (sec < 0) {
        sec = 0;
    }
    snprintf(out, n, "%lld.%03ld", sec, (long)(ts->tv_nsec / 1000000L));
}

static pid_t spawn(const logcat_proc_t *lp, const char *path, int rotate_kib,
                   int rotate_files, const char *since) {
    char rkib[16];
    char nfiles[16];
    const char *argv[24];
    int n = 0;
    pid_t pid;

    snprintf(rkib, sizeof(rkib), "%d", rotate_kib);
    snprintf(nfiles, sizeof(nfiles), "%d", rotate_files);

    argv[n++] = "logcat";
    argv[n++] = "-b";
    argv[n++] = "main,system,crash,radio,events";
    argv[n++] = "-v";
    argv[n++] = "threadtime";
    argv[n++] = "-v";
    argv[n++] = "usec";
    argv[n++] = "-v";
    argv[n++] = "printable";
    argv[n++] = "-f";
    argv[n++] = path;
    argv[n++] = "-r";
    argv[n++] = rkib;
    argv[n++] = "-n";
    argv[n++] = nfiles;
    if (since != NULL) {
        argv[n++] = "-T";
        argv[n++] = since;
    }
    argv[n] = NULL;

    (void)fs_ensure_parent(path);
    pid = fork();
    if (pid == 0) {
        execv(lp->bin, (char *const *)argv);
        _exit(127);
    }
    return pid;
}

static void stop_pid(pid_t pid) {
    struct timespec nap = {0, STOP_POLL_NS};

    if (pid <= 0) {
        return;
    }
    kill(pid, SIGTERM);
    for (int i = 0; i < STOP_POLLS; ++i) {
        pid_t r = waitpid(pid, NULL, WNOHANG);
        if (r == pid || (r < 0 && errno != EINTR)) {
            return;
        }
        nanosleep(&nap, NULL);
    }
    kill(pid, SIGKILL);
    while (waitpid(pid, NULL, 0) < 0 && errno == EINTR) {
    }
}

static int copy_path(logcat_proc_t *lp, const char *path) {
    size_t n = strlen(path);

    if (n >= sizeof(lp->path)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(lp->path, path, n + 1);
    return 0;
}

void logcat_init(logcat_proc_t *lp, const char *bin) {
    memset(lp, 0, sizeof(*lp));
    lp->bin = bin;
    lp->pid = -1;
}

int logcat_start(logcat_proc_t *lp, const char *path, int rotate_kib, int rotate_files,
                 long now_ms) {
    pid_t pid;

    if (copy_path(lp, path) < 0) {
        return -1;
    }
    lp->rotate_kib = rotate_kib;
    lp->rotate_files = rotate_files;
    pid = spawn(lp, lp->path, rotate_kib, rotate_files, NULL);
    if (pid < 0) {
        return -1;
    }
    lp->pid = pid;
    lp->enabled = true;
    lp->last_spawn_ms = now_ms;
    clock_gettime(CLOCK_REALTIME, &lp->last_alive);
    return 0;
}

int logcat_switch(logcat_proc_t *lp, const char *new_path, int rotate_kib,
                  int rotate_files, long now_ms) {
    struct timespec now;
    char since[32];
    pid_t old = lp->pid;
    pid_t fresh;

    if (strlen(new_path) >= sizeof(lp->path)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &now);
    logcat_format_since(since, sizeof(since), &now, SINCE_MARGIN_SECS);

    fresh = spawn(lp, new_path, rotate_kib, rotate_files, since);
    if (fresh < 0) {
        return -1;
    }
    /* New one is up; only now retire the old one so nothing is missed. */
    lp->pid = fresh;
    copy_path(lp, new_path);
    lp->rotate_kib = rotate_kib;
    lp->rotate_files = rotate_files;
    lp->enabled = true;
    lp->last_spawn_ms = now_ms;
    lp->last_alive = now;
    stop_pid(old);
    return 0;
}

void logcat_supervise(logcat_proc_t *lp, long now_ms) {
    struct timespec now;
    char since[32];
    pid_t r;
    pid_t fresh;

    if (!lp->enabled) {
        return;
    }
    clock_gettime(CLOCK_REALTIME, &now);
    if (lp->pid > 0) {
        r = waitpid(lp->pid, NULL, WNOHANG);
        if (r == 0 || (r < 0 && errno == EINTR)) {
            lp->last_alive = now;
            return;
        }
        lp->pid = -1; /* exited, or no longer ours to wait on */
    }
    if (now_ms - lp->last_spawn_ms < RESPAWN_BACKOFF_MS) {
        return;
    }
    /* Resume from the last moment we knew it was alive: logd still holds it. */
    logcat_format_since(since, sizeof(since), &lp->last_alive, SINCE_MARGIN_SECS);
    fresh = spawn(lp, lp->path, lp->rotate_kib, lp->rotate_files, since);
    lp->last_spawn_ms = now_ms;
    if (fresh > 0) {
        lp->pid = fresh;
        lp->last_alive = now;
    }
}

void logcat_stop(logcat_proc_t *lp) {
    lp->enabled = false;
    stop_pid(lp->pid);
    lp->pid = -1;
}
