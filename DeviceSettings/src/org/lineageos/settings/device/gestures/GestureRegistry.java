/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.gestures;

import android.view.KeyEvent;

import java.util.Collections;
import java.util.List;

/**
 * Registry of supported hardware off-screen gestures and their key mappings.
 */
public final class GestureRegistry {

    public static final GestureInfo SWIPE_UP = new GestureInfo(
            "gesture_swipe_up", "single_swipe_up_enable",
            KeyEvent.KEYCODE_DPAD_UP, true, GestureAction.NONE);
    public static final GestureInfo SWIPE_DOWN = new GestureInfo(
            "gesture_swipe_down", "single_swipe_down_enable",
            KeyEvent.KEYCODE_DPAD_DOWN, true, GestureAction.NONE);
    public static final GestureInfo SWIPE_LEFT = new GestureInfo(
            "gesture_swipe_left", "single_swipe_left_enable",
            KeyEvent.KEYCODE_DPAD_LEFT, true, GestureAction.NONE);
    public static final GestureInfo SWIPE_RIGHT = new GestureInfo(
            "gesture_swipe_right", "single_swipe_right_enable",
            KeyEvent.KEYCODE_DPAD_RIGHT, true, GestureAction.NONE);
    public static final GestureInfo DOUBLE_SWIPE_MEDIA = new GestureInfo(
            "gesture_double_swipe_media", "double_swipe_enable",
            0, false, GestureAction.NONE);
    public static final GestureInfo LETTER_C = new GestureInfo(
            "gesture_letter_c", "letter_c_enable",
            KeyEvent.KEYCODE_C, true, GestureAction.NONE);
    public static final GestureInfo LETTER_E = new GestureInfo(
            "gesture_letter_e", "letter_e_enable",
            KeyEvent.KEYCODE_E, true, GestureAction.NONE);
    public static final GestureInfo LETTER_M = new GestureInfo(
            "gesture_letter_m", "letter_m_enable",
            KeyEvent.KEYCODE_M, true, GestureAction.NONE);
    public static final GestureInfo LETTER_O = new GestureInfo(
            "gesture_letter_o", "letter_o_enable",
            KeyEvent.KEYCODE_O, true, GestureAction.NONE);
    public static final GestureInfo LETTER_S = new GestureInfo(
            "gesture_letter_s", "letter_s_enable",
            KeyEvent.KEYCODE_S, true, GestureAction.NONE);
    public static final GestureInfo LETTER_V = new GestureInfo(
            "gesture_letter_v", "letter_v_enable",
            KeyEvent.KEYCODE_V, true, GestureAction.NONE);
    public static final GestureInfo LETTER_W = new GestureInfo(
            "gesture_letter_w", "letter_w_enable",
            KeyEvent.KEYCODE_W, true, GestureAction.NONE);
    public static final GestureInfo LETTER_Z = new GestureInfo(
            "gesture_letter_z", "letter_z_enable",
            KeyEvent.KEYCODE_Z, true, GestureAction.NONE);
    public static final GestureInfo LETTER_F = new GestureInfo(
            "gesture_letter_f", "letter_f_enable",
            KeyEvent.KEYCODE_F, true, GestureAction.NONE);

    public static final List<GestureInfo> ALL = Collections.unmodifiableList(
            java.util.Arrays.asList(
                    SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT,
                    DOUBLE_SWIPE_MEDIA,
                    LETTER_C, LETTER_E, LETTER_M, LETTER_O,
                    LETTER_S, LETTER_V, LETTER_W, LETTER_Z, LETTER_F));

    public static final List<GestureInfo> INTERCEPTABLE = Collections.unmodifiableList(
            java.util.Arrays.asList(
                    SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT,
                    LETTER_C, LETTER_E, LETTER_M, LETTER_O,
                    LETTER_S, LETTER_V, LETTER_W, LETTER_Z, LETTER_F));

    private GestureRegistry() {
        // no instances
    }
}
