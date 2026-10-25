/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Fragment managing Encore performance profiles, game mode, and live telemetry.
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.encore;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.SwitchPreferenceCompat;

import org.lineageos.settings.device.R;

import vendor.cubot.hardware.encore.EncoreStats;
import vendor.cubot.hardware.encore.IEncore;

public class EncoreSettingsFragment extends PreferenceFragmentCompat
        implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "EncoreSettingsFragment";

    private static final String KEY_STATS = "encore_telemetry_stats";
    private static final String KEY_PROFILE = "encore_profile";
    private static final String KEY_GAMING_MODE = "encore_gaming_mode";
    private static final String KEY_TOUCH_BOOST = "encore_touch_boost";
    private static final String KEY_THERMAL = "encore_thermal_mitigation";

    private static final long TELEMETRY_REFRESH_MS = 1500L;

    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private final EncoreClient mClient = EncoreClient.getInstance();

    private Preference mStatsPref;
    private ListPreference mProfilePref;
    private SwitchPreferenceCompat mGamingModePref;
    private SwitchPreferenceCompat mTouchBoostPref;
    private SwitchPreferenceCompat mThermalPref;

    private TextView mBadgeStatus;
    private TextView mTextCpuLittle;
    private TextView mTextCpuBig;
    private TextView mTextGpuFreq;
    private TextView mTextThermalTemp;
    private TextView mTextActiveProfile;

    private final Runnable mTelemetryRunnable = new Runnable() {
        @Override
        public void run() {
            updateTelemetry();
            mHandler.postDelayed(this, TELEMETRY_REFRESH_MS);
        }
    };

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.encore_settings, rootKey);

        mStatsPref = findPreference(KEY_STATS);
        mProfilePref = findPreference(KEY_PROFILE);
        mGamingModePref = findPreference(KEY_GAMING_MODE);
        mTouchBoostPref = findPreference(KEY_TOUCH_BOOST);
        mThermalPref = findPreference(KEY_THERMAL);

        initPreferences();
    }

    private void initPreferences() {
        int currentProfile = mClient.getProfile();
        if (mProfilePref != null) {
            mProfilePref.setValue(String.valueOf(currentProfile));
            updateProfileSummary(mProfilePref, String.valueOf(currentProfile));
            mProfilePref.setOnPreferenceChangeListener(this);
        }

        if (mGamingModePref != null) {
            mGamingModePref.setChecked(mClient.isGameMode() || currentProfile == IEncore.PROFILE_GAMING_TURBO);
            mGamingModePref.setOnPreferenceChangeListener(this);
        }

        if (mTouchBoostPref != null) {
            mTouchBoostPref.setChecked(mClient.isTouchBoost());
            mTouchBoostPref.setOnPreferenceChangeListener(this);
        }

        if (mThermalPref != null) {
            mThermalPref.setChecked(mClient.isThermalMitigation());
            mThermalPref.setOnPreferenceChangeListener(this);
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        bindTelemetryViews(view);
    }

    private void bindTelemetryViews(View rootView) {
        if (rootView == null) return;
        mBadgeStatus = rootView.findViewById(R.id.badge_daemon_status);
        mTextCpuLittle = rootView.findViewById(R.id.text_cpu_little);
        mTextCpuBig = rootView.findViewById(R.id.text_cpu_big);
        mTextGpuFreq = rootView.findViewById(R.id.text_gpu_freq);
        mTextThermalTemp = rootView.findViewById(R.id.text_thermal_temp);
        mTextActiveProfile = rootView.findViewById(R.id.text_active_profile);
    }

    @Override
    public void onResume() {
        super.onResume();
        initPreferences();
        mHandler.post(mTelemetryRunnable);
    }

    @Override
    public void onPause() {
        super.onPause();
        mHandler.removeCallbacks(mTelemetryRunnable);
    }

    private void updateTelemetry() {
        if (getView() != null && mTextCpuLittle == null) {
            bindTelemetryViews(getView());
        }

        boolean daemonConnected = mClient.isAvailable();
        if (mBadgeStatus != null) {
            mBadgeStatus.setText(daemonConnected ? R.string.encore_status_connected : R.string.encore_status_direct);
        }

        EncoreStats stats = mClient.getStats();
        if (stats == null) {
            stats = mClient.getFallbackStats();
        }

        int littleMhz = stats.cpuFreqLittleKhz > 0 ? stats.cpuFreqLittleKhz / 1000 : 850;
        int bigMhz = stats.cpuFreqBigKhz > 0 ? stats.cpuFreqBigKhz / 1000 : 850;
        int gpuMhz = stats.gpuFreqMhz > 0 ? stats.gpuFreqMhz : 400;
        float tempC = stats.temperatureMilliCelsius > 0 ? stats.temperatureMilliCelsius / 1000.0f : 38.0f;

        if (mTextCpuLittle != null) {
            mTextCpuLittle.setText(String.format("%d MHz", littleMhz));
        }
        if (mTextCpuBig != null) {
            mTextCpuBig.setText(String.format("%d MHz", bigMhz));
        }
        if (mTextGpuFreq != null) {
            mTextGpuFreq.setText(String.format("%d MHz", gpuMhz));
        }
        if (mTextThermalTemp != null) {
            mTextThermalTemp.setText(String.format("%.1f °C", tempC));
        }

        String profileName = getProfileName(stats.activeProfile);
        String gameExtra = stats.isGameMode ? " (Gaming Turbo)" : "";
        if (mTextActiveProfile != null) {
            mTextActiveProfile.setText(getString(R.string.telemetry_active_profile_format, profileName, gameExtra));
        }

        // Also update standard preference summary for accessibility
        if (mStatsPref != null) {
            mStatsPref.setSummary(String.format(
                    "CPU: %d / %d MHz | GPU: %d MHz | Temp: %.1f °C",
                    littleMhz, bigMhz, gpuMhz, tempC));
        }
    }

    private String getProfileName(int profile) {
        switch (profile) {
            case 1: return "Gaming Turbo";
            case 2: return "Battery Saver";
            case 0:
            default: return "Balanced";
        }
    }

    private void updateProfileSummary(ListPreference pref, String value) {
        CharSequence[] entries = pref.getEntries();
        CharSequence[] values = pref.getEntryValues();
        if (entries != null && values != null) {
            for (int i = 0; i < values.length; i++) {
                if (values[i].toString().equals(value)) {
                    pref.setSummary(entries[i]);
                    return;
                }
            }
        }
        pref.setSummary(value);
    }

    @Override
    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
        String key = preference.getKey();
        if (KEY_PROFILE.equals(key)) {
            int profile = Integer.parseInt((String) newValue);
            boolean ok = mClient.setProfile(profile);
            updateProfileSummary((ListPreference) preference, (String) newValue);
            if (mGamingModePref != null) {
                mGamingModePref.setChecked(profile == 1);
            }
            updateTelemetry();
            return true;
        } else if (KEY_GAMING_MODE.equals(key)) {
            boolean enabled = (Boolean) newValue;
            boolean ok = mClient.setGameMode(enabled, "manual_ui");
            if (mProfilePref != null) {
                String targetProfile = enabled ? "1" : "0";
                mProfilePref.setValue(targetProfile);
                updateProfileSummary(mProfilePref, targetProfile);
            }
            updateTelemetry();
            return true;
        } else if (KEY_TOUCH_BOOST.equals(key)) {
            boolean ok = mClient.setTouchBoost((Boolean) newValue);
            return true;
        } else if (KEY_THERMAL.equals(key)) {
            boolean ok = mClient.setThermalMitigation((Boolean) newValue);
            return true;
        }
        return false;
    }
}
