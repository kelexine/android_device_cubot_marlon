/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Lights.h"

#include <algorithm>
#include <android-base/logging.h>
#include <fstream>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

#define LCD_BACKLIGHT_PATH "/sys/class/leds/lcd-backlight/brightness"
#define MT6370_BLED_PATH    "/sys/class/leds/mt6370_pmu_bled/brightness"
#define RED_LED_PATH       "/sys/class/leds/red/brightness"
#define RED_TRIGGER_PATH   "/sys/class/leds/red/trigger"
#define RED_DELAY_ON_PATH  "/sys/class/leds/red/delay_on"
#define RED_DELAY_OFF_PATH "/sys/class/leds/red/delay_off"
#define GREEN_LED_PATH     "/sys/class/leds/green/brightness"
#define BLUE_LED_PATH      "/sys/class/leds/blue/brightness"
#define BLUE_TRIGGER_PATH   "/sys/class/leds/blue/trigger"
#define BLUE_DELAY_ON_PATH  "/sys/class/leds/blue/delay_on"
#define BLUE_DELAY_OFF_PATH "/sys/class/leds/blue/delay_off"

static void writeSysfs(const std::string& path, uint32_t value) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return;
    }
    file << value << std::endl;
}

static void writeSysfsStr(const std::string& path, const std::string& value) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return;
    }
    file << value << std::endl;
}

Lights::Lights() {
    mSupportedLights.push_back({.id = static_cast<int32_t>(LightType::BACKLIGHT),
                               .ordinal = 0,
                               .type = LightType::BACKLIGHT});
    mSupportedLights.push_back({.id = static_cast<int32_t>(LightType::NOTIFICATIONS),
                               .ordinal = 0,
                               .type = LightType::NOTIFICATIONS});
    mSupportedLights.push_back({.id = static_cast<int32_t>(LightType::BATTERY),
                               .ordinal = 0,
                               .type = LightType::BATTERY});
}

void Lights::setBacklight(uint32_t color) {
    uint32_t alpha = (color >> 24) & 0xff;
    uint32_t red = (color >> 16) & 0xff;
    uint32_t green = (color >> 8) & 0xff;
    uint32_t blue = color & 0xff;

    uint32_t brightness = (77 * red + 150 * green + 29 * blue) >> 8;
    if (alpha > 0) {
        brightness = (brightness * alpha) / 255;
    }

    if (color != 0 && brightness == 0) {
        brightness = (color & 0xff);
        if (brightness < 10) {
            brightness = 10; // Minimum visible backlight level
        }
    }

    writeSysfs(LCD_BACKLIGHT_PATH, brightness);
    writeSysfs(MT6370_BLED_PATH, brightness);
}

void Lights::setLed(int id, const HwLightState& state) {
    uint32_t red = (state.color >> 16) & 0xff;
    uint32_t green = (state.color >> 8) & 0xff;
    uint32_t blue = state.color & 0xff;

    // Cubot P50 has a single MT6357 PMIC current-sink LED, physically emitting Blue light.
    // In stock DTB, Isink1 was registered under label "red", while some custom trees may label it "blue".
    // Calculate brightness from maximum color intensity so any notification/battery color activates it.
    uint32_t brightness = std::max({red, green, blue});

    if (state.flashMode != FlashMode::NONE && state.flashOnMs > 0 && state.flashOffMs > 0 && brightness > 0) {
        writeSysfsStr(RED_TRIGGER_PATH, "timer");
        writeSysfs(RED_DELAY_ON_PATH, state.flashOnMs);
        writeSysfs(RED_DELAY_OFF_PATH, state.flashOffMs);
        writeSysfs(RED_LED_PATH, brightness);

        writeSysfsStr(BLUE_TRIGGER_PATH, "timer");
        writeSysfs(BLUE_DELAY_ON_PATH, state.flashOnMs);
        writeSysfs(BLUE_DELAY_OFF_PATH, state.flashOffMs);
        writeSysfs(BLUE_LED_PATH, brightness);
    } else {
        writeSysfsStr(RED_TRIGGER_PATH, "none");
        writeSysfs(RED_LED_PATH, brightness);

        writeSysfsStr(BLUE_TRIGGER_PATH, "none");
        writeSysfs(BLUE_LED_PATH, brightness);
    }

    writeSysfs(GREEN_LED_PATH, green);
}

ndk::ScopedAStatus Lights::setLightState(int id, const HwLightState& state) {
    std::lock_guard<std::mutex> lock(mLock);

    LightType type = static_cast<LightType>(id);
    switch (type) {
        case LightType::BACKLIGHT:
            setBacklight(state.color);
            break;
        case LightType::NOTIFICATIONS:
        case LightType::BATTERY:
            setLed(id, state);
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight>* lights) {
    *lights = mSupportedLights;
    return ndk::ScopedAStatus::ok();
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
