/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 *
 * All node names and keycode mappings below are as reported against
 * ilitek_node.c:2498 / ilitek_touch.c:1076 / ilitek_plat_mtk.c:79.
 *
 * NOT included: the letter 'F' gesture mentioned in the driver's key-event
 * dispatch (ilitek_touch.c:1076) has no corresponding node name confirmed
 * in the procfs/sysfs interface list (ilitek_node.c:2498 names c, e, m, o,
 * s, v, w, z but not f). Wiring a guessed "letter_f_enable" path risks
 * silently writing to nothing (or worse, something else) under enforcing
 * SELinux with no error surfaced -- confirm the real node name before
 * adding it here.
 *
 * Also NOT included: DT2W is intentionally NOT in this registry -- it
 * wakes the device at the kernel level with no Android-side key event to
 * intercept, and already has its own dedicated
 * DoubleTapToWakeController/BootCompletedReceiver wiring from the previous
 * patch. Duplicating it into this generic, KeyHandler-oriented registry
 * would be redundant and risks two code paths fighting over the same node.
 */
package org.lineageos.settings.device.gestures;

import android.view.KeyEvent;

import java.util.Collections;
import java.util.List;

public final class GestureRegistry {

    // Single swipes -- configurable action, no fixed behavior of their own.
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

    // Two-finger swipe: fixed media-control behavior (down=play/pause,
    // left=previous, right=next). These are standard Linux media keycodes
    // that Android's global media session dispatch already understands --
    // KeyHandler does NOT intercept these, they're listed here only so the
    // enable toggle has a single place to live alongside everything else.
    public static final GestureInfo DOUBLE_SWIPE_MEDIA = new GestureInfo(
            "gesture_double_swipe_media", "double_swipe_enable",
            0, false, GestureAction.NONE);

    // Letters -- configurable action. Default to NONE across the board;
    // which letter should map to which action is a user choice, not
    // something to presume on their behalf.
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

    public static final List<GestureInfo> ALL = Collections.unmodifiableList(
            java.util.Arrays.asList(
                    SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT,
                    DOUBLE_SWIPE_MEDIA,
                    LETTER_C, LETTER_E, LETTER_M, LETTER_O,
                    LETTER_S, LETTER_V, LETTER_W, LETTER_Z));

    /**
     * Gestures whose Android-side key event KeyHandler should actually
     * intercept and remap to a configured action. Excludes DOUBLE_SWIPE_MEDIA
     * (native media key handling needs no interception) and anything with
     * keyCode == 0.
     */
    public static final List<GestureInfo> INTERCEPTABLE = Collections.unmodifiableList(
            java.util.Arrays.asList(
                    SWIPE_UP, SWIPE_DOWN, SWIPE_LEFT, SWIPE_RIGHT,
                    LETTER_C, LETTER_E, LETTER_M, LETTER_O,
                    LETTER_S, LETTER_V, LETTER_W, LETTER_Z));

    private GestureRegistry() {
        // no instances
    }
}
