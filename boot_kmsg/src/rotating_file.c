/*
 * Script/File: boot_kmsg/src/rotating_file.c
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Size-rotating append-only log sink that can be re-pointed at a new
 *          path without losing the stream.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rotating_file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BACKUP_PATH_BUF (RF_PATH_MAX + 16)

static int open_active(const char *path, bool truncate, off_t *size_out) {
    int flags = O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC;
    struct stat st;
    int fd;

    if (truncate) {
        flags |= O_TRUNC;
    }
    fd = open(path, flags, 0664);
    if (fd < 0) {
        return -1;
    }
    *size_out = (fstat(fd, &st) == 0) ? st.st_size : 0;
    return fd;
}

static int set_path(rotating_file_t *rf, const char *path) {
    size_t n = strlen(path);

    if (n >= sizeof(rf->path)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(rf->path, path, n + 1);
    return 0;
}

/* path.(n-2) -> path.(n-1) ... path -> path.1; the oldest is overwritten. */
static void shift_backups(const rotating_file_t *rf) {
    char from[BACKUP_PATH_BUF];
    char to[BACKUP_PATH_BUF];

    for (int i = rf->max_files - 1; i >= 1; --i) {
        snprintf(to, sizeof(to), "%s.%d", rf->path, i);
        if (i == 1) {
            snprintf(from, sizeof(from), "%s", rf->path);
        } else {
            snprintf(from, sizeof(from), "%s.%d", rf->path, i - 1);
        }
        (void)rename(from, to); /* ENOENT on a young history is expected */
    }
}

/* Moves history along and starts a fresh active file. On failure fd is -1
 * and the next write retries a plain reopen (no further shifting). */
static int rotate(rotating_file_t *rf) {
    shift_backups(rf);
    if (rf->fd >= 0) {
        close(rf->fd);
        rf->fd = -1;
    }
    rf->size = 0;
    rf->fd = open_active(rf->path, true, &rf->size);
    return rf->fd >= 0 ? 0 : -1;
}

static int reopen(rotating_file_t *rf) {
    rf->fd = open_active(rf->path, false, &rf->size);
    return rf->fd >= 0 ? 0 : -1;
}

int rf_open(rotating_file_t *rf, const char *path, off_t max_bytes, int max_files,
            bool truncate) {
    memset(rf, 0, sizeof(*rf));
    rf->fd = -1;
    rf->max_bytes = max_bytes;
    rf->max_files = max_files < 1 ? 1 : max_files;
    if (set_path(rf, path) < 0) {
        return -1;
    }
    rf->fd = open_active(rf->path, truncate, &rf->size);
    return rf->fd >= 0 ? 0 : -1;
}

int rf_write(rotating_file_t *rf, const char *buf, size_t len) {
    size_t done = 0;
    bool rotated_on_full = false;

    if (len == 0) {
        return 0;
    }
    if (rf->fd < 0 && reopen(rf) < 0) {
        rf->dropped++;
        return -1;
    }
    if (rf->max_bytes > 0 && rf->size > 0 && rf->size + (off_t)len > rf->max_bytes) {
        (void)rotate(rf);
        if (rf->fd < 0) {
            rf->dropped++;
            return -1;
        }
    }

    while (done < len) {
        ssize_t n = write(rf->fd, buf + done, len - done);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            if ((errno == ENOSPC || errno == EDQUOT) && !rotated_on_full) {
                /* Volume full: dropping the oldest backup frees space. */
                rotated_on_full = true;
                (void)rotate(rf);
                if (rf->fd >= 0) {
                    continue;
                }
            }
            rf->dropped++;
            return -1;
        }
        done += (size_t)n;
        rf->size += n;
    }
    return 0;
}

int rf_sync(rotating_file_t *rf) {
    if (rf->fd < 0) {
        return -1;
    }
    return fsync(rf->fd);
}

int rf_redirect(rotating_file_t *rf, const char *new_path, bool truncate,
                off_t max_bytes, int max_files) {
    rotating_file_t next;

    if (rf_open(&next, new_path, max_bytes, max_files, truncate) < 0) {
        return -1;
    }
    if (rf->fd >= 0) {
        fsync(rf->fd);
        close(rf->fd);
    }
    next.dropped = rf->dropped;
    *rf = next;
    return 0;
}

void rf_close(rotating_file_t *rf) {
    if (rf->fd >= 0) {
        fsync(rf->fd);
        close(rf->fd);
        rf->fd = -1;
    }
}
