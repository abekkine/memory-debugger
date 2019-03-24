#ifndef UDP_SOCKET_HPP_
#define UDP_SOCKET_HPP_

#include <string>

#include <winsock2.h>

class UdpSocket {
public:
    UdpSocket() {

    }
    ~UdpSocket() {

    }
    int Read(void * data, int size) {
        struct sockaddr_in remoteAddr;
        int slen = sizeof(remoteAddr);
        int rxBytes = -1;

        rxBytes = recvfrom(
                      sock_,
                      (char *)data,
                      size,
                      0,
                      (struct sockaddr *)&remoteAddr,
                      &slen
                  );

        return rxBytes;
    }
    int Write(void * data, int size) {
        int txBytes = -1;
        int retval = sendto(
                         sock_,
                         (char *)data,
                         size,
                         0,
                         (SOCKADDR *)&remote_host_,
                         sizeof(remote_host_)
                     );
        if (retval != SOCKET_ERROR) {
            txBytes = retval;
        }
        else {
            puts("Unable to send message!");
        }
        return txBytes;
    }
    void SetRemote(std::string addr, int port) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            throw std::string("Unable to initialize Winsock!");
        }
        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == INVALID_SOCKET) {
            throw std::string("Unable to create socket!");
        }
        remote_host_.sin_family = AF_INET;
        remote_host_.sin_port = htons(port);
        remote_host_.sin_addr.s_addr = inet_addr(addr.c_str());
    }
    void Init(int port) {
        port_ = port;
        struct sockaddr_in local_addr_;

        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            throw std::string("Unable to initialize Winsock!");
        }

        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == INVALID_SOCKET) {
            throw std::string("Unable to create socket!");
        }

        unsigned long nonBlocking = 1;
        if (ioctlsocket(sock_, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
            throw std::string("Unable to enable non-blocking i/o!");
        }

        local_addr_.sin_family = AF_INET;
        local_addr_.sin_addr.s_addr = INADDR_ANY;
        local_addr_.sin_port = htons(port_);

        if (bind(sock_, (struct sockaddr *)&local_addr_, sizeof(local_addr_)) == SOCKET_ERROR) {
            throw std::string("Unable to bind socket!");
        }
    }

    void Close() {

    }
private:
    SOCKET sock_;
    int port_;
    sockaddr_in remote_host_;
};

#endif // UDP_SOCKET_HPP_
