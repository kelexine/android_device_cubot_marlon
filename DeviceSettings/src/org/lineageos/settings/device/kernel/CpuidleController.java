/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-09-18
 * Purpose: Controller for CPUIdle governor sysfs interface.
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.kernel;

import android.util.Log;

import org.lineageos.settings.device.utils.FileUtils;

import java.util.Arrays;
import java.util.List;

public final class CpuidleController {

    private static final String TAG = "DeviceSettings-Cpuidle";
    public static final String KEY_CPUIDLE_GOVERNOR = "kernel_cpuidle_governor";
    public static final String DEFAULT_GOVERNOR = "teo";

    private static final String NODE_CURRENT_GOVERNOR =
            "/sys/devices/system/cpu/cpuidle/current_governor";
    private static final String NODE_AVAILABLE_GOVERNORS =
            "/sys/devices/system/cpu/cpuidle/available_governors";

    private CpuidleController() {
        // no instances
    }

    public static boolean isSupported() {
        return FileUtils.isFileReadable(NODE_CURRENT_GOVERNOR)
                && FileUtils.isFileWritable(NODE_CURRENT_GOVERNOR);
    }

    public static String getCurrentGovernor() {
        String gov = FileUtils.readOneLine(NODE_CURRENT_GOVERNOR);
        return (gov != null) ? gov.trim() : DEFAULT_GOVERNOR;
    }

    public static boolean setGovernor(String governor) {
        if (governor == null || governor.isEmpty()) {
            return false;
        }
        boolean success = FileUtils.writeLine(NODE_CURRENT_GOVERNOR, governor.trim());
        if (!success) {
            Log.w(TAG, "Failed to write governor: " + governor);
        }
        return success;
    }

    public static List<String> getAvailableGovernors() {
        String line = FileUtils.readOneLine(NODE_AVAILABLE_GOVERNORS);
        if (line != null && !line.trim().isEmpty()) {
            String[] govs = line.trim().split("\\s+");
            return Arrays.asList(govs);
        }
        return Arrays.asList("teo", "menu", "ladder", "mtk_governor");
    }
}
