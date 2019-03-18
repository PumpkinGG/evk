#include "evk/slice.h"
#include "evk/socket_ops.h"
#include "evk/inet_address.h"

#include <iostream>

int main(int argc, char* argv[]) {
    //struct sockaddr_in s_addr;
    //evk::sock::ParseFromIpPort("192.168.3.18", 1234, &s_addr);

    evk::InetAddress inetAddr("192.168.3.22", 1234);

    std::cout << "ToIpPort: " << inetAddr.ToIpPort() << std::endl;
    std::cout << "ToIp:" << inetAddr.ToIp() << std::endl;
    std::cout << "ToPort:" << inetAddr.ToPort() << std::endl;
    return 0;
}
