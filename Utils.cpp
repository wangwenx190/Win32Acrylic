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

#include "Utils.h"
#include "WindowsVersion.h"
#include "SystemLibraryManager.h"
#include <ShellScalingApi.h>
#include <ComBaseApi.h>
#include <DwmApi.h>
#include <cmath>

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI (96)
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER (92)
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT (reinterpret_cast<HINSTANCE>(&__ImageBase))
#endif

static constexpr int g_PROCESS_PER_MONITOR_DPI_AWARE_V2 = 3;

static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
static constexpr DWORD g_DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37;

static constexpr wchar_t g_personalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

static constexpr UINT g_defaultResizeBorderThickness = 8;
static constexpr UINT g_defaultCaptionHeight = 23;
static constexpr UINT g_defaultTitleBarHeight = 31;
static constexpr UINT g_defaultFrameBorderThickness = 1;
static constexpr UINT g_defaultWindowDPI = USER_DEFAULT_SCREEN_DPI;

HMONITOR Utils::GetWindowScreen(const HWND hWnd, const bool current) noexcept
{
    USER32_API(MonitorFromWindow);
    if (MonitorFromWindowFunc) {
        if (!hWnd) {
            return nullptr;
        }
        const HMONITOR mon = MonitorFromWindowFunc(hWnd, (current ? MONITOR_DEFAULTTONEAREST : MONITOR_DEFAULTTOPRIMARY));
        if (!mon) {
            PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow, L"Failed to retrieve the window's corresponding screen.")
            return nullptr;
        }
        return mon;
    } else {
        OutputDebugStringW(L"MonitorFromWindow() is not available.");
        return nullptr;
    }
}

HINSTANCE Utils::GetCurrentInstance() noexcept
{
    static const auto result = HINST_THISCOMPONENT;
    return result;
}

LPCWSTR Utils::GetWindowClassName(const ATOM atom) noexcept
{
    if (atom == INVALID_ATOM) {
        OutputDebugStringW(L"Failed to convert the given ATOM to window class name due to it's invalid.");
        return nullptr;
    } else {
        return reinterpret_cast<LPCWSTR>(static_cast<WORD>(MAKELONG(atom, 0)));
    }
}

LPCWSTR Utils::GetSystemErrorMessage(LPCWSTR function, const DWORD code) noexcept
{
    if (!function) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return nullptr;
    }
    if (code == ERROR_SUCCESS) {
        OutputDebugStringW(L"Operation succeeded.");
        return nullptr;
    }
    LPWSTR buf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 0, nullptr) == 0) {
        OutputDebugStringW(L"Failed to retrieve the system error message.");
        return nullptr;
    }
    const auto str = new wchar_t[MAX_PATH];
    SecureZeroMemory(str, sizeof(str));
    swprintf(str, L"%s failed with error %d: %s.", function, code, buf);
    LocalFree(buf);
    return str;
}

LPCWSTR Utils::GetSystemErrorMessage(LPCWSTR function, const HRESULT hr) noexcept
{
    if (!function) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return nullptr;
    }
    if (SUCCEEDED(hr)) {
        OutputDebugStringW(L"Operation succeeded.");
        return nullptr;
    }
    const DWORD dwError = HRESULT_CODE(hr);
    return GetSystemErrorMessage(function, dwError);
}

LPCWSTR Utils::GetSystemErrorMessage(LPCWSTR function) noexcept
{
    if (!function) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return nullptr;
    } else {
        const DWORD dwError = GetLastError();
        return GetSystemErrorMessage(function, dwError);
    }
}

void Utils::DisplayErrorDialog(LPCWSTR text) noexcept
{
    USER32_API(MessageBoxW);
    if (MessageBoxWFunc) {
        if (text) {
            OutputDebugStringW(text);
            MessageBoxWFunc(nullptr, text, L"Error", MB_ICONERROR | MB_OK);
        } else {
            OutputDebugStringW(L"Failed to show the message box due to the content is empty.");
        }
    } else {
        OutputDebugStringW(L"MessageBoxW() is not available.");
    }
}

