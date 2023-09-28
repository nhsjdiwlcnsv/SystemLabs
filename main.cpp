#ifndef UNICODE
#define UNICODE
#endif
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#include <iostream>
#include <windows.h>
// #include "CSVRow.h"


/*std::istream& operator>>(std::istream& str, CSVRow& data) {
    data.readNextRow(str);
    return str;
}*/


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"First Win32 program!",
        WS_OVERLAPPEDWINDOW,

        // x, y, width, height
        CW_USEDEFAULT,  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        nullptr, // parent
        nullptr, // menu
        hInstance, // instance handle
        nullptr // additional data
    );

    if (hwnd == nullptr)
        return 0;

    std::ifstream file("C:/Users/mishashkarubski/SystemLabs/test.csv");

    std::cout << "HNDFKJSNDFKJNSDFJKNSDKJFNJKSDNFJKSNDFJKNSDJFKND" << std::endl;

    CSVRow row;

    while (file >> row)
        std::cout << "4th Element(" << row[3] << ")\n";

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));

                EndPaint(hwnd, &ps);
            }

            return 0;

        case WM_CLOSE:
            if (MessageBox(hwnd, L"Ahuha", L"Aboba", MB_OKCANCEL) == IDOK)
                DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}