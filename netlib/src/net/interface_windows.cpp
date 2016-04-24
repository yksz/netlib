#include "net/interface.h"
#include <codecvt>
#include <winsock2.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")

namespace net {

error GetNetworkInterfaces(std::vector<NetworkInterface>* infs) {
    if (infs == nullptr) {
        assert(0 && "infs must not be nullptr");
        return error::illegal_argument;
    }

    ULONG bufSize = 15000;
    PIP_ADAPTER_ADDRESSES pAddrs = nullptr;
    do {
        pAddrs = (IP_ADAPTER_ADDRESSES*) malloc(bufSize);
        if (pAddrs == nullptr) {
            return error::nomem;
        }
        auto result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, pAddrs, &bufSize);
        if (result == NO_ERROR) {
            break;
        }
        free(pAddrs);
        pAddrs = nullptr;
        if (result != ERROR_BUFFER_OVERFLOW) {
            return error::io;
        }
    } while (true);

    for (auto pAddr = pAddrs; pAddr != nullptr; pAddr = pAddr->Next) {
        NetworkInterface inf = {0};
        inf.index = pAddr->IfIndex;
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        inf.name = converter.to_bytes(pAddr->FriendlyName);
        for (int i = 0; i < pAddr->PhysicalAddressLength; i++) {
            inf.hardwareAddress[i] = pAddr->PhysicalAddress[i];
        }
        if (pAddr->OperStatus == IfOperStatusUp) {
            inf.isUp = true;
        }
        if (pAddr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
            inf.isLoopback = true;
        }
        infs->push_back(std::move(inf));
    }
    free(pAddrs);
}

} // namespace net
