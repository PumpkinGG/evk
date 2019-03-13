#pragma once

#include "evk/inner_pre.h"
#include "evk/timestamp.h"

namespace evk {
class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;

typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

typedef std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)> MessageCallback;

namespace internal {
inline void DefaultConnectionCallback(const TcpConnectionPtr&) {}
inline void DefaultMessageCallback(const TcpConnectionPtr&, Buffer*, Timestamp) {}

} // namespace internal
} // namespace evk
