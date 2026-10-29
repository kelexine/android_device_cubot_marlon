/*
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Lights.h"

#include <android-base/logging.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

namespace {

constexpr const char* kLcdFile = "/sys/class/leds/lcd-backlight/brightness";
constexpr const char* kMt6370BledFile = "/sys/class/leds/mt6370_pmu_bled/brightness";
constexpr const char* kKeyboardFile = "/sys/class/leds/keyboard-backlight/brightness";
constexpr const char* kButtonFile = "/sys/class/leds/button-backlight/brightness";

constexpr const char* kRedLedFile = "/sys/class/leds/red/brightness";
constexpr const char* kRedTriggerFile = "/sys/class/leds/red/trigger";
constexpr const char* kRedDelayOnFile = "/sys/class/leds/red/delay_on";
constexpr const char* kRedDelayOffFile = "/sys/class/leds/red/delay_off";

constexpr int kMaxSupportedLightId = 6;
constexpr int kMaxKnownLightType = 10;
constexpr int kTriggerWaitRetries = 10;
constexpr int kTriggerWaitDelayMs = 5;
constexpr int kMinBacklight = 10;
constexpr int kFullBrightness = 255;

int writeInt(const char* path, int value) {
    static bool alreadyWarned = false;
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        if (!alreadyWarned) {
            LOG(ERROR) << "write_int failed to open " << path << ": " << strerror(errno);
            alreadyWarned = true;
        }
        return -errno;
    }

    char buffer[20];
    int bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
    ssize_t written = write(fd, buffer, static_cast<size_t>(bytes));
    close(fd);
    return (written == -1) ? -errno : 0;
}

int writeStr(const char* path, const char* str) {
    static bool alreadyWarned = false;
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        if (!alreadyWarned) {
            LOG(ERROR) << "write_str failed to open " << path << ": " << strerror(errno);
            alreadyWarned = true;
        }
        return -errno;
    }

    char buffer[20];
    int bytes = snprintf(buffer, sizeof(buffer), "%s", str);
    ssize_t written = write(fd, buffer, static_cast<size_t>(bytes));
    close(fd);
    return (written == -1) ? -errno : 0;
}

void ledWaitDelay(int ms) {
    usleep(static_cast<useconds_t>(ms) * 1000);
}

int rgbToBrightness(const HwLightState& state) {
    int color = state.color & 0x00ffffff;
    return (((color >> 16) & 0xff) * 77 +
            ((color >> 8) & 0xff) * 150 +
            (color & 0xff) * 29) >> 8;
}

int blinkLedChannel(int level, int onMS, int offMS, int* preStatus) {
    int nowStatus = (level == 0) ? 0 : ((onMS != 0 && offMS != 0) ? 1 : 2);
    if (*preStatus == nowStatus) {
        return 0;
    }

    if (nowStatus == 0) {
        writeInt(kRedLedFile, 0);
    } else if (nowStatus == 1) {
        writeStr(kRedTriggerFile, "timer");
        int retry = 0;
        while ((access(kRedDelayOffFile, F_OK) == -1 ||
                access(kRedDelayOffFile, R_OK | W_OK) != 0) &&
               retry < kTriggerWaitRetries) {
            ledWaitDelay(kTriggerWaitDelayMs);
            retry++;
        }
        writeInt(kRedDelayOffFile, offMS);
        writeInt(kRedDelayOnFile, onMS);
        writeStr(kRedTriggerFile, "pwm_mode");
        writeInt(kRedLedFile, kFullBrightness);
    } else {
        writeStr(kRedTriggerFile, "cc_mode");
        writeInt(kRedLedFile, kFullBrightness);
    }

    *preStatus = nowStatus;
    return 0;
}

}  // namespace

Lights::Lights() {
    for (int id = 0; id < kMaxSupportedLightId; ++id) {
        mSupportedLights.push_back({
            .id = id,
            .ordinal = 0,
            .type = static_cast<LightType>(id),
        });
    }
}

int Lights::setBacklightLocked(const HwLightState& state) {
    int brightness = rgbToBrightness(state);
    if ((state.color & 0x00ffffff) != 0 && brightness < kMinBacklight) {
        brightness = kMinBacklight;
    }
    if (access(kMt6370BledFile, W_OK) == 0) {
        writeInt(kMt6370BledFile, brightness);
    }
    return writeInt(kLcdFile, brightness);
}

int Lights::setKeyboardLightLocked(const HwLightState& state) {
    int on = (state.color & 0x00ffffff) != 0 ? kFullBrightness : 0;
    return writeInt(kKeyboardFile, on);
}

int Lights::setButtonsLightLocked(const HwLightState& state) {
    int on = (state.color & 0x00ffffff) != 0 ? kFullBrightness : 0;
    return writeInt(kButtonFile, on);
}

int Lights::blinkRed(int level, int onMS, int offMS) {
    return blinkLedChannel(level, onMS, offMS, &mRedStatus);
}

int Lights::blinkGreen(int level, int onMS, int offMS) {
    return blinkLedChannel(level, onMS, offMS, &mGreenStatus);
}

int Lights::blinkBlue(int level, int onMS, int offMS) {
    return blinkLedChannel(level, onMS, offMS, &mBlueStatus);
}

int Lights::setSpeakerLightLocked(const HwLightState& state) {
    uint32_t alpha = (static_cast<uint32_t>(state.color) >> 24) & 0xff;
    int red = 0;
    int green = 0;
    int blue = 0;

    if (alpha != 0) {
        red = (state.color >> 16) & 0xff;
        green = (state.color >> 8) & 0xff;
        blue = state.color & 0xff;
    }

    int onMS = 0;
    int offMS = 0;
    if (state.flashMode == FlashMode::TIMED) {
        onMS = state.flashOnMs;
        offMS = state.flashOffMs;
    }

    // Deactivate zeroed channels first so shared /sys/class/leds/red node is not clobbered
    // when transitioning between R, G, and B colors.
    if (red == 0) {
        blinkRed(0, onMS, offMS);
    }
    if (green == 0) {
        blinkGreen(0, onMS, offMS);
    }
    if (blue == 0) {
        blinkBlue(0, onMS, offMS);
    }

    if (red != 0) {
        blinkRed(red, onMS, offMS);
    }
    if (green != 0) {
        blinkGreen(green, onMS, offMS);
    }
    if (blue != 0) {
        blinkBlue(blue, onMS, offMS);
    }
    return 0;
}

ndk::ScopedAStatus Lights::setLightState(int id, const HwLightState& state) {
    if (id < 0 || id >= kMaxKnownLightType) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    std::lock_guard<std::mutex> lock(mLock);

    switch (id) {
        case static_cast<int>(LightType::BACKLIGHT): {
            int err = setBacklightLocked(state);
            if (err != 0) {
                return ndk::ScopedAStatus::fromServiceSpecificError(-1);
            }
            return ndk::ScopedAStatus::ok();
        }
        case static_cast<int>(LightType::KEYBOARD):
            setKeyboardLightLocked(state);
            return ndk::ScopedAStatus::ok();
        case static_cast<int>(LightType::BUTTONS):
            setButtonsLightLocked(state);
            return ndk::ScopedAStatus::ok();
        case static_cast<int>(LightType::BATTERY):
            mBattery = state;
            setSpeakerLightLocked(mBattery);
            if ((state.color & 0x00ffffff) == 0) {
                setSpeakerLightLocked(mNotification);
                setSpeakerLightLocked(mBattery);
            }
            return ndk::ScopedAStatus::ok();
        case static_cast<int>(LightType::NOTIFICATIONS):
            mNotification = state;
            setSpeakerLightLocked(mNotification);
            if ((state.color & 0x00ffffff) == 0) {
                setSpeakerLightLocked(mNotification);
                setSpeakerLightLocked(mBattery);
            }
            return ndk::ScopedAStatus::ok();
        case static_cast<int>(LightType::ATTENTION):
            if (state.flashMode == FlashMode::HARDWARE) {
                mAttention = state.flashOnMs;
            } else if (state.flashMode == FlashMode::NONE) {
                mAttention = 0;
            }
            return ndk::ScopedAStatus::ok();
        default:
            return ndk::ScopedAStatus::fromServiceSpecificError(-1);
    }
}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight>* lights) {
    if (lights == nullptr) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    *lights = mSupportedLights;
    return ndk::ScopedAStatus::ok();
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
