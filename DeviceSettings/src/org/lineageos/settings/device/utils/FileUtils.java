/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 *
 * Minimal, dependency-free file I/O helper for sysfs/procfs control nodes.
 * Deliberately self-contained rather than depending on a shared lineage
 * settings library, since this tree has no confirmed prebuilt of one.
 */
package org.lineageos.settings.device.utils;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

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
        return file.exists() && file.canWrite();
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
            if (isFileReadable(candidate) && isFileWritable(candidate)) {
                return candidate;
            }
        }
        return null;
    }

    public static String readOneLine(String path) {
        String line = null;
        try (BufferedReader reader = new BufferedReader(new FileReader(path))) {
            line = reader.readLine();
        } catch (IOException e) {
            Log.e(TAG, "Failed to read " + path, e);
        }
        return line;
    }

    public static boolean writeLine(String path, String value) {
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
