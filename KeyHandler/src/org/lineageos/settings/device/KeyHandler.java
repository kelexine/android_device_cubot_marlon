/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 *
 * Implements the standard LineageOS device-key-handler hook
 * (com.android.internal.os.DeviceKeyHandler), instantiated by
 * PhoneWindowManager via reflection per TARGET_KEY_HANDLER_LIBS /
 * TARGET_KEY_HANDLER_CLASS in BoardConfig.mk. Runs inside system_server,
 * NOT inside the DeviceSettings app process -- see sepolicy/vendor for the
 * matching system_server grant this depends on (separate from the
 * platform_app grant the Settings app itself uses).
 *
 * Per-gesture action choices are read from Settings.System (see
 * GestureActionSettingsStore in the DeviceSettings app) rather than the
 * app's private SharedPreferences -- system_server cannot reliably read
 * another UID's private app data under standard DAC+SELinux sandboxing,
 * but Settings.System is a ContentProvider designed for exactly this kind
 * of cross-process read.
 *
 * IMPORTANT: this file is compiled against internal/hidden framework APIs
 * (com.android.internal.os.DeviceKeyHandler) that aren't part of the public
 * SDK and aren't independently verifiable in this environment -- there is
 * no AOSP frameworks/base source tree here to confirm the interface's exact
 * method signature against. The signature below (boolean handleKeyEvent
 * (KeyEvent)) is the convention used across LineageOS device trees for
 * years, but build against your actual frameworks/base revision and treat
 * any compile error here as the source of truth over this comment.
 *
 * DPAD_UP/DOWN/LEFT/RIGHT are only ever produced by the touch gesture
 * driver while the screen is off (there is no real dpad hardware on this
 * device -- see mtk-kpd.kl / ACCDET.kl, neither declares dpad keys), so
 * gating on !isInteractive() is a safety margin, not the only thing
 * preventing misfires against real navigation input.
 */
package org.lineageos.settings.device;

import android.content.Context;
import android.os.PowerManager;
import android.provider.Settings;
import android.view.KeyEvent;

import com.android.internal.os.DeviceKeyHandler;

import org.lineageos.settings.device.gestures.ActionUtils;
import org.lineageos.settings.device.gestures.GestureAction;
import org.lineageos.settings.device.gestures.GestureInfo;
import org.lineageos.settings.device.gestures.GestureNodeController;
import org.lineageos.settings.device.gestures.GestureRegistry;

import java.util.HashMap;
import java.util.Map;

public class KeyHandler implements DeviceKeyHandler {

    private final Context mContext;
    private final PowerManager mPowerManager;
    private final Map<Integer, GestureInfo> mKeyCodeMap = new HashMap<>();

    public KeyHandler(Context context) {
        mContext = context;
        mPowerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);

        for (GestureInfo gesture : GestureRegistry.INTERCEPTABLE) {
            mKeyCodeMap.put(gesture.keyCode, gesture);
        }
    }

    @Override
    public KeyEvent handleKeyEvent(KeyEvent event) {
        if (event.getAction() != KeyEvent.ACTION_UP) {
            // Only act once per gesture, on release, matching how these
            // are reported as instant pulses rather than held keys.
            return event;
        }

        GestureInfo gesture = mKeyCodeMap.get(event.getKeyCode());
        if (gesture == null) {
            return event;
        }

        if (mPowerManager != null && mPowerManager.isInteractive()) {
            // Only meaningful as an off-screen gesture; if the screen is
            // already on, don't hijack what could be a real key press.
            return event;
        }

        if (!GestureNodeController.isEnabled(gesture)) {
            return event;
        }

        String stored = Settings.System.getString(
                mContext.getContentResolver(), gesture.actionPreferenceKey());
        GestureAction action = GestureAction.fromPreferenceValue(stored);

        ActionUtils.performAction(mContext, action);

        // Consumed -- don't let a raw KEY_C/KEY_M/etc event fall through
        // to whatever has focus.
        return null;
    }
}
