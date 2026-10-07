/*
 * File: EncoreService.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-07
 * Purpose: Implementation of EncoreService AIDL endpoints.
 */

#include "EncoreService.hpp"
#include <android-base/logging.h>

namespace vendor::cubot::encore {

using aidl::vendor::cubot::hardware::encore::EncoreStats;
using aidl::vendor::cubot::hardware::encore::IEncore;

EncoreService::EncoreService()
    : mCurrentProfile(IEncore::PROFILE_BALANCED),
      mSavedBaseProfile(IEncore::PROFILE_BALANCED),
      mGameMode(false),
      mTouchBoost(false),
      mThermalMitigation(false),
      mActivePackage("") {
    mTuner.applyProfile(mCurrentProfile);
}

::ndk::ScopedAStatus EncoreService::setProfile(int32_t in_profile) {
    std::lock_guard<std::mutex> lock(mLock);
    if (in_profile < IEncore::PROFILE_BALANCED || in_profile > IEncore::PROFILE_BATTERY_SAVER) {
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }

    mCurrentProfile = in_profile;
    if (!mGameMode) {
        mSavedBaseProfile = in_profile;
    }
    mTuner.applyProfile(in_profile);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus EncoreService::getProfile(int32_t* _aidl_return) {
    std::lock_guard<std::mutex> lock(mLock);
    *_aidl_return = mCurrentProfile;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus EncoreService::setGameMode(bool in_enabled, const std::string& in_packageName) {
    std::lock_guard<std::mutex> lock(mLock);
    mGameMode = in_enabled;
    mActivePackage = in_enabled ? in_packageName : "";

    if (in_enabled) {
        LOG(INFO) << "Entering Game Mode for: " << in_packageName;
        mTuner.applyProfile(IEncore::PROFILE_GAMING_TURBO);
        mCurrentProfile = IEncore::PROFILE_GAMING_TURBO;
        if (mTouchBoost) {
            mTuner.setTouchBoost(true);
        }
    } else {
        LOG(INFO) << "Exiting Game Mode. Restoring profile: " << mSavedBaseProfile;
        mCurrentProfile = mSavedBaseProfile;
        mTuner.applyProfile(mSavedBaseProfile);
        mTuner.setTouchBoost(false);
    }

    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus EncoreService::isGameMode(bool* _aidl_return) {
    std::lock_guard<std::mutex> lock(mLock);
    *_aidl_return = mGameMode;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus EncoreService::setTouchBoost(bool in_enabled) {
    std::lock_guard<std::mutex> lock(mLock);
    mTouchBoost = in_enabled;
    if (mGameMode || in_enabled) {
        mTuner.setTouchBoost(in_enabled);
    }
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus EncoreService::isTouchBoost(bool* _aidl_return) {
    std::lock_guard<std::mutex> lock(mLock);
    *_aidl_return = mTouchBoost;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus EncoreService::setThermalMitigation(bool in_enabled) {
    std::lock_guard<std::mutex> lock(mLock);
    mThermalMitigation = in_enabled;
    mTuner.setThermalMitigation(in_enabled);
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus EncoreService::isThermalMitigation(bool* _aidl_return) {
    std::lock_guard<std::mutex> lock(mLock);
    *_aidl_return = mThermalMitigation;
    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus EncoreService::getStats(EncoreStats* _aidl_return) {
    std::lock_guard<std::mutex> lock(mLock);
    TelemetrySnapshot snap = mMonitor.readSnapshot();

    _aidl_return->cpuFreqLittleKhz = snap.cpuFreqLittleKhz;
    _aidl_return->cpuFreqBigKhz = snap.cpuFreqBigKhz;
    _aidl_return->gpuFreqMhz = snap.gpuFreqMhz;
    _aidl_return->temperatureMilliCelsius = snap.temperatureMilliCelsius;
    _aidl_return->activeProfile = mCurrentProfile;
    _aidl_return->isGameMode = mGameMode;
    _aidl_return->activePackage = mActivePackage;

    return ::ndk::ScopedAStatus::ok();
}

} // namespace vendor::cubot::encore
