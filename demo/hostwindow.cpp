/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Windows.h>
#include <cstdio>
#include <cmath>
#include "am_apis.hpp"

static LPCWSTR g_windowClass = L"AcrylicManagerDemoApplicationWindowClass";
static LPCWSTR g_windowTitle = L"AcrylicManager Demo Application";

static HINSTANCE g_instance = nullptr;
static HWND g_window = nullptr;

static inline LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT: {
        PAINTSTRUCT ps = {};
        const HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(g_window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

EXTERN_C int APIENTRY
wWinMain(
    _In_ HINSTANCE     hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR        lpCmdLine,
    _In_ int           nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_instance = hInstance;

    if (!InitializeAcrylicManagerLibrary()) {
        MessageBoxW(nullptr, L"Failed to initialize AcrylicManager library.", g_windowTitle, MB_ICONERROR | MB_OK);
        return -1;
    }

    LPWSTR ver = nullptr;
    if (SUCCEEDED(am_GetVersion(&ver))) {
        const auto str = new wchar_t[MAX_PATH];
        SecureZeroMemory(str, sizeof(str));
        swprintf(str, L"AcrylicManager version: %s", ver);
        OutputDebugStringW(str);
        am_FreeStringW(ver);
        delete [] str;
    }

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(101));
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = g_windowClass;

    if (RegisterClassExW(&wcex) == 0) {
        MessageBoxW(nullptr, L"Failed to register demo application window class.", g_windowTitle, MB_ICONERROR | MB_OK);
        return -1;
    }

    g_window = CreateWindowExW(0L,
                               g_windowClass, g_windowTitle,
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               nullptr, nullptr, hInstance, nullptr);
    if (!g_window) {
        MessageBoxW(nullptr, L"Failed to create demo application window.", g_windowTitle, MB_ICONERROR | MB_OK);
        return -1;
    }

    ShowWindow(g_window, nCmdShow);
    if (UpdateWindow(g_window) == FALSE) {
        MessageBoxW(nullptr, L"Failed to update demo application window.", g_windowTitle, MB_ICONERROR | MB_OK);
        return -1;
    }

    RECT rect = {0, 0, 0, 0};
    if (GetWindowRect(g_window, &rect) == FALSE) {
        MessageBoxW(nullptr, L"Failed to retrieve demo application window rect.", g_windowTitle, MB_ICONERROR | MB_OK);
        return -1;
    }

    if (SUCCEEDED(am_CreateWindow(rect.left, rect.top, std::abs(rect.right - rect.left), std::abs(rect.bottom - rect.top)))) {
        am_SetWindowTranslucentBackgroundEnabled(g_window, true);
        am_SetWindowState(g_am_WindowState_Shown);
        am_SetHostWindow(g_window);
    }

    MSG msg = {};

    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    am_Release();

    return static_cast<int>(msg.wParam);
}
