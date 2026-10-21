/*
 * File: libshim_camera.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: Camera HAL PropertyMap compatibility shim for libmtkcam_stdutils.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>

#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

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
