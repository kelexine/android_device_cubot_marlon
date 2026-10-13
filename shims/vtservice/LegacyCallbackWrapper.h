/*
 * File: LegacyCallbackWrapper.h
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-13
 * Purpose: Bridge modern AudioTrack::IAudioTrackCallback to legacy function pointer.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <media/AudioTrack.h>

namespace android {

typedef void (*legacy_callback_t)(int event, void* user, void* info);

class LegacyCallbackWrapper : public AudioTrack::IAudioTrackCallback {
    const legacy_callback_t mCallback;
    void* const mData;

public:
    LegacyCallbackWrapper(legacy_callback_t callback, void* user)
        : mCallback(callback), mData(user) {}

    size_t onMoreData(const AudioTrack::Buffer& buffer) override {
        if (!mCallback) return 0;
        AudioTrack::Buffer copy = buffer;
        mCallback(AudioTrack::EVENT_MORE_DATA, mData, static_cast<void*>(&copy));
        return copy.size();
    }

    void onUnderrun() override {
        if (mCallback) mCallback(AudioTrack::EVENT_UNDERRUN, mData, nullptr);
    }

    void onLoopEnd(int32_t loopsRemaining) override {
        if (mCallback) mCallback(AudioTrack::EVENT_LOOP_END, mData, &loopsRemaining);
    }

    void onMarker(uint32_t markerPosition) override {
        if (mCallback) mCallback(AudioTrack::EVENT_MARKER, mData, &markerPosition);
    }

    void onNewPos(uint32_t newPos) override {
        if (mCallback) mCallback(AudioTrack::EVENT_NEW_POS, mData, &newPos);
    }

    void onBufferEnd() override {
        if (mCallback) mCallback(AudioTrack::EVENT_BUFFER_END, mData, nullptr);
    }

    void onNewIAudioTrack() override {
        if (mCallback) mCallback(AudioTrack::EVENT_NEW_IAUDIOTRACK, mData, nullptr);
    }

    void onStreamEnd() override {
        if (mCallback) mCallback(AudioTrack::EVENT_STREAM_END, mData, nullptr);
    }

    size_t onCanWriteMoreData(const AudioTrack::Buffer& buffer) override {
        if (!mCallback) return 0;
        AudioTrack::Buffer copy = buffer;
        mCallback(AudioTrack::EVENT_CAN_WRITE_MORE_DATA, mData, static_cast<void*>(&copy));
        return copy.size();
    }
};

} // namespace android
