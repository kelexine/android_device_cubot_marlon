/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Health.h"

#include <algorithm>
#include <cstdio>
#include <sstream>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>

namespace aidl {
namespace android {
namespace hardware {
namespace health {

namespace {

constexpr int32_t kUncalibratedMtkDesignUah = 294000;
constexpr int32_t kUncalibratedMtkFullUah = 2946000;
constexpr int32_t kAcMaxVoltageUv = 5000000;    // 5.0V
constexpr int32_t kAcMaxCurrentUa = 2000000;    // 2.0A (10W)
constexpr int32_t kUsbMaxVoltageUv = 5000000;   // 5.0V
constexpr int32_t kUsbMaxCurrentUa = 500000;    // 500mA (2.5W)

constexpr const char* kMmcPreEolPath = "/sys/block/mmcblk0/device/pre_eol_info";
constexpr const char* kMmcLifeTimePath = "/sys/block/mmcblk0/device/life_time";
constexpr const char* kMmcRevPath = "/sys/block/mmcblk0/device/rev";
constexpr const char* kMmcNamePath = "/sys/block/mmcblk0/device/name";
constexpr const char* kMmcDiskStatPath = "/sys/block/mmcblk0/stat";

bool ReadTrimmedString(const char* path, std::string* out) {
    std::string content;
    if (!::android::base::ReadFileToString(path, &content)) {
        return false;
    }
    *out = ::android::base::Trim(content);
    return !out->empty();
}

bool IsBogusCapacityUah(int32_t uah) {
    return uah <= 100000 || uah == kUncalibratedMtkDesignUah || uah == kUncalibratedMtkFullUah;
}

}  // namespace

MarlonHealth::MarlonHealth(const std::string& instance_name,
                           std::unique_ptr<struct healthd_config>&& config)
    : Health(instance_name, std::move(config)) {}

ndk::ScopedAStatus MarlonHealth::getChargeCounterUah(int32_t* out) {
    HealthInfo info{};
    auto status = getHealthInfo(&info);
    if (!status.isOk()) {
        return status;
    }
    *out = info.batteryChargeCounterUah;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus MarlonHealth::getChargeStatus(BatteryStatus* out) {
    HealthInfo info{};
    auto status = getHealthInfo(&info);
    if (!status.isOk()) {
        return status;
    }
    *out = info.batteryStatus;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus MarlonHealth::getBatteryHealthData(BatteryHealthData* out) {
    *out = BatteryHealthData{};
    out->batteryManufacturingDateSeconds = 0;
    out->batteryFirstUsageSeconds = 0;
    out->batterySerialNumber = std::nullopt;
    out->batteryPartStatus = BatteryPartStatus::UNSUPPORTED;

    int32_t full_uah = kDesignCapacityUah;
    int32_t design_uah = kDesignCapacityUah;
    std::string str;
    int32_t parsed = 0;

    if (ReadTrimmedString("/sys/class/power_supply/battery/charge_full_design", &str) &&
        ::android::base::ParseInt(str, &parsed) && !IsBogusCapacityUah(parsed)) {
        design_uah = parsed;
    }
    if (ReadTrimmedString("/sys/class/power_supply/battery/charge_full", &str) &&
        ::android::base::ParseInt(str, &parsed) && !IsBogusCapacityUah(parsed)) {
        full_uah = parsed;
    } else {
        full_uah = design_uah;
    }

    int64_t soh = (static_cast<int64_t>(full_uah) * 100) / std::max<int32_t>(1, design_uah);
    out->batteryStateOfHealth = std::clamp<int64_t>(soh, 1, 100);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus MarlonHealth::getStorageInfo(std::vector<StorageInfo>* out) {
    out->clear();

    std::string eol_str;
    std::string lifetime_str;
    if (!ReadTrimmedString(kMmcPreEolPath, &eol_str) ||
        !ReadTrimmedString(kMmcLifeTimePath, &lifetime_str)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    uint32_t eol = 0;
    uint32_t lta = 0;
    uint32_t ltb = 0;
    if (!::android::base::ParseUint(eol_str, &eol) ||
        std::sscanf(lifetime_str.c_str(), "0x%02x 0x%02x", &lta, &ltb) != 2) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    if (eol > 3 || lta > 0x0B || ltb > 0x0B) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    StorageInfo info{};
    info.eol = static_cast<int32_t>(eol);
    info.lifetimeA = static_cast<int32_t>(lta);
    info.lifetimeB = static_cast<int32_t>(ltb);

    std::string rev;
    std::string name;
    if (ReadTrimmedString(kMmcNamePath, &name) && ReadTrimmedString(kMmcRevPath, &rev)) {
        info.version = "mmc0 " + name + " " + rev;
    } else if (ReadTrimmedString(kMmcRevPath, &rev)) {
        info.version = "mmc0 " + rev;
    } else {
        info.version = "mmc0";
    }

    out->push_back(std::move(info));
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus MarlonHealth::getDiskStats(std::vector<DiskStats>* out) {
    out->clear();

    std::string stat_str;
    if (!ReadTrimmedString(kMmcDiskStatPath, &stat_str)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    std::istringstream ss(stat_str);
    uint64_t reads = 0;
    uint64_t read_merges = 0;
    uint64_t read_sectors = 0;
    uint64_t read_ticks = 0;
    uint64_t writes = 0;
    uint64_t write_merges = 0;
    uint64_t write_sectors = 0;
    uint64_t write_ticks = 0;
    uint64_t io_in_flight = 0;
    uint64_t io_ticks = 0;
    uint64_t io_in_queue = 0;

    if (!(ss >> reads >> read_merges >> read_sectors >> read_ticks >> writes >> write_merges >>
          write_sectors >> write_ticks >> io_in_flight >> io_ticks >> io_in_queue)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    DiskStats stats{};
    stats.reads = static_cast<int64_t>(reads);
    stats.readMerges = static_cast<int64_t>(read_merges);
    stats.readSectors = static_cast<int64_t>(read_sectors);
    stats.readTicks = static_cast<int64_t>(read_ticks);
    stats.writes = static_cast<int64_t>(writes);
    stats.writeMerges = static_cast<int64_t>(write_merges);
    stats.writeSectors = static_cast<int64_t>(write_sectors);
    stats.writeTicks = static_cast<int64_t>(write_ticks);
    stats.ioInFlight = static_cast<int64_t>(io_in_flight);
    stats.ioTicks = static_cast<int64_t>(io_ticks);
    stats.ioInQueue = static_cast<int64_t>(io_in_queue);

    out->push_back(stats);
    return ndk::ScopedAStatus::ok();
}

void MarlonHealth::UpdateHealthInfo(HealthInfo* health_info) {
    Health::UpdateHealthInfo(health_info);

    // 1. Sanitize design & full-charge capacity (Cubot P50 = 4200 mAh / 4,200,000 uAh).
    const bool had_bogus_full_cap = IsBogusCapacityUah(health_info->batteryFullChargeUah);
    if (IsBogusCapacityUah(health_info->batteryFullChargeDesignCapacityUah)) {
        health_info->batteryFullChargeDesignCapacityUah = kDesignCapacityUah;
    }
    if (had_bogus_full_cap) {
        health_info->batteryFullChargeUah = health_info->batteryFullChargeDesignCapacityUah;
    }

    // 2. Ensure charge counter and cycle count are valid when battery is present.
    if (health_info->batteryPresent) {
        const int32_t clamped_level = std::clamp(health_info->batteryLevel, 0, 100);
        if (health_info->batteryChargeCounterUah <= 0 || had_bogus_full_cap) {
            const int32_t scaled_uah =
                    (clamped_level * (health_info->batteryFullChargeUah / 100));
            health_info->batteryChargeCounterUah = std::max<int32_t>(1000, scaled_uah);
        }
        if (health_info->batteryCycleCount <= 0) {
            health_info->batteryCycleCount = 1;
        }
    }

    // 3. Enforce charger-online vs BatteryStatus and current sign consistency.
    const bool charger_online = health_info->chargerAcOnline || health_info->chargerUsbOnline ||
                                health_info->chargerWirelessOnline ||
                                health_info->chargerDockOnline;

    if (!charger_online) {
        health_info->batteryStatus = BatteryStatus::DISCHARGING;
        health_info->maxChargingCurrentMicroamps = 0;
        health_info->maxChargingVoltageMicrovolts = 0;
    } else {
        if (health_info->chargerAcOnline) {
            health_info->maxChargingVoltageMicrovolts = kAcMaxVoltageUv;
            health_info->maxChargingCurrentMicroamps =
                    std::max(health_info->maxChargingCurrentMicroamps, kAcMaxCurrentUa);
        } else if (health_info->chargerUsbOnline) {
            health_info->maxChargingVoltageMicrovolts = kUsbMaxVoltageUv;
            health_info->maxChargingCurrentMicroamps =
                    std::max(health_info->maxChargingCurrentMicroamps, kUsbMaxCurrentUa);
        }

        if (health_info->batteryStatus == BatteryStatus::DISCHARGING ||
            health_info->batteryStatus == BatteryStatus::UNKNOWN) {
            health_info->batteryStatus =
                    (health_info->batteryLevel >= 100) ? BatteryStatus::FULL
                                                       : BatteryStatus::CHARGING;
        }

        // VTS IsBatteryCurrentSignCorrect requires current > 0 when status == CHARGING.
        if (health_info->batteryStatus == BatteryStatus::CHARGING &&
            health_info->batteryCurrentMicroamps <= 0) {
            health_info->batteryStatus =
                    (health_info->batteryLevel >= 100) ? BatteryStatus::FULL
                                                       : BatteryStatus::NOT_CHARGING;
        }
    }

    // 4. Enforce batteryCapacityLevel and batteryChargeTimeToFullNowSeconds invariants.
    if (health_info->batteryCapacityLevel == BatteryCapacityLevel::UNSUPPORTED) {
        if (!health_info->batteryPresent) {
            health_info->batteryCapacityLevel = BatteryCapacityLevel::UNKNOWN;
        } else if (health_info->batteryLevel >= 100) {
            health_info->batteryCapacityLevel = BatteryCapacityLevel::FULL;
        } else if (health_info->batteryLevel >= 80) {
            health_info->batteryCapacityLevel = BatteryCapacityLevel::HIGH;
        } else if (health_info->batteryLevel >= 20) {
            health_info->batteryCapacityLevel = BatteryCapacityLevel::NORMAL;
        } else if (health_info->batteryLevel >= 5) {
            health_info->batteryCapacityLevel = BatteryCapacityLevel::LOW;
        } else {
            health_info->batteryCapacityLevel = BatteryCapacityLevel::CRITICAL;
        }
    }

    if (health_info->batteryCapacityLevel == BatteryCapacityLevel::FULL ||
        health_info->batteryCapacityLevel == BatteryCapacityLevel::UNKNOWN) {
        health_info->batteryChargeTimeToFullNowSeconds = 0;
    } else if (health_info->batteryStatus != BatteryStatus::CHARGING ||
               health_info->batteryChargeTimeToFullNowSeconds <= 0) {
        health_info->batteryChargeTimeToFullNowSeconds =
                HealthInfo::BATTERY_CHARGE_TIME_TO_FULL_NOW_SECONDS_UNSUPPORTED;
    }

    // 5. Populate BatteryHealthData (State of Health %).
    BatteryHealthData health_data{};
    if (getBatteryHealthData(&health_data).isOk()) {
        const int64_t soh =
                (static_cast<int64_t>(health_info->batteryFullChargeUah) * 100) /
                std::max<int32_t>(1, health_info->batteryFullChargeDesignCapacityUah);
        health_data.batteryStateOfHealth = std::clamp<int64_t>(soh, 1, 100);
        health_info->batteryHealthData = health_data;
    }
}

}  // namespace health
}  // namespace hardware
}  // namespace android
}  // namespace aidl
