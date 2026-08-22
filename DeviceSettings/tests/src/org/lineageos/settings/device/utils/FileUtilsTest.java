/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 *
 * NOTE ON RUNNING THESE: FileUtils itself only touches java.io + android.util.Log.
 * Log calls will no-op/throw under a plain JVM depending on the test runner's
 * Android stub jar; run these via the AOSP tree's standard device-tests
 * target (atest, or `m DeviceSettingsTests`) rather than a bare `javac`+junit
 * invocation, since that's the only environment with the Android stub jars
 * and an actual device/emulator (or Robolectric shadow) to back Log calls.
 * This sandbox has no AOSP source tree or Android SDK, so these are written
 * but not executed here -- verify with atest once this lands in your tree.
 */
package org.lineageos.settings.device.utils;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;

public class FileUtilsTest {

    private File mReadWriteFile;
    private File mNonexistentFile;

    @Before
    public void setUp() throws IOException {
        mReadWriteFile = File.createTempFile("device_settings_test_", ".node");
        mReadWriteFile.setReadable(true, false);
        mReadWriteFile.setWritable(true, false);

        mNonexistentFile = new File(mReadWriteFile.getParentFile(), "does_not_exist_node");
        if (mNonexistentFile.exists()) {
            mNonexistentFile.delete();
        }
    }

    @After
    public void tearDown() {
        if (mReadWriteFile != null && mReadWriteFile.exists()) {
            mReadWriteFile.delete();
        }
    }

    @Test
    public void isFileReadable_returnsFalse_forNullPath() {
        assertFalse(FileUtils.isFileReadable(null));
    }

    @Test
    public void isFileReadable_returnsFalse_forMissingFile() {
        assertFalse(FileUtils.isFileReadable(mNonexistentFile.getAbsolutePath()));
    }

    @Test
    public void isFileReadable_returnsTrue_forExistingReadableFile() {
        assertTrue(FileUtils.isFileReadable(mReadWriteFile.getAbsolutePath()));
    }

    @Test
    public void isFileWritable_returnsFalse_forNullPath() {
        assertFalse(FileUtils.isFileWritable(null));
    }

    @Test
    public void firstUsablePath_returnsNull_whenNoCandidateExists() {
        String result = FileUtils.firstUsablePath(
                mNonexistentFile.getAbsolutePath(),
                mNonexistentFile.getAbsolutePath() + "_2");
        assertNull(result);
    }

    @Test
    public void firstUsablePath_skipsMissingCandidates_returnsFirstUsableOne() {
        String result = FileUtils.firstUsablePath(
                mNonexistentFile.getAbsolutePath(),
                mReadWriteFile.getAbsolutePath());
        assertEquals(mReadWriteFile.getAbsolutePath(), result);
    }

    @Test
    public void firstUsablePath_returnsNull_forEmptyCandidateList() {
        assertNull(FileUtils.firstUsablePath());
    }

    @Test
    public void writeLine_thenReadOneLine_roundTrips() {
        String path = mReadWriteFile.getAbsolutePath();
        boolean written = FileUtils.writeLine(path, "1");
        assertTrue(written);
        assertEquals("1", FileUtils.readOneLine(path));
    }

    @Test
    public void setValue_true_writesOne_andGetValueReadsTrue() {
        String path = mReadWriteFile.getAbsolutePath();
        assertTrue(FileUtils.setValue(path, true));
        assertTrue(FileUtils.getValue(path, false));
    }

    @Test
    public void setValue_false_writesZero_andGetValueReadsFalse() {
        String path = mReadWriteFile.getAbsolutePath();
        assertTrue(FileUtils.setValue(path, false));
        assertFalse(FileUtils.getValue(path, true));
    }

    @Test
    public void getValue_returnsDefault_whenFileMissing() {
        boolean result = FileUtils.getValue(mNonexistentFile.getAbsolutePath(), true);
        assertTrue(result);

        result = FileUtils.getValue(mNonexistentFile.getAbsolutePath(), false);
        assertFalse(result);
    }
}
