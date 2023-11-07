//
// Created by Mikhail Shkarubski on 7.11.23.
//

#pragma comment(lib, "Ws2_32.lib")

#include "ClientNetwork.h"
#include <windows.h>


// Global variables for GUI controls
HWND hwndInput;     // Text input field
HWND hwndSendButton; // Send button
HWND hwndMessageLog; // Message display area
ClientNetwork clientNetwork;

void OnSendButtonClick(HWND hWnd, ClientNetwork& clientNetwork) {
    wchar_t wMessage[256];
    char message[256];

    GetWindowTextA(hwndInput, message, sizeof(message));
    GetWindowText(hwndInput, wMessage, sizeof(message));

    // Send the message to the server using clientNetwork
    if (clientNetwork.Send((message))) {
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (lParam == (LPARAM)hwndSendButton && HIWORD(wParam) == BN_CLICKED)
            OnSendButtonClick(hWnd, clientNetwork);

        break;

        // Handle other window messages here

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

    HWND hWnd = CreateWindow(L"Lab7ClientWindowClass", L"Lab78Client", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 450, 350, NULL, NULL, hInstance, NULL);

    // Create GUI controls
    hwndInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 10, 10, 300, 30, hWnd, NULL, hInstance, NULL);
    hwndSendButton = CreateWindow(L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 320, 10, 80, 30, hWnd, NULL, hInstance, NULL);
    hwndMessageLog = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY, 10, 50, 390, 240, hWnd, NULL, hInstance, NULL);

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
