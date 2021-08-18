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

#include "acrylicmanager.h"
#include "acrylicmanager_p.h"

#include <shobjidl_core.h>
#include <wininet.h>
#include <shlobj_core.h>
#include <atlbase.h>

#include <ShellApi.h>
#include <ShellScalingApi.h>
#include <UxTheme.h>
#include <DwmApi.h>

#include <Unknwn.h>

#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

#include <D2D1.h>
#include <WinCodec.h>

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT (reinterpret_cast<HINSTANCE>(&__ImageBase))
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI (96)
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER (92)
#endif

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED (0x02E0)
#endif

#ifndef IsMaximized
#define IsMaximized(window) (IsZoomed(window))
#endif

#ifndef IsMinimized
#define IsMinimized(window) (IsIconic(window))
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#ifndef RECT_WIDTH
#define RECT_WIDTH(rect) (std::abs((rect).right - (rect).left))
#endif

#ifndef RECT_HEIGHT
#define RECT_HEIGHT(rect) (std::abs((rect).bottom - (rect).top))
#endif

#ifndef BLACK_BACKGROUND_BRUSH
#define BLACK_BACKGROUND_BRUSH (reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)))
#endif

#ifndef CURRENT_SCREEN
#define CURRENT_SCREEN(window) (MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST))
#endif

#ifndef PRINT_ERROR_MESSAGE
#define PRINT_ERROR_MESSAGE(function) \
{ \
    const DWORD dwError = GetLastError(); \
    if (dwError != ERROR_SUCCESS) { \
        const HRESULT hr = HRESULT_FROM_WIN32(dwError); \
        if (FAILED(hr)) { \
            const HRESULT _hr = am_PrintErrorMessageFromHResult_p(L#function, hr); \
            static_cast<void>(_hr); \
        } \
    } \
}
#endif

// The thickness of an auto-hide taskbar in pixels.
static const int g_am_AutoHideTaskbarThicknessPx_p = 2;
static const int g_am_AutoHideTaskbarThicknessPy_p = g_am_AutoHideTaskbarThicknessPx_p;

static LPCWSTR g_am_PersonalizeRegistryKey_p = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";
static LPCWSTR g_am_DWMRegistryKey_p = LR"(Software\Microsoft\Windows\DWM)";
static LPCWSTR g_am_DesktopRegistryKey_p = LR"(Control Panel\Desktop)";
static LPCWSTR g_am_WindowClassNamePrefix_p = LR"(wangwenx190\AcrylicManager\WindowClass\)";
static LPCWSTR g_am_MainWindowClassNameSuffix_p = L"@MainWindow";
static LPCWSTR g_am_DragBarWindowClassNameSuffix_p = L"@DragBarWindow";

static LPWSTR g_am_MainWindowClassName_p = nullptr;
static LPWSTR g_am_DragBarWindowClassName_p = nullptr;
static LPCWSTR g_am_MainWindowTitle_p = L"AcrylicManager Main Window";
static LPCWSTR g_am_DragBarWindowTitle_p = nullptr;
static ATOM g_am_MainWindowAtom_p = 0;
static ATOM g_am_DragBarWindowAtom_p = 0;
static HWND g_am_MainWindowHandle_p = nullptr;
static HWND g_am_XAMLIslandWindowHandle_p = nullptr;
static HWND g_am_DragBarWindowHandle_p = nullptr;
static UINT g_am_CurrentDpi_p = 0;
static SystemTheme g_am_BrushTheme_p = SystemTheme::Invalid;
static LPWSTR g_am_WallpaperFilePath_p = nullptr;
static COLORREF g_am_DesktopBackgroundColor_p = RGB(0, 0, 0);
static WallpaperAspectStyle g_am_WallpaperAspectStyle_p = WallpaperAspectStyle::Invalid;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_am_XAMLManager_p = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_am_XAMLSource_p = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_am_RootGrid_p = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_am_BackgroundBrush_p = nullptr;

static CComPtr<ID2D1Factory> g_am_D2DFactory_p = nullptr;
static CComPtr<ID2D1HwndRenderTarget> g_am_D2DRenderTarget_p = nullptr;
static CComPtr<ID2D1BitmapBrush> g_am_D2DBitmapBrush_p = nullptr;
static CComPtr<ID2D1Bitmap> g_am_D2DWallpaperBitmap_p = nullptr;

static const bool g_am_IsWindows7OrGreater_p = []{
    bool result = false;
    return (SUCCEEDED(am_CompareSystemVersion_p(WindowsVersion::Windows7, VersionCompare::GreaterOrEqual, &result)) && result);
}();

static const bool g_am_IsWindows8OrGreater_p = []{
    bool result = false;
    return (SUCCEEDED(am_CompareSystemVersion_p(WindowsVersion::Windows8, VersionCompare::GreaterOrEqual, &result)) && result);
}();

static const bool g_am_IsWindows8Point1OrGreater_p = []{
    bool result = false;
    return (SUCCEEDED(am_CompareSystemVersion_p(WindowsVersion::Windows8_1, VersionCompare::GreaterOrEqual, &result)) && result);
}();

static const bool g_am_IsWindows10OrGreater_p = []{
    bool result = false;
    return (SUCCEEDED(am_CompareSystemVersion_p(WindowsVersion::Windows10, VersionCompare::GreaterOrEqual, &result)) && result);
}();

static const bool g_am_IsDarkModeAvailable_p = []{
    bool result = false;
    return (SUCCEEDED(am_CompareSystemVersion_p(WindowsVersion::Windows10_1607, VersionCompare::GreaterOrEqual, &result)) && result);
}();

static const bool g_am_IsXAMLIslandAvailable_p = []{
    bool result = false;
    return (SUCCEEDED(am_CompareSystemVersion_p(WindowsVersion::Windows10_19H1, VersionCompare::GreaterOrEqual, &result)) && result);
}();

static inline void am_Print_p(LPCWSTR text, const bool showUi = false, LPCWSTR title = nullptr)
{
    if (!text) {
        return;
    }
    OutputDebugStringW(text);
    if (showUi) {
        MessageBoxW(nullptr, text, (title ? title : L"Error"), MB_ICONERROR | MB_OK);
    }
}

[[nodiscard]] static inline HRESULT am_GetDevicePixelRatio_p(const UINT dpi, double *dpr)
{
    if ((dpi == 0) || !dpr) {
        return E_INVALIDARG;
    }
    *dpr = (static_cast<double>(dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
    return S_OK;
}

HRESULT am_GetWindowDpi_p(const HWND hWnd, UINT *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    {
        const UINT dpi = GetDpiForWindow(hWnd);
        if (dpi > 0) {
            *result = dpi;
            return S_OK;
        }
    }
    {
        const UINT dpi = GetSystemDpiForProcess(GetCurrentProcess());
        if (dpi > 0) {
            *result = dpi;
            return S_OK;
        }
    }
    {
        const UINT dpi = GetDpiForSystem();
        if (dpi > 0) {
            *result = dpi;
            return S_OK;
        }
    }
    {
        UINT dpiX = 0, dpiY = 0;
        if (SUCCEEDED(GetDpiForMonitor(CURRENT_SCREEN(hWnd), MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
            if ((dpiX > 0) && (dpiY > 0)) {
                *result = std::round(static_cast<double>(dpiX + dpiY) / 2.0);
                return S_OK;
            }
        }

    }
    {
        const HDC hdc = GetDC(nullptr);
        if (hdc) {
            const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(nullptr, hdc);
            if ((dpiX > 0) && (dpiY > 0)) {
                *result = std::round(static_cast<double>(dpiX + dpiY) / 2.0);
                return S_OK;
            }
        }
    }
    *result = USER_DEFAULT_SCREEN_DPI;
    return S_OK;
}

HRESULT am_IsMinimized_p(const HWND hWnd, bool *min)
{
    if (!hWnd || !min) {
        return E_INVALIDARG;
    }
    *min = (IsMinimized(hWnd) != FALSE);
    return S_OK;
}

HRESULT am_IsMaximized_p(const HWND hWnd, bool *max)
{
    if (!hWnd || !max) {
        return E_INVALIDARG;
    }
    *max = (IsMaximized(hWnd) != FALSE);
    return S_OK;
}

HRESULT am_GetResizeBorderThickness_p(const bool x, const UINT dpi, int *thickness)
{
    if ((dpi == 0) || !thickness) {
        return E_INVALIDARG;
    }
    // There is no "SM_CYPADDEDBORDER".
    const int result = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi)
            + GetSystemMetricsForDpi((x ? SM_CXSIZEFRAME : SM_CYSIZEFRAME), dpi);
    double dpr = 1.0;
    if (FAILED(am_GetDevicePixelRatio_p(dpi, &dpr))) {
        return E_FAIL;
    }
    const int preset = std::round(8.0 * dpr);
    *thickness = ((result > 0) ? result : preset);
    return S_OK;
}

HRESULT am_GetCaptionHeight_p(const UINT dpi, int *height)
{
    if ((dpi == 0) || !height) {
        return E_INVALIDARG;
    }
    const int result = GetSystemMetricsForDpi(SM_CYCAPTION, dpi);
    double dpr = 1.0;
    if (FAILED(am_GetDevicePixelRatio_p(dpi, &dpr))) {
        return E_FAIL;
    }
    const int preset = std::round(23.0 * dpr);
    *height = ((result > 0) ? result : preset);
    return S_OK;
}

HRESULT am_GetTitleBarHeight_p(const HWND hWnd, const UINT dpi, int *height)
{
    if (!hWnd || (dpi == 0) || !height) {
        return E_INVALIDARG;
    }
    RECT frame = {0, 0, 0, 0};
    if (AdjustWindowRectExForDpi(&frame,
                             (static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)) & ~WS_OVERLAPPED),
                             FALSE,
                             static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)),
                             dpi) != FALSE) {
        *height = std::abs(frame.top);
        return S_OK;
    }
    int rbtY = 0, cth = 0;
    if (SUCCEEDED(am_GetResizeBorderThickness_p(false, dpi, &rbtY)) && SUCCEEDED(am_GetCaptionHeight_p(dpi, &cth))) {
        *height = (rbtY + cth);
        return S_OK;
    }
    double dpr = 1.0;
    if (SUCCEEDED(am_GetDevicePixelRatio_p(dpi, &dpr))) {
        *height = std::round(31.0 * dpr);
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_GetMonitorInfo_p(const HWND hWnd, MONITORINFO *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    const HMONITOR mon = CURRENT_SCREEN(hWnd);
    if (!mon) {
        return E_FAIL;
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        return E_FAIL;
    }
    *result = mi;
    return S_OK;
}

HRESULT am_GetScreenGeometry_p(const HWND hWnd, RECT *rect)
{
    if (!hWnd || !rect) {
        return E_INVALIDARG;
    }
    MONITORINFO mi = {};
    if (FAILED(am_GetMonitorInfo_p(hWnd, &mi))) {
        return E_FAIL;
    }
    *rect = mi.rcMonitor;
    return S_OK;
}

HRESULT am_GetScreenAvailableGeometry_p(const HWND hWnd, RECT *rect)
{
    if (!hWnd || !rect) {
        return E_INVALIDARG;
    }
    MONITORINFO mi = {};
    if (FAILED(am_GetMonitorInfo_p(hWnd, &mi))) {
        return E_FAIL;
    }
    *rect = mi.rcWork;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetDWORDValueFromRegistry_p(const HKEY rootKey, LPCWSTR subKey,
                                                                 LPCWSTR valueName, DWORD *result)
{
    if (!rootKey || !subKey || !valueName || !result) {
        return E_INVALIDARG;
    }
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(rootKey, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        PRINT_ERROR_MESSAGE(RegOpenKeyExW)
        return E_FAIL;
    }
    DWORD dwValue = 0;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(dwValue);
    const bool success = (RegQueryValueExW(hKey, valueName, nullptr, &dwType,
                                reinterpret_cast<LPBYTE>(&dwValue), &dwSize) == ERROR_SUCCESS);
    if (!success) {
        PRINT_ERROR_MESSAGE(RegQueryValueExW)
    }
    if (RegCloseKey(hKey) != ERROR_SUCCESS) {
        PRINT_ERROR_MESSAGE(RegCloseKey)
    }
    *result = dwValue;
    return S_OK;
}

HRESULT am_IsCompositionEnabled_p(bool *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (g_am_IsWindows8OrGreater_p) {
        *result = true;
        return S_OK;
    }
    BOOL enabled = FALSE;
    if (SUCCEEDED(DwmIsCompositionEnabled(&enabled))) {
        *result = (enabled != FALSE);
        return S_OK;
    }
    DWORD dwmComp = 0;
    if (SUCCEEDED(am_GetDWORDValueFromRegistry_p(HKEY_CURRENT_USER, g_am_DWMRegistryKey_p, L"Composition", &dwmComp))) {
        *result = (dwmComp != 0);
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_IsFullScreened_p(const HWND hWnd, bool *full)
{
    if (!hWnd || !full) {
        return E_INVALIDARG;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &windowRect) == FALSE) {
        return E_FAIL;
    }
    const HMONITOR mon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    if (!mon) {
        return E_FAIL;
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        return E_FAIL;
    }
    const RECT screenRect = mi.rcMonitor;
    *full = ((windowRect.left == screenRect.left)
             && (windowRect.right == screenRect.right)
             && (windowRect.top == screenRect.top)
             && (windowRect.bottom == screenRect.bottom));
    return S_OK;
}

HRESULT am_IsWindowNoState_p(const HWND hWnd, bool *normal)
{
    if (!hWnd || !normal) {
        return E_INVALIDARG;
    }
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    if (GetWindowPlacement(hWnd, &wp) == FALSE) {
        return E_FAIL;
    }
    *normal = (wp.showCmd == SW_NORMAL);
    return S_OK;
}

HRESULT am_IsWindowVisible_p(const HWND hWnd, bool *visible)
{
    if (!hWnd || !visible) {
        return E_INVALIDARG;
    }
    *visible = (IsWindowVisible(hWnd) != FALSE);
    return S_OK;
}

HRESULT am_TriggerFrameChange_p(const HWND hWnd)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    const BOOL result = SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (result != FALSE) {
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_SetWindowTransitionsEnabled_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    const BOOL disabled = enable ? FALSE : TRUE;
    return DwmSetWindowAttribute(hWnd, DWMWAEX_TRANSITIONS_FORCEDISABLED, &disabled, sizeof(disabled));
}

HRESULT am_OpenSystemMenu_p(const HWND hWnd, const POINT pos)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    const HMENU menu = GetSystemMenu(hWnd, FALSE);
    if (!menu) {
        return E_FAIL;
    }
    // Update the options based on window state.
    MENUITEMINFOW mii;
    SecureZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fType = MFT_STRING;
    const auto setState = [&mii, menu](const UINT item, const bool enabled) -> bool {
        mii.fState = enabled ? MF_ENABLED : MF_DISABLED;
        return (SetMenuItemInfoW(menu, item, FALSE, &mii) != FALSE);
    };
    bool isMaximized = false;
    if (FAILED(am_IsMaximized_p(hWnd, &isMaximized))) {
        return E_FAIL;
    }
    if (!setState(SC_RESTORE, isMaximized)) {
        return E_FAIL;
    }
    if (!setState(SC_MOVE, !isMaximized)) {
        return E_FAIL;
    }
    if (!setState(SC_SIZE, !isMaximized)) {
        return E_FAIL;
    }
    if (!setState(SC_MINIMIZE, true)) {
        return E_FAIL;
    }
    if (!setState(SC_MAXIMIZE, !isMaximized)) {
        return E_FAIL;
    }
    if (!setState(SC_CLOSE, true)) {
        return E_FAIL;
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        return E_FAIL;
    }
    // ### TODO: support LTR layout.
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, pos.x, pos.y, 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            return E_FAIL;
        }
    }
    return S_OK;
}

HRESULT am_CompareSystemVersion_p(const WindowsVersion ver, const VersionCompare comp, bool *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    switch (ver) {
    case WindowsVersion::WindowsVista: {
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 6000; // Windows Vista with Service Pack 1: 6001
    } break;
    case WindowsVersion::Windows7: {
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 1;
        osvi.dwBuildNumber = 7600; // Windows 7 with Service Pack 1: 7601
    } break;
    case WindowsVersion::Windows8: {
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 2;
        osvi.dwBuildNumber = 9200;
    } break;
    case WindowsVersion::Windows8_1: {
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 3;
        osvi.dwBuildNumber = 9200; // Windows 8.1 with Update 1: 9600
    } break;
    case WindowsVersion::Windows10_1507: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 10240;
    } break;
    case WindowsVersion::Windows10_1511: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 10586;
    } break;
    case WindowsVersion::Windows10_1607: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 14393;
    } break;
    case WindowsVersion::Windows10_1703: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 15063;
    } break;
    case WindowsVersion::Windows10_1709: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 16299;
    } break;
    case WindowsVersion::Windows10_1803: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 17134;
    } break;
    case WindowsVersion::Windows10_1809: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 17763;
    } break;
    case WindowsVersion::Windows10_1903: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 18362;
    } break;
    case WindowsVersion::Windows10_1909: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 18363;
    } break;
    case WindowsVersion::Windows10_2004: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 19041;
    } break;
    case WindowsVersion::Windows10_20H2: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 19042;
    } break;
    case WindowsVersion::Windows10_21H1: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 19043;
    } break;
    case WindowsVersion::Windows11: {
        // FIXME: check the actual version number of Win11.
        osvi.dwMajorVersion = 11;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 0;
    } break;
    }
    BYTE op = 0;
    switch (comp) {
    case VersionCompare::Less:
        op = VER_LESS;
        break;
    case VersionCompare::Equal:
        op = VER_EQUAL;
        break;
    case VersionCompare::Greater:
        op = VER_GREATER;
        break;
    case VersionCompare::LessOrEqual:
        op = VER_LESS_EQUAL;
        break;
    case VersionCompare::GreaterOrEqual:
        op = VER_GREATER_EQUAL;
        break;
    }
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    *result = (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
    return S_OK;
}

