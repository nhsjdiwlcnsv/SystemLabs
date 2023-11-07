//
// Created by Mikhail Shkarubski on 7.11.23.
//
#pragma comment(lib, "Ws2_32.lib")

#include "ServerThread.h"
#include <iostream>

DWORD WINAPI ClientThread(LPVOID param) {
    ServerThread* serverThread = static_cast<ServerThread*>(param);

    while (serverThread->IsRunning()) {
        std::string clientName = "Client " + std::to_string(serverThread->GetClientSocket());

    char buffer[1024];
    int bytesRead = recv(serverThread->GetClientSocket(), buffer, 1024, 0);

    if (bytesRead <= 0)
        break;

    buffer[bytesRead] = '\0';

    std::string prefixedMsg = clientName + ": " + buffer;

    std::cout << "Message received by server" << std::endl;
    std::cout << buffer << std::endl;

    serverThread->BroadcastMessage(prefixedMsg.c_str());
}

return 0;
}

ServerThread::ServerThread(SOCKET clientSocket) : clientSocket(clientSocket), running(true) {
    threadHandle = CreateThread(NULL, 0, ClientThread, this, 0, NULL);
}

ServerThread::~ServerThread() {
    CloseHandle(threadHandle);
}

bool ServerThread::IsRunning() {
    return running;
}

void ServerThread::SetIsRunning(bool isRunning) {
    running = isRunning;
}

SOCKET ServerThread::GetClientSocket() {
    return clientSocket;
}

void ServerThread::BroadcastMessage(const char* message)
{
    for (SOCKET client : *connectedClients)
        if (client != clientSocket) {
            send(client, message, strlen(message), 0);
            std::cout << "Broadcasted message to client " << client << std::endl;
        }
    std::cout << std::endl;
}

