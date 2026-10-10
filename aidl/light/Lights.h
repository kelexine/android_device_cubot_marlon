/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <fstream>
#include <mutex>
#include <vector>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Lights : public BnLights {
  public:
    Lights();

    ndk::ScopedAStatus setLightState(int id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* lights) override;

  private:
    void setBacklight(uint32_t brightness);
    void setLed(int id, const HwLightState& state);

    std::mutex mLock;
    std::vector<HwLight> mSupportedLights;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
