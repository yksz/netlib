#include "cx/net/interface.h"
#include <codecvt>
#include <winsock2.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")

namespace cx {

error GetNetworkInterfaces(std::vector<NetworkInterface>* infs) {
    ULONG bufSize = 15000;
    PIP_ADAPTER_ADDRESSES pAddrs = NULL;
    do {
        pAddrs = (IP_ADAPTER_ADDRESSES*) malloc(bufSize);
        if (pAddrs == NULL) {
            return error::nomem;
        }
        auto result = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddrs, &bufSize);
        if (result == NO_ERROR) {
            break;
        }
        free(pAddrs);
        pAddrs = NULL;
        if (result != ERROR_BUFFER_OVERFLOW) {
            return error::io;
        }
    } while (true);

    for (auto pAddr = pAddrs; pAddr != NULL; pAddr = pAddr->Next) {
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

} // namespace cx