bool Utils::IsHighContrastModeEnabled() noexcept
{
    USER32_API(SystemParametersInfoW);
    if (SystemParametersInfoWFunc) {
        HIGHCONTRASTW hc;
        SecureZeroMemory(&hc, sizeof(hc));
        hc.cbSize = sizeof(hc);
        if (SystemParametersInfoWFunc(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SystemParametersInfoW, L"Failed to retrieve the high contrast mode state.")
            return false;
        }
        return (hc.dwFlags & HCF_HIGHCONTRASTON);
    } else {
        OutputDebugStringW(L"SystemParametersInfoW() is not available.");
        return false;
    }
}

bool Utils::ShouldAppsUseDarkMode() noexcept
{
    ADVAPI32_API(RegOpenKeyExW);
    ADVAPI32_API(RegQueryValueExW);
    ADVAPI32_API(RegCloseKey);
    if (RegOpenKeyExWFunc && RegQueryValueExWFunc && RegCloseKeyFunc) {
        HKEY hKey = nullptr;
        if (RegOpenKeyExWFunc(HKEY_CURRENT_USER, g_personalizeRegistryKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            PRINT_WIN32_ERROR_MESSAGE(RegOpenKeyExW, L"Failed to open the registry key to read.")
            return false;
        }
        DWORD dwValue = 0;
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(dwValue);
        const bool success = (RegQueryValueExWFunc(hKey, L"AppsUseLightTheme", nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwValue),&dwSize) == ERROR_SUCCESS);
        if (!success) {
            PRINT_WIN32_ERROR_MESSAGE(RegQueryValueExW, L"Failed to query the registry key value.")
        }
        if (RegCloseKeyFunc(hKey) != ERROR_SUCCESS) {
            PRINT_WIN32_ERROR_MESSAGE(RegCloseKey, L"Failed to close the registry key.")
        }
        return (success && (dwValue == 0));
    } else {
        OutputDebugStringW(L"RegOpenKeyExW(), RegQueryValueExW() and RegCloseKey() are not available.");
        return false;
    }
}

LPCWSTR Utils::GenerateGUID() noexcept
{
    OLE32_API(CoCreateGuid);
    OLE32_API(StringFromGUID2);
    if (CoCreateGuidFunc && StringFromGUID2Func) {
        GUID guid = {};
        const HRESULT hr = CoCreateGuidFunc(&guid);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(CoCreateGuid, hr, L"Failed to generate a new GUID.")
            return nullptr;
        }
        auto buf = new wchar_t[MAX_PATH];
        SecureZeroMemory(buf, sizeof(buf));
        if (StringFromGUID2Func(guid, buf, MAX_PATH) == 0) {
            delete [] buf;
            buf = nullptr;
            PRINT_WIN32_ERROR_MESSAGE(StringFromGUID2, L"Failed to convert GUID to string.")
            return nullptr;
        }
        return buf;
    } else {
        OutputDebugStringW(L"CoCreateGuid() and StringFromGUID2() are not available.");
        return nullptr;
    }
}

