#pragma comment(lib, "Ws2_32.lib")

#include "ServerNetwork.h"
#include "ServerThread.h"
#include <windows.h>
#include <iostream>
#include <vector>

int main() {
    ServerNetwork server;
    std::vector<SOCKET>* connectedClients = new std::vector<SOCKET>;

    if (!server.Initialize(12345))
        return 1;

    server.StartListening();

    while (true) {
        SOCKET clientSocket = accept(server.GetServerSocket(), NULL, NULL);

        if (clientSocket != INVALID_SOCKET) {
            connectedClients->push_back(clientSocket);
            ServerThread* thread = new ServerThread(clientSocket, connectedClients);
        }
        else {
            std::cout << "Invalid socket" << std::endl;
        }
    }

    return 0;
}
