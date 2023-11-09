#pragma comment(lib, "Ws2_32.lib")


#include "Lab7Client.h"
#include "ClientNetwork.h"
#include <windows.h>
#include <string>

// Global variables for GUI controls
HWND hwndInput;     // Text input field
HWND hwndSendButton; // Send button
HWND hwndMessageLog; // Message display area
HWND hwndUsernameDisplay; // Username display area
HWND hwndUsernameInput;  // Username input field
HWND hwndConnectButton; // Connect to the server button
HWND hwndDisconnectButton; // Disconnect from the server button
ClientNetwork* clientNetwork;


DWORD WINAPI ClientReceiveThread(LPVOID param) {
    while (true) {
        char buffer[1024];
        wchar_t wBuffer[1024];

        size_t convertedChars = 0;
        int bytesRead = recv(clientNetwork->GetClientSocket(), buffer, sizeof(buffer), 0);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            mbstowcs_s(&convertedChars, wBuffer, strlen(buffer) + 1, buffer, _TRUNCATE);
            wBuffer[bytesRead] = '\0';

            // Append the received message to the message log
            SendMessage(hwndMessageLog, EM_SETSEL, -1, -1);
            SendMessage(hwndMessageLog, EM_REPLACESEL, 0, (LPARAM)wBuffer);
            SendMessage(hwndMessageLog, EM_REPLACESEL, 0, (LPARAM)L"\r\n");
        }

        Sleep(1000);
    }

    return 0;
}


void OnSendButtonClick(HWND hWnd) {
    wchar_t wMessage[256];
    char message[256];

    GetWindowTextA(hwndInput, message, sizeof(message));
    GetWindowText(hwndInput, wMessage, sizeof(wMessage));

    // Send the message to the server using clientNetwork
    if (clientNetwork->Send(message)) {
        // Append the sent message to the message log
        SendMessage(hwndMessageLog, EM_SETSEL, -1, -1);
        SendMessage(hwndMessageLog, EM_REPLACESEL, 0, (LPARAM)wMessage);

        // Clear the input field
        SetWindowText(hwndInput, L"");
        SendMessage(hwndMessageLog, EM_REPLACESEL, 0, (LPARAM)L"\r\n");
    }
    else {
        MessageBox(hWnd, L"Failed to send a message. Check your connection to the host.", L"Error", NULL);
    };
}

int OnConnectButtonClick(HWND hWnd) {
    if (!clientNetwork->Initialize("127.0.0.1", 12345)) {
        MessageBox(hWnd, L"Failed to initialize client network.", L"Error", NULL);
        return 1;
    }

    if (!clientNetwork->ConnectToServer()) {
        MessageBox(hWnd, L"Failed to connect to the host. Check if the host is running.", L"Error", NULL);
        return 1;
    }

    wchar_t wUsername[256];
    char username[256];

    GetWindowTextA(hwndUsernameInput, username, sizeof(username));
    GetWindowText(hwndUsernameInput, wUsername, sizeof(username));

    std::wstring usernameDisplay = L"Logged in as ";
    usernameDisplay += !(std::wstring(wUsername).empty()) ? wUsername : std::to_wstring(clientNetwork->GetClientSocket());

    SetWindowText(hwndUsernameDisplay, usernameDisplay.c_str());

    clientNetwork->Send(std::string(username).empty() ? std::to_string(clientNetwork->GetClientSocket()).c_str() : username);

    if (clientNetwork->IsConnected()) {
        EnableWindow(hwndConnectButton, FALSE);
        EnableWindow(hwndDisconnectButton, TRUE);
    }
    else {
        MessageBox(hWnd, L"Unable to connect to server.", L"Error", NULL);
    }

    return 0;
}

void OnDisconnectButtonClick(HWND hWnd) {
    clientNetwork->DisconnectFromServer();

    if (!clientNetwork->IsConnected()) {
        EnableWindow(hwndConnectButton, TRUE);
        EnableWindow(hwndDisconnectButton, FALSE);
    }
    else {
        MessageBox(hWnd, L"Unable to disconnect from server.", L"Error", NULL);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND:
        {
            if (lParam == (LPARAM)hwndSendButton && HIWORD(wParam) == BN_CLICKED)
                OnSendButtonClick(hWnd);

            if (lParam == (LPARAM)hwndDisconnectButton && HIWORD(wParam) == BN_CLICKED)
                OnDisconnectButtonClick(hWnd);

            if (lParam == (LPARAM)hwndConnectButton && HIWORD(wParam) == BN_CLICKED) {
                int result = OnConnectButtonClick(hWnd);
                if (result == 1)
                    return 1;

                CreateThread(NULL, 0, ClientReceiveThread, hWnd, 0, NULL);
            }
        }

            break;

        case WM_DESTROY:
            clientNetwork->DisconnectFromServer();

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    clientNetwork = new ClientNetwork();

    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = L"Lab7ClientWindowClass";
    RegisterClassEx(&wcex);

    HWND hWnd = CreateWindow(
        L"Lab7ClientWindowClass",
        L"Lab7Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        510, 400, NULL, NULL,
        hInstance, NULL
    );

    hwndUsernameDisplay = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE, 10, 10, 290, 20, hWnd, NULL, NULL, NULL);
    hwndUsernameInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 320, 40, 160, 30, hWnd, NULL, NULL, NULL);
    hwndConnectButton = CreateWindow(L"BUTTON", L"Connect", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 320, 80, 160, 30, hWnd, NULL, NULL, NULL);
    hwndDisconnectButton = CreateWindow(L"BUTTON", L"Disconnect", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 320, 120, 160, 30, hWnd, NULL, NULL, NULL);
    hwndInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 10, 310, 200, 30, hWnd, NULL, NULL, NULL);
    hwndSendButton = CreateWindow(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 220, 310, 80, 30, hWnd, NULL, NULL, NULL);
    hwndMessageLog = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 10, 40, 290, 260, hWnd, NULL, NULL, NULL);

    EnableWindow(hwndDisconnectButton, FALSE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(h;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
