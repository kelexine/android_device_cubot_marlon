// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace pwroff {

// Read-only view of Android system properties, injectable for tests.
class PropertySource {
  public:
    virtual ~PropertySource() = default;
    // Current value, or "" when the property is unset.
    virtual std::string Get(const std::string& name) const = 0;
};

// Android: bionic's property area. Host builds read the environment variable
// PWROFF_PROP_<NAME> (upper-cased, '.' -> '_'), which keeps the real binary
// smoke-testable off-device.
class PlatformPropertySource final : public PropertySource {
  public:
    std::string Get(const std::string& name) const override;
};

}  // namespace pwroff
