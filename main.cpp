#ifndef UNICODE
#define UNICODE
#endif
#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

#include <iostream>
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>


HWND gListView;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::vector<std::wstring> GetCSVColumns(const std::wstring& filePath);
std::vector<std::wstring> ReadCSVFileLine(const std::wstring& line, wchar_t delimiter);
void DisplayCSVInListView(const std::wstring& filePath, HWND listViewHandle);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Data visualizer";

    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Data vizualizer – Main window",
        WS_OVERLAPPEDWINDOW,

        // x, y, width, height
        CW_USEDEFAULT,  CW_USEDEFAULT, DEFAULT_WIDTH, DEFAULT_HEIGHT,

        nullptr, // parent
        nullptr, // menu
        hInstance, // instance handle
        nullptr // additional data
    );

    if (hwnd == nullptr)
        return 1;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { };

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
        {
            gListView = CreateWindow(
                WC_LISTVIEW, L"",
                WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
                10, 10, 600, 300, hwnd, nullptr, nullptr, nullptr
            );

            std::vector<std::wstring> headers = GetCSVColumns(L"C:/Users/mishashkarubski/SystemLabs/test.csv");

            LV_COLUMN lvColumn;
            lvColumn.mask = LVCF_TEXT;

            for (int i = 0; i < headers.size(); ++i) {
                lvColumn.pszText = headers[i].data();
                ListView_InsertColumn(gListView, i, &lvColumn);
            }

            DisplayCSVInListView(L"C:/Users/mishashkarubski/SystemLabs/test.csv", gListView);
        };
        
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

std::vector<std::wstring> GetCSVColumns(const std::wstring& filePath) {
    std::vector<std::wstring> headers;
    std::wstring line;
    std::wifstream file(filePath);

    if (!file) {
        MessageBox(nullptr, L"Failed to open the file.", L"Error", MB_OK | MB_ICONERROR);
        return headers;
    }

    std::getline(file, line);
    headers = ReadCSVFileLine(line, L',');
    //line.replace(line.find(line), line.length(), L"");
    //file << line << std::endl;
    //file.close();

    file.close();

    return headers;
}


std::vector<std::wstring> ReadCSVFileLine(const std::wstring& line, wchar_t delimiter) {
    std::vector<std::wstring> tokens;
    std::wstringstream ss(line);
    std::wstring token;

    while (std::getline(ss, token, delimiter))
        tokens.push_back(token);

    return tokens;
}


void DisplayCSVInListView(const std::wstring& filePath, HWND listViewHandle) {
    ListView_DeleteAllItems(listViewHandle);

    std::wifstream file(filePath, std::ios::in | std::ios::binary);

    if (!file) {
        MessageBox(nullptr, L"Failes to open the file.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::wstring line;
    while (std::getline(file, line)) {
        std::vector<std::wstring> row = ReadCSVFileLine(line, L',');
        
        LVITEM lvItem;
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = 0;
        lvItem.iSubItem = 0;
        lvItem.pszText = (LPWSTR) row[0].c_str();

        int itemIndex = ListView_InsertItem(listViewHandle, &lvItem);

        for (size_t i = 1; i < row.size(); i++) {
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = itemIndex;
            lvItem.iSubItem = i;
            lvItem.pszText = (LPWSTR) row[i].c_str();

            ListView_SetItem(gListView, &lvItem);
        }
    }
        
    for (int i = 0; i < ListView_GetItemCount(listViewHandle); ++i)
        ListView_SetColumnWidth(listViewHandle, i, LVSCW_AUTOSIZE_USEHEADER);

    file.close();
}