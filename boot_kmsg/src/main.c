/*
 * Script/File: boot_kmsg/src/main.c
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-09 (updated 2026-11-19)
 * Purpose: Early boot kernel message (kmsg) & logcat collector for Cubot P50
 *          (marlon). Redirects output to booted_logs once sys.boot_completed=1
 *          without stopping the kmsg stream.
 *
 * Usage: boot_kmsg [boot-kmsg-path] [timeout-secs] [booted-dir]
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LOG_PATH "/cache/bootlog/boot-kmsg.log"
#define FALLBACK_LOG_PATH "/metadata/bootlog/boot-kmsg.log"
#define DEFAULT_BOOTED_DIR "/cache/booted_logs"
#define BOOTED_KMSG_NAME "booted-kmsg.log"
#define BOOT_COMPLETED_PROP "sys.boot_completed"

#define DEFAULT_TIMEOUT_SECS 30
#define BUFFER_SIZE 16384
#define PATH_BUF_SIZE 512
#define POLL_TIMEOUT_MS 100
#define SYNC_INTERVAL_BOOT_MS 250
#define SYNC_INTERVAL_BOOTED_MS 2000
#define BOOT_CHECK_INTERVAL_MS 500
#define STOP_POLL_ATTEMPTS 10
#define STOP_POLL_NS (50L * 1000000L)

static volatile sig_atomic_t g_running = 1;
static pid_t g_logcat_pid = -1;

static void handle_signal(int sig) {
    (void)sig;
    g_running = 0;
}

static void install_signals(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);
}

static long milliseconds(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static bool is_boot_completed(void) {
    char val[PROPERTY_VALUE_MAX] = {0};

    return property_get(BOOT_COMPLETED_PROP, val, "") > 0 && strcmp(val, "1") == 0;
}

static void ensure_parent_dir(const char *path) {
    char dir[PATH_BUF_SIZE];
    char *slash;

    if (strlen(path) >= sizeof(dir)) {
        return;
    }
    memcpy(dir, path, strlen(path) + 1);
    slash = strrchr(dir, '/');
    if (slash != NULL && slash != dir) {
        *slash = '\0';
        (void)mkdir(dir, 0775);
    }
}

static int derive_logcat_path(const char *kmsg_path, char *out, size_t size) {
    const char *ext = strstr(kmsg_path, "kmsg.log");
    int written;

    if (ext != NULL) {
        written = snprintf(out, size, "%.*slogcat.log%s", (int)(ext - kmsg_path), kmsg_path,
                           ext + strlen("kmsg.log"));
    } else {
        written = snprintf(out, size, "%s.logcat", kmsg_path);
    }
    return (written < 0 || (size_t)written >= size) ? -1 : 0;
}

static void stop_logcat(pid_t pid) {
    struct timespec nap = {0, STOP_POLL_NS};

    if (pid <= 0) {
        return;
    }
    kill(pid, SIGTERM);
    for (int i = 0; i < STOP_POLL_ATTEMPTS; ++i) {
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

static pid_t spawn_logcat(const char *kmsg_path, bool tail_only) {
    char logcat_path[PATH_BUF_SIZE];
    const char *argv[20];
    int n = 0;
    pid_t pid;

    if (derive_logcat_path(kmsg_path, logcat_path, sizeof(logcat_path)) < 0) {
        return -1;
    }
    ensure_parent_dir(logcat_path);

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
    argv[n++] = logcat_path;
    argv[n++] = "-r";
    argv[n++] = "4096";
    argv[n++] = "-n";
    argv[n++] = "3";
    if (tail_only) {
        argv[n++] = "-T";
        argv[n++] = "1";
    }
    argv[n] = NULL;

    pid = fork();
    if (pid == 0) {
        execv("/system/bin/logcat", (char *const *)argv);
        _exit(127);
    }
    return pid;
}

static int open_log_file(const char *path, bool truncate) {
    int flags = O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC;

    if (truncate) {
        flags |= O_TRUNC;
    }
    ensure_parent_dir(path);
    return open(path, flags, 0664);
}

static int write_all(int fd, const char *buf, size_t len) {
    size_t done = 0;

    while (done < len) {
        ssize_t n = write(fd, buf + done, len - done);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        done += (size_t)n;
    }
    return 0;
}

static void write_header(int fd, const char *phase) {
    char hdr[256];
    int n = snprintf(hdr, sizeof(hdr),
                     "========================================\n"
                     " Marlon kmsg & logcat collector (%s)\n"
                     " Device: Cubot P50 (marlon / MT6765)\n"
                     "========================================\n",
                     phase);

    if (n > 0) {
        size_t len = (size_t)n < sizeof(hdr) ? (size_t)n : sizeof(hdr) - 1;
        (void)write_all(fd, hdr, len);
        fsync(fd);
    }
}

static bool is_critical_line(const char *buf) {
    return strstr(buf, "init:") != NULL || strstr(buf, "FATAL") != NULL ||
           strstr(buf, "panic") != NULL || strstr(buf, "BUG:") != NULL ||
           strstr(buf, "Call trace:") != NULL;
}

static bool redirect_to_booted(int *out_fd, const char *booted_dir) {
    char booted_kmsg[PATH_BUF_SIZE];
    int n = snprintf(booted_kmsg, sizeof(booted_kmsg), "%s/%s", booted_dir, BOOTED_KMSG_NAME);
    int next_fd;
    pid_t next_logcat;

    if (n < 0 || (size_t)n >= sizeof(booted_kmsg)) {
        return false;
    }
    next_fd = open_log_file(booted_kmsg, false);
    if (next_fd < 0) {
        return false;
    }

    fsync(*out_fd);
    close(*out_fd);
    *out_fd = next_fd;
    write_header(*out_fd, "booted");

    next_logcat = spawn_logcat(booted_kmsg, true);
    if (next_logcat > 0) {
        pid_t old_logcat = g_logcat_pid;
        g_logcat_pid = next_logcat;
        stop_logcat(old_logcat);
    }
    return true;
}

static int open_initial_log(const char *target_path, const char *booted_dir, bool *redirected) {
    char booted_kmsg[PATH_BUF_SIZE];
    int fd;

    if (is_boot_completed()) {
        int n = snprintf(booted_kmsg, sizeof(booted_kmsg), "%s/%s", booted_dir, BOOTED_KMSG_NAME);
        if (n > 0 && (size_t)n < sizeof(booted_kmsg)) {
            fd = open_log_file(booted_kmsg, false);
            if (fd >= 0) {
                *redirected = true;
                g_logcat_pid = spawn_logcat(booted_kmsg, true);
                write_header(fd, "booted");
                return fd;
            }
        }
    }

    *redirected = false;
    fd = open_log_file(target_path, true);
    if (fd >= 0) {
        g_logcat_pid = spawn_logcat(target_path, false);
        write_header(fd, "boot");
        return fd;
    }

    fd = open_log_file(FALLBACK_LOG_PATH, true);
    if (fd >= 0) {
        g_logcat_pid = spawn_logcat(FALLBACK_LOG_PATH, false);
        write_header(fd, "boot");
    }
    return fd;
}

static void run_loop(int kmsg, int *out_fd, const char *booted_dir, bool redirected,
                     long timeout_secs) {
    char buffer[BUFFER_SIZE];
    long last_sync = milliseconds();
    long next_check = last_sync + BOOT_CHECK_INTERVAL_MS;
    long end_time = (timeout_secs > 0) ? (last_sync + timeout_secs * 1000L) : 0;

    while (g_running && (end_time == 0 || milliseconds() < end_time)) {
        struct pollfd pfd = {.fd = kmsg, .events = POLLIN};
        long now = milliseconds();
        int ret;
        ssize_t count;

        if (!redirected && now >= next_check) {
            next_check = now + BOOT_CHECK_INTERVAL_MS;
            if (is_boot_completed() && redirect_to_booted(out_fd, booted_dir)) {
                redirected = true;
                last_sync = milliseconds();
            }
        }

        ret = poll(&pfd, 1, POLL_TIMEOUT_MS);
        if (ret < 0 && errno != EINTR) {
            break;
        }
        if (ret <= 0) {
            continue;
        }

        count = read(kmsg, buffer, sizeof(buffer) - 1);
        if (count > 0) {
            long sync_interval = redirected ? SYNC_INTERVAL_BOOTED_MS : SYNC_INTERVAL_BOOT_MS;

            buffer[count] = '\0';
            if (write_all(*out_fd, buffer, (size_t)count) < 0) {
                continue;
            }
            now = milliseconds();
            if (now - last_sync >= sync_interval || is_critical_line(buffer)) {
                fsync(*out_fd);
                last_sync = now;
            }
        } else if (count < 0 && errno == EPIPE) {
            continue;
        } else if (count < 0 && errno != EAGAIN && errno != EINTR) {
            break;
        }
    }
}

int main(int argc, char **argv) {
    const char *target_path = DEFAULT_LOG_PATH;
    const char *booted_dir = DEFAULT_BOOTED_DIR;
    long timeout_secs = DEFAULT_TIMEOUT_SECS;
    bool redirected = false;
    int kmsg;
    int out_fd;

    if (argc >= 2 && argv[1][0] != '\0') {
        target_path = argv[1];
    }
    if (argc >= 3) {
        timeout_secs = strtol(argv[2], NULL, 10);
    }
    if (argc >= 4 && argv[3][0] != '\0') {
        booted_dir = argv[3];
    }

    install_signals();

    kmsg = open("/dev/kmsg", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (kmsg < 0) {
        return 1;
    }
    lseek(kmsg, 0, SEEK_SET);

    out_fd = open_initial_log(target_path, booted_dir, &redirected);
    if (out_fd < 0) {
        close(kmsg);
        return 2;
    }

    run_loop(kmsg, &out_fd, booted_dir, redirected, timeout_secs);

    stop_logcat(g_logcat_pid);
    g_logcat_pid = -1;
    fsync(out_fd);
    close(out_fd);
    close(kmsg);
    return 0;
}
