/*
 * File: MT6762Platform.hpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Platform paths and hardware constants for MediaTek MT6762 / MT6765 (Cubot P50).
 */

#pragma once

#include <string_view>

namespace vendor::cubot::encore::platform {

// Cluster Indices in MTK PPM (policy0 / cpu0-3 = MT_CPU_DVFS_L = Big, policy4 / cpu4-7 = MT_CPU_DVFS_LL = Little)
inline constexpr int CLUSTER_BIG = 0;
inline constexpr int CLUSTER_LITTLE = 1;

// Helio P22 / P35 Frequency Limits (kHz) matching kernel OPP tables
inline constexpr int LITTLE_FREQ_MIN = 400000;
inline constexpr int LITTLE_FREQ_SAVER_MAX = 1208000;
inline constexpr int LITTLE_FREQ_MID = 1314000;
inline constexpr int LITTLE_FREQ_MAX = 1500000;

inline constexpr int BIG_FREQ_MIN = 900000;
inline constexpr int BIG_FREQ_SAVER_MAX = 1484000;
inline constexpr int BIG_FREQ_MID = 1750000;
inline constexpr int BIG_FREQ_MAX = 2001000;

// MTK PPM Sysfs Nodes (system:system writable)
inline constexpr std::string_view PPM_USERLIMIT_MAX = "/proc/ppm/policy/userlimit_max_cpu_freq";
inline constexpr std::string_view PPM_USERLIMIT_MIN = "/proc/ppm/policy/userlimit_min_cpu_freq";
inline constexpr std::string_view PPM_POLICY_STATUS = "/proc/ppm/policy_status";

// MTK FPSGO Nodes (0 = force off, 1 = force on, 2 = free)
inline constexpr std::string_view FPSGO_FORCE_ONOFF = "/sys/kernel/fpsgo/common/force_onoff";
inline constexpr std::string_view FPSGO_FORCE_ON = "1";
inline constexpr std::string_view FPSGO_FREE = "2";

// MTK Perfmgr DRAM Boost Controller (-1 = unboosted, 0 = highest OPP)
inline constexpr std::string_view DRAM_BOOST_DDR = "/proc/perfmgr/boost_ctrl/dram_ctrl/ddr";
inline constexpr int DRAM_OPP_UNBOOSTED = -1;
inline constexpr int DRAM_OPP_TURBO = 0;

// MTK GED & PowerVR GE8320 Nodes (OPP indices: 0 = highest freq, level_count - 1 = lowest/unboosted)
inline constexpr std::string_view GED_BOOST_GPU = "/sys/kernel/ged/hal/custom_boost_gpu_freq";
inline constexpr std::string_view GED_TOTAL_LEVELS = "/sys/kernel/ged/hal/total_gpu_freq_level_count";
inline constexpr std::string_view GED_CURRENT_FREQ = "/sys/kernel/ged/hal/current_freqency";
inline constexpr int GED_TURBO_BOOST_LEVEL = 0;
inline constexpr int GED_DEFAULT_MAX_LEVEL = 2;
inline constexpr int GPU_DEFAULT_FREQ_MHZ = 400;

// Optional Touchscreen Game Mode Node (absent on ILITEK_V2 kernels)
inline constexpr std::string_view TOUCH_GAME_MODE = "/proc/touchpanel/game_mode";

// Telemetry Nodes (cpu0-3 = Big cluster policy0, cpu4-7 = Little cluster policy4)
inline constexpr std::string_view CPU_BIG_CUR_FREQ = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
inline constexpr std::string_view CPU_LITTLE_CUR_FREQ = "/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq";
inline constexpr std::string_view THERMAL_ZONE_TEMP = "/sys/class/thermal/thermal_zone0/temp";

} // namespace vendor::cubot::encore::platform
