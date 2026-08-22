/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.gestures;

/**
 * Immutable description of one off-screen gesture exposed by the ILITEK
 * driver (ilitek_node.c:2498). Each gesture has its own enable/disable
 * procfs node, mirrored under /sys/android_touch/* for GSI compatibility --
 * both are tried, in that order.
 */
public final class GestureInfo {

    public final String preferenceKey;
    public final String nodeName;
    public final int keyCode;
    public final boolean configurableAction;
    public final GestureAction defaultAction;

    /**
     * @param preferenceKey     SharedPreferences key for the enable toggle
     * @param nodeName          shared suffix under /proc/touchpanel/ and
     *                          /sys/android_touch/, e.g. "double_tap_enable"
     * @param keyCode           the android.view.KeyEvent code this gesture's
     *                          driver-side key event arrives as; 0 if it
     *                          doesn't apply (DT2W wakes at the kernel level,
     *                          no framework key event to intercept)
     * @param configurableAction whether the user can pick what this gesture
     *                          does (letters, single swipes) as opposed to a
     *                          fixed built-in behavior (DT2W, media swipes)
     * @param defaultAction     action applied when no user choice is stored
     *                          yet; irrelevant when configurableAction is
     *                          false
     */
    public GestureInfo(String preferenceKey, String nodeName, int keyCode,
            boolean configurableAction, GestureAction defaultAction) {
        this.preferenceKey = preferenceKey;
        this.nodeName = nodeName;
        this.keyCode = keyCode;
        this.configurableAction = configurableAction;
        this.defaultAction = defaultAction;
    }

    public String procNodePath() {
        return "/proc/touchpanel/" + nodeName;
    }

    public String sysfsNodePath() {
        return "/sys/android_touch/" + nodeName;
    }

    public String actionPreferenceKey() {
        return "marlon_" + preferenceKey + "_action";
    }
}
