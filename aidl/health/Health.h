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
    MarlonHealth(const std::string& instance_name, std::unique_ptr<struct healthd_config>&& config);

    ndk::ScopedAStatus getHealthInfo(HealthInfo* health_info) override;
};

}  // namespace health
}  // namespace hardware
}  // namespace android
}  // namespace aidl
