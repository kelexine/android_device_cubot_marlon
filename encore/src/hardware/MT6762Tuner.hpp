/*
 * File: MT6762Tuner.hpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Low-level hardware actuator managing MT6762 CPU/GPU/memory/thermal parameters.
 */

#pragma once

#include <string>
#include <string_view>

namespace vendor::cubot::encore {

class MT6762Tuner {
public:
    MT6762Tuner() = default;
    ~MT6762Tuner() = default;

    bool applyProfile(int profile);
    bool setTouchBoost(bool enabled);
    bool setThermalMitigation(bool enabled);

private:
    bool writeNode(std::string_view path, std::string_view value);
    bool setCpuClusterLimits(int cluster, int minFreqKhz, int maxFreqKhz);
    bool setThermalPpmPolicy(bool allowThrottling);
    bool setDvfsrcFloor(int freqKhz);
    bool setGedGpuBoost(int level);
    bool setFpsgoState(bool turbo);
};

} // namespace vendor::cubot::encore
