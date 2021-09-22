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

static constexpr int g_DPI_AWARENESS_PER_MONITOR_AWARE_V2 = 3;
static constexpr int g_PROCESS_PER_MONITOR_DPI_AWARE_V2 = 3;
static constexpr int g_DPI_AWARENESS_UNAWARE_GDISCALED = 4;
static constexpr int g_PROCESS_DPI_UNAWARE_GDISCALED = 4;

static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
static constexpr DWORD g_DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37;

static constexpr wchar_t g_personalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

static constexpr UINT g_defaultResizeBorderThickness = 8;
static constexpr UINT g_defaultCaptionHeight = 23;
static constexpr UINT g_defaultTitleBarHeight = 31;
static constexpr UINT g_defaultFrameBorderThickness = 1;
static constexpr UINT g_defaultWindowDPI = USER_DEFAULT_SCREEN_DPI;

[[nodiscard]] static inline SIZE GetWindowClientSize(const HWND hWnd) noexcept
{
    USER32_API(GetClientRect);
    if (GetClientRectFunc) {
        if (!hWnd) {
            return {};
        }
        RECT rect = {0, 0, 0, 0};
        if (GetClientRectFunc(hWnd, &rect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetClientRect, L"Failed to retrieve the client area size of the window.")
            return {};
        } else {
            return {rect.right, rect.bottom};
        }
    } else {
        OutputDebugStringW(L"GetClientRect() is not available.");
        return {};
    }
}

[[nodiscard]] static inline RECT GetWindowGeometry(const HWND hWnd) noexcept
{
    USER32_API(GetWindowRect);
    if (GetWindowRectFunc) {
        if (!hWnd) {
            return {};
        }
        RECT rect = {0, 0, 0, 0};
        if (GetWindowRectFunc(hWnd, &rect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the geometry of the window.")
            return {};
        } else {
            return rect;
        }
    } else {
        OutputDebugStringW(L"GetWindowRect() is not available.");
        return {};
    }
}

[[nodiscard]] static inline POINT GetWindowPosition(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return {};
    }
    const RECT geometry = GetWindowGeometry(hWnd);
    return {geometry.left, geometry.top};
}

[[nodiscard]] static inline bool IsWindowNoState(const HWND hWnd) noexcept
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

[[nodiscard]] static inline bool IsWindowMinimized(const HWND hWnd) noexcept
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

[[nodiscard]] static inline bool IsWindowMaximized(const HWND hWnd) noexcept
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

