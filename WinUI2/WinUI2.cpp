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
#include <UxTheme.h>
#include <DwmApi.h>
#include <Unknwn.h>
#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>
#include "Resource.h"

namespace Constants {
namespace Light {
static constexpr winrt::Windows::UI::Color TintColor = {255, 252, 252, 252};
static constexpr double TintOpacity = 0.0;
static constexpr double LuminosityOpacity = 0.85;
static constexpr winrt::Windows::UI::Color FallbackColor = {255, 249, 249, 249};
} // namespace Light
namespace Dark {
static constexpr winrt::Windows::UI::Color TintColor = {255, 44, 44, 44};
static constexpr double TintOpacity = 0.15;
static constexpr double LuminosityOpacity = 0.96;
static constexpr winrt::Windows::UI::Color FallbackColor = {255, 44, 44, 44};
} // namespace Dark
namespace HighContrast {
// ### TO BE IMPLEMENTED
} // namespace HighContrast
} // namespace Constants

static constexpr DWORD _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
static constexpr DWORD _DWMWA_USE_IMMERSIVE_DARK_MODE = 20;

static constexpr wchar_t g_personalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

static constexpr wchar_t g_windowClassName[] = LR"(wangwenx190\Win32Acrylic\WinUI2\WindowClass)";
static constexpr wchar_t g_windowTitle[] = L"Win32Acrylic WinUI2 Demo";

static HWND g_mainWindowHandle = nullptr;
static HWND g_xamlIslandHandle = nullptr;
static HINSTANCE g_instance = nullptr;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_manager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_source = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_backgroundBrush = nullptr;

[[nodiscard]] static inline bool IsWindowsNOrGreater(const int major, const int minor, const int build)
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = major;
    osvi.dwMinorVersion = minor;
    osvi.dwBuildNumber = build;
    const BYTE op = VER_GREATER_EQUAL;
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

[[nodiscard]] static inline bool IsWindows10RS1OrGreater()
{
    // Windows 10 Version 1607 (Anniversary Update)
    // Code name: Red Stone 1
    return IsWindowsNOrGreater(10, 0, 14393);
}

[[nodiscard]] static inline bool IsWindows1019H1OrGreater()
{
    // Windows 10 Version 1903 (May 2019 Update)
    // Code name: 19H1
    return IsWindowsNOrGreater(10, 0, 18362);
}

[[nodiscard]] static inline bool IsHighContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        OutputDebugStringW(L"Failed to retrieve the high contrast mode state.");
        return false;
    }
    return (hc.dwFlags & HCF_HIGHCONTRASTON);
}

[[nodiscard]] static inline bool ShouldAppsUseDarkMode()
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    // Starting from Windows 10 19H1, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    if (IsWindows1019H1OrGreater()) {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, g_personalizeRegistryKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            OutputDebugStringW(L"Failed to open the registry key to read.");
            return false;
        }
        DWORD dwValue = 0;
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(dwValue);
        const bool success = (RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwValue),&dwSize) == ERROR_SUCCESS);
        if (!success) {
            OutputDebugStringW(L"Failed to query the registry key value.");
        }
        if (RegCloseKey(hKey) != ERROR_SUCCESS) {
            OutputDebugStringW(L"Failed to close the registry key.");
        }
        return (success && (dwValue == 0));
    } else {
        static bool tried = false;
        using ShouldAppsUseDarkModeSig = BOOL(WINAPI *)();
        static ShouldAppsUseDarkModeSig ShouldAppsUseDarkModeFunc = nullptr;
        if (!ShouldAppsUseDarkModeFunc) {
            if (!tried) {
                tried = true;
                const HMODULE UxThemeDLL = LoadLibraryExW(L"UxTheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (UxThemeDLL) {
                    ShouldAppsUseDarkModeFunc = reinterpret_cast<ShouldAppsUseDarkModeSig>(GetProcAddress(UxThemeDLL, MAKEINTRESOURCEA(132)));
                    if (!ShouldAppsUseDarkModeFunc) {
                        OutputDebugStringW(L"Failed to resolve symbol ShouldAppsUseDarkMode().");
                    }
                    //FreeLibrary(UxThemeDLL);
                } else {
                    OutputDebugStringW(L"Failed to load dynamic link library UxTheme.dll.");
                }
            }
        }
        return (ShouldAppsUseDarkModeFunc ? (ShouldAppsUseDarkModeFunc() != FALSE) : false);
    }
}

