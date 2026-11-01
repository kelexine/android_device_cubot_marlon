/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CameraProviderExtension.h"

#include <fstream>
#include <unistd.h>
#include <vector>

static const std::vector<std::string> kTorchBrightnessPaths = {
    "/sys/devices/platform/flashlights-mt6370/torch_brightness",
    "/sys/class/flashlight/flashlights-mt6370/torch_brightness",
    "/sys/devices/platform/flashlights_mt6370/torch_brightness",
    "/sys/devices/platform/flashlights_mt6360/torchbrightness",
};

static std::string getTorchPath() {
    for (const auto& path : kTorchBrightnessPaths) {
        if (access(path.c_str(), W_OK) == 0) {
            return path;
        }
    }
    return "";
}

template <typename T>
static void writeValue(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

template <typename T>
static T readValue(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;
    file >> result;
    return file.fail() ? def : result;
}

bool supportsTorchStrengthControlExt() {
    return !getTorchPath().empty();
}

int32_t getTorchDefaultStrengthLevelExt() {
    return 3;
}

int32_t getTorchMaxStrengthLevelExt() {
    return 7;
}

int32_t getTorchStrengthLevelExt() {
    std::string path = getTorchPath();
    if (path.empty()) return 0;
    return readValue(path, 0);
}

void setTorchStrengthLevelExt(int32_t torchStrength, bool enabled) {
    std::string path = getTorchPath();
    if (path.empty()) return;
    writeValue(path, (enabled && torchStrength > 0) ? torchStrength : 0);
}
