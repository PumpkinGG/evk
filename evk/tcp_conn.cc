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



} // namespace evk
