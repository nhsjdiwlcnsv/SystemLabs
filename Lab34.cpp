#pragma once

#include <windows.h>
#include <CommCtrl.h>
#include <psapi.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#ifndef UNICODE
#define UNICODE
#endif

#define STOP_BTN_ID 999


HWND hwndListView;
HWND hwndTotalsView;
HWND hwndStopButton;
HANDLE hUpdateThread, hRefreshThread;
DWORD processes[1024], cbNeeded, cProcesses;
int totalsIndex;


bool getMemoryInfo(HANDLE hProcess, wchar_t(&memoryUsageDisplay)[256]) {
    PROCESS_MEMORY_COUNTERS_EX pmc;

    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
    {
        swprintf(memoryUsageDisplay, 256, L"%.2f", (double) ((double) pmc.WorkingSetSize / (1024 * 1024)));

        return true;
    }

    return false;
}


bool getCPUUsage(HANDLE hProcess, wchar_t (&cpuUsageDisplay) [256]) {
    FILETIME ftCreation, ftExit, ftKernel, ftUser;

    if (GetProcessTimes(hProcess, (FILETIME*)&ftCreation, (FILETIME*)&ftExit, (FILETIME*)&ftKernel, (FILETIME*)&ftUser)) {
        ULONGLONG processTime = ((ULONGLONG)(ftKernel.dwHighDateTime << 32) | ftKernel.dwLowDateTime);
        ULONGLONG systemTime = ((ULONGLONG)(ftUser.dwHighDateTime << 32) | ftUser.dwLowDateTime);

        //wchar_t cpuUsageDisplay[256];
        double cpuUsage = processTime * 100.0 / systemTime;

        swprintf(cpuUsageDisplay, 256, L"%.2f", cpuUsage);

        return true;
    }

    return false;
}


wchar_t* getActiveTime(HANDLE hProcess) {
    FILETIME ftCreation, ftExit, ftKernel, ftUser;

    if (GetProcessTimes(hProcess, (FILETIME*)&ftCreation, (FILETIME*)&ftExit, (FILETIME*)&ftKernel, (FILETIME*)&ftUser)) {
        FILETIME ftCurrent;

        GetSystemTimeAsFileTime(&ftCurrent);

        ULONGLONG activeTime = *((ULONGLONG*)&ftCurrent) - *((ULONGLONG*)&ftCreation);
        ULONGLONG activeSeconds = activeTime / 10000000;

        int hours = activeSeconds / 3600;
        int minutes = (activeSeconds % 3600) / 60;
        int seconds = activeSeconds % 60;

        wchar_t* activeTimeDisplay = _wcsdup((std::to_wstring(hours) + L":" + std::to_wstring(minutes) + L":" + std::to_wstring(seconds)).c_str());

        return activeTimeDisplay;
    }

    return (wchar_t*) L"Err";
}


wchar_t* getThreadCount(HANDLE hProcess) {
    DWORD threadCount;

    GetProcessHandleCount(hProcess, &threadCount);

    return _wcsdup(std::to_wstring(threadCount).c_str());
}


DWORD WINAPI RefreshThreadProc(LPVOID lpParam) {
    while (true) {
        HWND hwnd = (HWND)lpParam;

        EnumProcesses(processes, sizeof(processes), &cbNeeded);

        cProcesses = cbNeeded / sizeof(DWORD);

        Sleep(1000);
    }
}


