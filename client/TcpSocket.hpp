#ifndef TCP_SOCKET_HPP_
#define TCP_SOCKET_HPP_

#include <winsock2.h>

class TcpSocket {
public:
    TcpSocket(SOCKET sock) {
        sock_ = sock;
        unsigned long nonBlocking = 1;
        if (ioctlsocket(sock_, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
            throw std::string("Unable to enable non-blocking i/o!");
        }
    }
    ~TcpSocket() {}
    int Read(void * buffer, int size) {
        int rxBytes = recv(sock_, (char *)buffer, size, 0);
        if (rxBytes < 0) {
            int eCode = WSAGetLastError();
            if (eCode != 10035 &&
                eCode != 10054) {
                std::string eMsg = "Error (" + std::to_string(eCode) + ") during receive!";
                throw eMsg;
            }
            else if (eCode == 10054) {
                rxBytes = 0;
            }
        }
        return rxBytes;
    }

private:
    SOCKET sock_;
};

#endif // TCP_SOCKET_HPP_
