/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-09-18
 * Purpose: Main settings fragment managing gestures and kernel tunables.
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.SwitchPreferenceCompat;

import org.lineageos.settings.device.gestures.DoubleTapToWakeController;
import org.lineageos.settings.device.kernel.CompactionController;
import org.lineageos.settings.device.kernel.CpuidleController;
import org.lineageos.settings.device.kernel.GamingModeController;
import org.lineageos.settings.device.kernel.IoSchedulerController;
import org.lineageos.settings.device.kernel.TcpCongestionController;

import java.util.List;

public class DeviceSettingsFragment extends PreferenceFragmentCompat
        implements Preference.OnPreferenceChangeListener, Preference.OnPreferenceClickListener {

    private static final String TAG = "DeviceSettings-Main";
    private static final String KEY_DOUBLE_TAP_TO_WAKE =
            BootCompletedReceiver.KEY_DOUBLE_TAP_TO_WAKE;
    private static final String KEY_SCREEN_GESTURES = "key_screen_gestures";
    private static final String KEY_ENCORE_SETTINGS = "key_encore_settings";

    private TextView mChipCpuGov;
    private TextView mChipIoSched;
    private TextView mChipTcpCc;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.device_settings, rootKey);

        setUpDoubleTapToWake();
        setUpScreenGesturesNavigation();
        setUpEncoreNavigation();
        setUpGamingMode();
        setUpCpuidleGovernor();
        setUpIoScheduler();
        setUpReadahead();
        setUpTcpCongestion();
        setUpProactiveCompaction();
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        bindHeaderChips(view);
        updateHeaderChips();
    }

    private void bindHeaderChips(View rootView) {
        if (rootView == null) return;
        mChipCpuGov = rootView.findViewById(R.id.chip_cpu_gov);
        mChipIoSched = rootView.findViewById(R.id.chip_io_sched);
        mChipTcpCc = rootView.findViewById(R.id.chip_tcp_cc);
    }

    private void updateHeaderChips() {
        if (mChipCpuGov != null) {
            mChipCpuGov.setText("CPU: " + CpuidleController.getCurrentGovernor());
        }
        if (mChipIoSched != null) {
            mChipIoSched.setText("I/O: " + IoSchedulerController.getCurrentScheduler());
        }
        if (mChipTcpCc != null) {
            mChipTcpCc.setText("TCP: " + TcpCongestionController.getCurrentCongestion().toUpperCase());
        }
    }

    private void setUpDoubleTapToWake() {
        SwitchPreferenceCompat pref = findPreference(KEY_DOUBLE_TAP_TO_WAKE);
        if (pref == null) {
            return;
        }
        if (!DoubleTapToWakeController.isSupported()) {
            pref.setEnabled(false);
            pref.setSummary(R.string.feature_not_supported_summary);
            Log.i(TAG, "DT2W control node not found");
            return;
        }
        pref.setChecked(DoubleTapToWakeController.isEnabled());
        pref.setOnPreferenceChangeListener(this);
    }

    private void setUpScreenGesturesNavigation() {
        Preference pref = findPreference(KEY_SCREEN_GESTURES);
        if (pref != null) {
            pref.setOnPreferenceClickListener(this);
        }
    }

    private void setUpEncoreNavigation() {
        Preference pref = findPreference(KEY_ENCORE_SETTINGS);
        if (pref != null) {
            pref.setOnPreferenceClickListener(this);
        }
    }

    private void setUpGamingMode() {
        SwitchPreferenceCompat pref = findPreference(GamingModeController.KEY_GAMING_MODE);
        if (pref == null) {
            return;
        }
        if (!GamingModeController.isSupported()) {
            pref.setEnabled(false);
            pref.setSummary(R.string.feature_not_supported_summary);
            Log.i(TAG, "Gaming mode nodes not found");
            return;
        }
        pref.setChecked(GamingModeController.isGamingModeEnabled());
        pref.setOnPreferenceChangeListener(this);
    }

    private void setUpCpuidleGovernor() {
        ListPreference pref = findPreference(CpuidleController.KEY_CPUIDLE_GOVERNOR);
        if (pref == null) {
            return;
        }
        List<String> governors = CpuidleController.getAvailableGovernors();
        CharSequence[] entries = governors.toArray(new CharSequence[0]);
        pref.setEntries(entries);
        pref.setEntryValues(entries);
        String currentGov = CpuidleController.getCurrentGovernor();
        pref.setValue(currentGov);
        pref.setSummary(currentGov);
        pref.setOnPreferenceChangeListener(this);
    }

    private void setUpIoScheduler() {
        ListPreference pref = findPreference(IoSchedulerController.KEY_IO_SCHEDULER);
        if (pref == null) {
            return;
        }
        List<String> schedulers = IoSchedulerController.getAvailableSchedulers();
        CharSequence[] entries = schedulers.toArray(new CharSequence[0]);
        pref.setEntries(entries);
        pref.setEntryValues(entries);
        String currentSched = IoSchedulerController.getCurrentScheduler();
        pref.setValue(currentSched);
        pref.setSummary(currentSched);
        pref.setOnPreferenceChangeListener(this);
    }

    private void setUpReadahead() {
        ListPreference pref = findPreference(IoSchedulerController.KEY_READ_AHEAD);
        if (pref == null) {
            return;
        }
        String currentKb = IoSchedulerController.getCurrentReadaheadKb();
        pref.setValue(currentKb);
        pref.setSummary(currentKb + " KB");
        pref.setOnPreferenceChangeListener(this);
    }

    private void setUpTcpCongestion() {
        ListPreference pref = findPreference(TcpCongestionController.KEY_TCP_CONGESTION);
        if (pref == null) {
            return;
        }
        List<String> algos = TcpCongestionController.getAvailableCongestionAlgorithms();
        CharSequence[] entries = algos.toArray(new CharSequence[0]);
        pref.setEntries(entries);
        pref.setEntryValues(entries);
        String currentCc = TcpCongestionController.getCurrentCongestion();
        pref.setValue(currentCc);
        pref.setSummary(currentCc);
        pref.setOnPreferenceChangeListener(this);
    }

    private void setUpProactiveCompaction() {
        ListPreference pref = findPreference(CompactionController.KEY_PROACTIVE_COMPACTION);
        if (pref == null) {
            return;
        }
        String currentVal = CompactionController.getProactiveness();
        pref.setValue(currentVal);
        pref.setSummary(currentVal);
        pref.setOnPreferenceChangeListener(this);
    }

    @Override
    public boolean onPreferenceClick(@NonNull Preference preference) {
        if (KEY_SCREEN_GESTURES.equals(preference.getKey())) {
            getParentFragmentManager()
                    .beginTransaction()
                    .replace(com.android.settingslib.collapsingtoolbar.R.id.content_frame, new GesturesSettingsFragment())
                    .addToBackStack(null)
                    .commit();
            return true;
        } else if (KEY_ENCORE_SETTINGS.equals(preference.getKey())) {
            getParentFragmentManager()
                    .beginTransaction()
                    .replace(com.android.settingslib.collapsingtoolbar.R.id.content_frame, new org.lineageos.settings.device.encore.EncoreSettingsFragment())
                    .addToBackStack(null)
                    .commit();
            return true;
        }
        return false;
    }

    @Override
    public boolean onPreferenceChange(@NonNull Preference preference, Object newValue) {
        String key = preference.getKey();

        if (KEY_DOUBLE_TAP_TO_WAKE.equals(key)) {
            boolean enabled = (Boolean) newValue;
            return DoubleTapToWakeController.setEnabled(enabled);
        } else if (GamingModeController.KEY_GAMING_MODE.equals(key)) {
            boolean enabled = (Boolean) newValue;
            return GamingModeController.setGamingMode(enabled);
        } else if (CpuidleController.KEY_CPUIDLE_GOVERNOR.equals(key)) {
            String gov = (String) newValue;
            boolean applied = CpuidleController.setGovernor(gov);
            if (applied) {
                preference.setSummary(gov);
                updateHeaderChips();
            }
            return applied;
        } else if (IoSchedulerController.KEY_IO_SCHEDULER.equals(key)) {
            String sched = (String) newValue;
            boolean applied = IoSchedulerController.setScheduler(sched);
            if (applied) {
                preference.setSummary(sched);
                updateHeaderChips();
            }
            return applied;
        } else if (IoSchedulerController.KEY_READ_AHEAD.equals(key)) {
            String kb = (String) newValue;
            boolean applied = IoSchedulerController.setReadaheadKb(kb);
            if (applied) {
                preference.setSummary(kb + " KB");
            }
            return applied;
        } else if (TcpCongestionController.KEY_TCP_CONGESTION.equals(key)) {
            String cc = (String) newValue;
            boolean applied = TcpCongestionController.setCongestion(cc);
            if (applied) {
                preference.setSummary(cc);
                updateHeaderChips();
            }
            return applied;
        } else if (CompactionController.KEY_PROACTIVE_COMPACTION.equals(key)) {
            String val = (String) newValue;
            boolean applied = CompactionController.setProactiveness(val);
            if (applied) {
                preference.setSummary(val);
            }
            return applied;
        }

        return true;
    }
}
