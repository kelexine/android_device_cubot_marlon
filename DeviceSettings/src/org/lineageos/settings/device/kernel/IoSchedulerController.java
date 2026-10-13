/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-09-18
 * Purpose: Controller for Block I/O scheduler and readahead sysfs interface.
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.kernel;

import android.util.Log;

import org.lineageos.settings.device.utils.FileUtils;

import java.util.Arrays;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public final class IoSchedulerController {

    private static final String TAG = "DeviceSettings-IoSched";
    public static final String KEY_IO_SCHEDULER = "kernel_io_scheduler";
    public static final String KEY_READ_AHEAD = "kernel_read_ahead_kb";

    public static final String DEFAULT_SCHEDULER = "bfq";
    public static final String DEFAULT_READ_AHEAD_KB = "512";

    private static final String NODE_SCHEDULER =
            "/sys/block/mmcblk0/queue/scheduler";
    private static final String NODE_READ_AHEAD =
            "/sys/block/mmcblk0/queue/read_ahead_kb";

    private static final Pattern ACTIVE_SCHEDULER_PATTERN =
            Pattern.compile("\\[([^\\]]+)\\]");

    private IoSchedulerController() {
        // no instances
    }

    public static boolean isSchedulerSupported() {
        return FileUtils.isFileReadable(NODE_SCHEDULER)
                && FileUtils.isFileWritable(NODE_SCHEDULER);
    }

    public static boolean isReadaheadSupported() {
        return FileUtils.isFileReadable(NODE_READ_AHEAD)
                && FileUtils.isFileWritable(NODE_READ_AHEAD);
    }

    public static String getCurrentScheduler() {
        String line = FileUtils.readOneLine(NODE_SCHEDULER);
        if (line == null) {
            return DEFAULT_SCHEDULER;
        }
        Matcher matcher = ACTIVE_SCHEDULER_PATTERN.matcher(line);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return DEFAULT_SCHEDULER;
    }

    public static boolean setScheduler(String scheduler) {
        if (scheduler == null || scheduler.isEmpty()) {
            return false;
        }
        boolean success = FileUtils.writeLine(NODE_SCHEDULER, scheduler.trim());
        if (!success) {
            Log.w(TAG, "Failed to write scheduler: " + scheduler);
        }
        return success;
    }

    public static List<String> getAvailableSchedulers() {
        String line = FileUtils.readOneLine(NODE_SCHEDULER);
        if (line != null && !line.trim().isEmpty()) {
            String clean = line.replace("[", "").replace("]", "");
            String[] scheds = clean.trim().split("\\s+");
            return Arrays.asList(scheds);
        }
        return Arrays.asList("bfq", "mq-deadline", "none");
    }

    public static String getCurrentReadaheadKb() {
        String line = FileUtils.readOneLine(NODE_READ_AHEAD);
        return (line != null) ? line.trim() : DEFAULT_READ_AHEAD_KB;
    }

    public static boolean setReadaheadKb(String kb) {
        if (kb == null || kb.isEmpty()) {
            return false;
        }
        boolean success = FileUtils.writeLine(NODE_READ_AHEAD, kb.trim());
        if (!success) {
            Log.w(TAG, "Failed to write read_ahead_kb: " + kb);
        }
        return success;
    }
}
