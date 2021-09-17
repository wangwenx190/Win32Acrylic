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
#include "SystemLibrary.h"

#ifndef RESOLVE
#define RESOLVE(module, symbol) static const auto symbol##Func = reinterpret_cast<decltype(&::symbol)>(g_##module##DLL.GetSymbol(L#symbol))
#endif

class WindowsVersion
{
public:
    explicit constexpr WindowsVersion(const int major, const int minor, const int build) noexcept {
        m_major = major;
        m_minor = minor;
        m_build = build;
    }
    ~WindowsVersion() noexcept = default;

    inline int Major() const noexcept {
        return m_major;
    }

    inline int Minor() const noexcept {
        return m_minor;
    }

    inline int Build() const noexcept {
        return m_build;
    }

private:
    WindowsVersion(const WindowsVersion &) = delete;
    WindowsVersion &operator=(const WindowsVersion &) = delete;
    WindowsVersion(WindowsVersion &&) = delete;
    WindowsVersion &operator=(WindowsVersion &&) = delete;

private:
    int m_major = 0;
    int m_minor = 0;
    int m_build = 0;
};

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

static constexpr WindowsVersion g_Win10_RS1 = WindowsVersion(10, 0, 14393);
static constexpr WindowsVersion g_Win10_19H1 = WindowsVersion(10, 0, 18362);

static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE = 20;

static constexpr wchar_t g_personalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

static constexpr wchar_t g_defaultWindowClassName[] = L"{7C8ADF8C-896C-467B-A9E4-A43815267D78}";
static constexpr wchar_t g_defaultWindowTitle[] = L"Win32AcrylicHelper Application Main Window";

static SystemLibrary g_AdvApi32DLL = SystemLibrary(L"AdvApi32.dll");
static SystemLibrary g_User32DLL = SystemLibrary(L"User32.dll");
static SystemLibrary g_Gdi32DLL = SystemLibrary(L"Gdi32.dll");
static SystemLibrary g_Ole32DLL = SystemLibrary(L"Ole32.dll");
static SystemLibrary g_UxThemeDLL = SystemLibrary(L"UxTheme.dll");
static SystemLibrary g_DwmApiDLL = SystemLibrary(L"DwmApi.dll");

static ATOM g_mainWindowAtom = INVALID_ATOM;
static HWND g_mainWindowHandle = nullptr;
static HWND g_xamlIslandWindowHandle = nullptr;
static HINSTANCE g_instance = nullptr;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_manager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_source = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_backgroundBrush = nullptr;

[[nodiscard]] static inline bool IsWindowsOrGreater(const WindowsVersion &version) noexcept
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = version.Major();
    osvi.dwMinorVersion = version.Minor();
    osvi.dwBuildNumber = version.Build();
    const BYTE op = VER_GREATER_EQUAL;
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

[[nodiscard]] static inline bool IsWindows10RS1OrGreater() noexcept
{
    // Windows 10 Version 1607 (Anniversary Update)
    // Code name: Red Stone 1
    static const bool result = IsWindowsOrGreater(g_Win10_RS1);
    return result;
}

[[nodiscard]] static inline bool IsWindows1019H1OrGreater() noexcept
{
    // Windows 10 Version 1903 (May 2019 Update)
    // Code name: 19H1
    static const bool result = IsWindowsOrGreater(g_Win10_19H1);
    return result;
}

static inline void DisplayErrorDialog(LPCWSTR text) noexcept
{
    RESOLVE(User32, MessageBoxW);
    if (MessageBoxWFunc) {
        if (text && (wcslen(text) > 0)) {
            OutputDebugStringW(text);
            MessageBoxWFunc(nullptr, text, L"Error", MB_ICONERROR | MB_OK);
        } else {
            OutputDebugStringW(L"Failed to show the message box due to the content is empty.");
        }
    } else {
        OutputDebugStringW(L"MessageBoxW() is not available.");
    }
}

