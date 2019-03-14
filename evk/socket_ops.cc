#include "evk/inner_pre.h"
#include "evk/socket_ops.h"

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

int CreateNonblockingSocket() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
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

} // namespace sock
} // namespace evk