bool Utils::RefreshWindowTheme(const HWND hWnd) noexcept
{
    DWMAPI_API(DwmSetWindowAttribute);
    UXTHEME_API(SetWindowTheme);
    if (DwmSetWindowAttributeFunc && SetWindowThemeFunc) {
        if (!hWnd) {
            OutputDebugStringW(L"Failed to refresh the window theme due to the given window handle is null.");
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
        const HRESULT hr1 = DwmSetWindowAttributeFunc(hWnd, g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &useDarkFrame, sizeof(useDarkFrame));
        const HRESULT hr2 = DwmSetWindowAttributeFunc(hWnd, g_DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkFrame, sizeof(useDarkFrame));
        const HRESULT hr3 = SetWindowThemeFunc(hWnd, themeName, nullptr);
        if (FAILED(hr1) && FAILED(hr2)) {
            PRINT_HR_ERROR_MESSAGE(DwmSetWindowAttribute, hr2, L"Failed to set the window dark mode state.")
            return false;
        }
        if (FAILED(hr3)) {
            PRINT_HR_ERROR_MESSAGE(SetWindowTheme, hr3, L"Failed to set the window theme.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DwmSetWindowAttribute() and SetWindowTheme() are not available.");
        return false;
    }
}

bool Utils::CloseWindow(const HWND hWnd, const ATOM atom) noexcept
{
    USER32_API(DestroyWindow);
    USER32_API(UnregisterClassW);
    if (DestroyWindowFunc && UnregisterClassWFunc) {
        if (!hWnd) {
            OutputDebugStringW(L"Failed to close the window due to the given window handle is null.");
            return false;
        }
        if (atom == INVALID_ATOM) {
            OutputDebugStringW(L"Failed to close the window due to the given window ATOM is invalid.");
            return false;
        }
        if (DestroyWindowFunc(hWnd) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow, L"Failed to destroy the window.")
            return false;
        }
        if (UnregisterClassWFunc(GetWindowClassName(atom), GetCurrentInstance()) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW, L"Failed to unregister the window class.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DestroyWindow() and UnregisterClassW() are not available.");
        return false;
    }
}

bool Utils::IsWindowMinimized(const HWND hWnd) noexcept
{
    USER32_API(IsIconic);
    if (IsIconicFunc) {
        if (!hWnd) {
            return false;
        }
        return (IsIconicFunc(hWnd) != FALSE);
    } else {
        OutputDebugStringW(L"IsIconic() is not available.");
        return false;
    }
}

bool Utils::IsWindowMaximized(const HWND hWnd) noexcept
{
    USER32_API(IsZoomed);
    if (IsZoomedFunc) {
        if (!hWnd) {
            return false;
        }
        return (IsZoomedFunc(hWnd) != FALSE);
    } else {
        OutputDebugStringW(L"IsZoomed() is not available.");
        return false;
    }
}

bool Utils::IsWindowFullScreen(const HWND hWnd) noexcept
{
    USER32_API(GetMonitorInfoW);
    USER32_API(GetWindowRect);
    if (GetMonitorInfoWFunc && GetWindowRectFunc) {
        if (!hWnd) {
            return false;
        }
        const HMONITOR mon = GetWindowScreen(hWnd, false);
        if (!mon) {
            OutputDebugStringW(L"Failed to retrieve the primary screen.");
            return false;
        }
        MONITORINFO mi;
        SecureZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoWFunc(mon, &mi) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetMonitorInfoW, L"Failed to retrieve the screen information.")
            return false;
        }
        RECT rect = {0, 0, 0, 0};
        if (GetWindowRectFunc(hWnd, &rect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the window geometry.")
            return false;
        }
        const RECT windowRect = rect;
        const RECT screenRect = mi.rcMonitor;
        return ((windowRect.top == screenRect.top)
                && (windowRect.bottom == screenRect.bottom)
                && (windowRect.left == screenRect.left)
                && (windowRect.right == screenRect.right));
    } else {
        OutputDebugStringW(L"GetMonitorInfoW() and GetWindowRect() are not available.");
        return false;
    }
}

bool Utils::IsWindowNoState(const HWND hWnd) noexcept
{
    USER32_API(GetWindowPlacement);
    if (GetWindowPlacementFunc) {
        if (!hWnd) {
            return false;
        }
        WINDOWPLACEMENT wp;
        SecureZeroMemory(&wp, sizeof(wp));
        wp.length = sizeof(wp);
        if (GetWindowPlacementFunc(hWnd, &wp) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowPlacement, L"Failed to retrieve the current window state.")
            return false;
        }
        return (wp.showCmd == SW_NORMAL);
    } else {
        OutputDebugStringW(L"GetWindowPlacement() is not available.");
        return false;
    }
}

UINT Utils::GetWindowDPI(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return g_defaultWindowDPI;
    }
    USER32_API(GetDpiForWindow);
    if (GetDpiForWindowFunc) {
        return GetDpiForWindowFunc(hWnd);
    }
    USER32_API(GetSystemDpiForProcess);
    if (GetSystemDpiForProcessFunc) {
        GetSystemDpiForProcessFunc(GetCurrentProcess());
    }
    USER32_API(GetDpiForSystem);
    if (GetDpiForSystemFunc) {
        return GetDpiForSystemFunc();
    }
    SHCORE_API(GetDpiForMonitor);
    if (GetDpiForMonitorFunc) {
        const HMONITOR mon = GetWindowScreen(hWnd, true);
        if (mon) {
            UINT dpiX = 0, dpiY = 0;
            const HRESULT hr = GetDpiForMonitorFunc(mon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            if (SUCCEEDED(hr)) {
                return static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
            }
        }
    }
    USER32_API(GetDC);
    GDI32_API(GetDeviceCaps);
    USER32_API(ReleaseDC);
    if (GetDCFunc && GetDeviceCapsFunc && ReleaseDCFunc) {
        const HDC hdc = GetDCFunc(nullptr);
        if (hdc) {
            const int dpiX = GetDeviceCapsFunc(hdc, LOGPIXELSX);
            const int dpiY = GetDeviceCapsFunc(hdc, LOGPIXELSY);
            ReleaseDCFunc(nullptr, hdc);
            return static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
        }
    }
    return g_defaultWindowDPI;
}

