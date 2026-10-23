/*
 * File: libshim_bt.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-23
 * Purpose: Bluetooth HCI shim for MediaTek MT6765 / MT6631 combo chip on Android 16.
 *          Rewrites unexpected HCI Command Status events for Opcode 0x0C5A
 *          (HCI_Read_Default_Erroneous_Data_Reporting) to HCI Command Complete events
 *          to prevent Android 16 Gabeldorsche (system/gd/hci/hci_layer.cc) assertion panic.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <android/log.h>

#define LOG_TAG "ShimBT"

#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#if defined(__aarch64__)
struct HidlVecRaw {
    const uint8_t* mBuffer;
    uint32_t mSize;
    uint8_t mOwnsBuffer;
    uint8_t mPad[3];
};
static constexpr size_t PACKET_OFFSET = 8;
#else
struct HidlVecRaw {
    const uint8_t* mBuffer;
    uint32_t mSize;
    uint8_t mOwnsBuffer;
    uint8_t mPad[3];
};
static constexpr size_t PACKET_OFFSET = 4;
#endif

// Bluetooth HCI Constants
static constexpr uint8_t HCI_EVT_CMD_COMPLETE = 0x0E;
static constexpr uint8_t HCI_EVT_CMD_STATUS = 0x0F;
static constexpr uint16_t HCI_CMD_READ_ERRONEOUS_DATA_REPORTING = 0x0C5A;
static constexpr uint8_t HCI_STATUS_SUCCESS = 0x00;
static constexpr uint8_t HCI_ERRONEOUS_DATA_DISABLED = 0x00;

namespace android {
namespace hardware {
namespace bluetooth {
namespace hci {

class HciPacketizer {
public:
    const void* GetPacket() const;
};

/*
 * Intercept HciPacketizer::GetPacket() const
 * Original mangled symbol: _ZNK7android8hardware9bluetooth3hci13HciPacketizer9GetPacketEv
 * Original implementation: return *reinterpret_cast<const hidl_vec<uint8_t>*>((const uint8_t*)this + PACKET_OFFSET);
 */
extern "C" const void* _ZNK7android8hardware9bluetooth3hci13HciPacketizer9GetPacketEv(const void* thiz) {
    if (!thiz) {
        return nullptr;
    }

    const HidlVecRaw* orig_vec = reinterpret_cast<const HidlVecRaw*>((const uint8_t*)thiz + PACKET_OFFSET);
    if (!orig_vec || !orig_vec->mBuffer || orig_vec->mSize < 6) {
        return orig_vec;
    }

    const uint8_t* data = orig_vec->mBuffer;
    if (data[0] == HCI_EVT_CMD_STATUS) {
        uint16_t opcode = static_cast<uint16_t>(data[4]) | (static_cast<uint16_t>(data[5]) << 8);

        if (opcode == HCI_CMD_READ_ERRONEOUS_DATA_REPORTING) {
            static uint8_t s_fixed_complete[7];
            static HidlVecRaw s_rewritten_vec;

            uint8_t credits = data[3];

            // Build Command Complete Event:
            // [0] Event code: 0x0E (HCI_COMMAND_COMPLETE_EVENT)
            // [1] Param len:  0x05 (credits + opcode(2) + status(1) + erroneous_data_reporting(1))
            // [2] Num_Cmd:    credits
            // [3] Opcode LSB: 0x5A
            // [4] Opcode MSB: 0x0C
            // [5] Status:     0x00 (HCI_SUCCESS)
            // [6] Value:      0x00 (Erroneous Data Reporting: Disabled)
            s_fixed_complete[0] = HCI_EVT_CMD_COMPLETE;
            s_fixed_complete[1] = 0x05;
            s_fixed_complete[2] = credits;
            s_fixed_complete[3] = static_cast<uint8_t>(HCI_CMD_READ_ERRONEOUS_DATA_REPORTING & 0xFF);
            s_fixed_complete[4] = static_cast<uint8_t>((HCI_CMD_READ_ERRONEOUS_DATA_REPORTING >> 8) & 0xFF);
            s_fixed_complete[5] = HCI_STATUS_SUCCESS;
            s_fixed_complete[6] = HCI_ERRONEOUS_DATA_DISABLED;

            s_rewritten_vec.mBuffer = s_fixed_complete;
            s_rewritten_vec.mSize = sizeof(s_fixed_complete);
            s_rewritten_vec.mOwnsBuffer = 0;

            ALOGI("Intercepted Status (0x0F) for READ_DEFAULT_ERRONEOUS_DATA_REPORTING (0x0C5A), rewritten to Complete (0x0E) credits=%u", credits);
            return &s_rewritten_vec;
        }
    }

    return orig_vec;
}

} // namespace hci
} // namespace bluetooth
} // namespace hardware
} // namespace android
