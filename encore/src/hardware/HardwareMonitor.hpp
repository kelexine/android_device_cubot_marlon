/*
 * File: HardwareMonitor.hpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Telemetry collector reading real-time CPU, GPU, and thermal statistics on MT6762.
 */

#pragma once

#include <string_view>

namespace vendor::cubot::encore {

struct TelemetrySnapshot {
    int cpuFreqLittleKhz = 0;
    int cpuFreqBigKhz = 0;
    int gpuFreqMhz = 0;
    int temperatureMilliCelsius = 0;
};

class HardwareMonitor {
public:
    HardwareMonitor() = default;
    ~HardwareMonitor() = default;

    TelemetrySnapshot readSnapshot();

private:
    int readIntFromFile(std::string_view path, int fallback = 0);
};

} // namespace vendor::cubot::encore
