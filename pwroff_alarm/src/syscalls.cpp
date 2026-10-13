// Author: kelexine <https://github.com/kelexine>
// SPDX-License-Identifier: Apache-2.0

#include "pwroff_alarm/syscalls.h"

#include <fcntl.h>
#include <unistd.h>

namespace pwroff {

int RealSyscalls::Open(const char* path, int flags) {
    return ::open(path, flags);
}

int RealSyscalls::Ioctl(int fd, IoctlRequest request, const void* arg) {
    return ::ioctl(fd, request, arg);
}

void RealSyscalls::Close(int fd) {
    ::close(fd);
}

}  // namespace pwroff
