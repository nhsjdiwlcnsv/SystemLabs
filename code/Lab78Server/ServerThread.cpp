#pragma comment(lib, "Ws2_32.lib")

#include "ServerThread.h"
#include <iostream>

DWORD WINAPI ClientThread(LPVOID param) {
ServerThread* serverThread = static_cast<ServerThread*>(param);

while (serverThread->IsRunning()) {
char buffer[1024];
int bytesRead = recv(serverThread->GetClientSocket(), buffer, 1024, 0);

if (bytesRead <= 0) {
serverThread->SetIsRunning(false);
closesocket(serverThread->GetClientSocket());

auto it = std::find(serverThread->GetConnectedClients()->begin(), serverThread->GetConnectedClients()->end(), serverThread->GetClientSocket());
if (it != serverThread->GetConnectedClients()->end())
serverThread->GetConnectedClients()->erase(it);

break;
}

buffer[bytesRead] = '\0';
std::cout << "Message received by server" << std::endl;
std::cout << buffer << std::endl;

if (serverThread->IsFirstMessage()) {
serverThread->SetClientUsername(buffer);
serverThread->SetIsFirstMessage(false);

continue;
}

std::string prefixedMsg = serverThread->GetClientUsername() + ": " + buffer;

if (serverThread->GetConnectedClients()->size() > 0)
serverThread->BroadcastMessage(prefixedMsg.c_str());
}

return 0;
}

ServerThread::ServerThread(SOCKET clientSocket) : clientSocket(clientSocket), running(true), firstMessage(true) {
    threadHandle = CreateThread(NULL, 0, ClientThread, this, 0, NULL);
}

ServerThread::ServerThread(SOCKET clientSocket, std::vector<SOCKET>* connectedClients) : clientSocket(clientSocket), running(true), firstMessage(true), connectedClients(connectedClients) {
    threadHandle = CreateThread(NULL, 0, ClientThread, this, 0, NULL);
}

ServerThread::~ServerThread() {
    CloseHandle(threadHandle);
}

SOCKET ServerThread::GetClientSocket() {
    return clientSocket;
}

std::vector<SOCKET>* ServerThread::GetConnectedClients()
{
    return connectedClients;
}

void ServerThread::SetConnectedClients(std::vector<SOCKET>* connectedClients)
{
    connectedClients = connectedClients;
}

bool ServerThread::IsRunning() {
    return running;
}

void ServerThread::SetIsRunning(bool isRunning) {
    running = isRunning;
}

bool ServerThread::IsFirstMessage() {
    return firstMessage;
}

void ServerThread::SetIsFirstMessage(bool isFirstMessage) {
    firstMessage = isFirstMessage;
}

std::string ServerThread::GetClientUsername()
{
    return clientUsername;
}

void ServerThread::SetClientUsername(std::string newClientUsername)
{
    clientUsername = newClientUsername;
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
