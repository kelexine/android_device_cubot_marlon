/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-09-18
 * Purpose: Controller for Gaming & Performance mode (GED DVFS and FPSGO).
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.kernel;

import android.util.Log;

import org.lineageos.settings.device.utils.FileUtils;

public final class GamingModeController {

    private static final String TAG = "DeviceSettings-GamingMode";
    public static final String KEY_GAMING_MODE = "kernel_gaming_mode";

    private static final String NODE_GED_MARGIN =
            "/sys/module/ged/parameters/gx_tb_dvfs_margin";
    private static final String NODE_GED_BOTTOM_FREQ =
            "/sys/module/ged/parameters/gpu_bottom_freq";
    private static final String NODE_FPSGO_ENABLE =
            "/sys/kernel/fpsgo/common/fpsgo_enable";

    // Stock baseline vs Gaming Mode boosts
    private static final String STOCK_MARGIN = "45";
    private static final String GAMING_MARGIN = "60";

    private static final String STOCK_BOTTOM_FREQ = "0";
    private static final String GAMING_BOTTOM_FREQ = "400000";

    private GamingModeController() {
        // no instances
    }

    public static boolean isSupported() {
        return (FileUtils.isFileReadable(NODE_GED_MARGIN) && FileUtils.isFileWritable(NODE_GED_MARGIN))
                || (FileUtils.isFileReadable(NODE_GED_BOTTOM_FREQ) && FileUtils.isFileWritable(NODE_GED_BOTTOM_FREQ));
    }

    public static boolean isGamingModeEnabled() {
        String curMargin = FileUtils.readOneLine(NODE_GED_MARGIN);
        if (curMargin != null) {
            try {
                int margin = Integer.parseInt(curMargin.trim());
                return margin >= 55;
            } catch (NumberFormatException ignored) {
            }
        }
        String curBottom = FileUtils.readOneLine(NODE_GED_BOTTOM_FREQ);
        if (curBottom != null) {
            try {
                long freq = Long.parseLong(curBottom.trim());
                return freq > 0;
            } catch (NumberFormatException ignored) {
            }
        }
        return false;
    }

    public static boolean setGamingMode(boolean enabled) {
        boolean allOk = true;

        String targetMargin = enabled ? GAMING_MARGIN : STOCK_MARGIN;
        if (FileUtils.isFileWritable(NODE_GED_MARGIN)) {
            if (!FileUtils.writeLine(NODE_GED_MARGIN, targetMargin)) {
                allOk = false;
                Log.w(TAG, "Failed to write gx_tb_dvfs_margin=" + targetMargin);
            }
        }

        String targetBottomFreq = enabled ? GAMING_BOTTOM_FREQ : STOCK_BOTTOM_FREQ;
        if (FileUtils.isFileWritable(NODE_GED_BOTTOM_FREQ)) {
            if (!FileUtils.writeLine(NODE_GED_BOTTOM_FREQ, targetBottomFreq)) {
                allOk = false;
                Log.w(TAG, "Failed to write gpu_bottom_freq=" + targetBottomFreq);
            }
        }

        if (FileUtils.isFileWritable(NODE_FPSGO_ENABLE)) {
            FileUtils.writeLine(NODE_FPSGO_ENABLE, "1");
        }

        return allOk;
    }
}
