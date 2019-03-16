#include "evk/inner_pre.h"
#include "evk/acceptor.h"
#include "evk/event_loop.h"
#include "evk/inet_address.h"
#include "evk/socket_ops.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace evk {
Acceptor::Acceptor(EventLoop* loop, 
                   const InetAddress& listen_addr, bool reuse_port)
    : loop_(loop),
      accept_socket_(sock::CreateNonblockingSocket(listen_addr.Family())),
      accept_channel_(loop, accept_socket_.fd()),
      idlefd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
      listenning_(false) {
    DLOG_TRACE;
    assert(idlefd_ >= 0);
    accept_socket_.SetReuseAddr(true);
    accept_socket_.SetReusePort(reuse_port);
    accept_socket_.BindAddress(listen_addr);
    accept_channel_.SetReadCallback(
            std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {
    accept_channel_.DisableAllEvent();
    // accept_channel_.Remove();
    ::close(idlefd_);
}

void Acceptor::SetNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_callback_ = cb;
}

void Acceptor::Listen() {
    loop_->AssertInLoopThread();
    listenning_ = true;
    accept_socket_.Listen();
    accept_channel_.EnableReadEvent();
}

bool Acceptor::IsListenning() const {
    return listenning_;
}

void Acceptor::HandleRead() {
    loop_->AssertInLoopThread();
    InetAddress peer;
    int connfd = accept_socket_.Accept(&peer);
    if (connfd >= 0) {
        if (new_connection_callback_) {
            new_connection_callback_(connfd, peer);
        } else {
            sock::Close(connfd);
        }
    } else {
        LOG_ERROR << "Acceptor::HandleRead";
        // close new fd when the number of fd is Max
        if (errno == EMFILE) {
            ::close(idlefd_);
            idlefd_ = ::accept(accept_socket_.fd(), NULL, NULL);
            ::close(idlefd_);
            idlefd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} // namespace evk
