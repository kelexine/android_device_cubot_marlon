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

/**
 * Restores user-selected gesture settings on boot.
 */
public class BootCompletedReceiver extends BroadcastReceiver {

    private static final String TAG = "DeviceSettings-Boot";
    public static final String KEY_DOUBLE_TAP_TO_WAKE = "double_tap_to_wake";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (!DoubleTapToWakeController.isSupported()) {
            Log.i(TAG, "DT2W not supported on this build/kernel, skipping restore");
            return;
        }

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        boolean wantEnabled = prefs.getBoolean(KEY_DOUBLE_TAP_TO_WAKE, false);

        boolean applied = DoubleTapToWakeController.setEnabled(wantEnabled);
        if (!applied) {
            Log.w(TAG, "Failed to restore double_tap_to_wake=" + wantEnabled + " on boot");
        }
    }
}
