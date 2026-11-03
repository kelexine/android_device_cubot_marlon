/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <health-impl/Health.h>

namespace aidl {
namespace android {
namespace hardware {
namespace health {

class MarlonHealth : public Health {
  public:
    // Cubot P50 battery design capacity: 4200 mAh = 4,200,000 uAh
    static constexpr int32_t kDesignCapacityUah = 4200000;

    MarlonHealth(const std::string& instance_name, std::unique_ptr<struct healthd_config>&& config);

    ndk::ScopedAStatus getChargeCounterUah(int32_t* out) override;
    ndk::ScopedAStatus getChargeStatus(BatteryStatus* out) override;
    ndk::ScopedAStatus getBatteryHealthData(BatteryHealthData* out) override;
    ndk::ScopedAStatus getStorageInfo(std::vector<StorageInfo>* out) override;
    ndk::ScopedAStatus getDiskStats(std::vector<DiskStats>* out) override;

  protected:
    void UpdateHealthInfo(HealthInfo* health_info) override;
};

}  // namespace health
}  // namespace hardware
}  // namespace android
}  // namespace aidl
