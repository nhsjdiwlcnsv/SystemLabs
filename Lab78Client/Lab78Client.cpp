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
HWND hwndConnectButton; // Connet to server button
ClientNetwork clientNetwork;


DWORD WINAPI ClientReceiveThread(LPVOID param) {
    while (true) {
        char buffer[1024];
        wchar_t wBuffer[1024];

        size_t convertedChars = 0;
        size_t bytesRead = recv(clientNetwork.GetClientSocket(), buffer, sizeof(buffer), 0);

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


void OnSendButtonClick(HWND hWnd, ClientNetwork& clientNetwork) {
    wchar_t wMessage[256];
    char message[256];
    char username[256];

    GetWindowTextA(hwndInput, message, sizeof(message));
    GetWindowText(hwndInput, wMessage, sizeof(wMessage));
    GetWindowTextA(hwndUsernameInput, username, sizeof(username));

    // Create the message with the custom username prefix
    std::string prefixedMessage = username;
    prefixedMessage += ": ";
    prefixedMessage += message;

    // Send the message to the server using clientNetwork
    if (clientNetwork.Send((prefixedMessage.c_str()))) {
        // Append the sent message to the message log
        SendMessage(hwndMessageLog, EM_SETSEL, -1, -1);
        SendMessage(hwndMessageLog, EM_REPLACESEL, 0, (LPARAM)wMessage);
        SendMessage(hwndMessageLog, EM_REPLACESEL, 0, (LPARAM)L"\r\n");

        // Clear the input field
        SetWindowText(hwndInput, L"");
    }
    else {
        MessageBox(hWnd, L"FAILED", L"FAILED", NULL);
    };
}

void OnConnectButtonClick(HWND hWnd) {
    wchar_t wUsername[256];
    char username[256];

    GetWindowTextA(hwndUsernameInput, username, sizeof(username));
    GetWindowText(hwndUsernameInput, wUsername, sizeof(username));

    std::wstring usernameDisplay = L"Logged in as ";
    usernameDisplay += wUsername;

    SetWindowText(hwndUsernameDisplay, usernameDisplay.c_str());
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            // Start the message reception thread when the window is created
            CreateThread(NULL, 0, ClientReceiveThread, hWnd, 0, NULL);
            break;

        case WM_COMMAND:
            if (lParam == (LPARAM)hwndSendButton && HIWORD(wParam) == BN_CLICKED)
                OnSendButtonClick(hWnd, clientNetwork);

            if (lParam == (LPARAM)hwndConnectButton && HIWORD(wParam) == BN_CLICKED)
                OnConnectButtonClick(hWnd);

            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Main application logic and clientNetwork initialization
    if (!clientNetwork.Initialize("127.0.0.1", 12345)) {
        // Handle initialization error
        return 1;
    }

    if (!clientNetwork.ConnectToServer()) {
        // Handle connection error
        return 1;
    }

    // Create the main window
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
        800, 660, NULL, NULL,
        hInstance, NULL
    );

    // Create GUI controls
    //hwndInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 10, 10, 300, 30, hWnd, NULL, hInstance, NULL);
    //hwndSendButton = CreateWindow(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 320, 10, 80, 30, hWnd, NULL, hInstance, NULL);
    //hwndMessageLog = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 10, 50, 390, 240, hWnd, NULL, hInstance, NULL);

    hwndUsernameDisplay = CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE, 10, 10, 390, 30, hWnd, NULL, NULL, NULL);
    hwndUsernameInput = CreateWindow(L"EDIT", L"Your Username", WS_CHILD | WS_VISIBLE | WS_BORDER, 470, 50, 300, 30, hWnd, NULL, NULL, NULL);
    hwndConnectButton = CreateWindow(L"BUTTON", L"Connect to server", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 470, 90, 300, 30, hWnd, NULL, NULL, NULL);
    hwndInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 10, 560, 300, 30, hWnd, NULL, NULL, NULL);
    hwndSendButton = CreateWindow(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 320, 560, 80, 30, hWnd, NULL, NULL, NULL);
    hwndMessageLog = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 10, 50, 390, 500, hWnd, NULL, NULL, NULL);

    // Show the main window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Run the message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
