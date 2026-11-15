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
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unistd.h>

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

bool MT6762Tuner::setDramOppFloor(int oppIndex) {
    return writeNode(DRAM_BOOST_DDR, std::to_string(oppIndex));
}

int MT6762Tuner::resolveLowestGpuLevel() {
    std::string content;
    if (!android::base::ReadFileToString(std::string(GED_TOTAL_LEVELS), &content)) {
        return GED_DEFAULT_MAX_LEVEL;
    }
    content = android::base::Trim(content);
    char* end = nullptr;
    long levelCount = std::strtol(content.c_str(), &end, 10);
    if (end == content.c_str() || levelCount <= 0) {
        return GED_DEFAULT_MAX_LEVEL;
    }
    return std::max(0, static_cast<int>(levelCount) - 1);
}

bool MT6762Tuner::setGedGpuBoost(int level) {
    // GED OPP indices are inverted: 0 is highest freq (650 MHz), total_levels-1 is lowest/unboosted
    return writeNode(GED_BOOST_GPU, std::to_string(level));
}

bool MT6762Tuner::setFpsgoState(bool turbo) {
    return writeNode(FPSGO_FORCE_ONOFF, turbo ? FPSGO_FORCE_ON : FPSGO_FREE);
}

bool MT6762Tuner::setTouchBoost(bool enabled) {
    LOG(INFO) << "Setting touch game mode: " << (enabled ? "enabled" : "disabled");
    std::string path(TOUCH_GAME_MODE);
    if (access(path.c_str(), W_OK) != 0) {
        return true;
    }
    return writeNode(TOUCH_GAME_MODE, enabled ? "1" : "0");
}

bool MT6762Tuner::setThermalMitigation(bool enabled) {
    LOG(INFO) << "Setting thermal mitigation: " << (enabled ? "enabled" : "disabled");
    return setThermalPpmPolicy(!enabled);
}

bool MT6762Tuner::applyProfile(int profile) {
    LOG(INFO) << "Applying hardware profile: " << profile;
    const int lowestGpuLevel = resolveLowestGpuLevel();
    switch (profile) {
    case 1: // GAMING_TURBO
        setCpuClusterLimits(CLUSTER_LITTLE, LITTLE_FREQ_MID, LITTLE_FREQ_MAX);
        setCpuClusterLimits(CLUSTER_BIG, BIG_FREQ_MID, BIG_FREQ_MAX);
        setThermalPpmPolicy(false);
        setDramOppFloor(DRAM_OPP_TURBO);
        setGedGpuBoost(GED_TURBO_BOOST_LEVEL);
        setFpsgoState(true);
        setTouchBoost(true);
        return true;

    case 2: // BATTERY_SAVER
        setCpuClusterLimits(CLUSTER_LITTLE, LITTLE_FREQ_MIN, LITTLE_FREQ_SAVER_MAX);
        setCpuClusterLimits(CLUSTER_BIG, BIG_FREQ_MIN, BIG_FREQ_SAVER_MAX);
        setThermalPpmPolicy(true);
        setDramOppFloor(DRAM_OPP_UNBOOSTED);
        setGedGpuBoost(lowestGpuLevel);
        setFpsgoState(false);
        setTouchBoost(false);
        return true;

    case 0: // BALANCED
    default:
        setCpuClusterLimits(CLUSTER_LITTLE, LITTLE_FREQ_MIN, LITTLE_FREQ_MAX);
        setCpuClusterLimits(CLUSTER_BIG, BIG_FREQ_MIN, BIG_FREQ_MAX);
        setThermalPpmPolicy(true);
        setDramOppFloor(DRAM_OPP_UNBOOSTED);
        setGedGpuBoost(lowestGpuLevel);
        setFpsgoState(false);
        setTouchBoost(false);
        return true;
    }
}

} // namespace vendor::cubot::encore