DWORD WINAPI UpdateThreadProc(LPVOID lpParam) {
    int itemCount = ListView_GetItemCount(hwndListView);
    HWND hwnd = (HWND)lpParam;

    while (true) {

        int scrollPos = GetScrollPos(hwndListView, SB_VERT);

        if (itemCount != cProcesses) {
            ListView_DeleteAllItems(hwndListView);

            int trueCounter = 0;
            for (DWORD i = 0; i < cProcesses; i++)
            {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);

                if (hProcess == NULL)
                    continue;

                TCHAR szProcessName[MAX_PATH] = L"";
                HMODULE hModule;
                DWORD cbNeeded;

                if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded))
                    GetModuleBaseName(hProcess, hModule, szProcessName, sizeof(szProcessName));

                LVITEM lvItem = { 0 };
                lvItem.mask = LVIF_TEXT;
                lvItem.iItem = trueCounter++;

                int itemIndex = ListView_InsertItem(hwndListView, &lvItem);

                CloseHandle(hProcess);
            }

            itemCount = cProcesses;
        }

        int trueCounter = 0;
        for (DWORD i = 0; i < cProcesses; i++)
        {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);

            if (hProcess == NULL)
                continue;

            TCHAR szProcessName[MAX_PATH] = L"";
            HMODULE hModule;
            DWORD cbNeeded;

            if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded))
                GetModuleBaseName(hProcess, hModule, szProcessName, sizeof(szProcessName));

            // MEMORY USAGE DISPLAY
            wchar_t memoryUsageDisplay[256];
            getMemoryInfo(hProcess, memoryUsageDisplay);

            // CPU USAGE DISPLAY
            wchar_t cpuUsageDisplay[256];
            getCPUUsage(hProcess, cpuUsageDisplay);

            // ACTIVE TIME DISPLAY
            wchar_t* activeTimeDisplay = getActiveTime(hProcess);

            // THREADS PER PROCESS DISPLAY
            wchar_t* threadCountDisplay = getThreadCount(hProcess);

            // PID
            wchar_t* pidDisplay = _wcsdup(std::to_wstring(GetProcessId(hProcess)).c_str());

            ListView_SetItemText(hwndListView, trueCounter, 0, szProcessName);
            ListView_SetItemText(hwndListView, trueCounter, 1, memoryUsageDisplay);
            ListView_SetItemText(hwndListView, trueCounter, 2, cpuUsageDisplay);
            ListView_SetItemText(hwndListView, trueCounter, 3, activeTimeDisplay);
            ListView_SetItemText(hwndListView, trueCounter, 4, threadCountDisplay);
            ListView_SetItemText(hwndListView, trueCounter, 5, pidDisplay);

            trueCounter++;

            CloseHandle(hProcess);

        }

        SetScrollPos(hwndListView, SB_VERT, scrollPos, TRUE);

        // TOTALS

        DWORD totalProcesses = ListView_GetItemCount(hwndListView);
        wchar_t totalProcessesDisplay[256];

        _itow_s(totalProcesses, totalProcessesDisplay, 10);

        DWORD totalThreads = 0;
        DWORD totalMemory = 0;

        for (DWORD i = 0; i < totalProcesses; i++) {
            wchar_t threadCountDisplay[256];
            wchar_t processMemoryUsageDisplay[256];

            ListView_GetItemText(hwndListView, i, 4, threadCountDisplay, sizeof(threadCountDisplay));
            ListView_GetItemText(hwndListView, i, 1, processMemoryUsageDisplay, sizeof(processMemoryUsageDisplay));

            totalThreads += _wtoi(threadCountDisplay);
            totalMemory += _wtoi(processMemoryUsageDisplay);
        }

        wchar_t totalThreadsDisplay[256];
        _itow_s(totalThreads, totalThreadsDisplay, 10);

        wchar_t totalMemoryUsageDisplay[256];
        _itow_s(totalMemory, totalMemoryUsageDisplay, 10);

        ListView_SetItemText(hwndTotalsView, totalsIndex, 0, totalProcessesDisplay);
        ListView_SetItemText(hwndTotalsView, totalsIndex, 1, totalThreadsDisplay);
        ListView_SetItemText(hwndTotalsView, totalsIndex, 2, totalMemoryUsageDisplay);

        Sleep(250);
    }

    return 0;
}


