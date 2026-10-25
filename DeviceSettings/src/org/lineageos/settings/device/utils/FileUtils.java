/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.device.utils;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

/**
 * File I/O helper for sysfs/procfs control nodes.
 */
public final class FileUtils {

    private static final String TAG = "DeviceSettings-FileUtils";

    private FileUtils() {
        // no instances
    }

    public static boolean isFileReadable(String path) {
        if (path == null) {
            return false;
        }
        File file = new File(path);
        return file.exists() && file.canRead();
    }

    public static boolean isFileWritable(String path) {
        if (path == null) {
            return false;
        }
        File file = new File(path);
        if (!file.exists()) {
            return false;
        }
        if (file.canWrite()) {
            return true;
        }
        // Sysfs and procfs virtual inodes often report false to Java File.canWrite()
        // even when DAC permissions and SELinux grant write access to system/root.
        return path.startsWith("/sys/") || path.startsWith("/proc/");
    }

    /**
     * Returns the first path in {@code candidates} that is both readable and
     * writable, or {@code null} if none qualify. Used to pick between the
     * multiple known-equivalent control node paths a single feature may be
     * exposed under (kernel/driver version dependent).
     */
    public static String firstUsablePath(String... candidates) {
        if (candidates == null) {
            return null;
        }
        for (String candidate : candidates) {
            if (candidate == null) {
                continue;
            }
            if (isFileReadable(candidate) && isFileWritable(candidate)) {
                return candidate;
            }
        }
        return null;
    }

    public static String readOneLine(String path) {
        if (path == null) {
            return null;
        }
        String line = null;
        try (BufferedReader reader = new BufferedReader(new FileReader(path))) {
            line = reader.readLine();
        } catch (IOException e) {
            Log.e(TAG, "Failed to read " + path, e);
        }
        return line;
    }

    public static boolean writeLine(String path, String value) {
        if (path == null || value == null) {
            return false;
        }
        try (FileWriter writer = new FileWriter(path)) {
            writer.write(value);
            return true;
        } catch (IOException e) {
            Log.e(TAG, "Failed to write '" + value + "' to " + path, e);
            return false;
        }
    }

    public static boolean setValue(String path, boolean enabled) {
        return writeLine(path, enabled ? "1" : "0");
    }

    public static boolean getValue(String path, boolean defaultValue) {
        String line = readOneLine(path);
        if (line == null) {
            return defaultValue;
        }
        return "1".equals(line.trim());
    }
}
