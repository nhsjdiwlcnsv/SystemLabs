#pragma comment(lib, "Ws2_32.lib")

#include "ServerNetwork.h"

ServerNetwork::~ServerNetwork() {
    closesocket(serverSocket);
    WSACleanup();
}

bool ServerNetwork::Initialize(int port) {
    WSAData wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    SOCKADDR_IN serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    return true;
}

void ServerNetwork::StartListening() {
    listen(serverSocket, SOMAXCONN);
}

SOCKET ServerNetwork::GetServerSocket() {
    return serverSocket;
}