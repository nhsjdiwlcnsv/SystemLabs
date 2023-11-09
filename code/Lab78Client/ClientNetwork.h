#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>


class ClientNetwork {
public:
    ClientNetwork();
    ~ClientNetwork();

    bool Initialize(const char* serverIP, int serverPort);
    bool ConnectToServer();
    void DisconnectFromServer();
    bool Send(const char* message);
    bool Receive(char* buffer);
    bool IsConnected() const;

    SOCKET GetClientSocket();

private:
    SOCKET clientSocket;
    sockaddr_in serverAddress;
    bool connected;
};

