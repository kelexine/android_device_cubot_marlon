/*
 * File: libshim_camera.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: Camera HAL PropertyMap compatibility shim for libmtkcam_stdutils.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

#include <log/log.h>
#include <utils/String8.h>

namespace android {

/*
 * PropertyMap symbols for libmtkcam_stdutils.so / camerahalserver.
 * Backed by an out-of-line thread-safe map keyed by instance pointer so
 * caller stack/heap size assumptions are never violated.
 */
class PropertyMap {
public:
    PropertyMap();
    ~PropertyMap();
    void clear();
    void addProperty(const String8& key, const String8& val);
    bool hasProperty(const String8& key) const;
    bool tryGetProperty(const String8& key, String8& out) const;
    bool tryGetProperty(const String8& key, bool& out) const;
    bool tryGetProperty(const String8& key, float& out) const;
    bool tryGetProperty(const String8& key, int& out) const;
};

namespace {

std::mutex& getRegistryMutex() {
    static std::mutex m;
    return m;
}

std::unordered_map<const PropertyMap*, std::map<std::string, std::string>>& getRegistry() {
    static std::unordered_map<const PropertyMap*, std::map<std::string, std::string>> reg;
    return reg;
}

bool lookupRawProperty(const PropertyMap* self, const String8& key, std::string& outVal) {
    std::lock_guard<std::mutex> lock(getRegistryMutex());
    auto it = getRegistry().find(self);
    if (it == getRegistry().end()) {
        return false;
    }
    auto prop = it->second.find(key.c_str());
    if (prop == it->second.end()) {
        return false;
    }
    outVal = prop->second;
    return true;
}

} // anonymous namespace

PropertyMap::PropertyMap() {
    std::lock_guard<std::mutex> lock(getRegistryMutex());
    getRegistry()[this].clear();
}

PropertyMap::~PropertyMap() {
    std::lock_guard<std::mutex> lock(getRegistryMutex());
    getRegistry().erase(this);
}

void PropertyMap::clear() {
    std::lock_guard<std::mutex> lock(getRegistryMutex());
    auto it = getRegistry().find(this);
    if (it != getRegistry().end()) {
        it->second.clear();
    }
}

void PropertyMap::addProperty(const String8& key, const String8& val) {
    std::lock_guard<std::mutex> lock(getRegistryMutex());
    getRegistry()[this][key.c_str()] = val.c_str();
}

bool PropertyMap::hasProperty(const String8& key) const {
    std::string unused;
    return lookupRawProperty(this, key, unused);
}

bool PropertyMap::tryGetProperty(const String8& key, String8& out) const {
    std::string val;
    if (!lookupRawProperty(this, key, val)) {
        return false;
    }
    out.setTo(val.c_str());
    return true;
}

bool PropertyMap::tryGetProperty(const String8& key, bool& out) const {
    std::string val;
    if (!lookupRawProperty(this, key, val) || val.empty()) {
        return false;
    }
    const char* s = val.c_str();
    if (strcasecmp(s, "1") == 0 || strcasecmp(s, "true") == 0 ||
        strcasecmp(s, "y") == 0 || strcasecmp(s, "yes") == 0 ||
        strcasecmp(s, "on") == 0) {
        out = true;
        return true;
    }
    if (strcasecmp(s, "0") == 0 || strcasecmp(s, "false") == 0 ||
        strcasecmp(s, "n") == 0 || strcasecmp(s, "no") == 0 ||
        strcasecmp(s, "off") == 0) {
        out = false;
        return true;
    }
    return false;
}

bool PropertyMap::tryGetProperty(const String8& key, float& out) const {
    std::string val;
    if (!lookupRawProperty(this, key, val) || val.empty()) {
        return false;
    }
    char* end = nullptr;
    errno = 0;
    float parsed = strtof(val.c_str(), &end);
    if (errno != 0 || end == val.c_str() || *end != '\0') {
        return false;
    }
    out = parsed;
    return true;
}

bool PropertyMap::tryGetProperty(const String8& key, int& out) const {
    std::string val;
    if (!lookupRawProperty(this, key, val) || val.empty()) {
        return false;
    }
    char* end = nullptr;
    errno = 0;
    long parsed = strtol(val.c_str(), &end, 0);
    if (errno != 0 || end == val.c_str() || *end != '\0' ||
        parsed < INT_MIN || parsed > INT_MAX) {
        return false;
    }
    out = static_cast<int>(parsed);
    return true;
}

} // namespace android

