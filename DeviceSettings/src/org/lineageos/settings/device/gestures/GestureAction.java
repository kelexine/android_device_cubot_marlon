/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.gestures;

/**
 * Built-in actions a configurable gesture (letters, single swipes) can be
 * bound to. Deliberately a fixed small set rather than a full app-launch
 * picker for this pass -- straightforward to extend to arbitrary component
 * launch later if wanted.
 */
public enum GestureAction {
    NONE,
    CAMERA,
    FLASHLIGHT,
    BROWSER,
    VOICE_ASSIST;

    public static GestureAction fromPreferenceValue(String value) {
        if (value == null) {
            return NONE;
        }
        try {
            return GestureAction.valueOf(value);
        } catch (IllegalArgumentException e) {
            return NONE;
        }
    }
}