HRESULT am_GetWindowVisibleFrameBorderThickness_p(const HWND hWnd, const UINT dpi, int *result)
{
    if (!hWnd || (dpi == 0) || !result) {
        return E_INVALIDARG;
    }
    UINT value = 0;
    if (SUCCEEDED(DwmGetWindowAttribute(hWnd, DWMWAEX_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value)))) {
        *result = value;
        return S_OK;
    }
    double dpr = 1.0;
    if (FAILED(am_GetDevicePixelRatio_p(dpi, &dpr))) {
        return E_FAIL;
    }
    bool normal = false;
    if (FAILED(am_IsWindowNoState_p(hWnd, &normal))) {
        return E_FAIL;
    }
    *result = (normal ? std::round(1.0 * dpr) : 0);
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_UpdateFrameMargins_p(const HWND hWnd, const UINT dpi)
{
    if (!hWnd || (dpi == 0)) {
        return E_INVALIDARG;
    }
    bool normal = false;
    if (FAILED(am_IsWindowNoState_p(hWnd, &normal))) {
        return E_FAIL;
    }
    int thickness = 0;
    if (FAILED(am_GetWindowVisibleFrameBorderThickness_p(hWnd, dpi, &thickness))) {
        return E_FAIL;
    }
    const LONG topFrameMargin = (normal ? thickness : 0);
    // We removed the whole top part of the frame (see handling of
    // WM_NCCALCSIZE) so the top border is missing now. We add it back here.
    // Note #1: You might wonder why we don't remove just the title bar instead
    //  of removing the whole top part of the frame and then adding the little
    //  top border back. I tried to do this but it didn't work: DWM drew the
    //  whole title bar anyways on top of the window. It seems that DWM only
    //  wants to draw either nothing or the whole top part of the frame.
    // Note #2: For some reason if you try to set the top margin to just the
    //  top border height (what we want to do), then there is a transparency
    //  bug when the window is inactive, so I've decided to add the whole top
    //  part of the frame instead and then we will hide everything that we
    //  don't need (that is, the whole thing but the little 1 pixel wide border
    //  at the top) in the WM_PAINT handler. This eliminates the transparency
    //  bug and it's what a lot of Win32 apps that customize the title bar do
    //  so it should work fine.
    const MARGINS margins = {0, 0, topFrameMargin, 0};
    return DwmExtendFrameIntoClientArea(hWnd, &margins);
}

