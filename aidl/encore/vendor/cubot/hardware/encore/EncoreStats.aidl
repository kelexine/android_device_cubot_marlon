/*
 * File: EncoreStats.aidl
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Parcelable carrying live telemetry metrics for Cubot P50 hardware.
 */

package vendor.cubot.hardware.encore;

@VintfStability
parcelable EncoreStats {
    int cpuFreqLittleKhz;
    int cpuFreqBigKhz;
    int gpuFreqMhz;
    int temperatureMilliCelsius;
    int activeProfile;
    boolean isGameMode;
    String activePackage;
}
