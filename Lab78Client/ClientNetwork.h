//
// Created by Mikhail Shkarubski on 7.11.23.
//

#ifndef SYSTEMLABS_CLIENTNETWORK_H
#define SYSTEMLABS_CLIENTNETWORK_H

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
    bool Receive(char* buffer, int bufferSize);
    bool IsConnected() const;

private:
    SOCKET clientSocket;
    sockaddr_in serverAddress;
    bool connected;
};

#endif //SYSTEMLABS_CLIENTNETWORK_H
