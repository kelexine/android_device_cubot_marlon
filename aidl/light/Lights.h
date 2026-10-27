/*
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>
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
    int setBacklightLocked(const HwLightState& state);
    int setKeyboardLightLocked(const HwLightState& state);
    int setButtonsLightLocked(const HwLightState& state);
    int setSpeakerLightLocked(const HwLightState& state);

    int blinkRed(int level, int onMS, int offMS);
    int blinkGreen(int level, int onMS, int offMS);
    int blinkBlue(int level, int onMS, int offMS);

    std::mutex mLock;
    std::vector<HwLight> mSupportedLights;

    HwLightState mBattery{};
    HwLightState mNotification{};
    int32_t mAttention = 0;

    int mRedStatus = 0;
    int mGreenStatus = 0;
    int mBlueStatus = 0;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
