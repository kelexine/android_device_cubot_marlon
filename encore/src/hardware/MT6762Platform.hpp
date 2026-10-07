/*
 * File: MT6762Platform.hpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Platform paths and hardware constants for MediaTek MT6762 / MT6765 (Cubot P50).
 */

#pragma once

#include <string_view>

namespace vendor::cubot::encore::platform {

// Cluster Indices in MTK PPM
inline constexpr int CLUSTER_LITTLE = 0;
inline constexpr int CLUSTER_BIG = 1;

// Helio P22 / P35 Frequency Limits (kHz)
inline constexpr int LITTLE_FREQ_MIN = 850000;
inline constexpr int LITTLE_FREQ_MID = 1500000;
inline constexpr int LITTLE_FREQ_MAX = 1800000;

inline constexpr int BIG_FREQ_MIN = 850000;
inline constexpr int BIG_FREQ_MID = 1800000;
inline constexpr int BIG_FREQ_MAX = 2300000;

// MTK PPM Sysfs Nodes
inline constexpr std::string_view PPM_USERLIMIT_MAX = "/proc/ppm/policy/hard_userlimit_max_cpu_freq";
inline constexpr std::string_view PPM_USERLIMIT_MIN = "/proc/ppm/policy/hard_userlimit_min_cpu_freq";
inline constexpr std::string_view PPM_POLICY_STATUS = "/proc/ppm/policy_status";

// MTK FPSGO Nodes
inline constexpr std::string_view FPSGO_FORCE_ONOFF = "/sys/kernel/fpsgo/common/force_onoff";
inline constexpr std::string_view FPSGO_FSTB_LEVEL = "/sys/kernel/fpsgo/fstb/fstb_level";

// MTK DVFSRC (Memory Bus / Interconnect)
inline constexpr std::string_view DVFSRC_MIN_FREQ = "/sys/class/devfreq/mtk-dvfsrc-devfreq/min_freq";
inline constexpr std::string_view DVFSRC_MAX_FREQ = "/sys/class/devfreq/mtk-dvfsrc-devfreq/max_freq";
inline constexpr std::string_view DVFSRC_CUR_FREQ = "/sys/class/devfreq/mtk-dvfsrc-devfreq/cur_freq";

// MTK GED & PowerVR GE8320 Nodes
inline constexpr std::string_view GED_BOOST_GPU = "/sys/kernel/ged/hal/custom_boost_gpu_freq";
inline constexpr std::string_view GED_UPBOUND_GPU = "/sys/kernel/ged/hal/custom_upbound_gpu_freq";

// Battery Overcurrent Throttling Override
inline constexpr std::string_view BATOC_THROTTLING_STOP = "/proc/mtk_batoc_throttling/battery_oc_protect_stop";

// Touchscreen Game Mode Sampling Rate
inline constexpr std::string_view TOUCH_GAME_MODE = "/proc/touchpanel/game_mode";

// Telemetry Nodes
inline constexpr std::string_view CPU_LITTLE_CUR_FREQ = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
inline constexpr std::string_view CPU_BIG_CUR_FREQ = "/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq";
inline constexpr std::string_view THERMAL_ZONE_TEMP = "/sys/class/thermal/thermal_zone0/temp";

} // namespace vendor::cubot::encore::platform
