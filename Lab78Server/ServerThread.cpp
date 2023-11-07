//
// Created by Mikhail Shkarubski on 7.11.23.
//

#pragma comment(lib, "Ws2_32.lib")

#include "ServerThread.h"
#include <iostream>


DWORD WINAPI ClientThread(LPVOID param) {
    ServerThread* serverThread = static_cast<ServerThread*>(param);

    while (serverThread->IsRunning()) {
        char buffer[1024];
        int bytesRead = recv(serverThread->GetClientSocket(), buffer, 1024, 0);

        if (bytesRead <= 0)
            break;

        buffer[bytesRead] = '\0';

        const char* reply = "Message received by server";
        send(serverThread->GetClientSocket(), reply, strlen(reply), 0);

        std::cout << reply << std::endl;
        std::cout << buffer << std::endl;
        std::cout << std::endl;

        serverThread->BroadcastMessage(buffer);
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

void ServerThread::BroadcastMessage(char* message)
{
    for (SOCKET client : *connectedClients)
        if (client != clientSocket) {
            send(client, message, strlen(message), 0);
            std::cout << "There's more than one client!" << std::endl;
        }
}
