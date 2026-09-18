/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-09-18
 * Purpose: Controller for Linux 5.8 proactive memory compaction sysfs/procfs interface.
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.kernel;

import android.util.Log;

import org.lineageos.settings.device.utils.FileUtils;

public final class CompactionController {

    private static final String TAG = "DeviceSettings-Compaction";
    public static final String KEY_PROACTIVE_COMPACTION = "kernel_proactive_compaction";
    public static final String DEFAULT_PROACTIVENESS = "20";

    private static final String NODE_PROACTIVENESS =
            "/proc/sys/vm/compaction_proactiveness";

    private CompactionController() {
        // no instances
    }

    public static boolean isSupported() {
        return FileUtils.isFileReadable(NODE_PROACTIVENESS)
                && FileUtils.isFileWritable(NODE_PROACTIVENESS);
    }

    public static String getProactiveness() {
        String val = FileUtils.readOneLine(NODE_PROACTIVENESS);
        return (val != null) ? val.trim() : DEFAULT_PROACTIVENESS;
    }

    public static boolean setProactiveness(String value) {
        if (value == null || value.isEmpty()) {
            return false;
        }
        boolean success = FileUtils.writeLine(NODE_PROACTIVENESS, value.trim());
        if (!success) {
            Log.w(TAG, "Failed to write compaction_proactiveness: " + value);
        }
        return success;
    }
}
