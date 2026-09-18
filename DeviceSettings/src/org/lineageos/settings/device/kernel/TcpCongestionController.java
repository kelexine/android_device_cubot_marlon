/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-09-18
 * Purpose: Controller for TCP congestion control sysfs/procfs interface.
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.kernel;

import android.util.Log;

import org.lineageos.settings.device.utils.FileUtils;

import java.util.Arrays;
import java.util.List;

public final class TcpCongestionController {

    private static final String TAG = "DeviceSettings-TcpCc";
    public static final String KEY_TCP_CONGESTION = "kernel_tcp_congestion";
    public static final String DEFAULT_CONGESTION = "bbr";

    private static final String NODE_CURRENT_CONGESTION =
            "/proc/sys/net/ipv4/tcp_congestion_control";
    private static final String NODE_AVAILABLE_CONGESTION =
            "/proc/sys/net/ipv4/tcp_available_congestion_control";

    private TcpCongestionController() {
        // no instances
    }

    public static boolean isSupported() {
        return FileUtils.isFileReadable(NODE_CURRENT_CONGESTION)
                && FileUtils.isFileWritable(NODE_CURRENT_CONGESTION);
    }

    public static String getCurrentCongestion() {
        String cc = FileUtils.readOneLine(NODE_CURRENT_CONGESTION);
        return (cc != null) ? cc.trim() : DEFAULT_CONGESTION;
    }

    public static boolean setCongestion(String cc) {
        if (cc == null || cc.isEmpty()) {
            return false;
        }
        boolean success = FileUtils.writeLine(NODE_CURRENT_CONGESTION, cc.trim());
        if (!success) {
            Log.w(TAG, "Failed to write tcp congestion algorithm: " + cc);
        }
        return success;
    }

    public static List<String> getAvailableCongestionAlgorithms() {
        String line = FileUtils.readOneLine(NODE_AVAILABLE_CONGESTION);
        if (line != null && !line.trim().isEmpty()) {
            String[] algos = line.trim().split("\\s+");
            return Arrays.asList(algos);
        }
        return Arrays.asList("bbr", "cubic");
    }
}
