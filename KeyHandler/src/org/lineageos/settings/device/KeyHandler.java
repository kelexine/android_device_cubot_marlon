/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
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

/**
 * Intercepts off-screen hardware gesture key events in system_server.
 */
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