[[nodiscard]] static inline HRESULT am_ShowFullScreen_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    auto style = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE));
    style &= ~(WS_OVERLAPPEDWINDOW); // fixme: check
    style |= WS_POPUP;
    SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    if (enable) {
        // fixme: rethink
        RECT rect = {};
        if (FAILED(am_GetScreenGeometry_p(hWnd, &rect))) {
            return E_FAIL;
        }
        if (MoveWindow(hWnd, rect.left, rect.top,
                        RECT_WIDTH(rect), RECT_HEIGHT(rect), TRUE) != FALSE) {
            return S_OK;
        }
    } else {
        // todo
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_GetSystemTheme_p(SystemTheme *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    bool highContrast = false;
    if (FAILED(am_IsHighContrastModeOn_p(&highContrast))) {
        return E_FAIL;
    }
    // Dark mode was first introduced in Windows 10 1607.
    if (g_am_IsDarkModeAvailable_p) {
        bool lightModeOn = false;
        if (FAILED(am_ShouldAppsUseLightTheme_p(&lightModeOn))) {
            if (FAILED(am_ShouldSystemUsesLightTheme_p(&lightModeOn))) {
                return E_FAIL;
            }
        }
        *result = (lightModeOn ? SystemTheme::Light : SystemTheme::Dark);
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_GetWindowGeometry_p(const HWND hWnd, RECT *rect)
{
    if (!hWnd || !rect) {
        return E_INVALIDARG;
    }
    RECT geo = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &geo) == FALSE) {
        return E_FAIL;
    }
    *rect = geo;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetWindowSize_p(const HWND hWnd, SIZE *size)
{
    if (!hWnd || !size) {
        return E_INVALIDARG;
    }
    RECT rect = {};
    if (FAILED(am_GetWindowGeometry_p(hWnd, &rect))) {
        return E_FAIL;
    }
    *size = {RECT_WIDTH(rect), RECT_HEIGHT(rect)};
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_MoveWindow_p(const HWND hWnd, const int x, const int y)
{
    if (!hWnd || (x <= 0) || (y <= 0)) {
        return E_INVALIDARG;
    }
    SIZE size = {};
    if (FAILED(am_GetWindowSize_p(hWnd, &size))) {
        return E_FAIL;
    }
    if (MoveWindow(hWnd, x, y, size.cx, size.cy, TRUE) != FALSE) {
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_ResizeWindow_p(const HWND hWnd, const int w, const int h)
{
    if (!hWnd || (w <= 0) || (h <= 0)) {
        return E_INVALIDARG;
    }
    RECT rect = {};
    if (FAILED(am_GetWindowGeometry_p(hWnd, &rect))) {
        return E_FAIL;
    }
    if (MoveWindow(hWnd, rect.left, rect.top, w, h, TRUE) != FALSE) {
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_CenterWindow_p(const HWND hWnd)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &windowRect) == FALSE) {
        return E_FAIL;
    }
    const int windowWidth = RECT_WIDTH(windowRect);
    const int windowHeight = RECT_HEIGHT(windowRect);
    RECT screenRect = {};
    if (FAILED(am_GetScreenGeometry_p(hWnd, &screenRect))) {
        return E_FAIL;
    }
    const int screenWidth = RECT_WIDTH(screenRect);
    const int screenHeight = RECT_HEIGHT(screenRect);
    const int newX = (screenWidth - windowWidth) / 2;
    const int newY = (screenHeight - windowHeight) / 2;
    if (MoveWindow(hWnd, newX, newY, windowWidth, windowHeight, TRUE) != FALSE) {
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_GetWindowState_p(const HWND hWnd, WindowState *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    bool state = false;
    if (SUCCEEDED(am_IsFullScreened_p(hWnd, &state)) && state) {
        *result = WindowState::FullScreened;
        return S_OK;
    } else if (SUCCEEDED(am_IsMaximized_p(hWnd, &state)) && state) {
        *result = WindowState::Maximized;
        return S_OK;
    } else if (SUCCEEDED(am_IsMinimized_p(hWnd, &state)) && state) {
        *result = WindowState::Minimized;
        return S_OK;
    } else if (SUCCEEDED(am_IsWindowNoState_p(hWnd, &state)) && state) {
        *result = WindowState::Normal;
        return S_OK;
    } else if (SUCCEEDED(am_IsWindowVisible_p(hWnd, &state)) && state) {
        *result = WindowState::Shown;
        return S_OK;
    } else if (SUCCEEDED(am_IsWindowVisible_p(hWnd, &state)) && !state) {
        *result = WindowState::Hidden;
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_SetWindowState_p(const HWND hWnd, const WindowState state)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    switch (state) {
    case WindowState::Normal:
        ShowWindow(hWnd, SW_NORMAL);
        return S_OK;
    case WindowState::Maximized:
        ShowWindow(hWnd, SW_MAXIMIZE);
        return S_OK;
    case WindowState::Minimized:
        ShowWindow(hWnd, SW_MINIMIZE);
        return S_OK;
    case WindowState::FullScreened:
        return am_ShowFullScreen_p(hWnd, true);
    case WindowState::Hidden:
        ShowWindow(hWnd, SW_HIDE);
        return S_OK;
    case WindowState::Shown:
        ShowWindow(hWnd, SW_SHOW);
        return S_OK;
    default:
        break;
    }
    return E_FAIL;
}

static inline void am_Cleanup_p()
{
    // Direct2D
    if (g_am_D2DWallpaperBitmap_p) {
        g_am_D2DWallpaperBitmap_p.Release();
        g_am_D2DWallpaperBitmap_p = nullptr;
    }
    if (g_am_D2DBitmapBrush_p) {
        g_am_D2DBitmapBrush_p.Release();
        g_am_D2DBitmapBrush_p = nullptr;
    }
    if (g_am_D2DRenderTarget_p) {
        g_am_D2DRenderTarget_p.Release();
        g_am_D2DRenderTarget_p = nullptr;
    }
    if (g_am_D2DFactory_p) {
        g_am_D2DFactory_p.Release();
        g_am_D2DFactory_p = nullptr;
    }
    if (g_am_WallpaperFilePath_p) {
        delete [] g_am_WallpaperFilePath_p;
        g_am_WallpaperFilePath_p = nullptr;
    }
    g_am_DesktopBackgroundColor_p = RGB(0, 0, 0);
    g_am_WallpaperAspectStyle_p = WallpaperAspectStyle::Invalid;

    // XAML Island
    if (g_am_XAMLSource_p) {
        g_am_XAMLSource_p.Close();
        g_am_XAMLSource_p = nullptr;
    }
    g_am_RootGrid_p = nullptr;
    g_am_BackgroundBrush_p = nullptr;
    if (g_am_XAMLManager_p) {
        g_am_XAMLManager_p.Close();
        g_am_XAMLManager_p = nullptr;
    }

    // Drag bar window
    if (g_am_DragBarWindowHandle_p) {
        if (DestroyWindow(g_am_DragBarWindowHandle_p) == FALSE) {
            PRINT_ERROR_MESSAGE(DestroyWindow)
        }
        g_am_DragBarWindowHandle_p = nullptr;
    }
    if (g_am_DragBarWindowAtom_p != 0) {
        if (g_am_DragBarWindowClassName_p) {
            if (UnregisterClassW(g_am_DragBarWindowClassName_p, HINST_THISCOMPONENT) == FALSE) {
                PRINT_ERROR_MESSAGE(UnregisterClassW)
            }
            delete [] g_am_DragBarWindowClassName_p;
            g_am_DragBarWindowClassName_p = nullptr;
        }
        g_am_DragBarWindowAtom_p = 0;
    }

    // Main window
    if (g_am_MainWindowHandle_p) {
        if (DestroyWindow(g_am_MainWindowHandle_p) == FALSE) {
            PRINT_ERROR_MESSAGE(DestroyWindow)
        }
        g_am_MainWindowHandle_p = nullptr;
    }
    if (g_am_MainWindowAtom_p != 0) {
        if (g_am_MainWindowClassName_p) {
            if (UnregisterClassW(g_am_MainWindowClassName_p, HINST_THISCOMPONENT) == FALSE) {
                PRINT_ERROR_MESSAGE(UnregisterClassW)
            }
            delete [] g_am_MainWindowClassName_p;
            g_am_MainWindowClassName_p = nullptr;
        }
        g_am_MainWindowAtom_p = 0;
    }

    // Misc
    g_am_CurrentDpi_p = 0;
    g_am_BrushTheme_p = SystemTheme::Invalid;
}

[[nodiscard]] static inline HRESULT am_GetTintColor_p(int *r, int *g, int *b, int *a)
{
    if (!g_am_BackgroundBrush_p || !r || !g || !b || !a) {
        return E_INVALIDARG;
    }
    const winrt::Windows::UI::Color color = g_am_BackgroundBrush_p.TintColor();
    *r = static_cast<int>(color.R);
    *g = static_cast<int>(color.G);
    *b = static_cast<int>(color.B);
    *a = static_cast<int>(color.A);
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetTintColor_p(const int r, const int g, const int b, const int a)
{
    if (!g_am_BackgroundBrush_p) {
        return E_INVALIDARG;
    }
    const auto red = static_cast<uint8_t>(std::clamp(r, 0, 255));
    const auto green = static_cast<uint8_t>(std::clamp(g, 0, 255));
    const auto blue = static_cast<uint8_t>(std::clamp(b, 0, 255));
    const auto alpha = static_cast<uint8_t>(std::clamp(a, 0, 255));
    g_am_BackgroundBrush_p.TintColor({alpha, red, green, blue}); // ARGB
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetTintOpacity_p(double *opacity)
{
    if (!g_am_BackgroundBrush_p || !opacity) {
        return E_INVALIDARG;
    }
    const double value = g_am_BackgroundBrush_p.TintOpacity();
    *opacity = value;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetTintOpacity_p(const double opacity)
{
    if (!g_am_BackgroundBrush_p) {
        return E_INVALIDARG;
    }
    const double value = std::clamp(opacity, 0.0, 1.0);
    g_am_BackgroundBrush_p.TintOpacity(value);
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetTintLuminosityOpacity_p(double *opacity)
{
    if (!g_am_BackgroundBrush_p || !opacity) {
        return E_INVALIDARG;
    }
    const double value = g_am_BackgroundBrush_p.TintLuminosityOpacity().GetDouble();
    *opacity = value;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetTintLuminosityOpacity_p(const double opacity)
{
    if (!g_am_BackgroundBrush_p) {
        return E_INVALIDARG;
    }
    const double value = std::clamp(opacity, 0.0, 1.0);
    g_am_BackgroundBrush_p.TintLuminosityOpacity(value);
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetFallbackColor_p(int *r, int *g, int *b, int *a)
{
    if (!g_am_BackgroundBrush_p || !r || !g || !b || !a) {
        return E_INVALIDARG;
    }
    const winrt::Windows::UI::Color color = g_am_BackgroundBrush_p.FallbackColor();
    *r = static_cast<int>(color.R);
    *g = static_cast<int>(color.G);
    *b = static_cast<int>(color.B);
    *a = static_cast<int>(color.A);
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetFallbackColor_p(const int r, const int g, const int b, const int a)
{
    if (!g_am_BackgroundBrush_p) {
        return E_INVALIDARG;
    }
    const auto red = static_cast<uint8_t>(std::clamp(r, 0, 255));
    const auto green = static_cast<uint8_t>(std::clamp(g, 0, 255));
    const auto blue = static_cast<uint8_t>(std::clamp(b, 0, 255));
    const auto alpha = static_cast<uint8_t>(std::clamp(a, 0, 255));
    g_am_BackgroundBrush_p.FallbackColor({alpha, red, green, blue}); // ARGB
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SwitchAcrylicBrushTheme_p(const SystemTheme theme)
{
    if (!g_am_BackgroundBrush_p || (theme == SystemTheme::Invalid) || (theme == SystemTheme::HighContrast)) {
        return E_INVALIDARG;
    }
    winrt::Windows::UI::Color tc = {};
    double to = 0.0;
    double tlo = 0.0;
    winrt::Windows::UI::Color fbc = {};
    if (theme == SystemTheme::Light) {
        tc = {255, 252, 252, 252}; // #FCFCFC, ARGB
        to = 0.0;
        tlo = 0.85;
        fbc = {255, 249, 249, 249}; // #F9F9F9, ARGB
    } else {
        tc = {255, 44, 44, 44}; // #2C2C2C, ARGB
        to = 0.15;
        tlo = 0.96;
        fbc = {255, 44, 44, 44}; // #2C2C2C, ARGB
    }
    if (FAILED(am_SetTintColor_p(static_cast<int>(tc.R),
                           static_cast<int>(tc.G),
                           static_cast<int>(tc.B),
                           static_cast<int>(tc.A)))) {
        return E_FAIL;
    }
    if (FAILED(am_SetTintOpacity_p(to))) {
        return E_FAIL;
    }
    if (FAILED(am_SetTintLuminosityOpacity_p(tlo))) {
        return E_FAIL;
    }
    if (FAILED(am_SetFallbackColor_p(static_cast<int>(fbc.R),
                               static_cast<int>(fbc.G),
                               static_cast<int>(fbc.B),
                               static_cast<int>(fbc.A)))) {
        return E_FAIL;
    }
    g_am_BrushTheme_p = theme;
    return S_OK;
}

HRESULT am_GenerateGUID_p(LPWSTR *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    if (FAILED(CoInitialize(nullptr))) {
        return E_FAIL;
    }
    GUID guid = {};
    if (FAILED(CoCreateGuid(&guid))) {
        CoUninitialize();
        return E_FAIL;
    }
    const auto buf = new wchar_t[MAX_PATH];
    SecureZeroMemory(buf, sizeof(buf));
    if (StringFromGUID2(guid, buf, MAX_PATH) == 0) {
        CoUninitialize();
        return E_FAIL;
    }
    CoUninitialize();
    *result = buf;
    return S_OK;
}

HRESULT am_ShouldAppsUseLightTheme_p(bool *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    DWORD value = 0;
    if (FAILED(am_GetDWORDValueFromRegistry_p(HKEY_CURRENT_USER, g_am_PersonalizeRegistryKey_p, L"AppsUseLightTheme", &value))) {
        return E_FAIL;
    }
    *result = (value != 0);
    return S_OK;
}

HRESULT am_ShouldSystemUsesLightTheme_p(bool *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    DWORD value = 0;
    if (FAILED(am_GetDWORDValueFromRegistry_p(HKEY_CURRENT_USER, g_am_PersonalizeRegistryKey_p, L"SystemUsesLightTheme", &value))) {
        return E_FAIL;
    }
    *result = (value != 0);
    return S_OK;
}

HRESULT am_IsHighContrastModeOn_p(bool *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        PRINT_ERROR_MESSAGE(SystemParametersInfoW)
        return E_FAIL;
    }
    *result = (hc.dwFlags & HCF_HIGHCONTRASTON);
    return S_OK;
}

HRESULT am_SetWindowCompositionAttribute_p(const HWND hWnd, LPWINDOWCOMPOSITIONATTRIBDATA pwcad)
{
    if (!hWnd || !pwcad) {
        return E_INVALIDARG;
    }
    static bool tried = false;
    using sig = BOOL(WINAPI *)(HWND, LPWINDOWCOMPOSITIONATTRIBDATA);
    static sig func = nullptr;
    if (!func) {
        if (tried) {
            return E_FAIL;
        } else {
            tried = true;
            const HMODULE dll = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (!dll) {
                PRINT_ERROR_MESSAGE(LoadLibraryExW)
                return E_FAIL;
            }
            func = reinterpret_cast<sig>(GetProcAddress(dll, "SetWindowCompositionAttribute"));
            if (!func) {
                PRINT_ERROR_MESSAGE(GetProcAddress)
                return E_FAIL;
            }
        }
    }
    return ((func(hWnd, pwcad) != FALSE) ? S_OK : E_FAIL);
}

HRESULT am_IsWindowTransitionsEnabled_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    BOOL disabled = FALSE;
    if (FAILED(DwmGetWindowAttribute(hWnd, DWMWAEX_TRANSITIONS_FORCEDISABLED, &disabled, sizeof(disabled)))) {
        return E_FAIL;
    }
    *result = (disabled == FALSE);
    return S_OK;
}

HRESULT am_IsWindowUsingDarkFrame_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    BOOL enabled = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hWnd, DWMWAEX_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &enabled, sizeof(enabled)))) {
        *result = (enabled != FALSE);
        return S_OK;
    }
    if (SUCCEEDED(DwmGetWindowAttribute(hWnd, DWMWAEX_USE_IMMERSIVE_DARK_MODE, &enabled, sizeof(enabled)))) {
        *result = (enabled != FALSE);
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_SetWindowDarkFrameEnabled_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    const BOOL enabled = enable ? TRUE : FALSE;
    if (SUCCEEDED(DwmSetWindowAttribute(hWnd, DWMWAEX_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &enabled, sizeof(enabled)))) {
        return S_OK;
    }
    if (SUCCEEDED(DwmSetWindowAttribute(hWnd, DWMWAEX_USE_IMMERSIVE_DARK_MODE, &enabled, sizeof(enabled)))) {
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_GetColorizationColor_p(COLORREF *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    COLORREF color = RGB(0, 0, 0);
    BOOL opaque = FALSE;
    if (FAILED(DwmGetColorizationColor(&color, &opaque))) {
        return E_FAIL;
    }
    *result = color;
    return S_OK;
}

HRESULT am_GetColorizationArea_p(ColorizationArea *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    // todo: check which specific win10.
    if (!g_am_IsWindows10OrGreater_p) {
        return E_FAIL;
    }
    const HKEY rootKey = HKEY_CURRENT_USER;
    LPCWSTR valueName = L"ColorPrevalence";
    DWORD dwTheme = 0;
    if (FAILED(am_GetDWORDValueFromRegistry_p(rootKey, g_am_PersonalizeRegistryKey_p, valueName, &dwTheme))) {
        return E_FAIL;
    }
    DWORD dwDWM = 0;
    if (FAILED(am_GetDWORDValueFromRegistry_p(rootKey, g_am_DWMRegistryKey_p, valueName, &dwDWM))) {
        return E_FAIL;
    }
    const bool theme = (dwTheme != 0);
    const bool dwm = (dwDWM != 0);
    if (theme && dwm) {
        *result = ColorizationArea::All;
        return S_OK;
    } else if (theme) {
        *result = ColorizationArea::StartMenu_TaskBar_ActionCenter;
        return S_OK;
    } else if (dwm) {
        *result = ColorizationArea::TitleBar_WindowBorder;
        return S_OK;
    }
    *result = ColorizationArea::None;
    return S_OK;
}

HRESULT am_FreeString(LPWSTR str)
{
    if (!str) {
        return E_INVALIDARG;
    }
    delete [] str;
    str = nullptr;
    return S_OK;
}

HRESULT am_PrintErrorMessageFromHResult_p(LPCWSTR function, const HRESULT hr)
{
    if (!function) {
        return E_INVALIDARG;
    }

    if (SUCCEEDED(hr)) {
        return S_OK;
    }

    LPVOID lpMsgBuf = nullptr;
    LPVOID lpDisplayBuf = nullptr;

    const DWORD dwError = HRESULT_CODE(hr);

    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&lpMsgBuf), 0, nullptr);

    lpDisplayBuf = reinterpret_cast<LPVOID>(LocalAlloc(LMEM_ZEROINIT,
        (wcslen(reinterpret_cast<LPCWSTR>(lpMsgBuf)) + wcslen(reinterpret_cast<LPCWSTR>(function)) + 40) * sizeof(wchar_t)));
    swprintf_s(reinterpret_cast<LPWSTR>(lpDisplayBuf), LocalSize(lpDisplayBuf) / sizeof(wchar_t),
               L"%s failed with error %d: %s", function, dwError, reinterpret_cast<LPCWSTR>(lpMsgBuf));
    am_Print_p(reinterpret_cast<LPCWSTR>(lpDisplayBuf), true);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetWindowGeometry_p(const HWND hWnd, const int x, const int y, const int w, const int h)
{
    if (!hWnd || (x <= 0) || (y <= 0) || (w <= 0) || (h <= 0)) {
        return E_INVALIDARG;
    }
    if (MoveWindow(hWnd, x, y, w, h, TRUE) != FALSE) {
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_IsWindowActive_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    *result = (GetActiveWindow() == hWnd);
    return S_OK;
}

HRESULT am_GetWallpaperFilePath_p(const int screen, LPWSTR *result)
{
    if ((screen < 0) || !result) {
        return E_INVALIDARG;
    }
    if (g_am_IsWindows8OrGreater_p) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            CComPtr<IDesktopWallpaper> pDesktopWallpaper = nullptr;
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pDesktopWallpaper)))) {
                UINT monitorCount = 0;
                if (SUCCEEDED(pDesktopWallpaper->GetMonitorDevicePathCount(&monitorCount))) {
                    if (screen < monitorCount) {
                        LPWSTR monitorId = nullptr;
                        if (SUCCEEDED(pDesktopWallpaper->GetMonitorDevicePathAt(screen, &monitorId))) {
                            LPWSTR wallpaperPath = nullptr;
                            if (SUCCEEDED(pDesktopWallpaper->GetWallpaper(monitorId, &wallpaperPath))) {
                                CoTaskMemFree(monitorId);
                                const auto _path = new wchar_t[MAX_PATH];
                                SecureZeroMemory(_path, sizeof(_path));
                                memcpy(_path, wallpaperPath, wcslen(wallpaperPath));
                                *result = _path;
                                CoTaskMemFree(wallpaperPath);
                                CoUninitialize();
                                return S_OK;
                            } else {
                                CoTaskMemFree(monitorId);
                            }
                        }
                    }
                }
            }
            CoUninitialize();
        }
    }
    if (SUCCEEDED(CoInitialize(nullptr))) {
        CComPtr<IActiveDesktop> pActiveDesktop = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pActiveDesktop)))) {
            const auto wallpaperPath = new wchar_t[MAX_PATH];
            SecureZeroMemory(wallpaperPath, sizeof(wallpaperPath));
            // TODO: AD_GETWP_BMP, AD_GETWP_IMAGE, AD_GETWP_LAST_APPLIED. What's the difference?
            if (SUCCEEDED(pActiveDesktop->GetWallpaper(wallpaperPath, MAX_PATH, AD_GETWP_LAST_APPLIED))) {
                *result = wallpaperPath;
                CoUninitialize();
                return S_OK;
            }
        }
        CoUninitialize();
    }
    const auto wallpaperPath = new wchar_t[MAX_PATH];
    SecureZeroMemory(wallpaperPath, sizeof(wallpaperPath));
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, wallpaperPath, 0) != FALSE) {
        *result = wallpaperPath;
        return S_OK;
    }
    //const QSettings registry(g_desktopRegistryKey, QSettings::NativeFormat);
    //return QImage(registry.value(QStringLiteral("WallPaper")).toString());
    return E_FAIL;
}

