/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.gestures;

import org.lineageos.settings.device.utils.FileUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * Generic enable/disable node controller for any {@link GestureInfo}.
 * Resolves and caches whichever of the procfs/sysfs node pair actually
 * exists on this build, mirroring the fallback approach already used by
 * DoubleTapToWakeController for consistency.
 */
public final class GestureNodeController {

    private static final Map<String, String> sResolvedNodes = new HashMap<>();

    private GestureNodeController() {
        // no instances
    }

    public static synchronized boolean isSupported(GestureInfo gesture) {
        return resolveNode(gesture) != null;
    }

    public static synchronized boolean isEnabled(GestureInfo gesture) {
        String node = resolveNode(gesture);
        if (node == null) {
            return false;
        }
        return FileUtils.getValue(node, false);
    }

    public static synchronized boolean setEnabled(GestureInfo gesture, boolean enabled) {
        String node = resolveNode(gesture);
        if (node == null) {
            return false;
        }
        return FileUtils.setValue(node, enabled);
    }

    private static String resolveNode(GestureInfo gesture) {
        if (sResolvedNodes.containsKey(gesture.preferenceKey)) {
            return sResolvedNodes.get(gesture.preferenceKey);
        }
        String resolved = FileUtils.firstUsablePath(
                gesture.procNodePath(), gesture.sysfsNodePath());
        sResolvedNodes.put(gesture.preferenceKey, resolved);
        return resolved;
    }
}
