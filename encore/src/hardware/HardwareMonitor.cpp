/*
 * File: HardwareMonitor.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Implementation of HardwareMonitor reading MT6762 sysfs nodes.
 */

#include "HardwareMonitor.hpp"
#include "MT6762Platform.hpp"

#include <android-base/file.h>
#include <android-base/strings.h>
#include <cstdlib>
#include <sstream>

namespace vendor::cubot::encore {

using namespace platform;

int HardwareMonitor::readIntFromFile(std::string_view path, int fallback) {
    std::string content;
    if (!android::base::ReadFileToString(std::string(path), &content)) {
        return fallback;
    }
    content = android::base::Trim(content);
    char* end = nullptr;
    long val = std::strtol(content.c_str(), &end, 10);
    return (end != content.c_str()) ? static_cast<int>(val) : fallback;
}

int HardwareMonitor::readGpuFreqMhz() {
    std::string content;
    if (!android::base::ReadFileToString(std::string(GED_CURRENT_FREQ), &content)) {
        return GPU_DEFAULT_FREQ_MHZ;
    }
    // current_freqency formats as "<opp_idx> <freq_khz>" (e.g., "0 650000")
    std::istringstream iss(content);
    int oppIdx = 0;
    int freqKhz = 0;
    if ((iss >> oppIdx >> freqKhz) && freqKhz > 0) {
        return freqKhz / 1000;
    }
    return GPU_DEFAULT_FREQ_MHZ;
}

TelemetrySnapshot HardwareMonitor::readSnapshot() {
    TelemetrySnapshot snapshot;
    snapshot.cpuFreqLittleKhz = readIntFromFile(CPU_LITTLE_CUR_FREQ, LITTLE_FREQ_MIN);
    snapshot.cpuFreqBigKhz = readIntFromFile(CPU_BIG_CUR_FREQ, BIG_FREQ_MIN);
    snapshot.temperatureMilliCelsius = readIntFromFile(THERMAL_ZONE_TEMP, 35000);
    snapshot.gpuFreqMhz = readGpuFreqMhz();

    return snapshot;
}

} // namespace vendor::cubot::encore
