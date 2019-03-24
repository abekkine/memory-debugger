#ifndef TCP_SERVER_SOCKET_HPP_
#define TCP_SERVER_SOCKET_HPP_

#include <winsock2.h>

class TcpServerSocket {
public:
    TcpServerSocket() {}
    ~TcpServerSocket() {}
    void Init(int port) {
        // defaults
        listen_sock_ = INVALID_SOCKET;

        // initialize WinSock2
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            throw std::string("Unable to initialize Winsock!");
        }

        listen_sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listen_sock_ == INVALID_SOCKET) {
            throw std::string("Unable to create listen socket!");
        }

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(port);

        int rv = bind(listen_sock_, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
        if (rv == SOCKET_ERROR) {
            throw std::string("Unable to bind socket!");
        }

        unsigned long nonBlocking = 1;
        if (ioctlsocket(listen_sock_, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
            throw std::string("Unable to enable non-blocking i/o!");
        }

        // Number of allowed connections is '3'.
        rv = listen(listen_sock_, 3);
        if (rv == SOCKET_ERROR) {
            throw std::string("Unable to listen socket!");
        }
    }
    bool Accept(SOCKET & socket) {
        socket = accept(listen_sock_, NULL, NULL);
        if (socket == INVALID_SOCKET) {
            return false;
        }
        return true;
    }

private:
    SOCKET listen_sock_;
};

#endif // TCP_SERVER_SOCKET_HPP_
