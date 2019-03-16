#include "evk/event_loop.h"
#include "evk/acceptor.h"
#include "evk/inet_address.h"
#include "evk/logging.h"

#include <stdio.h>
#include <unistd.h>

void NewConnection(int sockfd, const evk::InetAddress& peer) {
    printf("newConnection(): accepted a new connection from %s\n", 
           peer.ToIpPort().data());
    evk::sock::Write(sockfd, "Hello\n", 6);
    evk::sock::Close(sockfd); 
}

int main() {
    printf("\nmain(): pid = %d\n\n", getpid());
    
    evk::InetAddress listenAddr(8812);

    printf("\nlocalhost: %s\n\n", listenAddr.ToIpPort().data());
    evk::EventLoop loop;
    evk::Acceptor acceptor(&loop, listenAddr);
    acceptor.SetNewConnectionCallback(NewConnection);
    acceptor.Listen();

    loop.Run();
}
