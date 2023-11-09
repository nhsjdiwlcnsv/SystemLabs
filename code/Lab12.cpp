#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifndef UNICODE
#define UNICODE
#endif
#define DEFAULT_WIDTH 1200
#define DEFAULT_HEIGHT 900

#include <iostream>
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>


HWND gListView;
HWND pathInputHandle;
HWND pathButtonHandle;
HWND xInputHandle;
HWND yInputHandle;
HWND graphButtonHandle;
HWND graphHandle;

struct Point {
    int x;
    int y;
};

std::vector<Point> points;
wchar_t csvFilePath[128];

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::vector<std::wstring> GetCSVColumns(const std::wstring& filePath);
std::vector<std::wstring> ReadCSVFileLine(const std::wstring& line, wchar_t delimiter);
void DisplayCSVInListView(const std::wstring& filePath, HWND listViewHandle);
void LinePlot(HWND hwnd);


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

    pathInputHandle = CreateWindowEx(
        0, L"EDIT", L"Path to .csv file",
        WS_VISIBLE | WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
        10, 10, 160, 40, hwnd, nullptr, hInstance, nullptr
    );

    pathButtonHandle = CreateWindowEx(
        0, L"BUTTON", L"Submit",
        WS_VISIBLE | WS_CHILD,
        200, 10, 160, 40, hwnd, nullptr, hInstance, nullptr
    );

    xInputHandle = CreateWindowEx(
        0, L"EDIT", L"X",
        WS_VISIBLE | WS_CHILD | WS_VISIBLE | WS_BORDER,
        400, 520, 80, 40, hwnd, nullptr, hInstance, nullptr
    );

    yInputHandle = CreateWindowEx(
        0, L"EDIT", L"Y",
        WS_VISIBLE | WS_CHILD | WS_VISIBLE | WS_BORDER,
        500, 520, 80, 40, hwnd, nullptr, hInstance, nullptr
    ); 

    graphButtonHandle = CreateWindowEx(
        0, L"BUTTON", L"Add to graph",
        WS_VISIBLE | WS_CHILD,
        600, 520, 160, 40, hwnd, nullptr, hInstance, nullptr
    );

    graphHandle = CreateWindowEx(
        0, L"STATIC", L"Lineplot",
        WS_VISIBLE | WS_CHILD | SS_BLACKRECT,
        400, 100, 600, 400, hwnd, nullptr, hInstance, nullptr
    );

    HFONT font = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));

    SendMessage(pathInputHandle, WM_SETFONT, (WPARAM) font, TRUE);
    SendMessage(pathButtonHandle, WM_SETFONT, (WPARAM) font, TRUE);
    SendMessage(xInputHandle, WM_SETFONT, (WPARAM) font, TRUE);
    SendMessage(yInputHandle, WM_SETFONT, (WPARAM) font, TRUE);
    SendMessage(graphButtonHandle, WM_SETFONT, (WPARAM) font, TRUE);

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
        case WM_COMMAND:
        {
            if (HIWORD(wParam) == BN_CLICKED) {
                if ((HWND)lParam == graphButtonHandle) {
                    wchar_t xText[128];
                    wchar_t yText[128];

                    GetWindowText(xInputHandle, xText, 128);
                    GetWindowText(yInputHandle, yText, 128);

                    int xValue = _wtoi(xText);
                    int yValue = _wtoi(yText);

                    if ((std::strcmp((const char*) xText, "0") && xValue == 0) || (std::strcmp((const char*)yText, "0") && yValue == 0)) {
                        MessageBox(nullptr, L"Ng wha?", L"ANOGA", MB_OK | MB_ICONERROR);
                        break;
                    }

                    points.push_back({ xValue, yValue });

                    // Update the graph
                    LinePlot(graphHandle);
                }

                else if ((HWND)lParam == pathButtonHandle) {
                    GetWindowText(pathInputHandle, csvFilePath, 128);
                    
                    gListView = CreateWindow(
                        WC_LISTVIEW, L"",
                        WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
                        10, 100, 400, 300, hwnd, nullptr, nullptr, nullptr
                    );

                    std::vector<std::wstring> headers = GetCSVColumns(csvFilePath);

                    if (!headers.size())
                        return 0;

                    LV_COLUMN lvColumn;
                    lvColumn.mask = LVCF_TEXT;

                    for (int i = 0; i < headers.size(); ++i) {
                        lvColumn.pszText = headers[i].data();
                        ListView_InsertColumn(gListView, i, &lvColumn);
                    }

                    DisplayCSVInListView(csvFilePath, gListView);
                }
            }
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
        MessageBox(nullptr, L"Ng wha?", L"ANOGA", MB_OK | MB_ICONERROR);
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
        MessageBox(nullptr, L"Ng wha?", L"ANOGA", MB_OK | MB_ICONERROR);
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
        ListView_SetColumnWidth(listViewHandle, i, LVSCW_AUTOSIZE_USEHEADER | LVTVIF_FIXEDWIDTH);

    file.close();
}

void LinePlot(HWND hwnd) {
    HWND graphHandle = GetDlgItem(hwnd, 4);
    RECT rect;

    GetClientRect(hwnd, &rect);

    HDC hdc = GetDC(hwnd);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
    HBRUSH brush = CreateSolidBrush(RGB(245, 245, 245));

    SelectObject(hdc, pen);
    SelectObject(hdc, brush);
    FillRect(hdc, &rect, brush);

    Point prevPoint;
    bool isFirstPoint = true;

    int ySpan = (rect.bottom + rect.top) / 2;
    int xSpan = (rect.left + rect.right) / 2;

    for (int i = 0; i < points.size(); ++i) {
        Point point = points[i];

        SelectObject(hdc, brush);

        if ((rect.bottom + rect.top) / 2 - point.y < rect.top || (rect.left + rect.right) / 2 + point.x > rect.right) {
            points.erase(points.begin() + i);
            continue;
        }
        

        Ellipse(
            hdc,
            xSpan + point.x - 4, ySpan - point.y - 4,
            xSpan + point.x + 4, ySpan - point.y + 4
        );

        if (isFirstPoint) {
            prevPoint.x = point.x;
            prevPoint.y = point.y;
            isFirstPoint = false;
        } else {
            MoveToEx(hdc, xSpan + prevPoint.x, ySpan - prevPoint.y, nullptr);
            LineTo(hdc, xSpan + point.x, ySpan - point.y);
            prevPoint.x = point.x;
            prevPoint.y = point.y;
        }
    }

    ReleaseDC(graphHandle, hdc);
    DeleteObject(brush);
}