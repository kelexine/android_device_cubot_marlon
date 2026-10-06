// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include "pwroff_alarm/status.h"

#include <cstring>

namespace pwroff {

std::string Status::ToString() const {
    if (ok()) {
        return "ok";
    }
    std::string out = context_.empty() ? std::string("error") : context_;
    out += ": ";
    out += std::strerror(err_);
    return out;
}

}  // namespace pwroff
