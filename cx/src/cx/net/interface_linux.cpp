#include "cx/net/interface.h"
#include <cerrno>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace cx {

static error getIndex(int fd, struct ifreq* ifr, int* index) {
    if (ioctl(fd, SIOCGIFINDEX, ifr) == -1) {
        return GetOSError(errno);
    }
    *index = ifr->ifr_ifindex;
    return error::nil;
}

static error getName(int fd, struct ifreq* ifr, std::string* name) {
#ifdef SIOCGIFNAME
    if (ioctl(fd, SIOCGIFNAME, ifr) == -1) {
        return GetOSError(errno);
    }
    *name = ifr->ifr_name;
#endif
    return error::nil;
}

static error getHardwareAddress(int fd, struct ifreq* ifr,
        std::array<unsigned char, 6>* hardwareAddress) {
    if (ioctl(fd, SIOCGIFHWADDR, ifr) == -1) {
        return GetOSError(errno);
    }
    for (size_t i = 0; i < hardwareAddress->size(); i++) {
        (*hardwareAddress)[i] = ifr->ifr_hwaddr.sa_data[i];
    }
    return error::nil;
}

static error getMTU(int fd, struct ifreq* ifr, int* mtu) {
    if (ioctl(fd, SIOCGIFMTU, ifr) == -1) {
        return GetOSError(errno);
    }
    *mtu = ifr->ifr_mtu;
    return error::nil;
}

static error getIsLoopback(int fd, struct ifreq* ifr, bool* isLoopback) {
    if (ioctl(fd, SIOCGIFFLAGS, ifr) == -1) {
        return GetOSError(errno);
    }
    *isLoopback = ifr->ifr_flags & IFF_LOOPBACK;
    return error::nil;
}

error GetNetworkInterfaces(std::vector<NetworkInterface>* infs) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        return GetOSError(errno);
    }

    struct ifreq ifr[16]; // max number of interfaces
    struct ifconf ifc;
    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_req = ifr;
    if (ioctl(fd, SIOCGIFCONF, &ifc) == -1) {
        int err = errno;
        close(fd);
        return GetOSError(err);
    }

    error err;
    int nifs = ifc.ifc_len / sizeof(struct ifreq);
    for (int i = 0; i < nifs; i++) {
        NetworkInterface inf = {0};
        err = getIndex(fd, &ifr[i], &inf.index);
        if (err != error::nil) {
            goto fail;
        }
        err = getName(fd, &ifr[i], &inf.name);
        if (err != error::nil) {
            goto fail;
        }
        err = getHardwareAddress(fd, &ifr[i], &inf.hardwareAddress);
        if (err != error::nil) {
            goto fail;
        }
        err = getMTU(fd, &ifr[i], &inf.mtu);
        if (err != error::nil) {
            goto fail;
        }
        err = getIsLoopback(fd, &ifr[i], &inf.isLoopback);
        if (err != error::nil) {
            goto fail;
        }
        infs->push_back(inf);
    }

fail:
    close(fd);
    return err;
}

} // namespace cx
