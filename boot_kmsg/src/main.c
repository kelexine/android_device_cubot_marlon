/*
 * Script/File: boot_kmsg/src/main.c
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-09
 * Purpose: Early boot kernel message (kmsg) diagnostic collector for Cubot P50 (marlon).
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_LOG_PATH "/cache/bootlog/boot-kmsg.log"
#define FALLBACK_LOG_PATH "/metadata/bootlog/boot-kmsg.log"
#define DEFAULT_TIMEOUT_SECS 30
#define BUFFER_SIZE 16384

static long milliseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void ensure_parent_dir(const char *path) {
    char dir[512];
    strncpy(dir, path, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    char *slash = strrchr(dir, '/');
    if (slash && slash != dir) {
        *slash = '\0';
        mkdir(dir, 0775);
    }
}

int main(int argc, char **argv) {
    const char *target_path = DEFAULT_LOG_PATH;
    int timeout_secs = DEFAULT_TIMEOUT_SECS;

    if (argc >= 2 && argv[1][0] != '\0') {
        target_path = argv[1];
    }
    if (argc >= 3) {
        timeout_secs = atoi(argv[2]);
    }

    /* Open /dev/kmsg non-blocking */
    int kmsg = open("/dev/kmsg", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (kmsg < 0) {
        return 1;
    }

    /* Rewind kmsg stream to start of boot ring buffer */
    lseek(kmsg, 0, SEEK_SET);

    /* Attempt to open target log file; fallback to metadata if cache fails */
    ensure_parent_dir(target_path);
    int out = open(target_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0664);
    if (out < 0) {
        target_path = FALLBACK_LOG_PATH;
        ensure_parent_dir(target_path);
        out = open(target_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0664);
        if (out < 0) {
            close(kmsg);
            return 2;
        }
    }

    const char *header = "========================================\n"
                         " Marlon early kmsg collector (boot_kmsg)\n"
                         " Device: Cubot P50 (marlon / MT6765)\n"
                         "========================================\n";
    write(out, header, strlen(header));
    fsync(out);

    long last_sync = milliseconds();
    long end_time = (timeout_secs > 0) ? (last_sync + ((long)timeout_secs * 1000L)) : 0;
    char buffer[BUFFER_SIZE];

    while (end_time == 0 || milliseconds() < end_time) {
        struct pollfd pfd = {.fd = kmsg, .events = POLLIN};
        int ret = poll(&pfd, 1, 100);
        if (ret < 0 && errno != EINTR) {
            break;
        }
        if (ret == 0) {
            continue;
        }

        ssize_t count = read(kmsg, buffer, sizeof(buffer) - 1);
        if (count > 0) {
            buffer[count] = '\0';
            ssize_t done = 0;
            while (done < count) {
                ssize_t n = write(out, buffer + done, count - done);
                if (n <= 0) {
                    goto finish;
                }
                done += n;
            }

            long now = milliseconds();
            /* Immediate sync on critical kernel panic / init failure messages */
            if (now - last_sync >= 250 ||
                strstr(buffer, "init:") != NULL ||
                strstr(buffer, "FATAL") != NULL ||
                strstr(buffer, "panic") != NULL ||
                strstr(buffer, "BUG:") != NULL ||
                strstr(buffer, "Call trace:") != NULL) {
                fsync(out);
                last_sync = now;
            }
        } else if (count < 0 && errno == EPIPE) {
            /* Ring buffer overrun, continue reading next record */
            continue;
        } else if (count < 0 && errno != EAGAIN && errno != EINTR) {
            break;
        }
    }

finish:
    fsync(out);
    close(out);
    close(kmsg);
    return 0;
}
