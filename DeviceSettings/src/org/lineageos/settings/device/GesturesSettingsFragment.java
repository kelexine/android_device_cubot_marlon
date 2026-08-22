/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 */
package org.lineageos.settings.device;

import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.SwitchPreferenceCompat;

import org.lineageos.settings.device.gestures.DoubleTapToWakeController;

public class GesturesSettingsFragment extends PreferenceFragmentCompat
        implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "DeviceSettings-Gestures";
    private static final String KEY_DOUBLE_TAP_TO_WAKE =
            BootCompletedReceiver.KEY_DOUBLE_TAP_TO_WAKE;

    private SwitchPreferenceCompat mDoubleTapToWakePref;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.gestures_settings, rootKey);

        mDoubleTapToWakePref = findPreference(KEY_DOUBLE_TAP_TO_WAKE);
        if (mDoubleTapToWakePref == null) {
            return;
        }

        if (!DoubleTapToWakeController.isSupported()) {
            // No usable control node on this kernel/driver build -- hide
            // rather than show a toggle that silently does nothing.
            mDoubleTapToWakePref.setVisible(false);
            Log.i(TAG, "DT2W control node not found, hiding preference");
            return;
        }

        mDoubleTapToWakePref.setChecked(DoubleTapToWakeController.isEnabled());
        mDoubleTapToWakePref.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
        if (KEY_DOUBLE_TAP_TO_WAKE.equals(preference.getKey())) {
            boolean enabled = (Boolean) newValue;
            boolean applied = DoubleTapToWakeController.setEnabled(enabled);
            if (!applied) {
                Log.w(TAG, "Failed to apply double_tap_to_wake=" + enabled);
                return false;
            }
            return true;
        }
        return false;
    }
}