[[nodiscard]] static inline bool IsWindowFullScreen(const HWND hWnd) noexcept
{
    USER32_API(GetMonitorInfoW);
    if (GetMonitorInfoWFunc) {
        if (!hWnd) {
            return false;
        }
        const HMONITOR mon = Utils::GetWindowScreen(hWnd, false);
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
        const RECT windowRect = GetWindowGeometry(hWnd);
        const RECT screenRect = mi.rcMonitor;
        return ((windowRect.top == screenRect.top)
                && (windowRect.bottom == screenRect.bottom)
                && (windowRect.left == screenRect.left)
                && (windowRect.right == screenRect.right));
    } else {
        OutputDebugStringW(L"GetMonitorInfoW() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool IsHighContrastModeEnabled() noexcept
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

[[nodiscard]] static inline bool ShouldAppsUseDarkMode() noexcept
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

[[nodiscard]] static inline UINT GetWindowDPI(const HWND hWnd) noexcept
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
        const HMONITOR mon = Utils::GetWindowScreen(hWnd, true);
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

[[nodiscard]] static inline UINT GetResizeBorderThickness(const HWND hWnd, const bool x) noexcept
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

[[nodiscard]] static inline UINT GetCaptionHeight(const HWND hWnd) noexcept
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

[[nodiscard]] static inline UINT GetTitleBarHeight(const HWND hWnd) noexcept
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

[[nodiscard]] static inline UINT GetFrameBorderThickness(const HWND hWnd) noexcept
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

HINSTANCE Utils::GetCurrentModuleInstance() noexcept
{
    static const auto result = HINST_THISCOMPONENT;
    return result;
}

HINSTANCE Utils::GetWindowInstance(const HWND hWnd) noexcept
{
#if (defined(WIN64) || defined(_WIN64))
    USER32_API(GetWindowLongPtrW);
    if (GetWindowLongPtrWFunc)
#else
    USER32_API(GetWindowLongW);
    if (GetWindowLongWFunc)
#endif
    {
        if (!hWnd) {
            return nullptr;
        }
        const LONG_PTR result =
#if (defined(WIN64) || defined(_WIN64))
            GetWindowLongPtrWFunc
#else
            GetWindowLongWFunc
#endif
            (hWnd, GWLP_HINSTANCE);
        if (result == 0) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowLongPtrW, L"Failed to retrieve the window's HINSTANCE.")
            return nullptr;
        } else {
            return reinterpret_cast<HINSTANCE>(result);
        }
    } else {
        OutputDebugStringW(L"GetWindowLongPtrW() is not available.");
        return nullptr;
    }
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

LPCWSTR Utils::GetWindowClassName(const HWND hWnd) noexcept
{
#if (defined(WIN64) || defined(_WIN64))
    USER32_API(GetClassLongPtrW);
    if (GetClassLongPtrWFunc)
#else
    USER32_API(GetClassLongW);
    if (GetClassLongWFunc)
#endif
    {
        if (!hWnd) {
            return nullptr;
        }
        const ULONG_PTR result =
#if (defined(WIN64) || defined(_WIN64))
            GetClassLongPtrWFunc
#else
            GetClassLongWFunc
#endif
            (hWnd, GCW_ATOM);
        if (result == 0) {
            PRINT_WIN32_ERROR_MESSAGE(GetClassLongPtrW, L"Failed to retrieve the class ATOM of the window.")
            return nullptr;
        } else {
            return GetWindowClassName(static_cast<ATOM>(result));
        }
    } else {
        OutputDebugStringW(L"GetClassLongPtrW() is not available.");
        return nullptr;
    }
}

LPCWSTR Utils::GetSystemErrorMessage(LPCWSTR function, const DWORD code) noexcept
{
    if (!function || (wcscmp(function, L"") == 0)) {
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
    if (!function || (wcscmp(function, L"") == 0)) {
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
    if (!function || (wcscmp(function, L"") == 0)) {
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
        if (text && (wcscmp(text, L"") != 0)) {
            OutputDebugStringW(text);
            MessageBoxWFunc(nullptr, text, L"Error", MB_ICONERROR | MB_OK);
        } else {
            OutputDebugStringW(L"Failed to show the message box due to the content is empty.");
        }
    } else {
        OutputDebugStringW(L"MessageBoxW() is not available.");
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
        if (UnregisterClassWFunc(GetWindowClassName(atom), GetWindowInstance(hWnd)) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW, L"Failed to unregister the window class.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DestroyWindow() and UnregisterClassW() are not available.");
        return false;
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

bool Utils::IsWindowsVersionOrGreater(const VersionNumber &version) noexcept
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = version.Major();
    osvi.dwMinorVersion = version.Minor();
    osvi.dwBuildNumber = version.Patch();
    const BYTE op = VER_GREATER_EQUAL;
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

WindowTheme Utils::GetSystemTheme() noexcept
{
    if (IsHighContrastModeEnabled()) {
        return WindowTheme::HighContrast;
    } else if (ShouldAppsUseDarkMode()) {
        return WindowTheme::Dark;
    } else {
        return WindowTheme::Light;
    }
}

bool Utils::SetWindowTheme(const HWND hWnd, const WindowTheme theme) noexcept
{
    DWMAPI_API(DwmSetWindowAttribute);
    UXTHEME_API(SetWindowTheme);
    if (DwmSetWindowAttributeFunc && SetWindowThemeFunc) {
        if (!hWnd) {
            return false;
        }
        BOOL enableDarkFrame = FALSE;
        LPCWSTR themeName = nullptr;
        switch (theme) {
        case WindowTheme::Light: {
            enableDarkFrame = FALSE;
            themeName = nullptr;
        } break;
        case WindowTheme::Dark: {
            enableDarkFrame = TRUE;
            themeName = L"Dark_Explorer";
        } break;
        case WindowTheme::HighContrast: {
            // ### TODO
        } break;
        }
        const HRESULT hr1 = DwmSetWindowAttributeFunc(hWnd, g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &enableDarkFrame, sizeof(enableDarkFrame));
        const HRESULT hr2 = DwmSetWindowAttributeFunc(hWnd, g_DWMWA_USE_IMMERSIVE_DARK_MODE, &enableDarkFrame, sizeof(enableDarkFrame));
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

WindowState Utils::GetWindowState(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return WindowState::Normal;
    }
    if (IsWindowMinimized(hWnd)) {
        return WindowState::Minimized;
    } else if (IsWindowNoState(hWnd)) {
        return WindowState::Normal;
    } else if (IsWindowMaximized(hWnd)) {
        return WindowState::Maximized;
    } else if (IsWindowFullScreen(hWnd)) {
        return WindowState::FullScreen;
    } else {
        OutputDebugStringW(L"Failed to retrieve the window state.");
        return WindowState::Normal;
    }
}

bool Utils::SetWindowState(const HWND hWnd, const WindowState state) noexcept
{
    USER32_API(ShowWindow);
    if (ShowWindowFunc) {
        if (!hWnd) {
            return false;
        }
        int nCmdShow = SW_SHOW;
        switch (state) {
        case WindowState::Minimized: {
            nCmdShow = SW_MINIMIZE;
        } break;
        case WindowState::Normal: {
            nCmdShow = SW_RESTORE;
        } break;
        case WindowState::Maximized: {
            nCmdShow = SW_MAXIMIZE;
        } break;
        case WindowState::FullScreen: {
            // ### TODO
        } break;
        }
        // Don't check it's result because it returns
        // the previous window state rather than the
        // operation result of itself.
        ShowWindowFunc(hWnd, nCmdShow);
        const DWORD dwError = GetLastError();
        if (dwError != ERROR_SUCCESS) {
            //
            return false;
        } else {
            return true;
        }
    } else {
        OutputDebugStringW(L"ShowWindow() is not available.");
        return false;
    }
}

DPIAwareness Utils::GetProcessDPIAwareness() noexcept
{
    USER32_API(GetThreadDpiAwarenessContext);
    USER32_API(GetAwarenessFromDpiAwarenessContext);
    if (GetThreadDpiAwarenessContextFunc && GetAwarenessFromDpiAwarenessContextFunc) {
        const DPI_AWARENESS_CONTEXT context = GetThreadDpiAwarenessContextFunc();
        if (context) {
            const auto awareness = static_cast<int>(GetAwarenessFromDpiAwarenessContextFunc(context));
            switch (awareness) {
            case g_DPI_AWARENESS_PER_MONITOR_AWARE_V2: {
                return DPIAwareness::PerMonitorV2;
            } break;
            case DPI_AWARENESS_PER_MONITOR_AWARE: {
                return DPIAwareness::PerMonitor;
            } break;
            case DPI_AWARENESS_SYSTEM_AWARE: {
                return DPIAwareness::System;
            } break;
            case g_DPI_AWARENESS_UNAWARE_GDISCALED: {
                return DPIAwareness::GdiScaled;
            } break;
            case DPI_AWARENESS_UNAWARE: {
                return DPIAwareness::Unaware;
            } break;
            case DPI_AWARENESS_INVALID: {
                PRINT_WIN32_ERROR_MESSAGE(GetAwarenessFromDpiAwarenessContext, L"Failed to extract the DPI awareness from the context.")
                return DPIAwareness::Unaware;
            } break;
            }
            return DPIAwareness::Unaware;
        } else {
            PRINT_WIN32_ERROR_MESSAGE(GetThreadDpiAwarenessContext, L"Failed to retrieve the DPI awareness context of the current thread.")
            return DPIAwareness::Unaware;
        }
    } else {
        OutputDebugStringW(L"GetThreadDpiAwarenessContext() and GetAwarenessFromDpiAwarenessContext() are not available.");
        SHCORE_API(GetProcessDpiAwareness);
        if (GetProcessDpiAwarenessFunc) {
            int awareness = 0;
            const HRESULT hr = GetProcessDpiAwarenessFunc(nullptr, reinterpret_cast<PROCESS_DPI_AWARENESS *>(&awareness));
            if (SUCCEEDED(hr)) {
                switch (awareness) {
                case g_PROCESS_PER_MONITOR_DPI_AWARE_V2: {
                    return DPIAwareness::PerMonitorV2;
                } break;
                case PROCESS_PER_MONITOR_DPI_AWARE: {
                    return DPIAwareness::PerMonitor;
                } break;
                case PROCESS_SYSTEM_DPI_AWARE: {
                    return DPIAwareness::System;
                } break;
                case g_PROCESS_DPI_UNAWARE_GDISCALED: {
                    return DPIAwareness::GdiScaled;
                } break;
                case PROCESS_DPI_UNAWARE: {
                    return DPIAwareness::Unaware;
                } break;
                }
                return DPIAwareness::Unaware;
            } else {
                PRINT_HR_ERROR_MESSAGE(GetProcessDpiAwareness, hr, L"Failed to retrieve the DPI awareness of the current process.")
                return DPIAwareness::Unaware;
            }
        } else {
            OutputDebugStringW(L"GetProcessDpiAwareness() is not available.");
            USER32_API(IsProcessDPIAware);
            if (IsProcessDPIAwareFunc) {
                if (IsProcessDPIAwareFunc() == FALSE) {
                    return DPIAwareness::Unaware;
                } else {
                    return DPIAwareness::System;
                }
            } else {
                OutputDebugStringW(L"IsProcessDPIAware() is not available.");
                return DPIAwareness::Unaware;
            }
        }
    }
}

bool Utils::SetProcessDPIAwareness(const DPIAwareness dpiAwareness) noexcept
{
    DPI_AWARENESS_CONTEXT dac = DPI_AWARENESS_CONTEXT_UNAWARE;
    PROCESS_DPI_AWARENESS pda = PROCESS_DPI_UNAWARE;
    switch (dpiAwareness) {
    case DPIAwareness::PerMonitorV2: {
        dac = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
        pda = static_cast<PROCESS_DPI_AWARENESS>(g_PROCESS_PER_MONITOR_DPI_AWARE_V2);
    } break;
    case DPIAwareness::PerMonitor: {
        dac = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
        pda = PROCESS_PER_MONITOR_DPI_AWARE;
    } break;
    case DPIAwareness::System: {
        dac = DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
        pda = PROCESS_SYSTEM_DPI_AWARE;
    } break;
    case DPIAwareness::GdiScaled: {
        dac = DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED;
        pda = static_cast<PROCESS_DPI_AWARENESS>(g_PROCESS_DPI_UNAWARE_GDISCALED);
    } break;
    case DPIAwareness::Unaware: {
        dac = DPI_AWARENESS_CONTEXT_UNAWARE;
        pda = PROCESS_DPI_UNAWARE;
    } break;
    }
    USER32_API(SetProcessDpiAwarenessContext);
    if (SetProcessDpiAwarenessContextFunc) {
        if (SetProcessDpiAwarenessContextFunc(dac) == FALSE) {
            //PRINT_WIN32_ERROR_MESSAGE(SetProcessDpiAwarenessContext, L"Failed to set DPI awareness for the process.")
            return false;
        } else {
            return true;
        }
    } else {
        OutputDebugStringW(L"SetProcessDpiAwarenessContext() is not available.");
        SHCORE_API(SetProcessDpiAwareness);
        if (SetProcessDpiAwarenessFunc) {
            const HRESULT hr = SetProcessDpiAwarenessFunc(pda);
            if (FAILED(hr)) {
                //PRINT_HR_ERROR_MESSAGE(SetProcessDpiAwareness, hr, L"Failed to set DPI awareness for the process.")
                return false;
            } else {
                return true;
            }
        } else {
            OutputDebugStringW(L"SetProcessDpiAwareness() is not available.");
            USER32_API(SetProcessDPIAware);
            if (SetProcessDPIAwareFunc) {
                if (SetProcessDPIAwareFunc() == FALSE) {
                    //PRINT_WIN32_ERROR_MESSAGE(SetProcessDPIAware, L"Failed to set DPI awareness for the process.")
                    return false;
                } else {
                    return true;
                }
            } else {
                OutputDebugStringW(L"SetProcessDPIAware() is not available.");
                return false;
            }
        }
    }
}

UINT Utils::GetWindowMetrics(const HWND hWnd, const WindowMetrics metrics) noexcept
{
    if (!hWnd) {
        return 0;
    }
    switch (metrics) {
    case WindowMetrics::X: {
        return GetWindowPosition(hWnd).x;
    } break;
    case WindowMetrics::Y: {
        return GetWindowPosition(hWnd).y;
    } break;
    case WindowMetrics::Width: {
        return GetWindowClientSize(hWnd).cx;
    } break;
    case WindowMetrics::Height: {
        return GetWindowClientSize(hWnd).cy;
    } break;
    case WindowMetrics::FrameWidth: {
        const RECT geometry = GetWindowGeometry(hWnd);
        return std::abs(geometry.right - geometry.left);
    } break;
    case WindowMetrics::FrameHeight: {
        const RECT geometry = GetWindowGeometry(hWnd);
        return std::abs(geometry.bottom - geometry.top);
    } break;
    case WindowMetrics::DotsPerInch: {
        return GetWindowDPI(hWnd);
    } break;
    case WindowMetrics::ResizeBorderThicknessX: {
        return GetResizeBorderThickness(hWnd, true);
    } break;
    case WindowMetrics::ResizeBorderThicknessY: {
        return GetResizeBorderThickness(hWnd, false);
    } break;
    case WindowMetrics::CaptionHeight: {
        return GetCaptionHeight(hWnd);
    } break;
    case WindowMetrics::TitleBarHeight: {
        return GetTitleBarHeight(hWnd);
    } break;
    case WindowMetrics::FrameBorderThickness: {
        return GetFrameBorderThickness(hWnd);
    } break;
    }
    return 0;
}
