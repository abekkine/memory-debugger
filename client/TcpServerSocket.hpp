#ifndef TCP_SERVER_SOCKET_HPP_
#define TCP_SERVER_SOCKET_HPP_

#include "SocketInterface.h"

#define DEBUG_IT

class TcpServerSocket : public SocketInterface {
private:
    enum {
        eListening,
        eReading,
    };
public:
    TcpServerSocket() {}
    ~TcpServerSocket() {}
    void Init(int port) {
        // defaults
        connected_ = false;
        sock_ = INVALID_SOCKET;
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

        // Number of allowed connections is '1'.
        rv = listen(listen_sock_, 1);
        if (rv == SOCKET_ERROR) {
            throw std::string("Unable to listen socket!");
        }

        state_ = eListening;
    }
    int Read(void * buffer, int size) {
        int rxBytes = -1;
        if (state_ == eReading) {
            if (connected_) {
                rxBytes = recv(sock_, (char *)buffer, size, 0);
                if (rxBytes < 0) {
                    int eCode = WSAGetLastError();
                    if (eCode != 10035 &&
                            eCode != 10054) {
                        std::string eMsg = "Error (" + std::to_string(eCode) + ") during receive!";
                        throw eMsg;
                    }
                    else if (eCode == 10054) {
                        connected_ = false;
                        state_ = eListening;
                    }
                }
                else if (rxBytes == 0) {
                    // disconnection.
                    connected_ = false;
                    state_ = eListening;
                }
            }
            else {
                throw std::string("Unabled to read when not connected!");
            }
        }
        else if (state_ == eListening) {
            sock_ = accept(listen_sock_, NULL, NULL);
            // if (sock_ == INVALID_SOCKET) {
            //      throw std::string("Unable to accept socket!");
            // }
            if (sock_ != INVALID_SOCKET) {
                unsigned long nonBlocking = 1;
                if (ioctlsocket(sock_, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
                    throw std::string("Unable to enable non-blocking i/o!");
                }

#ifdef DEBUG_IT
                puts("Client connected!");
#endif

                state_ = eReading;
                connected_ = true;
            }
        }
        return rxBytes;
    }

private:
    bool connected_;
    int state_;
    SOCKET sock_;
    SOCKET listen_sock_;
};

#endif // TCP_SERVER_SOCKET_HPP_
