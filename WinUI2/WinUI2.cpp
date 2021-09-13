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

#include <SDKDDKVer.h>
#include <Windows.h>
#include <Unknwn.h>
#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>
#include "Resource.h"

static constexpr wchar_t g_windowClassName[] = L"Win32AcrylicDemoApplicationWindowClass";
static constexpr wchar_t g_windowTitle[] = L"Win32Acrylic WinUI2 Demo";

static HWND g_mainWindowHandle = nullptr;
static HWND g_xamlIslandHandle = nullptr;
static HINSTANCE g_instance = nullptr;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_manager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_source = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_backgroundBrush = nullptr;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

[[nodiscard]] static inline bool IsWindows1019H1OrGreater()
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber = 18362;
    const BYTE op = VER_GREATER_EQUAL;
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
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

    if (!IsWindows1019H1OrGreater()) {
        MessageBoxW(nullptr, L"This application only supports Windows 10 19H1 and onwards.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }

    g_instance = hInstance;

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = g_windowClassName;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON));
    wcex.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APPICON_SMALL));

    if (!RegisterClassExW(&wcex)) {
        MessageBoxW(nullptr, L"Failed to register the window class.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }

    g_mainWindowHandle = CreateWindowExW(
        WS_EX_NOREDIRECTIONBITMAP,
        g_windowClassName,
        g_windowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    if (!g_mainWindowHandle) {
        MessageBoxW(nullptr, L"Failed to create the window.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }

    // XAML Island section:
    // The call to winrt::init_apartment() initializes COM. By default, in a multithreaded apartment.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // Initialize the XAML framework's core window for the current thread.
    g_manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    g_source = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
    // Get handle to the core window.
    const auto interop = g_source.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        MessageBoxW(nullptr, L"Failed to retrieve XAML Island's core window.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }
    // Parent the DesktopWindowXamlSource object to the current window.
    winrt::check_hresult(interop->AttachToWindow(g_mainWindowHandle));
    // Get the new child window's HWND.
    winrt::check_hresult(interop->get_WindowHandle(&g_xamlIslandHandle));
    if (!g_xamlIslandHandle) {
        MessageBoxW(nullptr, L"Failed to retrieve the window handle of XAML Island's core window.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(g_mainWindowHandle, &rect) == FALSE) {
        MessageBoxW(nullptr, L"Failed to retrieve the client area rect of the window.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }
    if (SetWindowPos(g_xamlIslandHandle, nullptr, 0, 0, rect.right, rect.bottom,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
        MessageBoxW(nullptr, L"Failed to change the geometry of the XAML Island window.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }
    // Create the XAML content.
    g_rootGrid = winrt::Windows::UI::Xaml::Controls::Grid();
    g_backgroundBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
    g_backgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_rootGrid.Background(g_backgroundBrush);
    //g_rootGrid.Children().Clear();
    //g_rootGrid.Children().Append(/* some UWP control */);
    //g_rootGrid.UpdateLayout();
    g_source.Content(g_rootGrid);
    // End XAML Island section.

    ShowWindow(g_mainWindowHandle, nCmdShow);
    if (UpdateWindow(g_mainWindowHandle) == FALSE) {
        MessageBoxW(nullptr, L"Failed to update the window.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CLOSE: {
        if (g_source != nullptr) {
            g_source.Close();
            g_source = nullptr;
        }
        if (g_manager != nullptr) {
            g_manager.Close();
            g_manager = nullptr;
        }
        if (DestroyWindow(hWnd) == FALSE) {
            MessageBoxW(nullptr, L"Failed to destroy the window.", L"Error", MB_ICONERROR | MB_OK);
            break;
        }
        if (UnregisterClassW(g_windowClassName, g_instance) == FALSE) {
            MessageBoxW(nullptr, L"Failed to unregister the window class.", L"Error", MB_ICONERROR | MB_OK);
            break;
        }
        return 0;
    } break;
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    } break;
    case WM_SIZE: {
        if (g_xamlIslandHandle) {
            const UINT width = LOWORD(lParam);
            const UINT height = HIWORD(lParam);
            if (MoveWindow(g_xamlIslandHandle, 0, 0, width, height, TRUE) == FALSE) {
                MessageBoxW(nullptr, L"Failed to change the geometry of the XAML Island window.", L"Error", MB_ICONERROR | MB_OK);
            }
        }
    } break;
    default:
        break;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}
