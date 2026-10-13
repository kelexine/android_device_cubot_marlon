/*
 * File: libshim_crypto.cpp
 * Author: kelexine <https://github.com/kelexine>
 * Date: 2026-10-11
 * Purpose: BoringSSL crypto and socket shim for legacy Widevine DRM and wpa_supplicant.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

extern "C" {

struct CBS {
    const uint8_t *data;
    size_t len;
};

/*
 * BoringSSL CBS_init shim for legacy Widevine DRM (libwvhidl.so / libwvdrmengine.so).
 */
void CBS_init(struct CBS *cbs, const uint8_t *data, size_t len) {
    if (cbs) {
        cbs->data = data;
        cbs->len = len;
    }
}

/*
 * BoringSSL sk_dup shim for vendor wpa_supplicant.
 */
void *OPENSSL_sk_dup(const void *sk);

void *sk_dup(const void *sk) {
    return OPENSSL_sk_dup(sk);
}

/*
 * In-library bind() interception for wpa_supplicant control sockets:
 * Unlinks stale AF_UNIX control sockets on collision so wpa_supplicant
 * can bind without failing with EADDRINUSE on Wi-Fi toggle / restart.
 */
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (addr && addr->sa_family == AF_UNIX) {
        const struct sockaddr_un *un = (const struct sockaddr_un *)addr;
        if (un->sun_path[0] != '\0') {
            unlink(un->sun_path);
        }
    }

    typedef int (*real_bind_t)(int, const struct sockaddr *, socklen_t);
    static real_bind_t real_bind = (real_bind_t)dlsym(RTLD_NEXT, "bind");
    if (real_bind) {
        return real_bind(sockfd, addr, addrlen);
    }
    return -1;
}

} // extern "C"

