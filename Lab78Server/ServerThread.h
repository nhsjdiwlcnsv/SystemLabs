//
// Created by Mikhail Shkarubski on 7.11.23.
//

#ifndef SYSTEMLABS_SERVERTHREAD_H
#define SYSTEMLABS_SERVERTHREAD_H

#pragma once

#include <WinSock2.h>
#include <vector>


class ServerThread {
public:
    ServerThread(SOCKET clientSocket);
    ~ServerThread();

    bool IsRunning();
    void SetIsRunning(bool isRunning);
    SOCKET GetClientSocket();
    void BroadcastMessage(char* message);

    std::vector<SOCKET>* connectedClients;

private:
    SOCKET clientSocket;
    HANDLE threadHandle;
    bool running;
};

#endif //SYSTEMLABS_SERVERTHREAD_H
