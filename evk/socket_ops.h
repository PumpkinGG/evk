#pragma once

#include <arpa/inet.h>

namespace evk {
namespace sock {
//
// Create a non-blocking socket file descriptor,
// Abort if any error
//
int CreateNonblockingSocket();

int Connect(int sockfd, const struct sockaddr* addr);
int Accept(int sockfd, struct sockaddr_in6* addr);
void Bind(int sockfd, const struct sockaddr* addr);
void Listen(int sockfd);
void Close(int sockfd);
void ShutdownWrite(int sockfd);
ssize_t Read(int sockfd, void *buf, size_t count);
ssize_t Readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t Write(int sockfd, const void *buf, size_t count);

int GetSocketError(int sockfd);

// 
// address struct <==> char*
//
void ToIpPort(char* buf, size_t size, const struct sockaddr* addr);
void ToIp(char* buf, size_t size, const struct sockaddr* addr);

void ParseFromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void ParseFromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

struct sockaddr_in6 GetLocalAddr(int sockfd);
struct sockaddr_in6 GetPeerAddr(int sockfd);
bool IsSelfConnect(int sockfd);

// 
// address struct transfor
//
template<typename To, typename From>
inline To implicit_cast(From const& f) {
    return f;
}

inline const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr) {
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}
inline const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr) {
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}
inline struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr) {
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}
inline const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}
inline const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr) {
    return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

} // namespace socks
} // namespace evk
