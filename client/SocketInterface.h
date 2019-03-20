#ifndef SOCKET_INTERFACE_H_
#define SOCKET_INTERFACE_H_

class SocketInterface {
public:
    virtual ~SocketInterface() {}
    virtual void Init(int port) = 0;
    virtual int Read(void * data, int size) = 0;
};

#endif // SOCKET_INTERFACE_H_
