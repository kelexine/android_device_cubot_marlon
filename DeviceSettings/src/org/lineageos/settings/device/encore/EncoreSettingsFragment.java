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

import androidx.annotation.NonNull;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.SwitchPreferenceCompat;

import org.lineageos.settings.device.R;

import vendor.cubot.hardware.encore.EncoreStats;

public class EncoreSettingsFragment extends PreferenceFragmentCompat
        implements Preference.OnPreferenceChangeListener {

    private static final String TAG = "EncoreSettingsFragment";

    private static final String KEY_STATS = "encore_telemetry_stats";
    private static final String KEY_PROFILE = "encore_profile";
    private static final String KEY_GAMING_MODE = "encore_gaming_mode";
    private static final String KEY_TOUCH_BOOST = "encore_touch_boost";
    private static final String KEY_THERMAL = "encore_thermal_mitigation";

    private static final long TELEMETRY_REFRESH_MS = 2000L;

    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private final EncoreClient mClient = EncoreClient.getInstance();

    private Preference mStatsPref;
    private ListPreference mProfilePref;
    private SwitchPreferenceCompat mGamingModePref;
    private SwitchPreferenceCompat mTouchBoostPref;
    private SwitchPreferenceCompat mThermalPref;

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
        if (!mClient.isAvailable()) {
            if (mStatsPref != null) {
                mStatsPref.setSummary(R.string.encore_stats_loading);
            }
            return;
        }

        if (mProfilePref != null) {
            int currentProfile = mClient.getProfile();
            mProfilePref.setValue(String.valueOf(currentProfile));
            mProfilePref.setOnPreferenceChangeListener(this);
        }

        if (mGamingModePref != null) {
            mGamingModePref.setChecked(mClient.isGameMode());
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
        if (mStatsPref == null || !mClient.isAvailable()) {
            return;
        }

        EncoreStats stats = mClient.getStats();
        if (stats == null) {
            mStatsPref.setSummary(R.string.encore_stats_loading);
            return;
        }

        String summary = String.format(
                "CPU Little: %d MHz | Big: %d MHz\n" +
                "GPU: %d MHz | Thermal: %.1f °C\n" +
                "Active Mode: %s%s",
                stats.cpuFreqLittleKhz / 1000,
                stats.cpuFreqBigKhz / 1000,
                stats.gpuFreqMhz,
                stats.temperatureMilliCelsius / 1000.0f,
                getProfileName(stats.activeProfile),
                stats.isGameMode ? " (Gaming Turbo)" : ""
        );
        mStatsPref.setSummary(summary);
    }

    private String getProfileName(int profile) {
        switch (profile) {
            case 1: return "Gaming Turbo";
            case 2: return "Battery Saver";
            case 0:
            default: return "Balanced";
        }
    }

    @Override
    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
        String key = preference.getKey();
        if (KEY_PROFILE.equals(key)) {
            int profile = Integer.parseInt((String) newValue);
            boolean ok = mClient.setProfile(profile);
            if (ok && mGamingModePref != null) {
                mGamingModePref.setChecked(profile == 1);
            }
            return ok;
        } else if (KEY_GAMING_MODE.equals(key)) {
            boolean enabled = (Boolean) newValue;
            boolean ok = mClient.setGameMode(enabled, "manual_ui");
            if (ok && mProfilePref != null) {
                mProfilePref.setValue(enabled ? "1" : "0");
            }
            return ok;
        } else if (KEY_TOUCH_BOOST.equals(key)) {
            return mClient.setTouchBoost((Boolean) newValue);
        } else if (KEY_THERMAL.equals(key)) {
            return mClient.setThermalMitigation((Boolean) newValue);
        }
        return false;
    }
}
