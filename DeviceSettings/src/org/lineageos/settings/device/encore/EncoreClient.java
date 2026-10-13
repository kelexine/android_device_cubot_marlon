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

    private synchronized IEncore connectToService() {
        if (mService != null) {
            return mService;
        }

        IBinder binder = ServiceManager.getService(SERVICE_NAME);
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
        if (service == null) return null;
        try {
            return service.getStats();
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in getStats", e);
            return null;
        }
    }
}
