/*
 * Authored for device/cubot/marlon.
 * Author: kelexine (https://github.com/kelexine)
 */
package org.lineageos.settings.device;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

public class DeviceSettingsActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        if (savedInstanceState == null) {
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.content_frame, new GesturesSettingsFragment())
                    .commit();
        }
    }
}
