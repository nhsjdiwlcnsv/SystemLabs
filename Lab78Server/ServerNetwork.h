//
// Created by Mikhail Shkarubski on 7.11.23.
//

#ifndef SYSTEMLABS_SERVERNETWORK_H
#define SYSTEMLABS_SERVERNETWORK_H

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

#endif //SYSTEMLABS_SERVERNETWORK_H
