/*
 * File: libshim_camera.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: Camera HAL PropertyMap compatibility shim for libmtkcam_stdutils.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>

namespace android {

class String8;

/*
 * PropertyMap symbols for libmtkcam_stdutils.so / camerahalserver
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

PropertyMap::PropertyMap() {}
PropertyMap::~PropertyMap() {}
void PropertyMap::clear() {}
void PropertyMap::addProperty(const String8&, const String8&) {}
bool PropertyMap::hasProperty(const String8&) const { return false; }
bool PropertyMap::tryGetProperty(const String8&, String8&) const { return false; }
bool PropertyMap::tryGetProperty(const String8&, bool&) const { return false; }
bool PropertyMap::tryGetProperty(const String8&, float&) const { return false; }
bool PropertyMap::tryGetProperty(const String8&, int&) const { return false; }

} // namespace android
