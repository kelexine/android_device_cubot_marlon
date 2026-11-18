/*
 * Script/File: boot_kmsg/src/fsutil.c
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Minimal directory helpers for the log collector.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "fsutil.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FS_PATH_BUF 512

int fs_ensure_dir(const char *path) {
    struct stat st;

    if (path == NULL || path[0] == '\0') {
        errno = EINVAL;
        return -1;
    }
    if (mkdir(path, 0775) == 0) {
        return 0;
    }
    if (errno != EEXIST) {
        return -1;
    }
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return 0;
    }
    errno = ENOTDIR;
    return -1;
}

int fs_ensure_parent(const char *path) {
    char dir[FS_PATH_BUF];
    char *slash;

    if (path == NULL || strlen(path) >= sizeof(dir)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(dir, path, strlen(path) + 1);
    slash = strrchr(dir, '/');
    if (slash == NULL || slash == dir) {
        return 0;
    }
    *slash = '\0';
    return fs_ensure_dir(dir);
}
