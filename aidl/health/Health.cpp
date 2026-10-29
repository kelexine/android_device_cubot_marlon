/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Health.h"
#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

namespace aidl {
namespace android {
namespace hardware {
namespace health {

MarlonHealth::MarlonHealth(const std::string& instance_name,
                           std::unique_ptr<struct healthd_config>&& config)
    : Health(instance_name, std::move(config)) {}

ndk::ScopedAStatus MarlonHealth::getHealthInfo(HealthInfo* health_info) {
    ndk::ScopedAStatus status = Health::getHealthInfo(health_info);
    if (!status.isOk()) {
        return status;
    }

    // Cubot P50 (MT6765): Ensure battery status is accurately reflected if kernel driver
    // reports discharging or not charging when charger is detached.
    std::string status_str;
    if (::android::base::ReadFileToString("/sys/class/power_supply/battery/status", &status_str)) {
        status_str = ::android::base::Trim(status_str);
        if (status_str == "Discharging") {
            health_info->batteryStatus = BatteryStatus::DISCHARGING;
        } else if (status_str == "Charging") {
            health_info->batteryStatus = BatteryStatus::CHARGING;
        } else if (status_str == "Not charging") {
            health_info->batteryStatus = BatteryStatus::NOT_CHARGING;
        } else if (status_str == "Full") {
            health_info->batteryStatus = BatteryStatus::FULL;
        }
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace health
}  // namespace hardware
}  // namespace android
}  // namespace aidl