UINT Utils::GetResizeBorderThickness(const HWND hWnd, const bool x) noexcept
{
    USER32_API(GetSystemMetricsForDpi);
    if (GetSystemMetricsForDpiFunc) {
        if (!hWnd) {
            return g_defaultResizeBorderThickness;
        }
        const UINT dpi = GetWindowDPI(hWnd);
        // There is no "SM_CYPADDEDBORDER".
        const int paddedBorderThickness = GetSystemMetricsForDpiFunc(SM_CXPADDEDBORDER, dpi);
        const int sizeFrameThickness = GetSystemMetricsForDpiFunc((x ? SM_CXSIZEFRAME : SM_CYSIZEFRAME), dpi);
        return static_cast<UINT>(paddedBorderThickness + sizeFrameThickness);
    } else {
        OutputDebugStringW(L"GetSystemMetricsForDpi() is not available.");
        return g_defaultResizeBorderThickness;
    }
}

UINT Utils::GetCaptionHeight(const HWND hWnd) noexcept
{
    USER32_API(GetSystemMetricsForDpi);
    if (GetSystemMetricsForDpiFunc) {
        if (!hWnd) {
            return g_defaultCaptionHeight;
        }
        const UINT dpi = GetWindowDPI(hWnd);
        return static_cast<UINT>(GetSystemMetricsForDpiFunc(SM_CYCAPTION, dpi));
    } else {
        OutputDebugStringW(L"GetSystemMetricsForDpi() is not available.");
        return g_defaultCaptionHeight;
    }
}

UINT Utils::GetTitleBarHeight(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return g_defaultTitleBarHeight;
    }
    const UINT captionHeight = GetCaptionHeight(hWnd);
    if (IsWindowMaximized(hWnd) || IsWindowFullScreen(hWnd)) {
        return captionHeight;
    } else {
        return (captionHeight + GetResizeBorderThickness(hWnd, false));
    }
}

UINT Utils::GetFrameBorderThickness(const HWND hWnd) noexcept
{
    DWMAPI_API(DwmGetWindowAttribute);
    if (DwmGetWindowAttributeFunc) {
        if (!hWnd) {
            return 0;
        }
        if (IsWindowMaximized(hWnd) || IsWindowFullScreen(hWnd)) {
            return 0;
        }
        const auto dpr = (static_cast<double>(GetWindowDPI(hWnd)) / static_cast<double>(g_defaultWindowDPI));
        UINT value = 0;
        const HRESULT hr = DwmGetWindowAttributeFunc(hWnd, g_DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
        if (SUCCEEDED(hr)) {
            return static_cast<UINT>(std::round(static_cast<double>(value) * dpr));
        } else {
            // We just eat this error because this enum value was introduced in a very
            // late Windows 10 version, so querying it's value will always result in
            // a "parameter error" (code: 87) on systems before that value was introduced.
            return static_cast<UINT>(std::round(static_cast<double>(g_defaultFrameBorderThickness) * dpr));
        }
    } else {
        OutputDebugStringW(L"DwmGetWindowAttribute() is not available.");
        return g_defaultFrameBorderThickness;
    }
}

bool Utils::OpenSystemMenu(const HWND hWnd, const POINT pos) noexcept
{
    USER32_API(GetSystemMenu);
    USER32_API(SetMenuItemInfoW);
    USER32_API(SetMenuDefaultItem);
    USER32_API(TrackPopupMenu);
    USER32_API(PostMessageW);
    if (GetSystemMenuFunc && SetMenuItemInfoWFunc && SetMenuDefaultItemFunc && TrackPopupMenuFunc && PostMessageWFunc) {
        if (!hWnd) {
            return false;
        }
        const HMENU menu = GetSystemMenuFunc(hWnd, FALSE);
        if (!menu) {
            PRINT_WIN32_ERROR_MESSAGE(GetSystemMenu, L"Failed to retrieve the system menu of the window.")
            return false;
        }
        // Update the options based on window state.
        MENUITEMINFOW mii;
        SecureZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE;
        mii.fType = MFT_STRING;
        const auto setState = [&mii, menu](const UINT item, const bool enabled) -> bool {
            mii.fState = (enabled ? MF_ENABLED : MF_DISABLED);
            if (SetMenuItemInfoWFunc(menu, item, FALSE, &mii) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW, L"Failed to set menu item information.")
                return false;
            }
            return true;
        };
        const bool maxOrFull = (IsWindowMaximized(hWnd) || IsWindowFullScreen(hWnd));
        if (!setState(SC_RESTORE, maxOrFull)) {
            return false;
        }
        if (!setState(SC_MOVE, !maxOrFull)) {
            return false;
        }
        if (!setState(SC_SIZE, !maxOrFull)) {
            return false;
        }
        if (!setState(SC_MINIMIZE, true)) {
            return false;
        }
        if (!setState(SC_MAXIMIZE, !maxOrFull)) {
            return false;
        }
        if (!setState(SC_CLOSE, true)) {
            return false;
        }
        if (SetMenuDefaultItemFunc(menu, UINT_MAX, FALSE) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetMenuDefaultItem, L"Failed to set default menu item.")
            return false;
        }
        // ### TODO: support RTL layout: TPM_LAYOUTRTL
        const auto ret = TrackPopupMenuFunc(menu, TPM_RETURNCMD, pos.x, pos.y, 0, hWnd, nullptr);
        if (ret != 0) {
            if (PostMessageWFunc(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(PostMessageW, L"Failed to post message.")
                return false;
            }
        }
        return true;
    } else {
        OutputDebugStringW(L"GetSystemMenu(), SetMenuItemInfoW(), SetMenuDefaultItem(), TrackPopupMenu() and PostMessageW() are not available.");
        return false;
    }
}

