/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * Author: Onogwu Franklin Kelechi (kelexine) <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Singleton client for Encore native AIDL hardware service on Cubot P50.
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.encore;

import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import org.lineageos.settings.device.utils.FileUtils;

import vendor.cubot.hardware.encore.EncoreStats;
import vendor.cubot.hardware.encore.IEncore;

public final class EncoreClient {

    private static final String TAG = "EncoreClient";
    private static final String SERVICE_NAME = "vendor.cubot.hardware.encore.IEncore/default";

    private static volatile EncoreClient sInstance;
    private IEncore mService;

    private final IBinder.DeathRecipient mDeathRecipient = new IBinder.DeathRecipient() {
        @Override
        public void binderDied() {
            Log.w(TAG, "Encore native service binder died, resetting connection");
            synchronized (EncoreClient.this) {
                mService = null;
            }
        }
    };

    private EncoreClient() {
        connectToService();
    }

    public static EncoreClient getInstance() {
        if (sInstance == null) {
            synchronized (EncoreClient.class) {
                if (sInstance == null) {
                    sInstance = new EncoreClient();
                }
            }
        }
        return sInstance;
    }

    public synchronized IEncore connectToService() {
        if (mService != null) {
            return mService;
        }

        IBinder binder = ServiceManager.getService(SERVICE_NAME);
        if (binder == null) {
            binder = ServiceManager.checkService(SERVICE_NAME);
        }
        if (binder == null) {
            try {
                binder = ServiceManager.waitForDeclaredService(SERVICE_NAME);
            } catch (Throwable t) {
                Log.d(TAG, "waitForDeclaredService: " + t.getMessage());
            }
        }

        if (binder == null) {
            Log.w(TAG, "Encore AIDL service unavailable: " + SERVICE_NAME);
            return null;
        }

        try {
            binder.linkToDeath(mDeathRecipient, 0);
            mService = IEncore.Stub.asInterface(binder);
            Log.i(TAG, "Successfully connected to Encore AIDL service");
        } catch (RemoteException e) {
            Log.e(TAG, "Failed to link to death for Encore service", e);
            mService = null;
        }

        return mService;
    }

    public boolean isAvailable() {
        return connectToService() != null;
    }

    public boolean setProfile(int profile) {
        IEncore service = connectToService();
        if (service == null) return false;
        try {
            service.setProfile(profile);
            return true;
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in setProfile", e);
            return false;
        }
    }

    public int getProfile() {
        IEncore service = connectToService();
        if (service == null) return IEncore.PROFILE_BALANCED;
        try {
            return service.getProfile();
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in getProfile", e);
            return IEncore.PROFILE_BALANCED;
        }
    }

    public boolean setGameMode(boolean enabled, String packageName) {
        IEncore service = connectToService();
        if (service == null) return false;
        try {
            service.setGameMode(enabled, packageName != null ? packageName : "");
            return true;
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in setGameMode", e);
            return false;
        }
    }

    public boolean isGameMode() {
        IEncore service = connectToService();
        if (service == null) return false;
        try {
            return service.isGameMode();
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in isGameMode", e);
            return false;
        }
    }

    public boolean setTouchBoost(boolean enabled) {
        IEncore service = connectToService();
        if (service == null) return false;
        try {
            service.setTouchBoost(enabled);
            return true;
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in setTouchBoost", e);
            return false;
        }
    }

    public boolean isTouchBoost() {
        IEncore service = connectToService();
        if (service == null) return false;
        try {
            return service.isTouchBoost();
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in isTouchBoost", e);
            return false;
        }
    }

    public boolean setThermalMitigation(boolean enabled) {
        IEncore service = connectToService();
        if (service == null) return false;
        try {
            service.setThermalMitigation(enabled);
            return true;
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in setThermalMitigation", e);
            return false;
        }
    }

    public boolean isThermalMitigation() {
        IEncore service = connectToService();
        if (service == null) return false;
        try {
            return service.isThermalMitigation();
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in isThermalMitigation", e);
            return false;
        }
    }

    public EncoreStats getStats() {
        IEncore service = connectToService();
        if (service != null) {
            try {
                return service.getStats();
            } catch (RemoteException e) {
                Log.e(TAG, "RemoteException in getStats", e);
            }
        }
        return getFallbackStats();
    }

    public EncoreStats getFallbackStats() {
        EncoreStats stats = new EncoreStats();
        // Little cluster (cpu4 / policy4: 400 MHz - 1.8 GHz)
        String freqLittle = FileUtils.readOneLine("/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq");
        if (freqLittle != null) {
            try {
                stats.cpuFreqLittleKhz = Integer.parseInt(freqLittle.trim());
            } catch (Exception ignored) {}
        } else {
            stats.cpuFreqLittleKhz = 1800000;
        }

        // Big cluster (cpu0 / policy0: 900 MHz - 2.3 GHz)
        String freqBig = FileUtils.readOneLine("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
        if (freqBig != null) {
            try {
                stats.cpuFreqBigKhz = Integer.parseInt(freqBig.trim());
            } catch (Exception ignored) {}
        } else {
            stats.cpuFreqBigKhz = 2301000;
        }

        // Thermal zone 0
        String temp = FileUtils.readOneLine("/sys/class/thermal/thermal_zone0/temp");
        if (temp != null) {
            try {
                stats.temperatureMilliCelsius = Integer.parseInt(temp.trim());
            } catch (Exception ignored) {}
        } else {
            stats.temperatureMilliCelsius = 38000;
        }

        stats.gpuFreqMhz = 400; // Baseline GE8320 frequency
        stats.activeProfile = IEncore.PROFILE_BALANCED;
        stats.isGameMode = false;
        stats.activePackage = "";
        return stats;
    }
}
