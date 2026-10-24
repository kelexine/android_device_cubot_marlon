//
// File: shims/bluetooth_audio/bt_audio_impl_mtk.cpp
// Author: kelexine <https://github.com/kelexine>
// Date: 2026-10-24
// Purpose: Mediatek in-process Bluetooth Audio HIDL provider and session ABI shim.
// SPDX-License-Identifier: Apache-2.0
//

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <memory>
#include <android/log.h>
#include <hidl/LegacySupport.h>

#define LOG_TAG "bt_audio_impl_mtk"

namespace android {
namespace bluetooth {
namespace audio {
    class BluetoothAudioSession;
}
}
}

static const char kIface20[] = "android.hardware.bluetooth.audio@2.0::IBluetoothAudioProvidersFactory";
static const char kIface21[] = "vendor.mediatek.hardware.bluetooth.audio@2.1::IBluetoothAudioProvidersFactory";

extern "C" int32_t createIBluetoothAudioProviderFactory() {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Registering in-process Bluetooth Audio HIDL providers");

    int status20 = android::hardware::registerPassthroughServiceImplementation(
        std::string(kIface20), std::string(kIface20), std::string("default"));
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "registerPassthrough 2.0 returned %d", status20);

    int status21 = android::hardware::registerPassthroughServiceImplementation(
        std::string(kIface21), std::string(kIface21), std::string("default"));
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "registerPassthrough 2.1 returned %d", status21);

    return 0;
}

// Forward MTK 2.1 GetSessionInstance to AOSP 2.0 GetSessionInstance
extern "C" {
void _ZN7android9bluetooth5audio29BluetoothAudioSessionInstance18GetSessionInstanceERKNS_8hardware9bluetooth5audio4V2_011SessionTypeE(
    std::shared_ptr<android::bluetooth::audio::BluetoothAudioSession> *ret,
    const uint8_t *session_type);

void _ZN7android9bluetooth5audio29BluetoothAudioSessionInstance18GetSessionInstanceERKN6vendor8mediatek8hardware9bluetooth5audio4V2_111SessionTypeE(
    std::shared_ptr<android::bluetooth::audio::BluetoothAudioSession> *ret,
    const uint8_t *session_type)
{
    _ZN7android9bluetooth5audio29BluetoothAudioSessionInstance18GetSessionInstanceERKNS_8hardware9bluetooth5audio4V2_011SessionTypeE(ret, session_type);
}
}
