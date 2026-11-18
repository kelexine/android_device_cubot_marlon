/*
 * Script/File: boot_kmsg/src/fsutil.h
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-11-18
 * Purpose: Minimal directory helpers for the log collector.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BOOT_KMSG_FSUTIL_H
#define BOOT_KMSG_FSUTIL_H

/* Creates `path` (one level) if missing. 0 if it is a directory afterwards,
 * -1 with errno set otherwise (ENOTDIR when a non-directory is in the way). */
int fs_ensure_dir(const char *path);

/* fs_ensure_dir() on the parent directory of `path`. Paths without a parent
 * component succeed trivially. */
int fs_ensure_parent(const char *path);

#endif /* BOOT_KMSG_FSUTIL_H */
