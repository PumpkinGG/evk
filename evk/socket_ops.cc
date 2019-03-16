#include "evk/inner_pre.h"
#include "evk/socket_ops.h"

#include <stdio.h> // snprintf
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h> // readv

namespace evk {
namespace sock {

namespace {
void SetNonBlockAndCloseOnExec(int sockfd) {
  // non-block
  int flags = ::fcntl(sockfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(sockfd, F_SETFL, flags);
  // FIXME check

  // close-on-exec
  flags = ::fcntl(sockfd, F_GETFD, 0);
  flags |= FD_CLOEXEC;
  ret = ::fcntl(sockfd, F_SETFD, flags);
  // FIXME check

  (void)ret;
}

} // namespace

int CreateNonblockingSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        LOG_FATAL << "sock::CreateNonblockingSocket";
    }
    return sockfd;
}

int Connect(int sockfd, const struct sockaddr* addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(*addr)));
}

int Accept(int sockfd, struct sockaddr_in6* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    // ::accept4 is another choice
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    SetNonBlockAndCloseOnExec(connfd);
    if (connfd < 0) {
        int saved_errno = errno;
        LOG_ERROR << "sock::accept";
        switch (saved_errno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                // expected errors
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept" << saved_errno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept" << saved_errno;
                break;
        }
    }
    return connfd;
}

void Bind(int sockfd, const struct sockaddr* addr) {
    if (::bind(sockfd, addr, static_cast<socklen_t>(sizeof(*addr))) < 0) {
        LOG_FATAL << "sock::Bind";
    }
}

void Listen(int sockfd) {
    if (::listen(sockfd, SOMAXCONN) < 0) {
        LOG_FATAL << "sock::Listen";
    }
}

void Close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOG_ERROR << "sock::Close";
    }
}

void ShutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        LOG_ERROR << "sock::ShutdownWrite";
    }
}

ssize_t Read(int sockfd, void *buf, size_t count) {
    return ::read(sockfd, buf, count);
}

ssize_t Readv(int sockfd, const struct iovec *iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}

ssize_t Write(int sockfd, const void *buf, size_t count) {
    return ::write(sockfd, buf, count);
}

int GetSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

void ToIpPort(char* buf, size_t size, const struct sockaddr* addr) {
    ToIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = ::ntohs(addr4->sin_port);
    assert(size > end);
    snprintf(buf+end, size-end, ":%u", port);
}

void ToIp(char* buf, size_t size, const struct sockaddr* addr) {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    } else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void ParseFromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = ::htons(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        LOG_ERROR << "sock::FromIpPort";
    }
}

void ParseFromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = ::htons(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        LOG_ERROR << "sock::FromIpPort";
    }
}

struct sockaddr_in6 GetLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0) {
        LOG_ERROR << "sock::GetLocalAddr";
    }
    return localaddr;
}

struct sockaddr_in6 GetPeerAddr(int sockfd) {
    struct sockaddr_in6 peeraddr;
    memset(&peeraddr, 0, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0) {
        LOG_ERROR << "sock::GetPeerAddr";
    }
    return peeraddr;
}

bool IsSelfConnect(int sockfd) {
    struct sockaddr_in6 localaddr = GetLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = GetPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const struct sockaddr_in* lhaddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* rhaddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return (lhaddr4->sin_port == rhaddr4->sin_port)
            && (lhaddr4->sin_addr.s_addr == rhaddr4->sin_addr.s_addr);
    } else if (localaddr.sin6_family == AF_INET6) {
        return (localaddr.sin6_port == peeraddr.sin6_port)
            && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof(localaddr.sin6_addr) == 0);
    } else {
        return false;
    }
}

} // namespace sock
} // namespace evk
