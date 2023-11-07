//
// Created by Mikhail Shkarubski on 7.11.23.
//

#pragma comment(lib, "Ws2_32.lib")

#include "ClientNetwork.h"

ClientNetwork::ClientNetwork() : clientSocket(INVALID_SOCKET), connected(false) {}

ClientNetwork::~ClientNetwork() {
    DisconnectFromServer();
}

bool ClientNetwork::Initialize(const char* serverIP, int serverPort) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return false;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP, &(serverAddress.sin_addr));

    return true;
}

bool ClientNetwork::ConnectToServer() {
    if (connected)
        return true;

    int result = connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
    if (result == SOCKET_ERROR)
        return false;

    connected = true;

    return true;
}

void ClientNetwork::DisconnectFromServer() {
    if (connected) {
        closesocket(clientSocket);
        WSACleanup();
        connected = false;
    }
}

bool ClientNetwork::Send(const char* message) {
    if (!connected)
        return false;

    int result = send(clientSocket, message, strlen(message), 0);
    if (result == SOCKET_ERROR)
        return false;

    return true;
}

bool ClientNetwork::Receive(char* buffer, int bufferSize) {
    if (!connected)
        return false;

    int bytesRead = recv(clientSocket, buffer, bufferSize - 1, 0);
    if (bytesRead <= 0)
        return false;

    buffer[bytesRead] = '\0';
    return true;
}

bool ClientNetwork::IsConnected() const {
    return connected;
}