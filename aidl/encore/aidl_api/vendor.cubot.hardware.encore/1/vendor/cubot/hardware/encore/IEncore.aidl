/*
 * File: IEncore.aidl
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: AIDL interface for Encore hardware performance daemon on Cubot P50.
 */

package vendor.cubot.hardware.encore;

import vendor.cubot.hardware.encore.EncoreStats;

@VintfStability
interface IEncore {
    const int PROFILE_BALANCED = 0;
    const int PROFILE_GAMING_TURBO = 1;
    const int PROFILE_BATTERY_SAVER = 2;

    void setProfile(in int profile);
    int getProfile();

    void setGameMode(in boolean enabled, in @utf8InCpp String packageName);
    boolean isGameMode();

    void setTouchBoost(in boolean enabled);
    boolean isTouchBoost();

    void setThermalMitigation(in boolean enabled);
    boolean isThermalMitigation();

    EncoreStats getStats();
}
