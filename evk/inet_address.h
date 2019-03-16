#pragma once

#include "evk/inner_pre.h"
#include "evk/socket_ops.h"
#include "evk/slice.h"

#include <netinet/in.h>

namespace evk {
//
// warpper of sockaddr_in
//
class InetAddress {
public:
    // construct an endpoint with given port number
    explicit InetAddress(uint16_t port = 0, 
                         bool loop_back_only = false, bool ipv6 = false);
    // construct an endpoint with given ip and port
    InetAddress(Slice ip, uint16_t port, bool ipv6 = false);

    // construct an endpoint with given struct sockaddr_in
    explicit InetAddress(const struct sockaddr_in& addr);
    explicit InetAddress(const struct sockaddr_in6& addr);

public:
    const struct sockaddr* GetSockAddr() const;
    sa_family_t Family() const;
    Slice ToIpPort() const;
    Slice ToIp() const;
    uint16_t ToPort() const;

    uint32_t IpNetEndian() const;
    uint16_t PortNetEndian() const;

public:
    void SetSockAddrInet6(const struct sockaddr_in6& addr6);

    // resolve hostname to IP address,
    // not changing port or sin_family
    static bool Resolve(Slice hostname, InetAddress* result);

    // set ipv6 scopeID
    void SetScopeID(uint32_t scope_id);

private:
    union {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;     
    };
};

} // namespace evk
