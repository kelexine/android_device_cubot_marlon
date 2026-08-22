/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.gestures;

import android.content.ContentResolver;
import android.provider.Settings;

import androidx.preference.PreferenceDataStore;

/**
 * Backs per-gesture action preferences with Settings.System for cross-process
 * readability by KeyHandler in system_server.
 */
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
