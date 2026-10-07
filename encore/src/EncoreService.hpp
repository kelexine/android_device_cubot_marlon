/*
 * File: EncoreService.hpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Native AIDL implementation of IEncore for Cubot P50.
 */

#pragma once

#include <aidl/vendor/cubot/hardware/encore/BnEncore.h>
#include "hardware/HardwareMonitor.hpp"
#include "hardware/MT6762Tuner.hpp"

#include <mutex>
#include <string>

namespace vendor::cubot::encore {

class EncoreService : public aidl::vendor::cubot::hardware::encore::BnEncore {
public:
    EncoreService();
    virtual ~EncoreService() = default;

    ::ndk::ScopedAStatus setProfile(int32_t in_profile) override;
    ::ndk::ScopedAStatus getProfile(int32_t* _aidl_return) override;

    ::ndk::ScopedAStatus setGameMode(bool in_enabled, const std::string& in_packageName) override;
    ::ndk::ScopedAStatus isGameMode(bool* _aidl_return) override;

    ::ndk::ScopedAStatus setTouchBoost(bool in_enabled) override;
    ::ndk::ScopedAStatus isTouchBoost(bool* _aidl_return) override;

    ::ndk::ScopedAStatus setThermalMitigation(bool in_enabled) override;
    ::ndk::ScopedAStatus isThermalMitigation(bool* _aidl_return) override;

    ::ndk::ScopedAStatus getStats(aidl::vendor::cubot::hardware::encore::EncoreStats* _aidl_return) override;

private:
    std::mutex mLock;
    MT6762Tuner mTuner;
    HardwareMonitor mMonitor;

    int32_t mCurrentProfile;
    int32_t mSavedBaseProfile;
    bool mGameMode;
    bool mTouchBoost;
    bool mThermalMitigation;
    std::string mActivePackage;
};

} // namespace vendor::cubot::encore
