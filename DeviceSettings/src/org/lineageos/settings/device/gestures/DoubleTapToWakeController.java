/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 *
 * Wraps the three known-equivalent DT2W control paths reported for the
 * ILITEK driver on this kernel (ilitek_node.c:2498):
 *   /proc/touchpanel/double_tap_enable
 *   /proc/ilitek/double_tap_enable
 *   /sys/android_touch/double_tap_enable
 *
 * All three are attempted in order since which one is actually present can
 * vary by kernel/driver build; the first one that exists AND is writable
 * under the current SELinux policy is used and cached for the process
 * lifetime. See sepolicy/vendor/{file.te,genfs_contexts,platform_app.te}
 * for the matching sysfs_touchpanel label + write grant this depends on.
 */
package org.lineageos.settings.device.gestures;

import org.lineageos.settings.device.utils.FileUtils;

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
