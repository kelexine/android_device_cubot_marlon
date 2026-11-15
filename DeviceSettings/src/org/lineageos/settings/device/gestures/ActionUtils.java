/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.gestures;

import android.app.KeyguardManager;
import android.content.Context;
import android.content.Intent;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.os.SystemClock;
import android.os.UserHandle;
import android.provider.MediaStore;
import android.util.Log;

public final class ActionUtils {

    private static final String TAG = "DeviceSettings-Action";
    private static final String WAKE_REASON = "MarlonKeyHandler";
    private static final long GESTURE_WAKELOCK_MS = 3000L;

    private static final Object sTorchLock = new Object();
    private static volatile boolean sTorchEnabled = false;
    private static volatile boolean sTorchCallbackRegistered = false;
    private static String sTorchCameraId = null;

    private ActionUtils() {
        // no instances
    }

    public static void performAction(Context context, GestureAction action) {
        switch (action) {
            case CAMERA:
                launchCamera(context);
                break;
            case FLASHLIGHT:
                toggleFlashlight(context);
                break;
            case BROWSER:
                launchBrowser(context);
                break;
            case VOICE_ASSIST:
                launchVoiceAssist(context);
                break;
            case NONE:
            default:
                // nothing configured for this gesture
                break;
        }
    }

    private static void wakeAndStartActivity(Context context, Intent intent) {
        PowerManager powerManager = context.getSystemService(PowerManager.class);
        if (powerManager != null) {
            PowerManager.WakeLock wakeLock = powerManager.newWakeLock(
                    PowerManager.PARTIAL_WAKE_LOCK, TAG + ":GestureWake");
            wakeLock.acquire(GESTURE_WAKELOCK_MS);
            powerManager.wakeUp(
                    SystemClock.uptimeMillis(), PowerManager.WAKE_REASON_GESTURE, WAKE_REASON);
        }
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
        context.startActivityAsUser(intent, UserHandle.CURRENT);
    }

    private static void launchCamera(Context context) {
        try {
            KeyguardManager km = context.getSystemService(KeyguardManager.class);
            boolean locked = km != null && km.isKeyguardLocked();
            String action = locked
                    ? MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE
                    : MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA;
            wakeAndStartActivity(context, new Intent(action));
        } catch (Exception e) {
            Log.e(TAG, "Failed to launch camera", e);
        }
    }

    private static void launchBrowser(Context context) {
        try {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.addCategory(Intent.CATEGORY_APP_BROWSER);
            wakeAndStartActivity(context, intent);
        } catch (Exception e) {
            Log.e(TAG, "Failed to launch browser", e);
        }
    }

    private static void launchVoiceAssist(Context context) {
        try {
            wakeAndStartActivity(context, new Intent(Intent.ACTION_VOICE_COMMAND));
        } catch (Exception e) {
            Log.e(TAG, "Failed to launch voice assist", e);
        }
    }

    private static void ensureTorchCallbackRegistered(CameraManager cameraManager) {
        if (sTorchCallbackRegistered) {
            return;
        }
        cameraManager.registerTorchCallback(new CameraManager.TorchCallback() {
            @Override
            public void onTorchModeUnavailable(String cameraId) {
                if (cameraId.equals(sTorchCameraId)) {
                    sTorchEnabled = false;
                }
            }

            @Override
            public void onTorchModeChanged(String cameraId, boolean enabled) {
                if (cameraId.equals(sTorchCameraId)) {
                    sTorchEnabled = enabled;
                }
            }
        }, new Handler(Looper.getMainLooper()));
        sTorchCallbackRegistered = true;
    }

    private static String findRearFlashCameraId(CameraManager cameraManager)
            throws CameraAccessException {
        if (sTorchCameraId != null) {
            return sTorchCameraId;
        }
        for (String id : cameraManager.getCameraIdList()) {
            CameraCharacteristics chars = cameraManager.getCameraCharacteristics(id);
            Boolean hasFlash = chars.get(CameraCharacteristics.FLASH_INFO_AVAILABLE);
            Integer facing = chars.get(CameraCharacteristics.LENS_FACING);
            if (Boolean.TRUE.equals(hasFlash)
                    && facing != null
                    && facing == CameraCharacteristics.LENS_FACING_BACK) {
                sTorchCameraId = id;
                return id;
            }
        }
        return null;
    }

    private static void toggleFlashlight(Context context) {
        CameraManager cameraManager = context.getSystemService(CameraManager.class);
        if (cameraManager == null) {
            Log.w(TAG, "No CameraManager available, cannot toggle flashlight");
            return;
        }
        synchronized (sTorchLock) {
            try {
                String torchCameraId = findRearFlashCameraId(cameraManager);
                if (torchCameraId == null) {
                    Log.w(TAG, "No rear camera with flash found");
                    return;
                }
                ensureTorchCallbackRegistered(cameraManager);
                boolean targetState = !sTorchEnabled;
                cameraManager.setTorchMode(torchCameraId, targetState);
                sTorchEnabled = targetState;
            } catch (CameraAccessException e) {
                Log.e(TAG, "Failed to toggle flashlight", e);
            }
        }
    }
}