HRESULT am_GetDesktopBackgroundColor_p(COLORREF *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    if (g_am_IsWindows8OrGreater_p) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            CComPtr<IDesktopWallpaper> pDesktopWallpaper = nullptr;
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pDesktopWallpaper)))) {
                COLORREF color = RGB(0, 0, 0);
                if (SUCCEEDED(pDesktopWallpaper->GetBackgroundColor(&color))) {
                    *result = color;
                    CoUninitialize();
                    return S_OK;
                }
            }
            CoUninitialize();
        }
    }
    // TODO: Is there any other way to get the background color? Traditional Win32 API? Registry?
    // Is there a COM API for Win7?
    *result = RGB(0, 0, 0);
    return S_OK;
}

HRESULT am_GetWallpaperAspectStyle_p(const int screen, WallpaperAspectStyle *result)
{
    if ((screen < 0) || !result) {
        return E_INVALIDARG;
    }
    if (g_am_IsWindows8OrGreater_p) {
        if (SUCCEEDED(CoInitialize(nullptr))) {
            CComPtr<IDesktopWallpaper> pDesktopWallpaper = nullptr;
            if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pDesktopWallpaper)))) {
                DESKTOP_WALLPAPER_POSITION position = DWPOS_FILL;
                if (SUCCEEDED(pDesktopWallpaper->GetPosition(&position))) {
                    switch (position) {
                    case DWPOS_CENTER:
                        *result = WallpaperAspectStyle::Central;
                        break;
                    case DWPOS_TILE:
                        *result = WallpaperAspectStyle::Tiled;
                        break;
                    case DWPOS_STRETCH:
                        *result = WallpaperAspectStyle::IgnoreRatioFit;
                        break;
                    case DWPOS_FIT:
                        *result = WallpaperAspectStyle::KeepRatioFit;
                        break;
                    case DWPOS_FILL:
                        *result = WallpaperAspectStyle::KeepRatioByExpanding;
                        break;
                    case DWPOS_SPAN:
                        *result = WallpaperAspectStyle::Span;
                        break;
                    }
                    CoUninitialize();
                    return S_OK;
                }
            }
            CoUninitialize();
        }
    }
    if (SUCCEEDED(CoInitialize(nullptr))) {
        CComPtr<IActiveDesktop> pActiveDesktop = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pActiveDesktop)))) {
            WALLPAPEROPT opt;
            SecureZeroMemory(&opt, sizeof(opt));
            opt.dwSize = sizeof(opt);
            if (SUCCEEDED(pActiveDesktop->GetWallpaperOptions(&opt, 0))) {
                switch (opt.dwStyle) {
                case WPSTYLE_CENTER:
                    *result = WallpaperAspectStyle::Central;
                    break;
                case WPSTYLE_TILE:
                    *result = WallpaperAspectStyle::Tiled;
                    break;
                case WPSTYLE_STRETCH:
                    *result = WallpaperAspectStyle::IgnoreRatioFit;
                    break;
                case WPSTYLE_KEEPASPECT:
                    *result = WallpaperAspectStyle::KeepRatioFit;
                    break;
                case WPSTYLE_CROPTOFIT:
                    *result = WallpaperAspectStyle::KeepRatioByExpanding;
                    break;
                case WPSTYLE_SPAN:
                    *result = WallpaperAspectStyle::Span;
                    break;
                }
                CoUninitialize();
                return S_OK;
            }
        }
        CoUninitialize();
    }
    const HKEY rootKey = HKEY_CURRENT_USER;
    DWORD dwStyle = 0;
    if (SUCCEEDED(am_GetDWORDValueFromRegistry_p(rootKey, g_am_DesktopRegistryKey_p, L"WallpaperStyle", &dwStyle))) {
        switch (dwStyle) {
        case 0: {
            DWORD dwTile = 0;
            if (SUCCEEDED(am_GetDWORDValueFromRegistry_p(rootKey, g_am_DesktopRegistryKey_p, L"TileWallpaper", &dwTile))
                    && (dwTile != 0)) {
                *result = WallpaperAspectStyle::Tiled;
            } else {
                *result = WallpaperAspectStyle::Central;
            }
        } break;
        case 2:
            *result = WallpaperAspectStyle::IgnoreRatioFit;
            break;
        case 6:
            *result = WallpaperAspectStyle::KeepRatioFit;
            break;
        case 10:
            *result = WallpaperAspectStyle::KeepRatioByExpanding;
            break;
        case 22:
            *result = WallpaperAspectStyle::Span;
            break;
        default:
            return E_FAIL;
        }
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_GetWindowDpiAwareness_p(const HWND hWnd, int *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    // todo
    return E_FAIL;
}

HRESULT am_SetWindowDpiAwareness_p(const HWND hWnd, int awareness)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    // todo
    return E_FAIL;
}

