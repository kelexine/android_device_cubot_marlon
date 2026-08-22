/*
 * Distilled from device/xiaomi/certus (kelexine) for device/cubot/marlon.
 * Ported-by: kelexine (https://github.com/kelexine)
 *
 * OPT-IN / DISABLED BY DEFAULT -- see PhoneStateService.java for the
 * rationale and the caveats around porting this workaround unverified.
 */
package org.lineageos.cubot.imsinit;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

import com.android.ims.ImsManager;

public class BootCompletedReceiver extends BroadcastReceiver {
    private static final String LOG_TAG = "ImsInit";

    @Override
    public void onReceive(final Context context, Intent intent) {
        Log.i(LOG_TAG, "onBoot");

        context.startService(new Intent(context, PhoneStateService.class));
    }
}
