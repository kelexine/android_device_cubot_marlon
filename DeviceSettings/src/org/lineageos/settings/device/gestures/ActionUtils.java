/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 */
package org.lineageos.settings.device.gestures;

import android.content.Context;
import android.content.Intent;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.provider.MediaStore;
import android.util.Log;

public final class ActionUtils {

    private static final String TAG = "DeviceSettings-Action";

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

    private static void launchCamera(Context context) {
        try {
            Intent intent = new Intent(MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            context.startActivity(intent);
        } catch (Exception e) {
            Log.e(TAG, "Failed to launch camera", e);
        }
    }

    private static void launchBrowser(Context context) {
        try {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.addCategory(Intent.CATEGORY_APP_BROWSER);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            context.startActivity(intent);
        } catch (Exception e) {
            Log.e(TAG, "Failed to launch browser", e);
        }
    }

    private static void launchVoiceAssist(Context context) {
        try {
            Intent intent = new Intent(Intent.ACTION_VOICE_COMMAND);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            context.startActivity(intent);
        } catch (Exception e) {
            Log.e(TAG, "Failed to launch voice assist", e);
        }
    }

    private static void toggleFlashlight(Context context) {
        CameraManager cameraManager =
                (CameraManager) context.getSystemService(Context.CAMERA_SERVICE);
        if (cameraManager == null) {
            Log.w(TAG, "No CameraManager available, cannot toggle flashlight");
            return;
        }
        try {
            String torchCameraId = null;
            for (String id : cameraManager.getCameraIdList()) {
                CameraCharacteristics chars = cameraManager.getCameraCharacteristics(id);
                Boolean hasFlash = chars.get(CameraCharacteristics.FLASH_INFO_AVAILABLE);
                Integer facing = chars.get(CameraCharacteristics.LENS_FACING);
                if (Boolean.TRUE.equals(hasFlash)
                        && facing != null
                        && facing == CameraCharacteristics.LENS_FACING_BACK) {
                    torchCameraId = id;
                    break;
                }
            }
            if (torchCameraId == null) {
                Log.w(TAG, "No rear camera with flash found");
                return;
            }
            // setTorchMode toggles based on current state tracked by
            // CameraManager itself; there's no direct "is torch on" query
            // here, so this always requests "on". A future revision could
            // track last-known state locally if strict toggle behavior
            // (on/off/on/off) turns out to matter more than "always on".
            cameraManager.setTorchMode(torchCameraId, true);
        } catch (CameraAccessException e) {
            Log.e(TAG, "Failed to toggle flashlight", e);
        }
    }
}
