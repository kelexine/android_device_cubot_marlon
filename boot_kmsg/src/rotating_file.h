/*
 * Script/File: boot_kmsg/src/rotating_file.h
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Size-rotating append-only log sink that can be re-pointed at a new
 *          path without losing the stream. Never blocks the collector: write
 *          failures drop the record and are counted.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BOOT_KMSG_ROTATING_FILE_H
#define BOOT_KMSG_ROTATING_FILE_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#define RF_PATH_MAX 512

typedef struct {
    char path[RF_PATH_MAX];
    int fd;                 /* -1 while closed / awaiting reopen */
    off_t size;             /* bytes in the active file */
    off_t max_bytes;        /* rotate threshold, 0 = never rotate */
    int max_files;          /* files kept incl. the active one (>= 1) */
    unsigned long dropped;  /* records lost to write/open failures */
} rotating_file_t;

/* Opens `path` for append (truncating first when `truncate`). 0 / -1+errno. */
int rf_open(rotating_file_t *rf, const char *path, off_t max_bytes, int max_files,
            bool truncate);

/* Appends one record, rotating first if it would overflow max_bytes. A record
 * is never split across files. 0 on success, -1 if it had to be dropped. */
int rf_write(rotating_file_t *rf, const char *buf, size_t len);

int rf_sync(rotating_file_t *rf);

/* Moves the sink to `new_path`. The new file is opened before the old one is
 * closed, so on failure (-1) the sink keeps writing to the old path untouched. */
int rf_redirect(rotating_file_t *rf, const char *new_path, bool truncate,
                off_t max_bytes, int max_files);

void rf_close(rotating_file_t *rf);

#endif /* BOOT_KMSG_ROTATING_FILE_H */
