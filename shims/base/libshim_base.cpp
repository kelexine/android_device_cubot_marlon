/*
 * File: libshim_base.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: Android base compatibility shim providing Basename, Trim, and WriteStringToFd.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dlfcn.h>

void* operator new(size_t size) { return malloc(size); }
void* operator new[](size_t size) { return malloc(size); }
void operator delete(void* ptr) noexcept { free(ptr); }
void operator delete[](void* ptr) noexcept { free(ptr); }
void operator delete(void* ptr, size_t) noexcept { free(ptr); }
void operator delete[](void* ptr, size_t) noexcept { free(ptr); }

namespace std {
inline namespace __1 {

template <class CharT>
struct char_traits {};

template <class T>
struct allocator {};

template <class CharT, class Traits = char_traits<CharT>, class Alloc = allocator<CharT>>
class basic_string {
public:
#if defined(__aarch64__)
    struct Long {
        size_t is_long_ : 1;
        size_t cap_ : 63;
        size_t size_;
        char* data_;
    };
    enum { min_cap = (sizeof(Long) - 1) / sizeof(char) };
    struct Short {
        unsigned char is_long_ : 1;
        unsigned char size_ : 7;
        char data_[min_cap];
    };
    union Rep {
        Long l_;
        Short s_;
    } r_;
#else
    struct Long {
        size_t is_long_ : 1;
        size_t cap_ : 31;
        size_t size_;
        char* data_;
    };
    enum { min_cap = (sizeof(Long) - 1) / sizeof(char) };
    struct Short {
        unsigned char is_long_ : 1;
        unsigned char size_ : 7;
        char data_[min_cap];
    };
    union Rep {
        Long l_;
        Short s_;
    } r_;
#endif

    basic_string() {
        memset(&r_, 0, sizeof(r_));
    }

    basic_string(const char* s, size_t n) {
        memset(&r_, 0, sizeof(r_));
        if (n < min_cap) {
            r_.s_.is_long_ = 0;
            r_.s_.size_ = n;
            memcpy(r_.s_.data_, s, n);
            r_.s_.data_[n] = '\0';
        } else {
            char* p = static_cast<char*>(malloc(n + 1));
            if (p) {
                memcpy(p, s, n);
                p[n] = '\0';
                r_.l_.is_long_ = 1;
                r_.l_.cap_ = n;
                r_.l_.size_ = n;
                r_.l_.data_ = p;
            }
        }
    }

    const char* data() const {
        return (r_.s_.is_long_ ? r_.l_.data_ : r_.s_.data_);
    }

    size_t size() const {
        return (r_.s_.is_long_ ? r_.l_.size_ : static_cast<size_t>(r_.s_.size_));
    }
};

template <class CharT, class Traits = char_traits<CharT>>
class basic_string_view {
public:
    const char* data_;
    size_t size_;
};

using string = basic_string<char>;
using string_view = basic_string_view<char>;

} // namespace __1
} // namespace std

namespace android {
namespace base {

using string = std::__1::string;
using string_view = std::__1::string_view;

struct borrowed_fd {
    int fd_;
};

string Basename(const string& path) {
    const char* d = path.data();
    size_t len = path.size();
    while (len > 0 && d[len - 1] == '/') {
        len--;
    }
    if (len == 0) return string("/", 1);
    size_t last_slash = len;
    for (size_t i = len; i > 0; --i) {
        if (d[i - 1] == '/') {
            last_slash = i;
            break;
        }
    }
    if (last_slash == len) {
        return string(d, len);
    }
    return string(d + last_slash, len - last_slash);
}

string Basename(string_view sv) {
    string tmp(sv.data_, sv.size_);
    return Basename(tmp);
}

string Trim(const string& s) {
    const char* d = s.data();
    size_t len = s.size();
    size_t start = 0;
    while (start < len && (d[start] == ' ' || d[start] == '\t' || d[start] == '\n' || d[start] == '\r')) {
        start++;
    }
    size_t end = len;
    while (end > start && (d[end - 1] == ' ' || d[end - 1] == '\t' || d[end - 1] == '\n' || d[end - 1] == '\r')) {
        end--;
    }
    return string(d + start, end - start);
}

string Trim(string_view sv) {
    string tmp(sv.data_, sv.size_);
    return Trim(tmp);
}

bool WriteStringToFd(string_view sv, int fd) {
    const char* d = sv.data_;
    size_t len = sv.size_;
    while (len > 0) {
        ssize_t n = write(fd, d, len);
        if (n <= 0) return false;
        d += n;
        len -= n;
    }
    return true;
}

bool WriteStringToFd(const string& s, borrowed_fd bfd) {
    string_view sv{s.data(), s.size()};
    return WriteStringToFd(sv, bfd.fd_);
}

} // namespace base
} // namespace android

/*
 * RefBase compatibility stubs for pre-Android 12 MediaTek blobs (e.g. pq@2.2-service).
 * Android 12+ enforces incStrongRequireStrong on RefBase and LightRefBase, which asserts
 * if an object starts a thread in its constructor before being assigned to an sp<>.
 * Resolving incStrong dynamically via dlsym avoids link-time undefined references while
 * safely bypassing the abort check.
 */