[[nodiscard]] static inline bool IsHighContrastModeEnabled() noexcept
{
    RESOLVE(User32, SystemParametersInfoW);
    if (SystemParametersInfoWFunc) {
        HIGHCONTRASTW hc;
        SecureZeroMemory(&hc, sizeof(hc));
        hc.cbSize = sizeof(hc);
        if (SystemParametersInfoWFunc(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
            OutputDebugStringW(L"Failed to retrieve the high contrast mode state.");
            return false;
        }
        return (hc.dwFlags & HCF_HIGHCONTRASTON);
    } else {
        OutputDebugStringW(L"SystemParametersInfoW() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool ShouldAppsUseDarkMode() noexcept
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    // Starting from Windows 10 19H1, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    if (IsWindows1019H1OrGreater()) {
        RESOLVE(AdvApi32, RegOpenKeyExW);
        RESOLVE(AdvApi32, RegQueryValueExW);
        RESOLVE(AdvApi32, RegCloseKey);
        if (RegOpenKeyExWFunc && RegQueryValueExWFunc && RegCloseKeyFunc) {
            HKEY hKey = nullptr;
            if (RegOpenKeyExWFunc(HKEY_CURRENT_USER, g_personalizeRegistryKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                OutputDebugStringW(L"Failed to open the registry key to read.");
                return false;
            }
            DWORD dwValue = 0;
            DWORD dwType = REG_DWORD;
            DWORD dwSize = sizeof(dwValue);
            const bool success = (RegQueryValueExWFunc(hKey, L"AppsUseLightTheme", nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwValue),&dwSize) == ERROR_SUCCESS);
            if (!success) {
                OutputDebugStringW(L"Failed to query the registry key value.");
            }
            if (RegCloseKeyFunc(hKey) != ERROR_SUCCESS) {
                OutputDebugStringW(L"Failed to close the registry key.");
            }
            return (success && (dwValue == 0));
        } else {
            OutputDebugStringW(L"RegOpenKeyExW(), RegQueryValueExW() and RegCloseKey() are not available.");
            return false;
        }
    } else {
        static const auto ShouldAppsUseDarkModeFunc = reinterpret_cast<BOOL(WINAPI *)()>(g_UxThemeDLL.GetSymbol(MAKEINTRESOURCEW(132)));
        if (ShouldAppsUseDarkModeFunc) {
            return (ShouldAppsUseDarkModeFunc() != FALSE);
        } else {
            OutputDebugStringW(L"ShouldAppsUseDarkMode() is not available.");
            return false;
        }
    }
}

[[nodiscard]] static inline bool RefreshWindowTheme() noexcept
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    RESOLVE(DwmApi, DwmSetWindowAttribute);
    RESOLVE(UxTheme, SetWindowTheme);
    if (DwmSetWindowAttributeFunc && SetWindowThemeFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to refresh main window theme due to the main window handle is null.");
            return false;
        }
        BOOL useDarkFrame = FALSE;
        LPCWSTR themeName = nullptr;
        if (IsHighContrastModeEnabled()) {
            // ### TO BE IMPLEMENTED
        } else if (ShouldAppsUseDarkMode()) {
            useDarkFrame = TRUE;
            themeName = L"Dark_Explorer";
        } else {
            useDarkFrame = FALSE;
            themeName = L"Explorer";
        }
        const HRESULT hr1 = DwmSetWindowAttributeFunc(g_mainWindowHandle, g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &useDarkFrame, sizeof(useDarkFrame));
        const HRESULT hr2 = DwmSetWindowAttributeFunc(g_mainWindowHandle, g_DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkFrame, sizeof(useDarkFrame));
        const HRESULT hr3 = SetWindowThemeFunc(g_mainWindowHandle, themeName, nullptr);
        if (FAILED(hr1) && FAILED(hr2)) {
            OutputDebugStringW(L"Failed to change the window dark mode state.");
            return false;
        }
        if (FAILED(hr3)) {
            OutputDebugStringW(L"Failed to change the window theme.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DwmSetWindowAttribute() and SetWindowTheme() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool RefreshBackgroundBrush() noexcept
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    if (g_backgroundBrush == nullptr) {
        OutputDebugStringW(L"Failed to refresh the background brush due to the brush is null.");
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

[[nodiscard]] static inline bool CloseMainWindow() noexcept
{
    RESOLVE(User32, DestroyWindow);
    RESOLVE(User32, UnregisterClassW);
    if (DestroyWindowFunc && UnregisterClassWFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to close main window due to the main window handle is null.");
            return false;
        }
        if (g_mainWindowAtom == INVALID_ATOM) {
            OutputDebugStringW(L"Failed to close main window due to the main window ATOM is invalid.");
            return false;
        }
        if (!g_instance) {
            OutputDebugStringW(L"Failed to close main window due to the HINSTANCE is null.");
            return false;
        }
        if (DestroyWindowFunc(g_mainWindowHandle) == FALSE) {
            DisplayErrorDialog(L"Failed to destroy main window.");
            return false;
        }
        if (UnregisterClassWFunc(reinterpret_cast<LPCWSTR>(static_cast<WORD>(MAKELONG(g_mainWindowAtom, 0))), g_instance) == FALSE) {
            DisplayErrorDialog(L"Failed to unregister main window class.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DestroyWindow() and UnregisterClassW() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool SyncXAMLIslandPosition(const UINT width, const UINT height) noexcept
{
    RESOLVE(User32, SetWindowPos);
    if (SetWindowPosFunc) {
        if (!g_xamlIslandWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
            return false;
        }
        if ((width == 0) || (height == 0)) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to invalid width and height.");
            return false;
        }
        if (SetWindowPosFunc(g_xamlIslandWindowHandle, nullptr, 0, 0, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
            DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"SetWindowPos() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool SyncXAMLIslandPosition() noexcept
{
    RESOLVE(User32, GetClientRect);
    if (GetClientRectFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to the main window handle is null.");
            return false;
        }
        if (!g_xamlIslandWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
            return false;
        }
        RECT clientRect = {0, 0, 0, 0};
        if (GetClientRectFunc(g_mainWindowHandle, &clientRect) == FALSE) {
            DisplayErrorDialog(L"Failed to retrieve the client rect of the window.");
            return false;
        }
        return SyncXAMLIslandPosition(clientRect.right, clientRect.bottom);
    } else {
        OutputDebugStringW(L"GetClientRect() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool GenerateGUID(LPCWSTR *str) noexcept
{
    RESOLVE(Ole32, CoCreateGuid);
    RESOLVE(Ole32, StringFromGUID2);
    if (CoCreateGuidFunc && StringFromGUID2Func) {
        if (!str) {
            OutputDebugStringW(L"Failed to generate GUID due to the given string address is null.");
            return false;
        }
        GUID guid = {};
        if (FAILED(CoCreateGuidFunc(&guid))) {
            DisplayErrorDialog(L"Failed to generate GUID.");
            return false;
        }
        auto buf = new wchar_t[MAX_PATH];
        SecureZeroMemory(buf, sizeof(buf));
        if (StringFromGUID2Func(guid, buf, MAX_PATH) == 0) {
            delete [] buf;
            buf = nullptr;
            DisplayErrorDialog(L"Failed to convert GUID to string.");
            return false;
        }
        *str = buf;
        return true;
    } else {
        OutputDebugStringW(L"CoCreateGuid() and StringFromGUID2() are not available.");
        return false;
    }
}

[[nodiscard]] static inline LRESULT CALLBACK MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (message) {
    case WM_CLOSE: {
        if (CloseMainWindow()) {
            g_mainWindowHandle = nullptr;
            g_mainWindowAtom = INVALID_ATOM;
            return 0;
        } else {
            DisplayErrorDialog(L"Failed to close the window.");
        }
    } break;
    case WM_DESTROY: {
        g_backgroundBrush = nullptr;
        g_rootGrid = nullptr;
        if (g_source != nullptr) {
            g_source.Close();
            g_source = nullptr;
        }
        if (g_manager != nullptr) {
            g_manager.Close();
            g_manager = nullptr;
        }
        g_xamlIslandWindowHandle = nullptr;
        g_instance = nullptr;
        g_AdvApi32DLL.Unload();
        g_Gdi32DLL.Unload();
        g_Ole32DLL.Unload();
        g_UxThemeDLL.Unload();
        g_DwmApiDLL.Unload();
        RESOLVE(User32, PostQuitMessage);
        if (PostQuitMessageFunc) {
            PostQuitMessageFunc(0);
            g_User32DLL.Unload();
            return 0;
        } else {
            OutputDebugStringW(L"PostQuitMessage() is not available.");
        }
    } break;
    case WM_SIZE: {
        if (g_xamlIslandWindowHandle) {
            const UINT width = LOWORD(lParam);
            const UINT height = HIWORD(lParam);
            if (!SyncXAMLIslandPosition(width, height)) {
                DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
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
            if (!RefreshWindowTheme()) {
                DisplayErrorDialog(L"Failed to refresh the window theme.");
            }
            if (!RefreshBackgroundBrush()) {
                DisplayErrorDialog(L"Failed to refresh the background brush.");
            }
        }
    } break;
    default:
        break;
    }
    RESOLVE(User32, DefWindowProcW);
    if (DefWindowProcWFunc) {
        return DefWindowProcWFunc(hWnd, message, wParam, lParam);
    } else {
        OutputDebugStringW(L"DefWindowProcW() is not available.");
        return 0;
    }
}

[[nodiscard]] static inline bool RegisterMainWindowClass(LPCWSTR name) noexcept
{
    RESOLVE(User32, LoadCursorW);
    RESOLVE(User32, LoadIconW);
    RESOLVE(User32, RegisterClassExW);
    if (LoadCursorWFunc && LoadIconWFunc && RegisterClassExWFunc) {
        if (!g_instance) {
            OutputDebugStringW(L"Failed to register main window class due to the HINSTANCE is null.");
            return false;
        }
        LPCWSTR guid = nullptr;
        if (!GenerateGUID(&guid)) {
            OutputDebugStringW(L"Failed to register the window class due to can't generate a new GUID.");
            return false;
        }
        // todo: getclassinfo, reuse the previous class
        WNDCLASSEXW wcex;
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = MessageHandler;
        wcex.hInstance = g_instance;
        wcex.lpszClassName = ((name && (wcslen(name) > 0)) ? name : guid);
        wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
        wcex.hIcon = LoadIconWFunc(g_instance, MAKEINTRESOURCEW(IDI_APPICON));
        wcex.hIconSm = LoadIconWFunc(g_instance, MAKEINTRESOURCEW(IDI_APPICON_SMALL));
        g_mainWindowAtom = RegisterClassExWFunc(&wcex);
        if (guid) {
            delete [] guid;
            guid = nullptr;
        }
        return (g_mainWindowAtom != INVALID_ATOM);
    } else {
        OutputDebugStringW(L"LoadCursorW(), LoadIconW() and RegisterClassExW() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool CreateMainWindow(LPCWSTR title) noexcept
{
    RESOLVE(User32, CreateWindowExW);
    if (CreateWindowExWFunc) {
        if (g_mainWindowAtom == INVALID_ATOM) {
            OutputDebugStringW(L"Failed to create main window due to the main window ATOM is invalid.");
            return false;
        }
        if (!g_instance) {
            OutputDebugStringW(L"Failed to create main window due to the HINSTANCE is null.");
            return false;
        }
        g_mainWindowHandle = CreateWindowExWFunc(
            WS_EX_NOREDIRECTIONBITMAP,
            reinterpret_cast<LPCWSTR>(static_cast<WORD>(MAKELONG(g_mainWindowAtom, 0))),
            [title]{
                if (title && (wcslen(title) > 0)) {
                    return title;
                } else {
                    return g_defaultWindowTitle;
                }
            }(),
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            nullptr, nullptr, g_instance, nullptr);
        return (g_mainWindowHandle != nullptr);
    } else {
        OutputDebugStringW(L"CreateWindowExW() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool ShowMainWindow(const int nCmdShow) noexcept
{
    RESOLVE(User32, ShowWindow);
    RESOLVE(User32, UpdateWindow);
    if (ShowWindowFunc && UpdateWindowFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to show the main window due to the main window handle is null.");
            return false;
        }
        ShowWindowFunc(g_mainWindowHandle, nCmdShow);
        if (UpdateWindowFunc(g_mainWindowHandle) == FALSE) {
            DisplayErrorDialog(L"Failed to update main window.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"ShowWindow() and UpdateWindow() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool InitializeXAMLIsland() noexcept
{
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
        DisplayErrorDialog(L"Failed to retrieve XAML Island's core window.");
        return false;
    }
    // Parent the DesktopWindowXamlSource object to the current window.
    if (FAILED(interop->AttachToWindow(g_mainWindowHandle))) {
        DisplayErrorDialog(L"Failed to attach the XAML Island window to the main window.");
        return false;
    }
    // Get the new child window's HWND.
    if (FAILED(interop->get_WindowHandle(&g_xamlIslandWindowHandle))) {
        DisplayErrorDialog(L"Failed to retrieve the XAML Island window's handle.");
        return false;
    }
    if (!g_xamlIslandWindowHandle) {
        DisplayErrorDialog(L"Failed to retrieve the window handle of XAML Island's core window.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    if (!SyncXAMLIslandPosition()) {
        DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
        return false;
    }
    // Create the XAML content.
    g_rootGrid = winrt::Windows::UI::Xaml::Controls::Grid();
    g_backgroundBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
    if (!RefreshBackgroundBrush()) {
        DisplayErrorDialog(L"Failed to refresh the background brush.");
        return false;
    }
    g_backgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_rootGrid.Background(g_backgroundBrush);
    //g_rootGrid.Children().Clear();
    //g_rootGrid.Children().Append(/* some UWP control */);
    //g_rootGrid.UpdateLayout();
    g_source.Content(g_rootGrid);
    return true;
}

[[nodiscard]] static inline int MessageLoop() noexcept
{
    RESOLVE(User32, GetMessageW);
    RESOLVE(User32, TranslateMessage);
    RESOLVE(User32, DispatchMessageW);
    if (GetMessageWFunc && TranslateMessageFunc && DispatchMessageWFunc) {
        MSG msg = {};
        while (GetMessageWFunc(&msg, nullptr, 0, 0) != FALSE) {
            BOOL filtered = FALSE;
            if (g_source != nullptr) {
                const auto interop2 = g_source.as<IDesktopWindowXamlSourceNative2>();
                if (interop2) {
                    interop2->PreTranslateMessage(&msg, &filtered);
                }
            }
            if (filtered == FALSE) {
                TranslateMessageFunc(&msg);
                DispatchMessageWFunc(&msg);
            }
        }
        return static_cast<int>(msg.wParam);
    } else {
        OutputDebugStringW(L"GetMessageW(), TranslateMessage() and DispatchMessageW() are not available.");
        return -1;
    }
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
        DisplayErrorDialog(L"This application only supports Windows 10 19H1 and onwards.");
        return -1;
    }

    g_instance = hInstance;

    if (!RegisterMainWindowClass(nullptr)) {
        DisplayErrorDialog(L"Failed to register the main window class.");
        return -1;
    }

    if (!CreateMainWindow(nullptr)) {
        DisplayErrorDialog(L"Failed to create the main window.");
        return -1;
    }

    if (!RefreshWindowTheme()) {
        DisplayErrorDialog(L"Failed to refresh the main window theme.");
        return -1;
    }

    if (!InitializeXAMLIsland()) {
        DisplayErrorDialog(L"Failed to initialize the XAML Island.");
        return -1;
    }

    if (!ShowMainWindow(nCmdShow)) {
        DisplayErrorDialog(L"Failed to show the main window.");
        return -1;
    }

    return MessageLoop();
}
