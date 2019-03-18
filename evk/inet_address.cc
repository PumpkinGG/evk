#include "evk/inner_pre.h"
#include "evk/inet_address.h"
#include "evk/socket_ops.h"

#include <netdb.h>
#include <netinet/in.h>

// INADDR_ANY use (type)value casting.
// Ignored c style cast warnning here.
#pragma GCC diagnostic ignored "-Wold-style-cast"
// Bind 0.0.0.0, can listen multi Network Interface Card
static const in_addr_t kInaddrAny = INADDR_ANY;
// Loopback ip 127.0.0.1
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

namespace evk {
// static_assert ----- C++11 Compile-time Assertion
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t port, bool loop_back_only, bool ipv6) {
    static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
    static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
    if (ipv6) {
        memset(&addr6_, 0, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loop_back_only ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = ::htons(port);
    } else {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        in_addr_t ip = loop_back_only ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = ::htonl(ip);
        addr_.sin_port = ::htons(port);
    }
}

InetAddress::InetAddress(Slice ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        memset(&addr6_, 0, sizeof(addr6_));
        sock::ParseFromIpPort(ip.data(), port, &addr6_);
    } else {
        memset(&addr_, 0, sizeof(addr_));
        sock::ParseFromIpPort(ip.data(), port, &addr_);
    }
}

InetAddress::InetAddress(const struct sockaddr_in& addr)
    : addr_(addr) {}

InetAddress::InetAddress(const struct sockaddr_in6& addr6)
    : addr6_(addr6) {}

const struct sockaddr* InetAddress::GetSockAddr() const {
    return sock::sockaddr_cast(&addr6_);
}

sa_family_t InetAddress::Family() const {
    return addr_.sin_family;
}

std::string InetAddress::ToIpPort() const {
    char buf[64];
    sock::ToIpPort(buf, sizeof(buf), GetSockAddr());
    return std::string(buf);
}

std::string InetAddress::ToIp() const {
    char buf[64];
    sock::ToIp(buf, sizeof(buf), GetSockAddr());
    return std::string(buf);
}

uint16_t InetAddress::ToPort() const {
    return ::ntohs(PortNetEndian());
}

uint32_t InetAddress::IpNetEndian() const {
    assert(Family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::PortNetEndian() const {
    return addr_.sin_port;
}

void InetAddress::SetSockAddrInet6(const struct sockaddr_in6& addr6) {
    addr6_ = addr6;
}

// using thread local to avoid malloc
static thread_local char resolve_buffer_t[64 * 1024];
bool InetAddress::Resolve(Slice hostname, InetAddress* result) {
    assert(result != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    memset(&hent, 0, sizeof(hent));
    int ret = gethostbyname_r(hostname.data(), &hent, resolve_buffer_t, 
                              sizeof(resolve_buffer_t), &he, &herrno);
    if (ret == 0 && he != NULL) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        result->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    } else {
        if (ret) {
            LOG_ERROR << "InetAddress::Resolve";
        }
        return false;
    }
}

void InetAddress::SetScopeID(uint32_t scope_id) {
    if (Family() == AF_INET6) {
        addr6_.sin6_scope_id = scope_id;
    }
}

} // namespace evk
