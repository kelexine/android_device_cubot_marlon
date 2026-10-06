// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include "pwroff_alarm/property_source.h"

#include <cstdlib>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#include <cstdint>
#endif

namespace pwroff {

#ifdef __ANDROID__

std::string PlatformPropertySource::Get(const std::string& name) const {
    const prop_info* info = __system_property_find(name.c_str());
    if (info == nullptr) {
        return std::string();
    }
    std::string value;
    __system_property_read_callback(
            info,
            [](void* cookie, const char* /*name*/, const char* val, uint32_t /*serial*/) {
                static_cast<std::string*>(cookie)->assign(val);
            },
            &value);
    return value;
}

#else

std::string PlatformPropertySource::Get(const std::string& name) const {
    std::string key = "PWROFF_PROP_";
    for (char c : name) {
        if (c == '.') {
            key += '_';
        } else if (c >= 'a' && c <= 'z') {
            key += static_cast<char>(c - 'a' + 'A');
        } else {
            key += c;
        }
    }
    const char* value = std::getenv(key.c_str());
    return value == nullptr ? std::string() : std::string(value);
}

#endif

}  // namespace pwroff
