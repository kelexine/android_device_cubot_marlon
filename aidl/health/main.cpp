/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Health.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <health-impl/ChargerUtils.h>
#include <health/utils.h>

using aidl::android::hardware::health::HalHealthLoop;
using aidl::android::hardware::health::MarlonHealth;
using aidl::android::hardware::health::charger::ChargerCallback;
using aidl::android::hardware::health::charger::ChargerModeMain;

static constexpr const char* gInstanceName = "default";
static constexpr const char* gChargerArg = "--charger";

static int marlon_energy_counter(int64_t* counter) {
    if (counter == nullptr) {
        return -1;
    }

    std::string charge_str;
    std::string voltage_str;
    int64_t charge_uah = 0;
    int64_t voltage_uv = 0;

    if (!android::base::ReadFileToString("/sys/class/power_supply/battery/charge_counter",
                                         &charge_str) ||
        !android::base::ParseInt(android::base::Trim(charge_str), &charge_uah) ||
        charge_uah <= 0) {
        std::string cap_str;
        int64_t cap = 0;
        if (!android::base::ReadFileToString("/sys/class/power_supply/battery/capacity",
                                             &cap_str) ||
            !android::base::ParseInt(android::base::Trim(cap_str), &cap) || cap < 0) {
            return -1;
        }
        charge_uah = std::max<int64_t>(1, cap) * (MarlonHealth::kDesignCapacityUah / 100);
    }

    if (!android::base::ReadFileToString("/sys/class/power_supply/battery/voltage_now",
                                         &voltage_str) ||
        !android::base::ParseInt(android::base::Trim(voltage_str), &voltage_uv) ||
        voltage_uv <= 0) {
        voltage_uv = 3850000;  // 3.85V nominal fallback in uV
    }

    // uAh * uV / 1000 = nWh
    *counter = (charge_uah * voltage_uv) / 1000;
    return 0;
}

int main(int argc, char** argv) {
    android::base::InitLogging(argv, android::base::LogdLogger(android::base::SYSTEM));

    auto config = std::make_unique<healthd_config>();
    ::android::hardware::health::InitHealthdConfig(config.get());

    // MT6765 Cubot P50 specific power supply sysfs paths
    config->batteryStatusPath = "/sys/class/power_supply/battery/status";
    config->batteryHealthPath = "/sys/class/power_supply/battery/health";
    config->batteryPresentPath = "/sys/class/power_supply/battery/present";
    config->batteryCapacityPath = "/sys/class/power_supply/battery/capacity";
    config->batteryVoltagePath = "/sys/class/power_supply/battery/voltage_now";
    config->batteryTemperaturePath = "/sys/class/power_supply/battery/temp";
    config->batteryTechnologyPath = "/sys/class/power_supply/battery/technology";
    config->batteryCurrentNowPath = "/sys/class/power_supply/battery/current_now";
    config->batteryCurrentAvgPath = "/sys/class/power_supply/battery/current_avg";
    config->batteryChargeCounterPath = "/sys/class/power_supply/battery/charge_counter";
    config->batteryCycleCountPath = "/sys/class/power_supply/battery/cycle_count";
    config->batteryFullChargePath = "/sys/class/power_supply/battery/charge_full";
    config->batteryFullChargeDesignCapacityUahPath =
            "/sys/class/power_supply/battery/charge_full_design";
    config->batteryCapacityLevelPath = "/sys/class/power_supply/battery/capacity_level";
    config->batteryChargeTimeToFullNowPath = "/sys/class/power_supply/battery/time_to_full_now";
    config->energyCounter = marlon_energy_counter;

    auto binder = ndk::SharedRefBase::make<MarlonHealth>(gInstanceName, std::move(config));

    if (argc >= 2 && argv[1] == std::string_view(gChargerArg)) {
        LOG(INFO) << "Starting Cubot P50 Health AIDL service in charger mode...";
        return ChargerModeMain(binder, std::make_shared<ChargerCallback>(binder));
    }

    LOG(INFO) << "Starting Cubot P50 Health AIDL HAL service...";
    auto hal_health_loop = std::make_shared<HalHealthLoop>(binder, binder);
    return hal_health_loop->StartLoop();
}