extern "C" {

void _ZNK7android7RefBase22incStrongRequireStrongEPKv(const void* thiz, const void* id) {
    typedef void (*incStrongFn)(const void*, const void*);
    static incStrongFn real_incStrong = (incStrongFn)dlsym(RTLD_DEFAULT, "_ZNK7android7RefBase9incStrongEPKv");
    if (real_incStrong) {
        real_incStrong(thiz, id);
    }
}

void _ZN7android47LightRefBase_reportIncStrongRequireStrongFailedEPKv(const void* /*thiz*/) {
    // No-op stub to prevent aborting on LightRefBase
}

/*
 * Thread::run compatibility wrapper.
 * In Android 12+, Thread::run inlines an assertion in libutils that aborts if the
 * thread object has not already been assigned to an sp<> (strong count is INITIAL_STRONG_VALUE).
 * Intercepting Thread::run and incrementing strong refcount via incStrong ensures the
 * inlined incStrongRequireStrong check passes, after which we decrement the refcount to balance.
 */
typedef int32_t (*RealThreadRun64)(void* thiz, const char* name, int32_t priority, size_t stack);
typedef int32_t (*RealThreadRun32)(void* thiz, const char* name, int32_t priority, uint32_t stack);
typedef void (*RefBaseIncStrong)(const void* thiz, const void* id);
typedef void (*RefBaseDecStrong)(const void* thiz, const void* id);

int32_t _ZN7android6Thread3runEPKcim(void* thiz, const char* name, int32_t priority, size_t stack) {
    static RealThreadRun64 real_run = nullptr;
    static RefBaseIncStrong real_inc = (RefBaseIncStrong)dlsym(RTLD_DEFAULT, "_ZNK7android7RefBase9incStrongEPKv");
    static RefBaseDecStrong real_dec = (RefBaseDecStrong)dlsym(RTLD_DEFAULT, "_ZNK7android7RefBase9decStrongEPKv");

    if (!real_run) {
        real_run = (RealThreadRun64)dlsym(RTLD_NEXT, "_ZN7android6Thread3runEPKcim");
        if (!real_run) {
            void* handle = dlopen("libutils.so", RTLD_NOW);
            if (handle) {
                real_run = (RealThreadRun64)dlsym(handle, "_ZN7android6Thread3runEPKcim");
            }
        }
    }

    if (real_inc) {
        real_inc(thiz, thiz);
    }
    int32_t ret = -1;
    if (real_run) {
        ret = real_run(thiz, name, priority, stack);
    }
    if (real_dec) {
        real_dec(thiz, thiz);
    }
    return ret;
}

int32_t _ZN7android6Thread3runEPKcij(void* thiz, const char* name, int32_t priority, uint32_t stack) {
    static RealThreadRun32 real_run = nullptr;
    static RefBaseIncStrong real_inc = (RefBaseIncStrong)dlsym(RTLD_DEFAULT, "_ZNK7android7RefBase9incStrongEPKv");
    static RefBaseDecStrong real_dec = (RefBaseDecStrong)dlsym(RTLD_DEFAULT, "_ZNK7android7RefBase9decStrongEPKv");

    if (!real_run) {
        real_run = (RealThreadRun32)dlsym(RTLD_NEXT, "_ZN7android6Thread3runEPKcij");
        if (!real_run) {
            void* handle = dlopen("libutils.so", RTLD_NOW);
            if (handle) {
                real_run = (RealThreadRun32)dlsym(handle, "_ZN7android6Thread3runEPKcij");
            }
        }
    }

    if (real_inc) {
        real_inc(thiz, thiz);
    }
    int32_t ret = -1;
    if (real_run) {
        ret = real_run(thiz, name, priority, stack);
    }
    if (real_dec) {
        real_dec(thiz, thiz);
    }
    return ret;
}

} // extern "C"
