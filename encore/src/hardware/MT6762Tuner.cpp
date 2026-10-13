/*
 * File: MT6762Tuner.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Implementation of MT6762 hardware tuning routines for Cubot P50.
 */

#include "MT6762Tuner.hpp"
#include "MT6762Platform.hpp"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>
#include <fstream>
#include <sstream>

namespace vendor::cubot::encore {

using namespace platform;

bool MT6762Tuner::writeNode(std::string_view path, std::string_view value) {
    if (!android::base::WriteStringToFile(std::string(value), std::string(path), true)) {
        PLOG(WARNING) << "Failed to write '" << value << "' to " << path;
        return false;
    }
    return true;
}

bool MT6762Tuner::setCpuClusterLimits(int cluster, int minFreqKhz, int maxFreqKhz) {
    std::string minCmd = std::to_string(cluster) + " " + std::to_string(minFreqKhz);
    std::string maxCmd = std::to_string(cluster) + " " + std::to_string(maxFreqKhz);

    bool okMin = writeNode(PPM_USERLIMIT_MIN, minCmd);
    bool okMax = writeNode(PPM_USERLIMIT_MAX, maxCmd);
    return okMin && okMax;
}

bool MT6762Tuner::setThermalPpmPolicy(bool allowThrottling) {
    std::string policyPath(PPM_POLICY_STATUS);
    std::ifstream policyFile(policyPath);
    if (!policyFile.is_open()) {
        return false;
    }

    std::string line;
    const std::string stateStr = allowThrottling ? "1" : "0";
    while (std::getline(policyFile, line)) {
        if (line.find("PWR_THRO") != std::string::npos || line.find("THERMAL") != std::string::npos) {
            if (line.length() >= 2 && line.front() == '[') {
                std::string idx(1, line[1]);
                writeNode(PPM_POLICY_STATUS, idx + " " + stateStr);
            }
        }
    }
    return true;
}

bool MT6762Tuner::setDvfsrcFloor(int freqKhz) {
    return writeNode(DVFSRC_MIN_FREQ, std::to_string(freqKhz));
}

bool MT6762Tuner::setGedGpuBoost(int level) {
    return writeNode(GED_BOOST_GPU, std::to_string(level));
}

bool MT6762Tuner::setFpsgoState(bool turbo) {
    bool ok1 = writeNode(FPSGO_FORCE_ONOFF, turbo ? "1" : "0");
    bool ok2 = writeNode(FPSGO_FSTB_LEVEL, turbo ? "2" : "0");
    return ok1 && ok2;
}

bool MT6762Tuner::setTouchBoost(bool enabled) {
    LOG(INFO) << "Setting touch game mode: " << (enabled ? "enabled" : "disabled");
    return writeNode(TOUCH_GAME_MODE, enabled ? "1" : "0");
}

bool MT6762Tuner::setThermalMitigation(bool enabled) {
    LOG(INFO) << "Setting thermal mitigation: " << (enabled ? "enabled" : "disabled");
    writeNode(BATOC_THROTTLING_STOP, enabled ? "stop 1" : "stop 0");
    return setThermalPpmPolicy(!enabled);
}

bool MT6762Tuner::applyProfile(int profile) {
    LOG(INFO) << "Applying hardware profile: " << profile;
    switch (profile) {
    case 1: // GAMING_TURBO
        setCpuClusterLimits(CLUSTER_LITTLE, LITTLE_FREQ_MID, LITTLE_FREQ_MAX);
        setCpuClusterLimits(CLUSTER_BIG, BIG_FREQ_MID, BIG_FREQ_MAX);
        setThermalPpmPolicy(false);
        setDvfsrcFloor(1800000);
        setGedGpuBoost(100);
        setFpsgoState(true);
        setTouchBoost(true);
        return true;

    case 2: // BATTERY_SAVER
        setCpuClusterLimits(CLUSTER_LITTLE, LITTLE_FREQ_MIN, 1200000);
        setCpuClusterLimits(CLUSTER_BIG, BIG_FREQ_MIN, 1500000);
        setThermalPpmPolicy(true);
        setDvfsrcFloor(800000);
        setGedGpuBoost(0);
        setFpsgoState(false);
        setTouchBoost(false);
        return true;

    case 0: // BALANCED
    default:
        setCpuClusterLimits(CLUSTER_LITTLE, LITTLE_FREQ_MIN, LITTLE_FREQ_MAX);
        setCpuClusterLimits(CLUSTER_BIG, BIG_FREQ_MIN, BIG_FREQ_MAX);
        setThermalPpmPolicy(true);
        setDvfsrcFloor(1200000);
        setGedGpuBoost(0);
        setFpsgoState(false);
        setTouchBoost(false);
        return true;
    }
}

} // namespace vendor::cubot::encore