[[nodiscard]] static inline bool RefreshWindowTheme(const HWND hWnd)
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    if (!hWnd) {
        return false;
    }
    BOOL enabled = FALSE;
    LPCWSTR themeName = nullptr;
    if (IsHighContrastModeEnabled()) {
        // ### TO BE IMPLEMENTED
    } else if (ShouldAppsUseDarkMode()) {
        enabled = TRUE;
        themeName = L"Dark_Explorer";
    } else {
        enabled = FALSE;
        themeName = L"Explorer";
    }
    DwmSetWindowAttribute(hWnd, _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &enabled, sizeof(enabled));
    DwmSetWindowAttribute(hWnd, _DWMWA_USE_IMMERSIVE_DARK_MODE, &enabled, sizeof(enabled));
    SetWindowTheme(hWnd, themeName, nullptr);
    return true;
}

[[nodiscard]] static inline bool RefreshBackgroundBrush()
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    if (g_backgroundBrush == nullptr) {
        return false;
    }
    if (IsHighContrastModeEnabled()) {
        // ### TO BE IMPLEMENTED
    } else if (ShouldAppsUseDarkMode()) {
        g_backgroundBrush.TintColor(Constants::Dark::TintColor);
        g_backgroundBrush.TintOpacity(Constants::Dark::TintOpacity);
        g_backgroundBrush.TintLuminosityOpacity(Constants::Dark::LuminosityOpacity);
        g_backgroundBrush.FallbackColor(Constants::Dark::FallbackColor);
    } else {
        g_backgroundBrush.TintColor(Constants::Light::TintColor);
        g_backgroundBrush.TintOpacity(Constants::Light::TintOpacity);
        g_backgroundBrush.TintLuminosityOpacity(Constants::Light::LuminosityOpacity);
        g_backgroundBrush.FallbackColor(Constants::Light::FallbackColor);
    }
    return true;
}

[[nodiscard]] static inline LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CLOSE: {
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
        if (g_source != nullptr) {
            g_source.Close();
            g_source = nullptr;
        }
        if (g_manager != nullptr) {
            g_manager.Close();
            g_manager = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    } break;
    case WM_SIZE: {
        if (g_xamlIslandHandle) {
            const UINT width = LOWORD(lParam);
            const UINT height = HIWORD(lParam);
            if (MoveWindow(g_xamlIslandHandle, 0, 0, width, height, TRUE) == FALSE) {
                MessageBoxW(nullptr, L"Failed to change the geometry of the XAML Island window.", L"Error", MB_ICONERROR | MB_OK);
                break;
            }
        }
    } break;
    case WM_SETTINGCHANGE: {
        if (!IsWindows10RS1OrGreater()) {
            break;
        }
        // wParam == 0: User-wide setting change
        // wParam == 1: System-wide setting change
        if (((wParam == 0) || (wParam == 1)) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            if (!RefreshWindowTheme(hWnd)) {
                MessageBoxW(nullptr, L"Failed to refresh the window theme.", L"Error", MB_ICONERROR | MB_OK);
                break;
            }
            if (!RefreshBackgroundBrush()) {
                MessageBoxW(nullptr, L"Failed to refresh the background brush.", L"Error", MB_ICONERROR | MB_OK);
                break;
            }
        }
    } break;
    default:
        break;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
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

    if (!RefreshWindowTheme(g_mainWindowHandle)) {
        MessageBoxW(nullptr, L"Failed to refresh the window theme.", L"Error", MB_ICONERROR | MB_OK);
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
    if (!RefreshBackgroundBrush()) {
        MessageBoxW(nullptr, L"Failed to refresh the background brush.", L"Error", MB_ICONERROR | MB_OK);
        return -1;
    }
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
