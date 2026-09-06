/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.preference.PreferenceManager;

import org.lineageos.settings.device.gestures.DoubleTapToWakeController;
import org.lineageos.settings.device.gestures.GestureInfo;
import org.lineageos.settings.device.gestures.GestureNodeController;
import org.lineageos.settings.device.gestures.GestureRegistry;

/**
 * Restores user-selected gesture settings on boot.
 */
public class BootCompletedReceiver extends BroadcastReceiver {

    private static final String TAG = "DeviceSettings-Boot";
    public static final String KEY_DOUBLE_TAP_TO_WAKE = "double_tap_to_wake";

    @Override
    public void onReceive(Context context, Intent intent) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);

        if (DoubleTapToWakeController.isSupported()) {
            boolean wantEnabled = prefs.getBoolean(KEY_DOUBLE_TAP_TO_WAKE, false);
            boolean applied = DoubleTapToWakeController.setEnabled(wantEnabled);
            if (!applied) {
                Log.w(TAG, "Failed to restore double_tap_to_wake=" + wantEnabled + " on boot");
            }
        } else {
            Log.i(TAG, "DT2W not supported on this build/kernel, skipping restore");
        }

        for (GestureInfo gesture : GestureRegistry.ALL) {
            if (GestureNodeController.isSupported(gesture)) {
                boolean enabled = prefs.getBoolean(gesture.preferenceKey, false);
                boolean applied = GestureNodeController.setEnabled(gesture, enabled);
                if (!applied) {
                    Log.w(TAG, "Failed to restore " + gesture.preferenceKey + "=" + enabled + " on boot");
                }
            }
        }
    }
}
