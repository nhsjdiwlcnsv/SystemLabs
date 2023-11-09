#pragma once

#include <WinSock2.h>

class ServerNetwork {
public:
    ~ServerNetwork();

    bool Initialize(int port);
    void StartListening();
    SOCKET GetServerSocket();

private:
    SOCKET serverSocket;
};

