/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device;

import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.SwitchPreferenceCompat;

import org.lineageos.settings.device.gestures.DoubleTapToWakeController;
import org.lineageos.settings.device.gestures.GestureActionSettingsStore;
import org.lineageos.settings.device.gestures.GestureInfo;
import org.lineageos.settings.device.gestures.GestureNodeController;
import org.lineageos.settings.device.gestures.GestureRegistry;

public class GesturesSettingsFragment extends PreferenceFragmentCompat
        implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "DeviceSettings-Gestures";
    private static final String KEY_DOUBLE_TAP_TO_WAKE =
            BootCompletedReceiver.KEY_DOUBLE_TAP_TO_WAKE;

    private GestureActionSettingsStore mActionStore;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.gestures_settings, rootKey);

        mActionStore = new GestureActionSettingsStore(
                requireContext().getContentResolver());

        setUpDoubleTapToWake();

        for (GestureInfo gesture : GestureRegistry.ALL) {
            setUpGestureToggle(gesture);
            if (gesture.configurableAction) {
                setUpActionPicker(gesture);
            }
        }
    }

    private void setUpDoubleTapToWake() {
        SwitchPreferenceCompat pref = findPreference(KEY_DOUBLE_TAP_TO_WAKE);
        if (pref == null) {
            return;
        }
        if (!DoubleTapToWakeController.isSupported()) {
            pref.setVisible(false);
            Log.i(TAG, "DT2W control node not found, hiding preference");
            return;
        }
        pref.setChecked(DoubleTapToWakeController.isEnabled());
        pref.setOnPreferenceChangeListener(this);
    }

    private void setUpGestureToggle(GestureInfo gesture) {
        SwitchPreferenceCompat pref = findPreference(gesture.preferenceKey);
        if (pref == null) {
            return;
        }

        if (!GestureNodeController.isSupported(gesture)) {
            // No usable control node on this kernel/driver build -- hide
            // both the toggle and its action picker rather than show
            // controls that silently do nothing.
            pref.setVisible(false);
            if (gesture.configurableAction) {
                Preference actionPref = findPreference(gesture.actionPreferenceKey());
                if (actionPref != null) {
                    actionPref.setVisible(false);
                }
            }
            Log.i(TAG, "No control node for " + gesture.preferenceKey + ", hiding");
            return;
        }

        pref.setChecked(GestureNodeController.isEnabled(gesture));
        pref.setOnPreferenceChangeListener(this);
    }

    private void setUpActionPicker(GestureInfo gesture) {
        ListPreference pref = findPreference(gesture.actionPreferenceKey());
        if (pref == null) {
            return;
        }
        // Route through Settings.System instead of this app's private
        // SharedPreferences -- KeyHandler reads this from system_server,
        // a different UID, which cannot reliably read our private prefs
        // file. See GestureActionSettingsStore for the full rationale.
        pref.setPreferenceDataStore(mActionStore);
    }

    @Override
    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
        String key = preference.getKey();

        if (KEY_DOUBLE_TAP_TO_WAKE.equals(key)) {
            boolean enabled = (Boolean) newValue;
            boolean applied = DoubleTapToWakeController.setEnabled(enabled);
            if (!applied) {
                Log.w(TAG, "Failed to apply double_tap_to_wake=" + enabled);
            }
            return applied;
        }

        for (GestureInfo gesture : GestureRegistry.ALL) {
            if (gesture.preferenceKey.equals(key)) {
                boolean enabled = (Boolean) newValue;
                boolean applied = GestureNodeController.setEnabled(gesture, enabled);
                if (!applied) {
                    Log.w(TAG, "Failed to apply " + key + "=" + enabled);
                }
                return applied;
            }
        }

        return true;
    }
}
