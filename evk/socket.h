#pragma once

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace evk {
class InetAddress;

//
// Wrapper of socket file descriptor,
// closes the sockfd when descstructs.
//
class Socket {
public:
    explicit Socket(int sockfd);
    ~Socket();

// getter    
public:
    int fd() const;
    // return true if success
    bool GetTcpInfo(struct tcp_info*) const;
    bool GetTcpInfoString(char* buf, int len) const;

// operations
public:
    // abort if address in use
    void BindAddress(const InetAddress& localaddr);
    // abort if address in use
    void Listen();
    // on success, returns sockfd
    // on error, returns -1
    int Accept(InetAddress* peeraddr);
    void ShutDownWrite();

// setter
public:
    // enable/disable TCP_NODELAY
    void SetTcpNoDelay(bool on);
    // enable/disable SO_REUSEADDR
    void SetReuseAddr(bool on);
    // enable/disable SO_REUSEPORT
    void SetReusePort(bool on);
    // enable/disable SO_KEEPALIVE
    void SetKeepAlive(bool on);

private:
    const int sockfd_;

};

} // namespace evk
