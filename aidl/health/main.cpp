/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Health.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <health/utils.h>

using aidl::android::hardware::health::HalHealthLoop;
using aidl::android::hardware::health::MarlonHealth;

static constexpr const char* gInstanceName = "default";
static constexpr const char* gChargerArg = "--charger";

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
    config->batteryCycleCountPath = "/sys/class/power_supply/battery/cycle_count";
    config->batteryFullChargePath = "/sys/class/power_supply/battery/charge_full";
    config->batteryDesignCapacityPath = "/sys/class/power_supply/battery/charge_full_design";

    auto binder = ndk::SharedRefBase::make<MarlonHealth>(gInstanceName, std::move(config));

    if (argc >= 2 && argv[1] == std::string_view(gChargerArg)) {
        LOG(INFO) << "Starting Cubot P50 Health AIDL service in charger mode...";
        return ChargerModeMain(binder, std::make_shared<ChargerCallback>(binder));
    }

    LOG(INFO) << "Starting Cubot P50 Health AIDL HAL service...";
    auto hal_health_loop = std::make_shared<HalHealthLoop>(binder, binder);
    return hal_health_loop->StartLoop();
}
