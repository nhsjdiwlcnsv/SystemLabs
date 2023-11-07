//
// Created by Mikhail Shkarubski on 7.11.23.
//

#pragma comment(lib, "Ws2_32.lib")

#include "ServerNetwork.h"
#include "ServerThread.h"
#include <windows.h>
#include <iostream>
#include <vector>


int main() {
    // Initialize the server network
    ServerNetwork server;
    std::vector<SOCKET> connectedClients;

    if (!server.Initialize(12345))  // Use the desired port number
        return 1;

    // Start listening for incoming connections
    server.StartListening();

    while (true) {
        // Accept incoming client connections and create threads for each client
        SOCKET clientSocket = accept(server.GetServerSocket(), NULL, NULL);

        if (clientSocket != INVALID_SOCKET) {
            ServerThread* thread = new ServerThread(clientSocket);
            thread->connectedClients = &connectedClients;
        }
        else std::cout << "Invalid socket" << std::endl;

        connectedClients.push_back(clientSocket);
    }

    // This is a simple example, and you should consider implementing proper shutdown procedures.

    return 0;
}