HRESULT am_WideToMultibyte_p(LPCWSTR in, const UINT codePage, LPSTR *out)
{
    if (!in || !out) {
        return E_INVALIDARG;
    }
    // todo
    return E_FAIL;
}

HRESULT am_MultibyteToWide_p(LPCSTR in, const UINT codePage, LPWSTR *out)
{
    if (!in || !out) {
        return E_INVALIDARG;
    }
    // todo
    return E_FAIL;
}

// Direct2D

[[nodiscard]] static inline HRESULT
am_CreateD2DBitmapBrushFromURI_p(
    ID2D1RenderTarget *pRenderTarget,
    LPCWSTR           uri,
    UINT              width,
    UINT              height,
    ID2D1Bitmap       **ppBitmap
)
{
    if (!g_am_D2DRenderTarget_p || !pRenderTarget || !uri || (width <= 0) || (height <= 0) || !ppBitmap) {
        return E_INVALIDARG;
    }
    CComPtr<IWICBitmapDecoder> pDecoder = nullptr;
    CComPtr<IWICBitmapFrameDecode> pSource = nullptr;
    CComPtr<IWICStream> pStream = nullptr;
    CComPtr<IWICFormatConverter> pConverter = nullptr;
    CComPtr<IWICBitmapScaler> pScalar = nullptr;
    CComPtr<IWICImagingFactory> pIWICFactory = nullptr;
    if (FAILED(CoInitialize(nullptr))) {
        return E_FAIL;
    }
    if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory)))) {
        if (SUCCEEDED(pIWICFactory->CreateDecoderFromFilename(uri, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder))) {
            if (SUCCEEDED(pDecoder->GetFrame(0, &pSource))) {
                if (SUCCEEDED(pIWICFactory->CreateFormatConverter(&pConverter))) {
                    if (SUCCEEDED(pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
                                                         nullptr, 0.0f, WICBitmapPaletteTypeMedianCut))) {
                        if (SUCCEEDED(pRenderTarget->CreateBitmapFromWicBitmap(pConverter, nullptr, ppBitmap))) {
                            if (SUCCEEDED(g_am_D2DRenderTarget_p->CreateBitmapBrush(*ppBitmap, &g_am_D2DBitmapBrush_p))) {
                                CoUninitialize();
                                return S_OK;
                            }
                        }
                    }
                }
            }
        }
    }
    CoUninitialize();
    return E_FAIL;
}

























































