/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.gestures;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.UserHandle;
import android.provider.Settings;

import org.lineageos.settings.device.utils.FileUtils;

/**
 * Controller for Double-Tap-To-Wake sysfs/procfs nodes.
 */
public final class DoubleTapToWakeController {

    private static final String PREF_KEY_DOUBLE_TAP_TO_WAKE = "double_tap_to_wake";

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

    /**
     * Applies DT2W to the kernel node and syncs {@link Settings.Secure#DOUBLE_TAP_TO_WAKE}.
     */
    public static synchronized boolean setEnabled(Context context, boolean enabled) {
        if (!setEnabled(enabled)) {
            return false;
        }
        Settings.Secure.putIntForUser(
                context.getContentResolver(),
                Settings.Secure.DOUBLE_TAP_TO_WAKE,
                enabled ? 1 : 0,
                UserHandle.USER_CURRENT);
        return true;
    }

    /**
     * Resolves DT2W state on boot, preferring {@link Settings.Secure#DOUBLE_TAP_TO_WAKE}
     * when present so Display Settings and DeviceSettings stay consistent.
     */
    public static synchronized boolean resolveDesiredState(
            Context context, SharedPreferences prefs) {
        int secureState = Settings.Secure.getIntForUser(
                context.getContentResolver(),
                Settings.Secure.DOUBLE_TAP_TO_WAKE,
                -1,
                UserHandle.USER_CURRENT);
        if (secureState != -1) {
            boolean enabled = secureState != 0;
            if (prefs.getBoolean(PREF_KEY_DOUBLE_TAP_TO_WAKE, !enabled) != enabled) {
                prefs.edit().putBoolean(PREF_KEY_DOUBLE_TAP_TO_WAKE, enabled).apply();
            }
            return enabled;
        }
        return prefs.getBoolean(PREF_KEY_DOUBLE_TAP_TO_WAKE, isEnabled());
    }

    private static String resolveNode() {
        if (!sResolutionAttempted) {
            sResolvedNode = FileUtils.firstUsablePath(CANDIDATE_NODES);
            sResolutionAttempted = true;
        }
        return sResolvedNode;
    }
}