bool Utils::EnableHiDPIScaling() noexcept
{
    USER32_API(SetProcessDpiAwarenessContext);
    SHCORE_API(SetProcessDpiAwareness);
    USER32_API(SetProcessDPIAware);
    if (SetProcessDpiAwarenessContextFunc && SetProcessDpiAwarenessFunc && SetProcessDPIAwareFunc) {
        if (SetProcessDpiAwarenessContextFunc(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != FALSE) {
            return true;
        }
        if (SetProcessDpiAwarenessContextFunc(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE) != FALSE) {
            return true;
        }
        if (SetProcessDpiAwarenessContextFunc(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE) != FALSE) {
            return true;
        }
        if (SetProcessDpiAwarenessContextFunc(DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED) != FALSE) {
            return true;
        }
        HRESULT hr = SetProcessDpiAwarenessFunc(static_cast<PROCESS_DPI_AWARENESS>(g_PROCESS_PER_MONITOR_DPI_AWARE_V2));
        if (SUCCEEDED(hr)) {
            return true;
        }
        hr = SetProcessDpiAwarenessFunc(PROCESS_PER_MONITOR_DPI_AWARE);
        if (SUCCEEDED(hr)) {
            return true;
        }
        hr = SetProcessDpiAwarenessFunc(PROCESS_SYSTEM_DPI_AWARE);
        if (SUCCEEDED(hr)) {
            return true;
        }
        if (SetProcessDPIAwareFunc() != FALSE) {
            return true;
        }
        return false;
    } else {
        OutputDebugStringW(L"SetProcessDpiAwarenessContext(), SetProcessDpiAwareness() and SetProcessDPIAware() are not available.");
        return false;
    }
}

bool Utils::UpdateFrameMargins(const HWND hWnd) noexcept
{
    DWMAPI_API(DwmExtendFrameIntoClientArea);
    if (DwmExtendFrameIntoClientAreaFunc) {
        if (!hWnd) {
            return false;
        }
        const bool maxOrFull = (IsWindowMaximized(hWnd) || IsWindowFullScreen(hWnd));
        const auto borderThickness = static_cast<int>(GetFrameBorderThickness(hWnd));
        const MARGINS margins = {0, 0, (maxOrFull ? 0 : borderThickness), 0};
        const HRESULT hr = DwmExtendFrameIntoClientAreaFunc(hWnd, &margins);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmExtendFrameIntoClientArea, hr, L"Failed to update the frame margins for the window.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DwmExtendFrameIntoClientArea() is not available.");
        return false;
    }
}