[[nodiscard]] static inline LRESULT CALLBACK am_MainWindowProc_p(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool systemThemeChanged = false;
    switch (uMsg)
    {
    case WM_NCCALCSIZE: {
        if (!wParam) {
            return 0;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        if (g_am_IsWindows10OrGreater_p) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcW(hWnd, uMsg, wParam, lParam);
            if (ret != 0) {
                return ret;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        }
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        bool nonClientAreaExists = false;
        bool max = false, full = false;
        if ((SUCCEEDED(am_IsMaximized_p(hWnd, &max)) && max)
                && (SUCCEEDED(am_IsFullScreened_p(hWnd, &full)) && !full)) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            int rbtY = 0;
            if (SUCCEEDED(am_GetResizeBorderThickness_p(false, g_am_CurrentDpi_p, &rbtY))) {
                clientRect->top += rbtY;
                if (!g_am_IsWindows10OrGreater_p) {
                    clientRect->bottom -= rbtY;
                    int rbtX = 0;
                    if (SUCCEEDED(am_GetResizeBorderThickness_p(true, g_am_CurrentDpi_p, &rbtX))) {
                        clientRect->left += rbtX;
                        clientRect->right -= rbtX;
                    }
                }
            }
            nonClientAreaExists = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (max || full) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            // First, check if we have an auto-hide taskbar at all:
            if (SHAppBarMessage(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) {
                bool top = false, bottom = false, left = false, right = false;
                // Due to "ABM_GETAUTOHIDEBAREX" only has effect since Windows 8.1,
                // we have to use another way to judge the edge of the auto-hide taskbar
                // when the application is running on Windows 7 or Windows 8.
                if (g_am_IsWindows8Point1OrGreater_p) {
                    RECT screenRect = {};
                    if (SUCCEEDED(am_GetScreenGeometry_p(hWnd, &screenRect))) {
                        // This helper can be used to determine if there's a
                        // auto-hide taskbar on the given edge of the monitor
                        // we're currently on.
                        const auto hasAutohideTaskbar = [&screenRect](const UINT edge) -> bool {
                            APPBARDATA abd2;
                            SecureZeroMemory(&abd2, sizeof(abd2));
                            abd2.cbSize = sizeof(abd2);
                            abd2.uEdge = edge;
                            abd2.rc = screenRect;
                            return (reinterpret_cast<HWND>(SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &abd2)) != nullptr);
                        };
                        top = hasAutohideTaskbar(ABE_TOP);
                        bottom = hasAutohideTaskbar(ABE_BOTTOM);
                        left = hasAutohideTaskbar(ABE_LEFT);
                        right = hasAutohideTaskbar(ABE_RIGHT);
                    }
                } else {
                    // The following code is copied from Mozilla Firefox with some modifications.
                    int edge = -1;
                    APPBARDATA _abd;
                    SecureZeroMemory(&_abd, sizeof(_abd));
                    _abd.cbSize = sizeof(_abd);
                    _abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
                    if (_abd.hWnd) {
                        const HMONITOR windowMonitor = CURRENT_SCREEN(hWnd);
                        const HMONITOR taskbarMonitor = MonitorFromWindow(_abd.hWnd, MONITOR_DEFAULTTOPRIMARY);
                        if (taskbarMonitor == windowMonitor) {
                            SHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
                            edge = _abd.uEdge;
                        }
                    }
                    top = (edge == ABE_TOP);
                    bottom = (edge == ABE_BOTTOM);
                    left = (edge == ABE_LEFT);
                    right = (edge == ABE_RIGHT);
                }
                // If there's a taskbar on any side of the monitor, reduce
                // our size a little bit on that edge.
                // Note to future code archeologists:
                // This doesn't seem to work for fullscreen on the primary
                // display. However, testing a bunch of other apps with
                // fullscreen modes and an auto-hiding taskbar has
                // shown that _none_ of them reveal the taskbar from
                // fullscreen mode. This includes Edge, Firefox, Chrome,
                // Sublime Text, PowerPoint - none seemed to support this.
                // This does however work fine for maximized.
                if (top) {
                    // Peculiarly, when we're fullscreen,
                    clientRect->top += g_am_AutoHideTaskbarThicknessPy_p;
                    nonClientAreaExists = true;
                } else if (bottom) {
                    clientRect->bottom -= g_am_AutoHideTaskbarThicknessPy_p;
                    nonClientAreaExists = true;
                } else if (left) {
                    clientRect->left += g_am_AutoHideTaskbarThicknessPx_p;
                    nonClientAreaExists = true;
                } else if (right) {
                    clientRect->right -= g_am_AutoHideTaskbarThicknessPx_p;
                    nonClientAreaExists = true;
                }
            }
        }
        // If the window bounds change, we're going to relayout and repaint
        // anyway. Returning WVR_REDRAW avoids an extra paint before that of
        // the old client pixels in the (now wrong) location, and thus makes
        // actions like resizing a window from the left edge look slightly
        // less broken.
        //
        // We cannot return WVR_REDRAW when there is nonclient area, or
        // Windows exhibits bugs where client pixels and child HWNDs are
        // mispositioned by the width/height of the upper-left nonclient
        // area.
        return nonClientAreaExists ? 0 : WVR_REDRAW;
    }
    case WM_NCHITTEST: {
        const POINT globalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT localPos = globalPos;
        if (ScreenToClient(hWnd, &localPos) == FALSE) {
            PRINT_ERROR_MESSAGE(ScreenToClient)
            break;
        }
        bool max = false, full = false, normal = false;
        if (FAILED(am_IsMaximized_p(hWnd, &max)) || FAILED(am_IsFullScreened_p(hWnd, &full))
                || FAILED(am_IsWindowNoState_p(hWnd, &normal))) {
            break;
        }
        const bool maxOrFull = (max || full);
        SIZE ws = {};
        if (FAILED(am_GetWindowSize_p(hWnd, &ws))) {
            break;
        }
        const LONG ww = ws.cx;
        int rbtX = 0, rbtY = 0, cth = 0;
        if (FAILED(am_GetResizeBorderThickness_p(true, g_am_CurrentDpi_p, &rbtX))
                || FAILED(am_GetResizeBorderThickness_p(false, g_am_CurrentDpi_p, &rbtY))
                || FAILED(am_GetCaptionHeight_p(g_am_CurrentDpi_p, &cth))) {
            break;
        }
        const bool hitTestVisible = /*am_IsHitTestVisibleInChrome_p(hWnd)*/false; // todo
        bool isTitleBar = false;
        if (maxOrFull) {
            isTitleBar = ((localPos.y >= 0) && (localPos.y <= cth)
                          && (localPos.x >= 0) && (localPos.x <= ww)
                          && !hitTestVisible);
        } else if (normal) {
            isTitleBar = ((localPos.y > rbtY) && (localPos.y <= (rbtY + cth))
                          && (localPos.x > rbtX) && (localPos.x < (ww - rbtX))
                          && !hitTestVisible);
        }
        const bool isTop = (normal ? (localPos.y <= rbtY) : false);
        if (g_am_IsWindows10OrGreater_p) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcW(hWnd, uMsg, wParam, lParam);
            if (originalRet != HTCLIENT) {
                return originalRet;
            }
            // At this point, we know that the cursor is inside the client area
            // so it has to be either the little border at the top of our custom
            // title bar or the drag bar. Apparently, it must be the drag bar or
            // the little border at the top which the user can use to move or
            // resize the window.
            if (isTop) {
                return HTTOP;
            }
            if (isTitleBar) {
                return HTCAPTION;
            }
            return HTCLIENT;
        } else {
            if (maxOrFull) {
                if (isTitleBar) {
                    return HTCAPTION;
                }
                return HTCLIENT;
            }
            const LONG wh = ws.cy;
            const bool isBottom = (normal ? (localPos.y >= (wh - rbtY)) : false);
            // Make the border a little wider to let the user easy to resize on corners.
            const double factor = (normal ? ((isTop || isBottom) ? 2.0 : 1.0) : 0.0);
            const bool isLeft = (normal ? (localPos.x <= std::round(static_cast<double>(rbtX) * factor)) : false);
            const bool isRight = (normal ? (localPos.x >= (ww - std::round(static_cast<double>(rbtX) * factor))) : false);
            if (isTop) {
                if (isLeft) {
                    return HTTOPLEFT;
                }
                if (isRight) {
                    return HTTOPRIGHT;
                }
                return HTTOP;
            }
            if (isBottom) {
                if (isLeft) {
                    return HTBOTTOMLEFT;
                }
                if (isRight) {
                    return HTBOTTOMRIGHT;
                }
                return HTBOTTOM;
            }
            if (isLeft) {
                return HTLEFT;
            }
            if (isRight) {
                return HTRIGHT;
            }
            if (isTitleBar) {
                return HTCAPTION;
            }
            return HTCLIENT;
        }
        return HTNOWHERE;
    }
    case WM_PAINT: {
        if (g_am_IsWindows10OrGreater_p) {
            PAINTSTRUCT ps = {};
            const HDC hdc = BeginPaint(hWnd, &ps);
            if (!hdc) {
                PRINT_ERROR_MESSAGE(BeginPaint)
                break;
            }
            // We removed the whole top part of the frame (see handling of
            // WM_NCCALCSIZE) so the top border is missing now. We add it back here.
            // Note #1: You might wonder why we don't remove just the title bar instead
            //  of removing the whole top part of the frame and then adding the little
            //  top border back. I tried to do this but it didn't work: DWM drew the
            //  whole title bar anyways on top of the window. It seems that DWM only
            //  wants to draw either nothing or the whole top part of the frame.
            // Note #2: For some reason if you try to set the top margin to just the
            //  top border height (what we want to do), then there is a transparency
            //  bug when the window is inactive, so I've decided to add the whole top
            //  part of the frame instead and then we will hide everything that we
            //  don't need (that is, the whole thing but the little 1 pixel wide border
            //  at the top) in the WM_PAINT handler. This eliminates the transparency
            //  bug and it's what a lot of Win32 apps that customize the title bar do
            //  so it should work fine.
            int borderThickness = 0;
            if (FAILED(am_GetWindowVisibleFrameBorderThickness_p(hWnd, g_am_CurrentDpi_p, &borderThickness))) {
                break;
            }
            if (ps.rcPaint.top < borderThickness) {
                RECT rcPaint = ps.rcPaint;
                rcPaint.bottom = borderThickness;
                // To show the original top border, we have to paint on top
                // of it with the alpha component set to 0. This page
                // recommends to paint the area in black using the stock
                // BLACK_BRUSH to do this:
                // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
                if (FillRect(hdc, &rcPaint, BLACK_BACKGROUND_BRUSH) == 0) {
                    PRINT_ERROR_MESSAGE(FillRect)
                    break;
                }
            }
            if (ps.rcPaint.bottom > borderThickness) {
                RECT rcPaint = ps.rcPaint;
                rcPaint.top = borderThickness;
                // To hide the original title bar, we have to paint on top
                // of it with the alpha component set to 255. This is a hack
                // to do it with GDI. See updateFrameMargins() for more information.
                HDC opaqueDc = nullptr;
                BP_PAINTPARAMS params;
                SecureZeroMemory(&params, sizeof(params));
                params.cbSize = sizeof(params);
                params.dwFlags = BPPF_NOCLIP | BPPF_ERASE;
                const HPAINTBUFFER buf = BeginBufferedPaint(hdc, &rcPaint, BPBF_TOPDOWNDIB, &params, &opaqueDc);
                if (!buf) {
                    PRINT_ERROR_MESSAGE(BeginBufferedPaint)
                    break;
                }
                if (FillRect(opaqueDc, &rcPaint,
                             reinterpret_cast<HBRUSH>(GetClassLongPtrW(hWnd, GCLP_HBRBACKGROUND))) == 0) {
                    PRINT_ERROR_MESSAGE(FillRect)
                    break;
                }
                if (FAILED(BufferedPaintSetAlpha(buf, nullptr, 255))) {
                    am_Print_p(L"WM_PAINT: BufferedPaintSetAlpha() failed.");
                    break;
                }
                if (FAILED(EndBufferedPaint(buf, TRUE))) {
                    am_Print_p(L"WM_PAINT: EndBufferedPaint() failed.");
                    break;
                }
            }
            if (EndPaint(hWnd, &ps) == FALSE) {
                PRINT_ERROR_MESSAGE(EndPaint)
                break;
            }
            return 0;
        } else {
            if (!g_am_D2DFactory_p || !g_am_D2DRenderTarget_p || !g_am_WallpaperFilePath_p) {
                break;
            }
            SIZE size = {};
            if (FAILED(am_GetWindowSize_p(hWnd, &size))) {
                break;
            }
            if (!g_am_D2DWallpaperBitmap_p) {
                if (FAILED(am_CreateD2DBitmapBrushFromURI_p(g_am_D2DRenderTarget_p, g_am_WallpaperFilePath_p, size.cx, size.cy, &g_am_D2DWallpaperBitmap_p))) {
                    break;
                }
            }
            g_am_D2DRenderTarget_p->BeginDraw();
            g_am_D2DRenderTarget_p->Clear(D2D1::ColorF(g_am_DesktopBackgroundColor_p));
            g_am_D2DRenderTarget_p->DrawBitmap(g_am_D2DWallpaperBitmap_p, D2D1::RectF(0, 0, size.cx, size.cy));
            if (FAILED(g_am_D2DRenderTarget_p->EndDraw())) {
                break;
            }
            return 0;
        }
    } break;
    case WM_DPICHANGED: {
        const double x = LOWORD(wParam);
        const double y = HIWORD(wParam);
        g_am_CurrentDpi_p = std::round((x + y) / 2.0);
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (MoveWindow(hWnd, prcNewWindow->left, prcNewWindow->top,
                   RECT_WIDTH(*prcNewWindow), RECT_HEIGHT(*prcNewWindow), TRUE) == FALSE) {
            PRINT_ERROR_MESSAGE(MoveWindow)
            break;
        }
        return 0;
    } break;
    case WM_SIZE: {
        bool full = false;
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)
                || (SUCCEEDED(am_IsFullScreened_p(hWnd, &full)) && full)) {
            if (FAILED(am_UpdateFrameMargins_p(hWnd, g_am_CurrentDpi_p))) {
                am_Print_p(L"WM_SIZE: Failed to update frame margins.");
                break;
            }
        }
        const auto width = LOWORD(lParam);
        const UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER;
        if (g_am_XAMLIslandWindowHandle_p) {
            // Give enough space to our thin homemade top border.
            int borderThickness = 0;
            if (SUCCEEDED(am_GetWindowVisibleFrameBorderThickness_p(hWnd, g_am_CurrentDpi_p, &borderThickness))) {
                const int height = (HIWORD(lParam) - borderThickness);
                if (SetWindowPos(g_am_XAMLIslandWindowHandle_p, HWND_BOTTOM, 0, borderThickness,
                             width, height, flags) == FALSE) {
                    PRINT_ERROR_MESSAGE(SetWindowPos)
                    break;
                }
            }
        }
        if (g_am_DragBarWindowHandle_p) {
            int tbh = 0;
            if (SUCCEEDED(am_GetTitleBarHeight_p(hWnd, g_am_CurrentDpi_p, &tbh))) {
                if (SetWindowPos(g_am_DragBarWindowHandle_p, HWND_TOP, 0, 0, width, tbh, flags) == FALSE) {
                    PRINT_ERROR_MESSAGE(SetWindowPos)
                    break;
                }
            }
        }
    } break;
    case WM_SETFOCUS: {
        if (g_am_XAMLIslandWindowHandle_p) {
            // Send focus to the XAML Island child window.
            SetFocus(g_am_XAMLIslandWindowHandle_p);
            return 0;
        }
    } break;
    case WM_SETCURSOR: {
        if (LOWORD(lParam) == HTCLIENT) {
            // Get the cursor position from the _last message_ and not from
            // `GetCursorPos` (which returns the cursor position _at the
            // moment_) because if we're lagging behind the cursor's position,
            // we still want to get the cursor position that was associated
            // with that message at the time it was sent to handle the message
            // correctly.
            const LRESULT hitTestResult = SendMessageW(hWnd, WM_NCHITTEST, 0, GetMessagePos());
            if (hitTestResult == HTTOP) {
                // We have to set the vertical resize cursor manually on
                // the top resize handle because Windows thinks that the
                // cursor is on the client area because it asked the asked
                // the drag window with `WM_NCHITTEST` and it returned
                // `HTCLIENT`.
                // We don't want to modify the drag window's `WM_NCHITTEST`
                // handling to return `HTTOP` because otherwise, the system
                // would resize the drag window instead of the top level
                // window!
                SetCursor(LoadCursorW(nullptr, IDC_SIZENS));
            } else {
                // Reset cursor
                SetCursor(LoadCursorW(nullptr, IDC_ARROW));
            }
            return TRUE;
        }
    } break;
    case WM_NCRBUTTONUP: {
        // The `DefWindowProc` function doesn't open the system menu for some
        // reason so we have to do it ourselves.
        if (wParam == HTCAPTION) {
            if (FAILED(am_OpenSystemMenu_p(hWnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}))) {
                am_Print_p(L"WM_NCRBUTTONUP: Failed to open the system menu.");
                break;
            }
        }
    } break;
    case WM_SETTINGCHANGE: {
        if ((wParam == 0) && (wcscmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            systemThemeChanged = true;
        }
    } break;
    case WM_THEMECHANGED:
    case WM_DWMCOLORIZATIONCOLORCHANGED:
        systemThemeChanged = true;
        break;
    case WM_DWMCOMPOSITIONCHANGED: {
        bool enabled = false;
        if (FAILED(am_IsCompositionEnabled_p(&enabled)) || !enabled) {
            am_Print_p(L"This application can't continue running when DWM composition is disabled.", true);
            std::exit(-1);
        }
    } break;
    case WM_ACTIVATE: {
        break; // test
        if (g_am_IsWindows10OrGreater_p) {
            break;
        }
        const auto status = LOWORD(wParam);
        const bool active = ((status == WA_ACTIVE) || (status == WA_CLICKACTIVE));
        const COLORREF color = (active ? RGB(0, 0, 0) : RGB(128, 128, 128));
        const HPEN hpen = CreatePen(PS_SOLID, 4, color);
        if (!hpen) {
            PRINT_ERROR_MESSAGE(CreatePen)
            break;
        }
        const HDC hdc = GetDC(hWnd);
        if (!hdc) {
            PRINT_ERROR_MESSAGE(GetDC)
            break;
        }
        HGDIOBJ result = SelectObject(hdc, hpen);
        if (!result || (result == HGDI_ERROR)) {
            PRINT_ERROR_MESSAGE(SelectObject)
            break;
        }
        result = SelectObject(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
        if (!result || (result == HGDI_ERROR)) {
            PRINT_ERROR_MESSAGE(SelectObject)
            break;
        }
        SIZE ws = {};
        if (FAILED(am_GetWindowSize_p(hWnd, &ws))) {
            break;
        }
        if (Rectangle(hdc, 0, 0, ws.cx, ws.cy) == FALSE) {
            PRINT_ERROR_MESSAGE(Rectangle)
            break;
        }
        if (ReleaseDC(hWnd, hdc) == 0) {
            PRINT_ERROR_MESSAGE(ReleaseDC)
            break;
        }
    } break;
    case WM_CLOSE: {
        am_Cleanup_p();
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }
    if (g_am_BackgroundBrush_p && (g_am_BrushTheme_p == SystemTheme::Auto) && systemThemeChanged) {
        SystemTheme systemTheme = SystemTheme::Invalid;
        if (SUCCEEDED(am_GetSystemTheme_p(&systemTheme))) {
            if ((systemTheme != SystemTheme::Invalid) && (systemTheme != SystemTheme::HighContrast)) {
                if (SUCCEEDED(am_SwitchAcrylicBrushTheme_p(systemTheme))) {
                    g_am_BrushTheme_p = SystemTheme::Auto;
                } else {
                    am_Print_p(L"Failed to switch acrylic brush theme.");
                }
            } else {
                am_Print_p(L"Failed to retrieve system theme or high contrast mode is on.");
            }
        }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

[[nodiscard]] static inline LRESULT CALLBACK am_DragBarWindowProc_p(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::optional<UINT> nonClientMessage = std::nullopt;

    switch (uMsg)
    {
    // Translate WM_* messages on the window to WM_NC* on the top level window.
    case WM_LBUTTONDOWN:
        nonClientMessage = WM_NCLBUTTONDOWN;
        break;
    case WM_LBUTTONUP:
        nonClientMessage = WM_NCLBUTTONUP;
        break;
    case WM_LBUTTONDBLCLK:
        nonClientMessage = WM_NCLBUTTONDBLCLK;
        break;
    case WM_MBUTTONDOWN:
        nonClientMessage = WM_NCMBUTTONDOWN;
        break;
    case WM_MBUTTONUP:
        nonClientMessage = WM_NCMBUTTONUP;
        break;
    case WM_MBUTTONDBLCLK:
        nonClientMessage = WM_NCMBUTTONDBLCLK;
        break;
    case WM_RBUTTONDOWN:
        nonClientMessage = WM_NCRBUTTONDOWN;
        break;
    case WM_RBUTTONUP:
        nonClientMessage = WM_NCRBUTTONUP;
        break;
    case WM_RBUTTONDBLCLK:
        nonClientMessage = WM_NCRBUTTONDBLCLK;
        break;
    case WM_XBUTTONDOWN:
        nonClientMessage = WM_NCXBUTTONDOWN;
        break;
    case WM_XBUTTONUP:
        nonClientMessage = WM_NCXBUTTONUP;
        break;
    case WM_XBUTTONDBLCLK:
        nonClientMessage = WM_NCXBUTTONDBLCLK;
        break;
    default:
        break;
    }

    if (nonClientMessage.has_value() && g_am_MainWindowHandle_p)
    {
        POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (ClientToScreen(hWnd, &pos) == FALSE) {
            PRINT_ERROR_MESSAGE(ClientToScreen)
            return 0;
        }
        const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
        // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
        // then pass that click through as an NC click in that location.
        const LRESULT hitTestResult = SendMessageW(g_am_MainWindowHandle_p, WM_NCHITTEST, 0, newLParam);
        SendMessageW(g_am_MainWindowHandle_p, nonClientMessage.value(), hitTestResult, newLParam);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

[[nodiscard]] static inline HRESULT am_RegisterMainWindowClass_p()
{
    if (g_am_MainWindowAtom_p != 0) {
        am_Cleanup_p();
        return E_INVALIDARG;
    }

    LPWSTR guid = nullptr;
    if (FAILED(am_GenerateGUID_p(&guid))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    g_am_MainWindowClassName_p = new wchar_t[MAX_PATH];
    SecureZeroMemory(g_am_MainWindowClassName_p, sizeof(g_am_MainWindowClassName_p));
    wcscat(g_am_MainWindowClassName_p, g_am_WindowClassNamePrefix_p);
    wcscat(g_am_MainWindowClassName_p, guid);
    delete [] guid;
    wcscat(g_am_MainWindowClassName_p, g_am_MainWindowClassNameSuffix_p);

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = am_MainWindowProc_p;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = g_am_MainWindowClassName_p;

    g_am_MainWindowAtom_p = RegisterClassExW(&wcex);

    if (g_am_MainWindowAtom_p == 0) {
        PRINT_ERROR_MESSAGE(RegisterClassExW)
        am_Cleanup_p();
        return E_FAIL;
    }

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_RegisterDragBarWindowClass_p()
{
    if (!g_am_IsWindows8OrGreater_p) {
        am_Print_p(L"Drag bar window is only available on Windows 8 and onwards.");
        am_Cleanup_p();
        return E_FAIL;
    }

    if ((g_am_MainWindowAtom_p == 0) || (g_am_DragBarWindowAtom_p != 0)) {
        am_Cleanup_p();
        return E_INVALIDARG;
    }

    LPWSTR guid = nullptr;
    if (FAILED(am_GenerateGUID_p(&guid))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    g_am_DragBarWindowClassName_p = new wchar_t[MAX_PATH];
    SecureZeroMemory(g_am_DragBarWindowClassName_p, sizeof(g_am_DragBarWindowClassName_p));
    wcscat(g_am_DragBarWindowClassName_p, g_am_WindowClassNamePrefix_p);
    wcscat(g_am_DragBarWindowClassName_p, guid);
    delete [] guid;
    wcscat(g_am_DragBarWindowClassName_p, g_am_DragBarWindowClassNameSuffix_p);

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = am_DragBarWindowProc_p;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = BLACK_BACKGROUND_BRUSH;
    wcex.lpszClassName = g_am_DragBarWindowClassName_p;

    g_am_DragBarWindowAtom_p = RegisterClassExW(&wcex);

    if (g_am_DragBarWindowAtom_p == 0) {
        PRINT_ERROR_MESSAGE(RegisterClassExW)
        am_Cleanup_p();
        return E_FAIL;
    }

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_CreateMainWindow_p(const int x, const int y, const int w, const int h)
{
    if ((g_am_MainWindowAtom_p == 0) || g_am_MainWindowHandle_p) {
        am_Cleanup_p();
        return E_INVALIDARG;
    }

    g_am_MainWindowHandle_p = CreateWindowExW(0L,
                                       g_am_MainWindowClassName_p, g_am_MainWindowTitle_p,
                                       WS_OVERLAPPEDWINDOW,
                                       ((x > 0) ? x : CW_USEDEFAULT),
                                       ((y > 0) ? y : CW_USEDEFAULT),
                                       ((w > 0) ? w : CW_USEDEFAULT),
                                       ((h > 0) ? h : CW_USEDEFAULT),
                                       nullptr, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!g_am_MainWindowHandle_p) {
        PRINT_ERROR_MESSAGE(CreateWindowExW)
        am_Cleanup_p();
        return E_FAIL;
    }

    if (FAILED(am_GetWindowDpi_p(g_am_MainWindowHandle_p, &g_am_CurrentDpi_p)) || (g_am_CurrentDpi_p == 0)) {
        g_am_CurrentDpi_p = USER_DEFAULT_SCREEN_DPI;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (FAILED(am_UpdateFrameMargins_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p))) {
        am_Print_p(L"Failed to update main window's frame margins.");
        am_Cleanup_p();
        return E_FAIL;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (FAILED(am_TriggerFrameChange_p(g_am_MainWindowHandle_p))) {
        am_Print_p(L"Failed to trigger frame change event for main window.");
        am_Cleanup_p();
        return E_FAIL;
    }
    // Ensure our window still has window transitions.
    if (FAILED(am_SetWindowTransitionsEnabled_p(g_am_MainWindowHandle_p, true))) {
        am_Print_p(L"Failed to enable window transitions for main window.");
        am_Cleanup_p();
        return E_FAIL;
    }

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_CreateDragBarWindow_p()
{
    if (!g_am_MainWindowHandle_p) {
        am_Cleanup_p();
        return E_INVALIDARG;
    }

    // Please refer to the "IMPORTANT NOTE" section below.
    if (!g_am_IsWindows8OrGreater_p) {
        am_Print_p(L"Drag bar window is only available on Windows 8 and onwards.");
        am_Cleanup_p();
        return E_FAIL;
    }
    if ((g_am_DragBarWindowAtom_p == 0) || g_am_DragBarWindowHandle_p) {
        am_Cleanup_p();
        return E_INVALIDARG;
    }

    // The drag bar window is a child window of the top level window that is put
    // right on top of the drag bar. The XAML island window "steals" our mouse
    // messages which makes it hard to implement a custom drag area. By putting
    // a window on top of it, we prevent it from "stealing" the mouse messages.
    //
    // IMPORTANT NOTE: The WS_EX_LAYERED style is supported for both top-level
    // windows and child windows since Windows 8. Previous Windows versions support
    // WS_EX_LAYERED only for top-level windows.
    g_am_DragBarWindowHandle_p = CreateWindowExW(WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP,
                                          g_am_DragBarWindowClassName_p, g_am_DragBarWindowTitle_p,
                                          WS_CHILD,
                                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                          g_am_MainWindowHandle_p, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!g_am_DragBarWindowHandle_p) {
        PRINT_ERROR_MESSAGE(CreateWindowExW)
        am_Cleanup_p();
        return E_FAIL;
    }

    // Layered window won't be visible until we call SetLayeredWindowAttributes()
    // or UpdateLayeredWindow().
    if (SetLayeredWindowAttributes(g_am_DragBarWindowHandle_p, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
        am_Print_p(L"SetLayeredWindowAttributes() failed.");
        am_Cleanup_p();
        return E_FAIL;
    }

    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(g_am_MainWindowHandle_p, &rect) == FALSE) {
        am_Print_p(L"Failed to retrieve client rect of main window.");
        am_Cleanup_p();
        return E_FAIL;
    }
    int tbh = 0;
    if (FAILED(am_GetTitleBarHeight_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p, &tbh))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    if (SetWindowPos(g_am_DragBarWindowHandle_p, HWND_TOP, 0, 0, rect.right,  tbh,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        am_Print_p(L"Failed to move drag bar window.");
        am_Cleanup_p();
        return E_FAIL;
    }

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_CreateXAMLIsland_p()
{
    if (!g_am_MainWindowHandle_p) {
        am_Cleanup_p();
        return E_INVALIDARG;
    }

    // XAML Island is only supported on Windows 10 19H1 and onwards.
    if (!g_am_IsXAMLIslandAvailable_p) {
        am_Print_p(L"XAML Island is only supported on Windows 10 19H1 and onwards.");
        am_Cleanup_p();
        return E_FAIL;
    }
    SystemTheme systemTheme = SystemTheme::Invalid;
    if (FAILED(am_GetSystemTheme_p(&systemTheme))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    if (systemTheme == SystemTheme::Invalid) {
        am_Print_p(L"Failed to retrieve system theme.");
        am_Cleanup_p();
        return E_FAIL;
    }
    if (systemTheme == SystemTheme::HighContrast) {
        am_Print_p(L"High contrast mode is on.");
        am_Cleanup_p();
        return E_FAIL;
    }

    winrt::init_apartment(winrt::apartment_type::single_threaded);
    g_am_XAMLManager_p = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();

    g_am_XAMLSource_p = {};
    const auto interop = g_am_XAMLSource_p.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        am_Print_p(L"Failed to retrieve IDesktopWindowXamlSourceNative.");
        am_Cleanup_p();
        return E_FAIL;
    }
    winrt::check_hresult(interop->AttachToWindow(g_am_MainWindowHandle_p));
    winrt::check_hresult(interop->get_WindowHandle(&g_am_XAMLIslandWindowHandle_p));
    if (!g_am_XAMLIslandWindowHandle_p) {
        am_Print_p(L"Failed to retrieve XAML Island window handle.");
        am_Cleanup_p();
        return E_FAIL;
    }
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(g_am_MainWindowHandle_p, &rect) == FALSE) {
        am_Print_p(L"Failed to retrieve client rect of main window.");
        am_Cleanup_p();
        return E_FAIL;
    }
    // Give enough space to our thin homemade top border.
    int borderThickness = 0;
    if (FAILED(am_GetWindowVisibleFrameBorderThickness_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p, &borderThickness))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    if (SetWindowPos(g_am_XAMLIslandWindowHandle_p, HWND_BOTTOM, 0,
                 borderThickness, rect.right, (rect.bottom - borderThickness),
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        am_Print_p(L"Failed to move XAML Island window.");
        am_Cleanup_p();
        return E_FAIL;
    }
    g_am_BackgroundBrush_p = {};
    if (FAILED(am_SwitchAcrylicBrushTheme_p((systemTheme == SystemTheme::Auto) ? SystemTheme::Dark : systemTheme))) {
        am_Print_p(L"Failed to change acrylic brush's theme.");
        am_Cleanup_p();
        return E_FAIL;
    }
    g_am_BrushTheme_p = SystemTheme::Auto;
    g_am_BackgroundBrush_p.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_am_RootGrid_p = {};
    g_am_RootGrid_p.Background(g_am_BackgroundBrush_p);
    //g_am_RootGrid_p.Children().Clear();
    //g_am_RootGrid_p.Children().Append(/* some UWP control */);
    //g_am_RootGrid_p.UpdateLayout();
    g_am_XAMLSource_p.Content(g_am_RootGrid_p);

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_InitializeDirect2DInfrastructure_p()
{
    if (!g_am_MainWindowHandle_p || g_am_D2DFactory_p || g_am_D2DRenderTarget_p) {
        am_Cleanup_p();
        return E_INVALIDARG;
    }
    const int screen = 0; // fixme: use the correct screen id.
    if (FAILED(am_GetWallpaperFilePath_p(screen, &g_am_WallpaperFilePath_p))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    if (FAILED(am_GetDesktopBackgroundColor_p(&g_am_DesktopBackgroundColor_p))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    if (FAILED(am_GetWallpaperAspectStyle_p(screen, &g_am_WallpaperAspectStyle_p))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_am_D2DFactory_p))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    SIZE size = {};
    if (FAILED(am_GetWindowSize_p(g_am_MainWindowHandle_p, &size))) {
        am_Cleanup_p();
        return E_FAIL;
    }
    if (FAILED(g_am_D2DFactory_p->CreateHwndRenderTarget({}, D2D1::HwndRenderTargetProperties(g_am_MainWindowHandle_p, D2D1::SizeU(size.cx, size.cy)), &g_am_D2DRenderTarget_p))) {
        am_Cleanup_p();
        return E_FAIL;
    }
#if 0
    if (FAILED(am_CreateD2DBitmapBrushFromURI_p(g_am_D2DRenderTarget_p, g_am_WallpaperFilePath_p, size.cx, size.cy, &g_am_D2DWallpaperBitmap_p))) {
        am_Cleanup_p();
        return E_FAIL;
    }
#endif
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_InitializeAcrylicManager_p(const int x, const int y, const int w, const int h)
{
    static bool tried = false;
    if (tried) {
        return E_FAIL;
    }
    tried = true;

    if (FAILED(am_RegisterMainWindowClass_p())) {
        am_Print_p(L"Failed to register main window class.", true);
        return E_FAIL;
    }
    if (FAILED(am_CreateMainWindow_p(x, y, w, h))) {
        am_Print_p(L"Failed to create main window.", true);
        return E_FAIL;
    }
    if (g_am_IsWindows10OrGreater_p) {
        if (g_am_IsXAMLIslandAvailable_p) {
            if (SUCCEEDED(am_CreateXAMLIsland_p())) {
                if (SUCCEEDED(am_RegisterDragBarWindowClass_p())) {
                    if (SUCCEEDED(am_CreateDragBarWindow_p())) {
                        return S_OK;
                    } else {
                        am_Print_p(L"Failed to create drag bar window.", true);
                        return E_FAIL;
                    }
                } else {
                    am_Print_p(L"Failed to register drag bar window class.", true);
                    return E_FAIL;
                }
            } else {
                am_Print_p(L"Failed to create XAML Island.", true);
                return E_FAIL;
            }
        }
    }
    return am_InitializeDirect2DInfrastructure_p();
}

[[nodiscard]] static inline HRESULT am_GetWindowHandle_p(HWND *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    *result = g_am_MainWindowHandle_p;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetBrushTheme_p(SystemTheme *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    *result = g_am_BrushTheme_p;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_MainWindowEventLoop_p(int *result)
{
    if (!result || !g_am_MainWindowHandle_p) {
        return E_INVALIDARG;
    }

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    *result = static_cast<int>(msg.wParam);

    return S_OK;
}

// Public interface

HRESULT am_GetVersion(LPWSTR *ver)
{
    if (!ver) {
        return E_INVALIDARG;
    }
    const auto str = new wchar_t[10]; // 10 should be enough for a version string...
    SecureZeroMemory(str, sizeof(str));
    memcpy(str, ACRYLICMANAGER_VERSION_STR, wcslen(ACRYLICMANAGER_VERSION_STR));
    *ver = str;
    return S_OK;
}

HRESULT am_CreateWindow(const int x, const int y, const int w, const int h)
{
    if (!g_am_IsWindows7OrGreater_p) {
        am_Print_p(L"This application cannot be run on such old systems.", true);
        return E_FAIL;
    }

    bool dwmComp = false;
    if (FAILED(am_IsCompositionEnabled_p(&dwmComp)) || !dwmComp) {
        am_Print_p(L"This application could not be started when DWM composition is disabled.", true);
        return E_FAIL;
    }

    return am_InitializeAcrylicManager_p(x, y, w, h);
}

HRESULT am_GetWindowGeometry(RECT *result)
{
    return am_GetWindowGeometry_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_SetWindowGeometry(const int x, const int y, const int w, const int h)
{
    return am_SetWindowGeometry_p(g_am_MainWindowHandle_p, x, y, w, h);
}

HRESULT am_MoveWindow(const int x, const int y)
{
    return am_MoveWindow_p(g_am_MainWindowHandle_p, x, y);
}

HRESULT am_GetWindowSize(SIZE *result)
{
    return am_GetWindowSize_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_ResizeWindow(const int w, const int h)
{
    return am_ResizeWindow_p(g_am_MainWindowHandle_p, w, h);
}

HRESULT am_CenterWindow()
{
    return am_CenterWindow_p(g_am_MainWindowHandle_p);
}

HRESULT am_GetWindowState(WindowState *result)
{
    return am_GetWindowState_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_SetWindowState(const WindowState state)
{
    return am_SetWindowState_p(g_am_MainWindowHandle_p, state);
}

HRESULT am_CloseWindow()
{
    am_Cleanup_p();
    return S_OK;
}

HRESULT am_GetWindowHandle(HWND *result)
{
    return am_GetWindowHandle_p(result);
}

HRESULT am_GetBrushTheme(SystemTheme *result)
{
    return am_GetBrushTheme_p(result);
}

HRESULT am_SetBrushTheme(const SystemTheme theme)
{
    return am_SwitchAcrylicBrushTheme_p(theme);
}

HRESULT am_GetTintColor(int *r, int *g, int *b, int *a)
{
    return am_GetTintColor_p(r, g, b, a);
}

HRESULT am_SetTintColor(const int r, const int g, const int b, const int a)
{
    return am_SetTintColor_p(r, g, b, a);
}

HRESULT am_GetTintOpacity(double *opacity)
{
    return am_GetTintOpacity_p(opacity);
}

HRESULT am_SetTintOpacity(const double opacity)
{
    return am_SetTintOpacity_p(opacity);
}

HRESULT am_GetTintLuminosityOpacity(double *opacity)
{
    return am_GetTintLuminosityOpacity_p(opacity);
}

HRESULT am_SetTintLuminosityOpacity(const double opacity)
{
    return am_SetTintLuminosityOpacity_p(opacity);
}

HRESULT am_GetFallbackColor(int *r, int *g, int *b, int *a)
{
    return am_GetFallbackColor_p(r, g, b, a);
}

HRESULT am_SetFallbackColor(const int r, const int g, const int b, const int a)
{
    return am_SetFallbackColor_p(r, g, b, a);
}

HRESULT am_EventLoopExec(int *result)
{
    return am_MainWindowEventLoop_p(result);
}

HRESULT am_IsWindowActive(bool *result)
{
    return am_IsWindowActive_p(g_am_MainWindowHandle_p, result);
}

// DLL entry point function

#ifndef ACRYLICMANAGER_STATIC
BOOL APIENTRY
DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif
