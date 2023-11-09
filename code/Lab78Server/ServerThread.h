#pragma once

#include <WinSock2.h>
#include <vector>
#include <string>

class ServerThread {
public:
    ServerThread(SOCKET clientSocket);
    ServerThread(SOCKET clientSocket, std::vector<SOCKET>* connectedClients);
    ~ServerThread();

    SOCKET GetClientSocket();
    std::vector<SOCKET>* GetConnectedClients();
    void SetConnectedClients(std::vector<SOCKET>* connectedClients);
    bool IsRunning();
    void SetIsRunning(bool isRunning);
    bool IsFirstMessage();
    void SetIsFirstMessage(bool isFirstMessage);
    std::string GetClientUsername();
    void SetClientUsername(std::string newClientUsername);

    void BroadcastMessage(const char* message);

private:
    HANDLE threadHandle;
    SOCKET clientSocket;
    std::vector<SOCKET>* connectedClients;
    bool running;
    bool firstMessage;
    std::string clientUsername;
};