extern "C" {

/*
 * libprocessgroup compatibility for camerahalserver / MTK camera HAL.
 * On 4.19 kernel trees, task profile and cgroup controller calls are stubbed to prevent crashes.
 */
void SetTaskProfiles(...) {
    // Stub implementation: do nothing
}

bool CgroupGetControllerPath(...) {
    return false;
}

} // extern "C"

namespace {

constexpr uint32_t kTagRequestAvailableCapabilities = 0xc000c;
constexpr uint8_t kCapabilityRaw = 3;

struct alignas(16) MtkIEntryBuf {
    uint8_t data[64]{};
    // Non-trivial destructor forces AAPCS64 indirect result (x8) return ABI.
    ~MtkIEntryBuf() {}
};

using UpdateRawCapabilityFn = void (*)(void* self, void* metadata);
using EntryForFn = MtkIEntryBuf (*)(const void* metadata, uint32_t tag, int32_t index);
using EntryCountFn = uint32_t (*)(const void* entry);
using EntryItemAtU8Fn = uint8_t (*)(const void* entry, uint32_t index, uint8_t typeTag);
using EntryPushBackU8Fn = void (*)(void* entry, const uint8_t* val, uint8_t typeTag);
using MetadataUpdateFn = int (*)(void* metadata, uint32_t tag, const void* entry);
using EntryDtorFn = void (*)(void* entry);

} // namespace

namespace android {
namespace NSMetadataProvider {

class MetadataProvider {
  public:
    void updateRawCapability(void* metadata);
};

void MetadataProvider::updateRawCapability(void* metadata) {
    static auto realUpdateRawCapability = reinterpret_cast<UpdateRawCapabilityFn>(
        dlsym(RTLD_NEXT,
              "_ZN7android18NSMetadataProvider16MetadataProvider19updateRawCapabilityERN5NSCam9IMetadataE"));
    static auto entryFor = reinterpret_cast<EntryForFn>(
        dlsym(RTLD_DEFAULT, "_ZNK5NSCam9IMetadata8entryForEji"));
    static auto entryCount = reinterpret_cast<EntryCountFn>(
        dlsym(RTLD_DEFAULT, "_ZNK5NSCam9IMetadata6IEntry5countEv"));
    static auto entryItemAt = reinterpret_cast<EntryItemAtU8Fn>(
        dlsym(RTLD_DEFAULT, "_ZNK5NSCam9IMetadata6IEntry6itemAtEjNS_9Type2TypeIhEE"));
    static auto entryPushBack = reinterpret_cast<EntryPushBackU8Fn>(
        dlsym(RTLD_DEFAULT, "_ZN5NSCam9IMetadata6IEntry9push_backERKhNS_9Type2TypeIhEE"));
    static auto metadataUpdate = reinterpret_cast<MetadataUpdateFn>(
        dlsym(RTLD_DEFAULT, "_ZN5NSCam9IMetadata6updateEjRKNS0_6IEntryE"));
    static auto entryDtor = reinterpret_cast<EntryDtorFn>(
        dlsym(RTLD_DEFAULT, "_ZN5NSCam9IMetadata6IEntryD1Ev"));

    if (metadata && entryFor && entryCount && entryItemAt && entryPushBack &&
        metadataUpdate && entryDtor) {
        MtkIEntryBuf entry = entryFor(metadata, kTagRequestAvailableCapabilities, 0);
        uint32_t count = entryCount(&entry);
        bool hasRaw = false;
        for (uint32_t i = 0; i < count; ++i) {
            if (entryItemAt(&entry, i, 0) == kCapabilityRaw) {
                hasRaw = true;
                break;
            }
        }
        if (count > 0 && !hasRaw) {
            uint8_t rawCap = kCapabilityRaw;
            entryPushBack(&entry, &rawCap, 0);
            metadataUpdate(metadata, kTagRequestAvailableCapabilities, &entry);
            ALOGI("libshim_camera: injected RAW16 capability into ANDROID_REQUEST_AVAILABLE_CAPABILITIES");
        }
        entryDtor(&entry);
    }

    if (realUpdateRawCapability) {
        realUpdateRawCapability(this, metadata);
    }
}

} // namespace NSMetadataProvider
} // namespace android
