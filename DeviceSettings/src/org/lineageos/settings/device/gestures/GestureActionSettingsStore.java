/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 *
 * Backs the per-gesture action ListPreferences with Settings.System instead
 * of the app's default (private, per-UID) SharedPreferences file. This
 * matters because KeyHandler reads these values from inside system_server,
 * a different UID than this app -- standard Android app-data sandboxing
 * (DAC + SELinux) means system_server cannot reliably read this app's
 * private shared_prefs file, but Settings.System is a ContentProvider
 * that's designed for exactly this kind of cross-process read. Enable/
 * disable toggles do NOT go through this store: their true state is the
 * hardware node itself (see GestureNodeController), read live by both the
 * app and KeyHandler, so there's nothing to share via Settings.System there.
 */
package org.lineageos.settings.device.gestures;

import android.content.ContentResolver;
import android.provider.Settings;

import androidx.preference.PreferenceDataStore;

public final class GestureActionSettingsStore extends PreferenceDataStore {

    private final ContentResolver mResolver;

    public GestureActionSettingsStore(ContentResolver resolver) {
        mResolver = resolver;
    }

    @Override
    public void putString(String key, String value) {
        Settings.System.putString(mResolver, key, value);
    }

    @Override
    public String getString(String key, String defValue) {
        String value = Settings.System.getString(mResolver, key);
        return value != null ? value : defValue;
    }
}
