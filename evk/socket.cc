#include "evk/inner_pre.h"
#include "evk/socket.h"
#include "evk/inet_address.h"
#include "evk/socket_ops.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h> // snprintf

namespace evk {
Socket::Socket(int sockfd)
    : sockfd_(sockfd) {}

Socket::~Socket() {
    sock::Close(sockfd_);
}

int Socket::fd() const {
    return sockfd_;
}

bool Socket::GetTcpInfo(struct tcp_info* info) const {
    socklen_t len = sizeof(*info);
    memset(info, 0, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, info, &len);
}

bool Socket::GetTcpInfoString(char* buf, int len) const {
    struct tcp_info info;
    bool isOK = GetTcpInfo(&info);
    if (isOK) {
        snprintf(buf, len, "unrecovered=%u "
                 "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                 "lost=%u retrans=%u rtt=%u rttvar=%u "
                 "sshthresh=%u cwnd=%u total_retrans=%u",
                 info.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                 info.tcpi_rto,          // Retransmit timeout in usec
                 info.tcpi_ato,          // Predicted tick of soft clock in usec
                 info.tcpi_snd_mss,
                 info.tcpi_rcv_mss,
                 info.tcpi_lost,         // Lost packets
                 info.tcpi_retrans,      // Retransmitted packets out
                 info.tcpi_rtt,          // Smoothed round trip time in usec
                 info.tcpi_rttvar,       // Medium deviation
                 info.tcpi_snd_ssthresh,
                 info.tcpi_snd_cwnd,
                 info.tcpi_total_retrans);  // Total retransmits for entire connection")
    }
    return isOK;
}

void Socket::BindAddress(const InetAddress& localaddr) {
    sock::Bind(sockfd_, localaddr.GetSockAddr());
}

void Socket::Listen() {
    sock::Listen(sockfd_);
}

int Socket::Accept(InetAddress* peeraddr) {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    int connfd = sock::Accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->SetSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::ShutDownWrite() {
    sock::ShutdownWrite(sockfd_);
}

void Socket::SetTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(optval));
}

void Socket::SetReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(optval));
}
void Socket::SetReusePort(bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                 &optval, static_cast<socklen_t>(optval));
    if (ret < 0 && on) {
        LOG_ERROR << "SO_REUSEPORT failed";
    }
#else
    if (on) {
        LOG_ERROR << "SO_REUSEPORT is not supported"
    }
#endif
}

void Socket::SetKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(optval));
}

} // namespace evk
