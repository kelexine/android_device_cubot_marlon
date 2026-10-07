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

TelemetrySnapshot HardwareMonitor::readSnapshot() {
    TelemetrySnapshot snapshot;
    snapshot.cpuFreqLittleKhz = readIntFromFile(CPU_LITTLE_CUR_FREQ, 850000);
    snapshot.cpuFreqBigKhz = readIntFromFile(CPU_BIG_CUR_FREQ, 850000);
    snapshot.temperatureMilliCelsius = readIntFromFile(THERMAL_ZONE_TEMP, 35000);

    // Read DVFSRC current interconnect clock as proxy for GPU/memory workload
    int dvfsrcKHz = readIntFromFile(DVFSRC_CUR_FREQ, 650000);
    snapshot.gpuFreqMhz = dvfsrcKHz / 1000;

    return snapshot;
}

} // namespace vendor::cubot::encore