// Window procedure function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // Initialize the task manager interface
        // You can create UI elements and set up data structures here

        // Create the list view control
        hwndListView = CreateWindowEx(
            0,                              // Optional window styles
            WC_LISTVIEW,                    // Window class name
            L"",                             // Window title
            WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT,  // Window style
            0, 0, 720, 400,                 // Size and position
            hwnd,                           // Parent window handle
            NULL,                           // Menu handle
            GetModuleHandle(NULL),          // Instance handle
            NULL                            // Additional application data
        );
        ListView_SetExtendedListViewStyle(hwndListView, LVS_EX_AUTOSIZECOLUMNS);

        hwndTotalsView = CreateWindowEx(
            0,                              // Optional window styles
            WC_LISTVIEW,                    // Window class name
            L"",                             // Window title
            WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT,  // Window style
            750, 0, 300, 40,                 // Size and position
            hwnd,                           // Parent window handle
            NULL,                           // Menu handle
            GetModuleHandle(NULL),          // Instance handle
            NULL                            // Additional application data
        );
        ListView_SetExtendedListViewStyle(hwndTotalsView, LVS_EX_AUTOSIZECOLUMNS);

        hwndStopButton = CreateWindow(
            L"BUTTON",
            L"Stop process",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            750, 50, 300, 30,
            hwnd,
            (HMENU) STOP_BTN_ID,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL
        );

        if (hwndListView == NULL or hwndTotalsView == NULL or hwndStopButton == NULL)
            return -1;

        // Set up the columns in the list view control
        LVCOLUMN lvColumn = { };
        lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
        lvColumn.cx = 200;
        lvColumn.pszText = (LPWSTR) L"Process Name";
        ListView_InsertColumn(hwndListView, 0, &lvColumn);

        lvColumn.cx = 100;
        lvColumn.pszText = (LPWSTR) L"Memory (MB)";
        ListView_InsertColumn(hwndListView, 1, &lvColumn);

        lvColumn.cx = 100;
        lvColumn.pszText = (LPWSTR) L"CPU (%)";
        ListView_InsertColumn(hwndListView, 2, &lvColumn);

        lvColumn.cx = 100;
        lvColumn.pszText = (LPWSTR)L"CPU Time (h:m:s)";
        ListView_InsertColumn(hwndListView, 3, &lvColumn);

        lvColumn.cx = 100;
        lvColumn.pszText = (LPWSTR)L"Threads";
        ListView_InsertColumn(hwndListView, 4, &lvColumn);

        lvColumn.cx = 100;
        lvColumn.pszText = (LPWSTR)L"PID";
        ListView_InsertColumn(hwndListView, 5, &lvColumn);

        DWORD processes[1024], cbNeeded, cProcesses;
        EnumProcesses(processes, sizeof(processes), &cbNeeded);
        cProcesses = cbNeeded / sizeof(DWORD);

        LVITEM lvItem = { 0 };
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = 0;

        for (DWORD i = 0; i < cProcesses; i++)
            ListView_InsertItem(hwndListView, &lvItem);


        LVCOLUMN tColumn = {};
        tColumn.mask = LVCF_TEXT | LVCF_WIDTH;
        tColumn.cx = 100;
        tColumn.pszText = (LPWSTR)L"Processes";
        ListView_InsertColumn(hwndTotalsView, 0, &tColumn);

        tColumn.cx = 100;
        tColumn.pszText = (LPWSTR)L"Threads";
        ListView_InsertColumn(hwndTotalsView, 1, &tColumn);

        tColumn.cx = 100;
        tColumn.pszText = (LPWSTR)L"Memory";
        ListView_InsertColumn(hwndTotalsView, 2, &tColumn);

        LVITEM tItem = {};
        tItem.mask = LVIF_TEXT;
        tItem.iItem = 0;

        totalsIndex = ListView_InsertItem(hwndTotalsView, &tItem);

        hUpdateThread = CreateThread(NULL, 0, UpdateThreadProc, hwnd, 0, NULL);
        hRefreshThread = CreateThread(NULL, 0, RefreshThreadProc, hwnd, 0, NULL);

        break;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == STOP_BTN_ID && HIWORD(wParam) == BN_CLICKED)
        {
            for (int i = 0; i < ListView_GetItemCount(hwndListView); ++i) {
                UINT state = ListView_GetItemState(hwndListView, i, LVIS_FOCUSED);

                bool isChecked = state == LVIS_FOCUSED;

                if (isChecked) {
                    LVITEM lvItem = { 0 };
                    lvItem.mask = LVIF_STATE | LVIF_TEXT;
                    lvItem.iItem = i;
                    lvItem.iSubItem = 5;
                    lvItem.cchTextMax = 256;

                    wchar_t buffer[256];
                    lvItem.pszText = buffer;
                    lvItem.stateMask = LVIS_STATEIMAGEMASK;

                    ListView_GetItem(hwndListView, &lvItem);

                    DWORD pid = _wtoi(buffer);

                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);

                    if (hProcess != NULL) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                    break;
                }
            }
        }
        break;
    }


    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        break;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register the window class
    const wchar_t CLASS_NAME[] = L"TaskManagerWindowClass";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,                               // Optional window styles
        CLASS_NAME,                      // Window class name
        L"Activity Monitor Analogue",         // Window title
        WS_OVERLAPPEDWINDOW,             // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,                            // Parent window
        NULL,                            // Menu
        hInstance,                       // Instance handle
        NULL                             // Additional application data
    );

    if (hwnd == NULL)
        return 0;

    // Display the window
    ShowWindow(hwnd, nCmdShow);

    // Run the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}