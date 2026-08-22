/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.gestures;

import org.lineageos.settings.device.utils.FileUtils;

/**
 * Controller for Double-Tap-To-Wake sysfs/procfs nodes.
 */
public final class DoubleTapToWakeController {

    private static final String[] CANDIDATE_NODES = {
            "/proc/touchpanel/double_tap_enable",
            "/proc/ilitek/double_tap_enable",
            "/sys/android_touch/double_tap_enable",
    };

    private static volatile String sResolvedNode;
    private static volatile boolean sResolutionAttempted;

    private DoubleTapToWakeController() {
        // no instances
    }

    /**
     * @return true if any of the known control nodes exist and are usable
     * on this device/kernel build.
     */
    public static synchronized boolean isSupported() {
        return resolveNode() != null;
    }

    public static synchronized boolean isEnabled() {
        String node = resolveNode();
        if (node == null) {
            return false;
        }
        return FileUtils.getValue(node, false);
    }

    public static synchronized boolean setEnabled(boolean enabled) {
        String node = resolveNode();
        if (node == null) {
            return false;
        }
        return FileUtils.setValue(node, enabled);
    }

    private static String resolveNode() {
        if (!sResolutionAttempted) {
            sResolvedNode = FileUtils.firstUsablePath(CANDIDATE_NODES);
            sResolutionAttempted = true;
        }
        return sResolvedNode;
    }
}
