#include "evk/inner_pre.h"
#include "evk/tcp_conn.h"
#include "evk/event_loop.h"
#include "evk/channel.h"
#include "evk/socket.h"
#include "evk/socket_ops.h"

#include <errno.h>

namespace evk {
TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& local,
                             const InetAddress& peer)
    : loop_(loop),
      name_(name),
      type_(kIncoming),
      status_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      local_addr_(local),
      peer_addr_(peer) {
    DLOG_TRACE << "TcpConnection::ctor[" <<  name_ << "] at " << this
               << " fd=" << sockfd;;
    channel_->SetReadCallback(
            std::bind(&TcpConnection::HandleRead, this));
    channel_->SetWriteCallback(
            std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetCloseCallback(
            std::bind(&TcpConnection::HandleClose, this));
    channel_->SetErrorCallback(
            std::bind(&TcpConnection::HandleError, this));
    socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    DLOG_TRACE;
    assert(status_ == kDisconnected);
}

void TcpConnection::OnConnectEstablished() {
    loop_->AssertInLoopThread();
    assert(status_ == kConnecting);
    SetStatus(kConnected);
    channel_->EnableReadEvent();
    conn_cb_(shared_from_this());
}

void TcpConnection::OnConnectDestroyed() {
    loop_->AssertInLoopThread();
    if (status_ == kConnected) {
        SetStatus(kDisconnected);
        channel_->DisableAllEvent();
        conn_cb_(shared_from_this());
    }
    channel_->Remove();
}

void TcpConnection::HandleRead() {
    loop_->AssertInLoopThread();
    int savedError = 0;
    ssize_t n = input_buffer_.ReadFromFD(channel_->fd(), &savedError);
    if (n > 0) {
        msg_cb_(shared_from_this(), &input_buffer_);
    } else if (n == 0) {
        HandleClose();
    } else {
        errno = savedError;
        LOG_ERROR << "TcpConnection::HandleRead";
        HandleError();
    }
}

void TcpConnection::HandleClose() {
    loop_->AssertInLoopThread();
    DLOG_TRACE << "fd = " << channel_->fd();
    assert(status_ == kConnected || status_ == kDisconnecting);
    // we don't close fd here
    SetStatus(kDisconnected);
    channel_->DisableAllEvent();
    close_cb_(shared_from_this());
}

void TcpConnection::HandleError() {
    int err = sock::GetSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::HandleError [" << name_
              << "] - SO_ERROR = " << err;
}


} // namespace evk
