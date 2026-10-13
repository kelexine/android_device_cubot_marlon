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
import org.lineageos.settings.device.kernel.CompactionController;
import org.lineageos.settings.device.kernel.CpuidleController;
import org.lineageos.settings.device.kernel.GamingModeController;
import org.lineageos.settings.device.kernel.IoSchedulerController;
import org.lineageos.settings.device.kernel.TcpCongestionController;

/**
 * Restores user-selected gesture and kernel settings on boot.
 */
public class BootCompletedReceiver extends BroadcastReceiver {

    private static final String TAG = "DeviceSettings-Boot";
    public static final String KEY_DOUBLE_TAP_TO_WAKE = "double_tap_to_wake";

    @Override
    public void onReceive(Context context, Intent intent) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);

        restoreGestures(prefs);
        restoreKernelSettings(prefs);
    }

    private void restoreGestures(SharedPreferences prefs) {
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

    private void restoreKernelSettings(SharedPreferences prefs) {
        if (GamingModeController.isSupported() && prefs.contains(GamingModeController.KEY_GAMING_MODE)) {
            boolean gaming = prefs.getBoolean(GamingModeController.KEY_GAMING_MODE, false);
            GamingModeController.setGamingMode(gaming);
        }

        if (CpuidleController.isSupported()) {
            String gov = prefs.getString(CpuidleController.KEY_CPUIDLE_GOVERNOR, CpuidleController.DEFAULT_GOVERNOR);
            CpuidleController.setGovernor(gov);
        }

        if (IoSchedulerController.isSchedulerSupported()) {
            String sched = prefs.getString(IoSchedulerController.KEY_IO_SCHEDULER, IoSchedulerController.DEFAULT_SCHEDULER);
            IoSchedulerController.setScheduler(sched);
        }

        if (IoSchedulerController.isReadaheadSupported()) {
            String ra = prefs.getString(IoSchedulerController.KEY_READ_AHEAD, IoSchedulerController.DEFAULT_READ_AHEAD_KB);
            IoSchedulerController.setReadaheadKb(ra);
        }

        if (TcpCongestionController.isSupported()) {
            String cc = prefs.getString(TcpCongestionController.KEY_TCP_CONGESTION, TcpCongestionController.DEFAULT_CONGESTION);
            TcpCongestionController.setCongestion(cc);
        }

        if (CompactionController.isSupported()) {
            String comp = prefs.getString(CompactionController.KEY_PROACTIVE_COMPACTION, CompactionController.DEFAULT_PROACTIVENESS);
            CompactionController.setProactiveness(comp);
        }
    }
}
