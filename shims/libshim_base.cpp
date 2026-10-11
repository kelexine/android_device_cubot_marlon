/*
 * File: libshim_base.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: Shim library providing self-contained replacements for legacy
 *          android::base (Basename, Trim, WriteStringToFd),
 *          unwindstack (Elf::GetRelPc, RegsArm/Arm64::Read),
 *          BoringSSL (CBS_init), BpBinder::handle,
 *          android::PropertyMap, and legacy C++ ABI exception handlers.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// Minimal operator new/delete for self-containment with -nostdlib++
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
            char* p = (char*)malloc(n + 1);
            memcpy(p, s, n);
            p[n] = '\0';
            r_.l_.is_long_ = 1;
            r_.l_.cap_ = n;
            r_.l_.size_ = n;
            r_.l_.data_ = p;
        }
    }
    const char* data() const {
        return (r_.s_.is_long_ ? r_.l_.data_ : r_.s_.data_);
    }
    size_t size() const {
        return (r_.s_.is_long_ ? r_.l_.size_ : (size_t)r_.s_.size_);
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

class String8;

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

extern "C" {

struct CBS {
    const uint8_t *data;
    size_t len;
};

/*
 * BoringSSL CBS_init shim for legacy Widevine DRM (libwvhidl.so).
 */
void CBS_init(struct CBS *cbs, const uint8_t *data, size_t len) {
    if (cbs) {
        cbs->data = data;
        cbs->len = len;
    }
}

/*
 * unwindstack::Elf::GetRelPc definitions
 */
uint64_t _ZN11unwindstack3Elf8GetRelPcEmPNS_7MapInfoE(uint64_t step_pc, void* /*map_info*/) {
    return step_pc;
}

uint64_t _ZN11unwindstack3Elf8GetRelPcEmPKNS_7MapInfoE(uint64_t step_pc, const void* /*map_info*/) {
    return step_pc;
}

/*
 * unwindstack::RegsArm::Read and RegsArm64::Read definitions
 */
bool _ZN11unwindstack7RegsArm4ReadEPKv(void* /*this_ptr*/, const void* /*data*/) {
    return false;
}

bool _ZN11unwindstack7RegsArm4ReadEPv(void* /*this_ptr*/, void* /*data*/) {
    return false;
}

bool _ZN11unwindstack9RegsArm644ReadEPKv(void* /*this_ptr*/, const void* /*data*/) {
    return false;
}

bool _ZN11unwindstack9RegsArm644ReadEPv(void* /*this_ptr*/, void* /*data*/) {
    return false;
}

/*
 * VNDK Binder compatibility:
 * android::BpBinder::handle() const
 */
int32_t _ZNK7android8BpBinder6handleEv(void* /*this_ptr*/) {
    return 0;
}

/*
 * Audio effects config:
 * android::deviceFromString(const std::string&, unsigned int&)
 */
bool _ZN7android16deviceFromStringERKNSt3__112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERj(const void* /*str*/, uint32_t* dev) {
    if (dev) *dev = 0;
    return true;
}

/*
 * Legacy C++ ABI Exception Handling for 32-bit VoLTE / RIL Daemons:
 * bip, volte_imcb, volte_stack, volte_ua, volte_imsm_93, wfca
 */
bool __cxa_type_match(void* /*throw_type*/, void* /*catch_type*/, bool /*is_reference*/, void** /*matched_object*/) {
    return false;
}

void __cxa_begin_cleanup(void* /*unwind_exception*/) {
}

void __cxa_call_unexpected(void* /*unwind_exception*/) {
}

/*
 * Legacy vndservicemanager Binder ProcessState / IPCThreadState stubs
 */
bool _ZN7android12ProcessState20becomeContextManagerEPFbRKNS_8String16ERKNS_2spINS_7IBinderEEEPvES9_(void* /*this_ptr*/, void* /*cb*/, void* /*userData*/) {
    return true;
}

int32_t _ZN7android12ProcessState32getStrongRefCountForNodeByHandleEi(void* /*this_ptr*/, int32_t /*handle*/) {
    return 1;
}

void _ZN7android14IPCThreadState19setTheContextObjectENS_2spINS_7BBinderEEE(void* /*this_ptr*/, void* /*obj*/) {
}

} // extern "C"
