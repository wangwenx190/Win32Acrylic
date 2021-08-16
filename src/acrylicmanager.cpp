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

#include <strsafe.h>

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
const bool __result_##function = am_ShowErrorMessageFromLastErrorCode_p(L#function); \
static_cast<void>(__result_##function);
#endif

// The thickness of an auto-hide taskbar in pixels.
static const int g_am_AutoHideTaskbarThicknessPx_p = 2;
static const int g_am_AutoHideTaskbarThicknessPy_p = g_am_AutoHideTaskbarThicknessPx_p;

static LPCWSTR g_am_PersonalizeRegistryKey_p = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";
static LPCWSTR g_am_WindowClassNamePrefix_p = LR"(wangwenx190\AcrylicManager\WindowClass\)";
static LPCWSTR g_am_MainWindowClassNameSuffix_p = L"@MainWindow";
static LPCWSTR g_am_DragBarWindowClassNameSuffix_p = L"@DragBarWindow";

static LPWSTR g_am_MainWindowClassName_p = nullptr;
static LPWSTR g_am_DragBarWindowClassName_p = nullptr;
static LPCWSTR g_am_MainWindowTitle_p = L"AcrylicManager Main Window";
static LPCWSTR g_am_DragBarWindowTitle_p = L"";
static ATOM g_am_MainWindowAtom_p = 0;
static ATOM g_am_DragBarWindowAtom_p = 0;
static HWND g_am_MainWindowHandle_p = nullptr;
static HWND g_am_XAMLIslandWindowHandle_p = nullptr;
static HWND g_am_DragBarWindowHandle_p = nullptr;
static UINT g_am_CurrentDpi_p = 0;
static SystemTheme g_am_BrushTheme_p = SystemTheme::Invalid;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_am_XAMLManager_p = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_am_XAMLSource_p = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_am_RootGrid_p = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_am_BackgroundBrush_p = nullptr;

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

[[nodiscard]] static inline double am_GetDevicePixelRatio_p(const UINT dpi)
{
    if (dpi == 0) {
        return 1.0;
    }
    return (static_cast<double>(dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
}

UINT am_GetWindowDpi_p(const HWND hWnd)
{
    if (!hWnd) {
        return USER_DEFAULT_SCREEN_DPI;
    }
    {
        const UINT dpi = GetDpiForWindow(hWnd);
        if (dpi > 0) {
            return dpi;
        }
    }
    {
        const UINT dpi = GetSystemDpiForProcess(GetCurrentProcess());
        if (dpi > 0) {
            return dpi;
        }
    }
    {
        const UINT dpi = GetDpiForSystem();
        if (dpi > 0) {
            return dpi;
        }
    }
    {
        UINT dpiX = 0, dpiY = 0;
        if (SUCCEEDED(GetDpiForMonitor(CURRENT_SCREEN(hWnd), MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
            if ((dpiX > 0) && (dpiY > 0)) {
                return std::round(static_cast<double>(dpiX + dpiY) / 2.0);
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
                return std::round(static_cast<double>(dpiX + dpiY) / 2.0);
            }
        }
    }
    return USER_DEFAULT_SCREEN_DPI;
}

bool am_IsMinimized_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsMinimized(hWnd) != FALSE);
}

bool am_IsMaximized_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsMaximized(hWnd) != FALSE);
}

int am_GetResizeBorderThickness_p(const bool x, const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    // There is no "SM_CYPADDEDBORDER".
    const int result = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, _dpi)
            + GetSystemMetricsForDpi((x ? SM_CXSIZEFRAME : SM_CYSIZEFRAME), _dpi);
    const int preset = std::round(8.0 * am_GetDevicePixelRatio_p(_dpi));
    return ((result > 0) ? result : preset);
}

int am_GetCaptionHeight_p(const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    const int result = GetSystemMetricsForDpi(SM_CYCAPTION, _dpi);
    const int preset = std::round(23.0 * am_GetDevicePixelRatio_p(_dpi));
    return ((result > 0) ? result : preset);
}

int am_GetTitleBarHeight_p(const HWND hWnd, const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    int titleBarHeight = 0;
    if (hWnd) {
        RECT frame = {0, 0, 0, 0};
        if (AdjustWindowRectExForDpi(&frame,
                                 (static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)) & ~WS_OVERLAPPED),
                                 FALSE,
                                 static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)),
                                 _dpi) == FALSE) {
            am_Print_p(L"AdjustWindowRectExForDpi() failed.");
        } else {
            titleBarHeight = std::abs(frame.top);
        }
    }
    if (titleBarHeight <= 0) {
        titleBarHeight = am_GetResizeBorderThickness_p(false, _dpi) + am_GetCaptionHeight_p(_dpi);
        if (titleBarHeight <= 0) {
            titleBarHeight = std::round(31.0 * am_GetDevicePixelRatio_p(_dpi));
        }
    }
    return titleBarHeight;
}

[[nodiscard]] static inline MONITORINFO am_GetMonitorInfo_p(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    const HMONITOR mon = CURRENT_SCREEN(hWnd);
    if (!mon) {
        am_Print_p(L"Failed to retrieve current screen.");
        return {};
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        am_Print_p(L"Failed to retrieve monitor information.");
        return {};
    }
    return mi;
}

RECT am_GetScreenGeometry_p(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    return am_GetMonitorInfo_p(hWnd).rcMonitor;
}

RECT am_GetScreenAvailableGeometry_p(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    return am_GetMonitorInfo_p(hWnd).rcWork;
}

bool am_IsCompositionEnabled_p()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (am_CompareSystemVersion_p(WindowsVersion::Windows8, VersionCompare::GreaterOrEqual)) {
        return true;
    }
    BOOL enabled = FALSE;
    return (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && (enabled != FALSE));
}

bool am_IsFullScreened_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &windowRect) == FALSE) {
        PRINT_ERROR_MESSAGE(GetWindowRect)
        return false;
    }
    const HMONITOR mon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    if (!mon) {
        PRINT_ERROR_MESSAGE(MonitorFromWindow)
        return false;
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        PRINT_ERROR_MESSAGE(GetMonitorInfoW)
        return false;
    }
    const RECT screenRect = mi.rcMonitor;
    return ((windowRect.left == screenRect.left)
            && (windowRect.right == screenRect.right)
            && (windowRect.top == screenRect.top)
            && (windowRect.bottom == screenRect.bottom));
}

bool am_IsWindowNoState_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    if (GetWindowPlacement(hWnd, &wp) == FALSE) {
        am_Print_p(L"Failed to retrieve window placement of main window.");
        return false;
    }
    return (wp.showCmd == SW_NORMAL);
}

bool am_IsWindowVisible_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsWindowVisible(hWnd) != FALSE);
}

bool am_TriggerFrameChange_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    const BOOL result = SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    return (result != FALSE);
}

bool am_SetWindowTransitionsEnabled_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return false;
    }
    const BOOL disabled = enable ? FALSE : TRUE;
    return SUCCEEDED(DwmSetWindowAttribute(hWnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disabled, sizeof(disabled)));
}

bool am_OpenSystemMenu_p(const HWND hWnd, const POINT pos)
{
    if (!hWnd) {
        return false;
    }
    const HMENU menu = GetSystemMenu(hWnd, FALSE);
    if (!menu) {
        am_Print_p(L"Failed to retrieve system menu of main window.");
        return false;
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
    const bool isMaximized = am_IsMaximized_p(hWnd);
    if (!setState(SC_RESTORE, isMaximized)) {
        am_Print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_MOVE, !isMaximized)) {
        am_Print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_SIZE, !isMaximized)) {
        am_Print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_MINIMIZE, true)) {
        am_Print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_MAXIMIZE, !isMaximized)) {
        am_Print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_CLOSE, true)) {
        am_Print_p(L"Failed to change menu item state.");
        return false;
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        am_Print_p(L"Failed to set default menu item.");
        return false;
    }
    // ### TODO: support LTR layout.
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, pos.x, pos.y, 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            am_Print_p(L"Failed to post system menu message to main window.");
            return false;
        }
    }
    return true;
}

bool am_CompareSystemVersion_p(const WindowsVersion ver, const VersionCompare comp)
{
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
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

[[nodiscard]] static inline int am_GetWindowVisibleFrameThickness_p(const HWND hWnd, const UINT dpi)
{
    // TODO: DwmGetWindowAttribute().
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    return ((hWnd && am_IsWindowNoState_p(hWnd)) ? std::round(1.0 * am_GetDevicePixelRatio_p(_dpi)) : 0);
}

[[nodiscard]] static inline bool am_UpdateFrameMargins_p(const HWND hWnd, const UINT dpi)
{
    if (!hWnd) {
        return false;
    }
    const bool normal = am_IsWindowNoState_p(hWnd);
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    const LONG border = (normal ? am_GetWindowVisibleFrameThickness_p(hWnd, _dpi) : 0);
    const LONG topFrameMargin = (normal ? /*am_GetTitleBarHeight_p(hWnd, _dpi)*/border : border);
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
    const MARGINS margins = {border, border, topFrameMargin, border};
    return SUCCEEDED(DwmExtendFrameIntoClientArea(hWnd, &margins));
}

[[nodiscard]] static inline bool am_ShowFullScreen_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return false;
    }
    auto style = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE));
    style &= ~(WS_OVERLAPPEDWINDOW); // fixme: check
    style |= WS_POPUP;
    SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    const RECT rect = am_GetScreenGeometry_p(hWnd);
    return (MoveWindow(hWnd, rect.left, rect.top,
                       RECT_WIDTH(rect), RECT_HEIGHT(rect), TRUE) != FALSE);
}

[[nodiscard]] static inline SystemTheme am_GetSystemTheme_p()
{
    if (am_IsHighContrastModeOn_p()) {
        am_Print_p(L"High contrast mode is on.");
        return SystemTheme::HighContrast;
    }
    // Dark mode was first introduced in Windows 10 1607.
    if (am_CompareSystemVersion_p(WindowsVersion::Windows10_1607, VersionCompare::GreaterOrEqual)) {
        bool lightModeOn = false;
        if (!am_ShouldAppsUseLightTheme_p(&lightModeOn)) {
            if (!am_ShouldSystemUsesLightTheme_p(&lightModeOn)) {
                am_Print_p(L"Failed to retrieve the system theme setting.");
                return SystemTheme::Invalid;
            }
        }
        return (lightModeOn ? SystemTheme::Light : SystemTheme::Dark);
    } else {
        am_Print_p(L"Current system version doesn't support dark mode.");
    }
    return SystemTheme::Invalid;
}

[[nodiscard]] static inline RECT am_GetWindowGeometry_p()
{
    if (!g_am_MainWindowHandle_p) {
        return {};
    }
    RECT geo = {0, 0, 0, 0};
    if (GetWindowRect(g_am_MainWindowHandle_p, &geo) == FALSE) {
        PRINT_ERROR_MESSAGE(GetWindowRect)
        return {};
    }
    return geo;
}

[[nodiscard]] static inline SIZE am_GetWindowSize_p()
{
    if (!g_am_MainWindowHandle_p) {
        return {};
    }
    const RECT geo = am_GetWindowGeometry_p();
    return {RECT_WIDTH(geo), RECT_HEIGHT(geo)};
}

[[nodiscard]] static inline bool am_MoveWindow_p(const int x, const int y)
{
    if (!g_am_MainWindowHandle_p) {
        return false;
    }
    if ((x <= 0) || (y <= 0)) {
        am_Print_p(L"The given top-left coordinate is not correct.");
        return false;
    }
    const SIZE s = am_GetWindowSize_p();
    return (MoveWindow(g_am_MainWindowHandle_p, x, y, s.cx, s.cy, TRUE) != FALSE);
}

[[nodiscard]] static inline bool am_ResizeWindow_p(const int w, const int h)
{
    if (!g_am_MainWindowHandle_p) {
        return false;
    }
    if ((w <= 0) || (h <= 0)) {
        am_Print_p(L"Can't resize window to empty or negative size.");
        return false;
    }
    const RECT geo = am_GetWindowGeometry_p();
    return (MoveWindow(g_am_MainWindowHandle_p, geo.left, geo.top, w, h, TRUE) != FALSE);
}

[[nodiscard]] static inline bool am_CenterWindow_p()
{
    if (!g_am_MainWindowHandle_p) {
        return false;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(g_am_MainWindowHandle_p, &windowRect) == FALSE) {
        am_Print_p(L"Failed to retrieve window rect of main window.");
        return false;
    }
    const int windowWidth = RECT_WIDTH(windowRect);
    const int windowHeight = RECT_HEIGHT(windowRect);
    const RECT screenRect = am_GetScreenGeometry_p(g_am_MainWindowHandle_p);
    const int screenWidth = RECT_WIDTH(screenRect);
    const int screenHeight = RECT_HEIGHT(screenRect);
    const int newX = (screenWidth - windowWidth) / 2;
    const int newY = (screenHeight - windowHeight) / 2;
    return (MoveWindow(g_am_MainWindowHandle_p, newX, newY, windowWidth, windowHeight, TRUE) != FALSE);
}

[[nodiscard]] static inline WindowState am_GetWindowState_p()
{
    if (!g_am_MainWindowHandle_p) {
        return WindowState::Invalid;
    }
    if (am_IsFullScreened_p(g_am_MainWindowHandle_p)) {
        return WindowState::FullScreened;
    } else if (am_IsMaximized_p(g_am_MainWindowHandle_p)) {
        return WindowState::Maximized;
    } else if (am_IsMinimized_p(g_am_MainWindowHandle_p)) {
        return WindowState::Minimized;
    } else if (am_IsWindowNoState_p(g_am_MainWindowHandle_p)) {
        return WindowState::Normal;
    } else if (am_IsWindowVisible_p(g_am_MainWindowHandle_p)) {
        return WindowState::Shown;
    } else if (!am_IsWindowVisible_p(g_am_MainWindowHandle_p)) {
        return WindowState::Hidden;
    }
    return WindowState::Invalid;
}

[[nodiscard]] static inline bool am_SetWindowState_p(const WindowState state)
{
    if (!g_am_MainWindowHandle_p) {
        return false;
    }
    switch (state) {
    case WindowState::Normal:
        ShowWindow(g_am_MainWindowHandle_p, SW_NORMAL);
        return true;
    case WindowState::Maximized:
        ShowWindow(g_am_MainWindowHandle_p, SW_MAXIMIZE);
        return true;
    case WindowState::Minimized:
        ShowWindow(g_am_MainWindowHandle_p, SW_MINIMIZE);
        return true;
    case WindowState::FullScreened:
        return am_ShowFullScreen_p(g_am_MainWindowHandle_p, true);
    case WindowState::Hidden:
        ShowWindow(g_am_MainWindowHandle_p, SW_HIDE);
        return true;
    case WindowState::Shown:
        ShowWindow(g_am_MainWindowHandle_p, SW_SHOW);
        return true;
    default:
        break;
    }
    return false;
}

static inline void am_Cleanup_p()
{
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
    if (g_am_DragBarWindowHandle_p) {
        DestroyWindow(g_am_DragBarWindowHandle_p);
        PRINT_ERROR_MESSAGE(DestroyWindow)
        g_am_DragBarWindowHandle_p = nullptr;
    }
    if (g_am_MainWindowHandle_p) {
        DestroyWindow(g_am_MainWindowHandle_p);
        PRINT_ERROR_MESSAGE(DestroyWindow)
        g_am_MainWindowHandle_p = nullptr;
    }
    if (g_am_DragBarWindowAtom_p != 0) {
        UnregisterClassW(g_am_DragBarWindowClassName_p, HINST_THISCOMPONENT);
        PRINT_ERROR_MESSAGE(UnregisterClassW)
        g_am_DragBarWindowAtom_p = 0;
        delete [] g_am_DragBarWindowClassName_p;
        g_am_DragBarWindowClassName_p = nullptr;
    }
    if (g_am_MainWindowAtom_p != 0) {
        UnregisterClassW(g_am_MainWindowClassName_p, HINST_THISCOMPONENT);
        PRINT_ERROR_MESSAGE(UnregisterClassW)
        g_am_MainWindowAtom_p = 0;
        delete [] g_am_MainWindowClassName_p;
        g_am_MainWindowClassName_p = nullptr;
    }
}

[[nodiscard]] static inline bool am_GetTintColor_p(int *r, int *g, int *b, int *a)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (!r || !g || !b || !a) {
        am_Print_p(L"Can't retrieve tint color: invalid parameters.");
        return false;
    }
    const winrt::Windows::UI::Color color = g_am_BackgroundBrush_p.TintColor();
    *r = static_cast<int>(color.R);
    *g = static_cast<int>(color.G);
    *b = static_cast<int>(color.B);
    *a = static_cast<int>(color.A);
    return true;
}

[[nodiscard]] static inline bool am_SetTintColor_p(const int r, const int g, const int b, const int a)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    const auto red = static_cast<uint8_t>(std::clamp(r, 0, 255));
    const auto green = static_cast<uint8_t>(std::clamp(g, 0, 255));
    const auto blue = static_cast<uint8_t>(std::clamp(b, 0, 255));
    const auto alpha = static_cast<uint8_t>(std::clamp(a, 0, 255));
    g_am_BackgroundBrush_p.TintColor({alpha, red, green, blue}); // ARGB
    return true;
}

[[nodiscard]] static inline bool am_GetTintOpacity_p(double *opacity)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (!opacity) {
        am_Print_p(L"Can't retrieve tint opacity: invalid parameter.");
        return false;
    }
    const double value = g_am_BackgroundBrush_p.TintOpacity();
    *opacity = value;
    return true;
}

[[nodiscard]] static inline bool am_SetTintOpacity_p(const double opacity)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    const double value = std::clamp(opacity, 0.0, 1.0);
    g_am_BackgroundBrush_p.TintOpacity(value);
    return true;
}

[[nodiscard]] static inline bool am_GetTintLuminosityOpacity_p(double *opacity)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (!opacity) {
        am_Print_p(L"Can't retrieve tint luminosity opacity: invalid parameter.");
        return false;
    }
    const double value = g_am_BackgroundBrush_p.TintLuminosityOpacity().GetDouble();
    *opacity = value;
    return true;
}

[[nodiscard]] static inline bool am_SetTintLuminosityOpacity_p(const double opacity)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    const double value = std::clamp(opacity, 0.0, 1.0);
    g_am_BackgroundBrush_p.TintLuminosityOpacity(value);
    return true;
}

[[nodiscard]] static inline bool am_GetFallbackColor_p(int *r, int *g, int *b, int *a)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (!r || !g || !b || !a) {
        am_Print_p(L"Can't retrieve fallback color: invalid parameters.");
        return false;
    }
    const winrt::Windows::UI::Color color = g_am_BackgroundBrush_p.FallbackColor();
    *r = static_cast<int>(color.R);
    *g = static_cast<int>(color.G);
    *b = static_cast<int>(color.B);
    *a = static_cast<int>(color.A);
    return true;
}

[[nodiscard]] static inline bool am_SetFallbackColor_p(const int r, const int g, const int b, const int a)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    const auto red = static_cast<uint8_t>(std::clamp(r, 0, 255));
    const auto green = static_cast<uint8_t>(std::clamp(g, 0, 255));
    const auto blue = static_cast<uint8_t>(std::clamp(b, 0, 255));
    const auto alpha = static_cast<uint8_t>(std::clamp(a, 0, 255));
    g_am_BackgroundBrush_p.FallbackColor({alpha, red, green, blue}); // ARGB
    return true;
}

[[nodiscard]] static inline bool am_SwitchAcrylicBrushTheme_p(const SystemTheme theme)
{
    if (!g_am_BackgroundBrush_p) {
        am_Print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (theme == SystemTheme::Invalid) {
        am_Print_p(L"The given theme type is not valid.");
        return false;
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
    if (!am_SetTintColor_p(static_cast<int>(tc.R),
                           static_cast<int>(tc.G),
                           static_cast<int>(tc.B),
                           static_cast<int>(tc.A))) {
        am_Print_p(L"Failed to change acrylic brush's tint color.");
        return false;
    }
    if (!am_SetTintOpacity_p(to)) {
        am_Print_p(L"Failed to change acrylic brush's tint opacity.");
        return false;
    }
    if (!am_SetTintLuminosityOpacity_p(tlo)) {
        am_Print_p(L"Failed to change acrylic brush's tint luminosity opacity.");
        return false;
    }
    if (!am_SetFallbackColor_p(static_cast<int>(fbc.R),
                               static_cast<int>(fbc.G),
                               static_cast<int>(fbc.B),
                               static_cast<int>(fbc.A))) {
        am_Print_p(L"Failed to change acrylic brush's fallback color.");
        return false;
    }
    g_am_BrushTheme_p = theme;
    return true;
}

bool am_GenerateGUID_p(LPWSTR *guid)
{
    if (!guid) {
        return false;
    }
    if (FAILED(CoInitialize(nullptr))) {
        am_Print_p(L"Failed to initialize COM.");
        return false;
    }
    GUID uuid = {};
    if (FAILED(CoCreateGuid(&uuid))) {
        am_Print_p(L"Failed to create GUID.");
        CoUninitialize();
        return false;
    }
    CoUninitialize();
    const auto buf = new wchar_t[64];
    SecureZeroMemory(buf, sizeof(buf));
    swprintf(buf,
            L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            uuid.Data1, uuid.Data2, uuid.Data3,
            uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3],
            uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    *guid = buf;
    return true;
}

[[nodiscard]] static inline DWORD am_GetDWORDValueFromRegistry_p(const HKEY rootKey, LPCWSTR subKey,
                                                                 LPCWSTR valueName, bool *ok = nullptr)
{
    if (ok) {
        *ok = false;
    }
    if (!rootKey || !subKey || !valueName) {
        return 0;
    }
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(rootKey, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return 0;
    }
    DWORD dwValue = 0;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(dwValue);
    const bool success = (RegQueryValueExW(hKey, valueName, nullptr, &dwType,
                                reinterpret_cast<LPBYTE>(&dwValue), &dwSize) == ERROR_SUCCESS);
    RegCloseKey(hKey);
    if (ok) {
        *ok = success;
    }
    return dwValue;
}

bool am_ShouldAppsUseLightTheme_p(bool *result)
{
    if (!result) {
        return false;
    }
    bool ok = false;
    const DWORD value = am_GetDWORDValueFromRegistry_p(HKEY_CURRENT_USER, g_am_PersonalizeRegistryKey_p, L"AppsUseLightTheme", &ok);
    if (!ok) {
        return false;
    }
    *result = (value != 0);
    return true;
}

bool am_ShouldSystemUsesLightTheme_p(bool *result)
{
    if (!result) {
        return false;
    }
    bool ok = false;
    const DWORD value = am_GetDWORDValueFromRegistry_p(HKEY_CURRENT_USER, g_am_PersonalizeRegistryKey_p, L"SystemUsesLightTheme", &ok);
    if (!ok) {
        return false;
    }
    *result = (value != 0);
    return true;
}

bool am_IsHighContrastModeOn_p()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        PRINT_ERROR_MESSAGE(SystemParametersInfoW)
        return false;
    }
    return (hc.dwFlags & HCF_HIGHCONTRASTON);
}

bool am_SetWindowCompositionAttribute_p(const HWND hWnd, LPWINDOWCOMPOSITIONATTRIBDATA pwcad)
{
    if (!hWnd || !pwcad) {
        return false;
    }
    static bool tried = false;
    using sig = BOOL(WINAPI *)(HWND, LPWINDOWCOMPOSITIONATTRIBDATA);
    static sig func = nullptr;
    if (!func) {
        if (tried) {
            return false;
        } else {
            tried = true;
            const HMODULE dll = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (!dll) {
                PRINT_ERROR_MESSAGE(LoadLibraryExW)
                return false;
            }
            func = reinterpret_cast<sig>(GetProcAddress(dll, "SetWindowCompositionAttribute"));
            if (!func) {
                PRINT_ERROR_MESSAGE(GetProcAddress)
                return false;
            }
        }
    }
    return (func(hWnd, pwcad) != FALSE);
}

bool am_IsWindowTransitionsEnabled_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    BOOL disabled = FALSE;
    if (FAILED(DwmGetWindowAttribute(hWnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disabled, sizeof(disabled)))) {
        am_Print_p(L"Failed to retrieve window transitions state.");
        return false;
    }
    return (disabled == FALSE);
}

bool am_ShouldWindowUseDarkFrame_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return false;
    }
    // todo
    return false;
}

COLORREF am_GetColorizationColor_p()
{
    COLORREF color = RGB(0, 0, 0);
    BOOL opaque = FALSE;
    if (FAILED(DwmGetColorizationColor(&color, &opaque))) {
        am_Print_p(L"Failed to retrieve the colorization color.");
        return RGB(128, 128, 128); // #808080, DarkGray
    }
    return color;
}

ColorizationArea am_GetColorizationArea_p()
{
    // todo
    return ColorizationArea::None;
}

void am_FreeStringA_p(void *mem)
{
    if (!mem) {
        return;
    }
    delete [] static_cast<LPSTR>(mem);
    mem = nullptr;
}

void am_FreeStringW_p(void *mem)
{
    if (!mem) {
        return;
    }
    delete [] static_cast<LPWSTR>(mem);
    mem = nullptr;
}

bool am_ShowErrorMessageFromLastErrorCode_p(LPCWSTR functionName)
{
    if (!functionName) {
        return false;
    }

    const DWORD dw = GetLastError();
    if (dw == 0) {
        return true;
    }

    LPVOID lpMsgBuf = nullptr;
    LPVOID lpDisplayBuf = nullptr;

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&lpMsgBuf),
        0, nullptr );

    lpDisplayBuf = reinterpret_cast<LPVOID>(LocalAlloc(LMEM_ZEROINIT,
        (wcslen(reinterpret_cast<LPCWSTR>(lpMsgBuf))
         + wcslen(reinterpret_cast<LPCWSTR>(functionName)) + 40) * sizeof(wchar_t)));
    StringCchPrintfW(reinterpret_cast<LPWSTR>(lpDisplayBuf),
        LocalSize(lpDisplayBuf) / sizeof(wchar_t),
        L"%s failed with error %d: %s",
        functionName, dw, lpMsgBuf);
    am_Print_p(reinterpret_cast<LPCWSTR>(lpDisplayBuf), true);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);

    return false;
}

[[nodiscard]] static inline bool am_SetWindowGeometry_p(const int x, const int y, const int w, const int h)
{
    if (!g_am_MainWindowHandle_p) {
        return false;
    }
    if ((x <= 0) || (y <= 0) || (w <= 0) || (h <= 0)) {
        return false;
    }
    return (MoveWindow(g_am_MainWindowHandle_p, x, y, w, h, TRUE) != FALSE);
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
        const bool win10 = am_CompareSystemVersion_p(WindowsVersion::Windows10, VersionCompare::GreaterOrEqual);
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        if (win10) {
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
        if (am_IsMaximized_p(hWnd) && !am_IsFullScreened_p(hWnd)) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            const int rbtY = am_GetResizeBorderThickness_p(false, g_am_CurrentDpi_p);
            clientRect->top += rbtY;
            if (!win10) {
                clientRect->bottom -= rbtY;
                const int rbtX = am_GetResizeBorderThickness_p(true, g_am_CurrentDpi_p);
                clientRect->left += rbtX;
                clientRect->right -= rbtX;
            }
            nonClientAreaExists = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (am_IsMaximized_p(hWnd) || am_IsFullScreened_p(hWnd)) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            // First, check if we have an auto-hide taskbar at all:
            if (SHAppBarMessage(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) {
                bool top = false, bottom = false, left = false, right = false;
                // Due to "ABM_GETAUTOHIDEBAREX" only has effect since Windows 8.1,
                // we have to use another way to judge the edge of the auto-hide taskbar
                // when the application is running on Windows 7 or Windows 8.
                if (am_CompareSystemVersion_p(WindowsVersion::Windows8_1, VersionCompare::GreaterOrEqual)) {
                    const RECT screenRect = am_GetScreenGeometry_p(hWnd);
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
        const bool maxOrFull = (am_IsMaximized_p(hWnd) || am_IsFullScreened_p(hWnd));
        const bool normal = am_IsWindowNoState_p(hWnd);
        const SIZE ws = am_GetWindowSize_p();
        const LONG ww = ws.cx;
        const int rbtX = am_GetResizeBorderThickness_p(true, g_am_CurrentDpi_p);
        const int rbtY = am_GetResizeBorderThickness_p(false, g_am_CurrentDpi_p);
        const int cth = am_GetCaptionHeight_p(g_am_CurrentDpi_p);
        const bool hitTestVisible = /*am_IsHitTestVisibleInChrome_p(hWnd)*/false; // todo
        bool isTitleBar = false;
        if (maxOrFull) {
            isTitleBar = ((localPos.y >= 0) && (localPos.y <= cth)
                          && (localPos.x >= 0) && (localPos.x <= ww)
                          && !hitTestVisible);
        } else if (normal) {
            isTitleBar = ((localPos.y > rbtY) && (localPos.y <= am_GetTitleBarHeight_p(hWnd, g_am_CurrentDpi_p))
                          && (localPos.x > rbtX) && (localPos.x < (ww - rbtX))
                          && !hitTestVisible);
        }
        const bool isTop = (normal ? (localPos.y <= rbtY) : false);
        if (am_CompareSystemVersion_p(WindowsVersion::Windows10, VersionCompare::GreaterOrEqual)) {
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
        PAINTSTRUCT ps = {};
        const HDC hdc = BeginPaint(hWnd, &ps);
        if (!hdc) {
            PRINT_ERROR_MESSAGE(BeginPaint)
            break;
        }
        const bool win10 = am_CompareSystemVersion_p(WindowsVersion::Windows10, VersionCompare::GreaterOrEqual);
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
        const LONG visibleFrameThickness = am_GetWindowVisibleFrameThickness_p(hWnd, g_am_CurrentDpi_p);
        if (ps.rcPaint.top < visibleFrameThickness) {
            RECT rcPaint = ps.rcPaint;
            if (win10) {
                rcPaint.bottom = visibleFrameThickness;
            }
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
        if (ps.rcPaint.bottom > visibleFrameThickness) {
            RECT rcPaint = ps.rcPaint;
            if (win10) {
                rcPaint.top = visibleFrameThickness;
            } else {
                rcPaint.top = rcPaint.top + visibleFrameThickness + 1;
                rcPaint.bottom = rcPaint.bottom - visibleFrameThickness - 1;
                rcPaint.left = rcPaint.left + visibleFrameThickness + 1;
                rcPaint.right = rcPaint.right - visibleFrameThickness - 1;
            }
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
    }
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
    }
    case WM_SIZE: {
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)
                || am_IsFullScreened_p(hWnd)) {
            if (!am_UpdateFrameMargins_p(hWnd, g_am_CurrentDpi_p)) {
                am_Print_p(L"WM_SIZE: Failed to update frame margins.");
                break;
            }
        }
        const auto width = LOWORD(lParam);
        const UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER;
        if (g_am_XAMLIslandWindowHandle_p) {
            // Give enough space to our thin homemade top border.
            const int visibleFrameThickness = am_GetWindowVisibleFrameThickness_p(hWnd, g_am_CurrentDpi_p);
            const int height = (HIWORD(lParam) - visibleFrameThickness);
            if (SetWindowPos(g_am_XAMLIslandWindowHandle_p, HWND_BOTTOM, 0, visibleFrameThickness,
                         width, height, flags) == FALSE) {
                PRINT_ERROR_MESSAGE(SetWindowPos)
                break;
            }
        }
        if (g_am_DragBarWindowHandle_p) {
            if (SetWindowPos(g_am_DragBarWindowHandle_p, HWND_TOP, 0, 0, width,
                         am_GetTitleBarHeight_p(hWnd, g_am_CurrentDpi_p), flags) == FALSE) {
                PRINT_ERROR_MESSAGE(SetWindowPos)
                break;
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
            if (!am_OpenSystemMenu_p(hWnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) {
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
        if (!am_IsCompositionEnabled_p()) {
            am_Print_p(L"This application can't continue running when DWM composition is disabled.", true);
            std::exit(-1);
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
        const SystemTheme systemTheme = am_GetSystemTheme_p();
        if ((systemTheme != SystemTheme::Invalid) && (systemTheme != SystemTheme::HighContrast)) {
            if (am_SwitchAcrylicBrushTheme_p(systemTheme)) {
                g_am_BrushTheme_p = SystemTheme::Auto;
            } else {
                am_Print_p(L"Failed to switch acrylic brush theme.");
            }
        } else {
            am_Print_p(L"Failed to retrieve system theme or high contrast mode is on.");
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

[[nodiscard]] static inline bool am_RegisterMainWindowClass_p()
{
    if (g_am_MainWindowAtom_p != 0) {
        return true;
    }

    LPWSTR guid = nullptr;
    if (!am_GenerateGUID_p(&guid)) {
        am_Print_p(L"Failed to generate main window class name.");
        return false;
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
    if (!am_ShowErrorMessageFromLastErrorCode_p(L"RegisterClassExW (MainWindow)")) {
        am_Cleanup_p();
        return false;
    }

    if (g_am_MainWindowAtom_p == 0) {
        am_Cleanup_p();
        return false;
    }

    return true;
}

[[nodiscard]] static inline bool am_RegisterDragBarWindowClass_p()
{
    if (am_CompareSystemVersion_p(WindowsVersion::Windows8, VersionCompare::Less)) {
        am_Print_p(L"Drag bar window is only available on Windows 8 and onwards.");
        am_Cleanup_p();
        return false;
    }

    if (g_am_DragBarWindowAtom_p != 0) {
        return true;
    }

    LPWSTR guid = nullptr;
    if (!am_GenerateGUID_p(&guid)) {
        am_Print_p(L"Failed to generate drag bar window class name.");
        return false;
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
    if (!am_ShowErrorMessageFromLastErrorCode_p(L"RegisterClassExW (DragBarWindow)")) {
        //am_Cleanup_p();
        //return false;
    }

    if (g_am_DragBarWindowAtom_p == 0) {
        am_Cleanup_p();
        return false;
    }

    return true;
}

[[nodiscard]] static inline bool am_CreateMainWindow_p(const int x, const int y, const int w, const int h)
{
    if (g_am_MainWindowAtom_p == 0) {
        am_Print_p(L"Main window class has not been registered.");
        am_Cleanup_p();
        return false;
    }

    g_am_MainWindowHandle_p = CreateWindowExW(0L,
                                       g_am_MainWindowClassName_p, g_am_MainWindowTitle_p,
                                       WS_OVERLAPPEDWINDOW,
                                       ((x > 0) ? x : CW_USEDEFAULT),
                                       ((y > 0) ? y : CW_USEDEFAULT),
                                       ((w > 0) ? w : CW_USEDEFAULT),
                                       ((h > 0) ? h : CW_USEDEFAULT),
                                       nullptr, nullptr, HINST_THISCOMPONENT, nullptr);
    if (!am_ShowErrorMessageFromLastErrorCode_p(L"CreateWindowExW (MainWindow)")) {
        am_Cleanup_p();
        return false;
    }

    if (!g_am_MainWindowHandle_p) {
        am_Print_p(L"Failed to create main window.");
        am_Cleanup_p();
        return false;
    }

    g_am_CurrentDpi_p = am_GetWindowDpi_p(g_am_MainWindowHandle_p);
    if (g_am_CurrentDpi_p == 0) {
        g_am_CurrentDpi_p = USER_DEFAULT_SCREEN_DPI;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (!am_UpdateFrameMargins_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p)) {
        am_Print_p(L"Failed to update main window's frame margins.");
        am_Cleanup_p();
        return false;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (!am_TriggerFrameChange_p(g_am_MainWindowHandle_p)) {
        am_Print_p(L"Failed to trigger frame change event for main window.");
        am_Cleanup_p();
        return false;
    }
    // Ensure our window still has window transitions.
    if (!am_SetWindowTransitionsEnabled_p(g_am_MainWindowHandle_p, true)) {
        am_Print_p(L"Failed to enable window transitions for main window.");
        am_Cleanup_p();
        return false;
    }

    return true;
}

[[nodiscard]] static inline bool am_CreateDragBarWindow_p()
{
    if (!g_am_MainWindowHandle_p) {
        am_Cleanup_p();
        return false;
    }

    // Please refer to the "IMPORTANT NOTE" section below.
    if (am_CompareSystemVersion_p(WindowsVersion::Windows8, VersionCompare::Less)) {
        am_Print_p(L"Drag bar window is only available on Windows 8 and onwards.");
        am_Cleanup_p();
        return false;
    }
    if (g_am_DragBarWindowAtom_p == 0) {
        am_Print_p(L"Drag bar window class has not been created.");
        am_Cleanup_p();
        return false;
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
    if (!am_ShowErrorMessageFromLastErrorCode_p(L"CreateWindowExW (DragBarWindow)")) {
        am_Cleanup_p();
        return false;
    }

    if (!g_am_DragBarWindowHandle_p) {
        am_Print_p(L"Failed to create drag bar window.");
        am_Cleanup_p();
        return false;
    }

    // Layered window won't be visible until we call SetLayeredWindowAttributes()
    // or UpdateLayeredWindow().
    if (SetLayeredWindowAttributes(g_am_DragBarWindowHandle_p, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
        am_Print_p(L"SetLayeredWindowAttributes() failed.");
        am_Cleanup_p();
        return false;
    }

    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(g_am_MainWindowHandle_p, &rect) == FALSE) {
        am_Print_p(L"Failed to retrieve client rect of main window.");
        am_Cleanup_p();
        return false;
    }
    if (SetWindowPos(g_am_DragBarWindowHandle_p, HWND_TOP, 0, 0, rect.right,
                 am_GetTitleBarHeight_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p),
                 SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        am_Print_p(L"Failed to move drag bar window.");
        am_Cleanup_p();
        return false;
    }

    return true;
}

[[nodiscard]] static inline bool am_CreateXAMLIsland_p()
{
    if (!g_am_MainWindowHandle_p) {
        am_Cleanup_p();
        return false;
    }

    // XAML Island is only supported on Windows 10 19H1 and onwards.
    if (am_CompareSystemVersion_p(WindowsVersion::Windows10_19H1, VersionCompare::Less)) {
        am_Print_p(L"XAML Island is only supported on Windows 10 19H1 and onwards.");
        am_Cleanup_p();
        return false;
    }
    const SystemTheme systemTheme = am_GetSystemTheme_p();
    if (systemTheme == SystemTheme::Invalid) {
        am_Print_p(L"Failed to retrieve system theme.");
        am_Cleanup_p();
        return false;
    }
    if (systemTheme == SystemTheme::HighContrast) {
        am_Print_p(L"High contrast mode is on.");
        am_Cleanup_p();
        return false;
    }

    winrt::init_apartment(winrt::apartment_type::single_threaded);
    g_am_XAMLManager_p = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();

    g_am_XAMLSource_p = {};
    const auto interop = g_am_XAMLSource_p.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        am_Print_p(L"Failed to retrieve IDesktopWindowXamlSourceNative.");
        am_Cleanup_p();
        return false;
    }
    winrt::check_hresult(interop->AttachToWindow(g_am_MainWindowHandle_p));
    winrt::check_hresult(interop->get_WindowHandle(&g_am_XAMLIslandWindowHandle_p));
    if (!g_am_XAMLIslandWindowHandle_p) {
        am_Print_p(L"Failed to retrieve XAML Island window handle.");
        am_Cleanup_p();
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(g_am_MainWindowHandle_p, &rect) == FALSE) {
        am_Print_p(L"Failed to retrieve client rect of main window.");
        am_Cleanup_p();
        return false;
    }
    // Give enough space to our thin homemade top border.
    const int visibleFrameThickness = am_GetWindowVisibleFrameThickness_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p);
    if (SetWindowPos(g_am_XAMLIslandWindowHandle_p, HWND_BOTTOM, 0,
                 visibleFrameThickness, rect.right, (rect.bottom - visibleFrameThickness),
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        am_Print_p(L"Failed to move XAML Island window.");
        am_Cleanup_p();
        return false;
    }
    g_am_BackgroundBrush_p = {};
    if (!am_SwitchAcrylicBrushTheme_p((systemTheme == SystemTheme::Auto) ? SystemTheme::Dark : systemTheme)) {
        am_Print_p(L"Failed to change acrylic brush's theme.");
        am_Cleanup_p();
        return false;
    }
    g_am_BrushTheme_p = SystemTheme::Auto;
    g_am_BackgroundBrush_p.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_am_RootGrid_p = {};
    g_am_RootGrid_p.Background(g_am_BackgroundBrush_p);
    //g_am_RootGrid_p.Children().Clear();
    //g_am_RootGrid_p.Children().Append(/* some UWP control */);
    //g_am_RootGrid_p.UpdateLayout();
    g_am_XAMLSource_p.Content(g_am_RootGrid_p);

    return true;
}

[[nodiscard]] static inline bool am_Initialize_p(const int x, const int y, const int w, const int h)
{
    static bool inited = false;
    if (inited) {
        am_Print_p(L"The AcrylicManager has been initialized already.");
        return false;
    }
    inited = true;

    if (!am_RegisterMainWindowClass_p()) {
        am_Print_p(L"Failed to register main window class.", true);
        return false;
    }
    if (!am_CreateMainWindow_p(x, y, w, h)) {
        am_Print_p(L"Failed to create main window.", true);
        return false;
    }
    if (am_CompareSystemVersion_p(WindowsVersion::Windows10, VersionCompare::GreaterOrEqual)) {
        if (am_CompareSystemVersion_p(WindowsVersion::Windows10_19H1, VersionCompare::GreaterOrEqual)) {
            if (am_CreateXAMLIsland_p()) {
                if (am_RegisterDragBarWindowClass_p()) {
                    if (!am_CreateDragBarWindow_p()) {
                        am_Print_p(L"Failed to create drag bar window.", true);
                        return false;
                    }
                } else {
                    am_Print_p(L"Failed to register drag bar window class.", true);
                    return false;
                }
            } else {
                am_Print_p(L"Failed to create XAML Island.", true);
                return false;
            }
        } else {
            am_Print_p(L"XAML Island is only supported on Windows 10 19H1 and onwards.", true);
            //return false;
        }
    } else {
        am_Print_p(L"This application only supports Windows 10 and onwards.", true);
        //return false;
    }

    return true;
}

[[nodiscard]] static inline HWND am_GetWindowHandle_p()
{
    return g_am_MainWindowHandle_p;
}

[[nodiscard]] static inline SystemTheme am_GetBrushTheme_p()
{
    return g_am_BrushTheme_p;
}

[[nodiscard]] static inline int am_MainWindowEventLoop_p()
{
    if (!g_am_MainWindowHandle_p) {
        return 0;
    }

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

// Public interface

bool am_CreateWindow(const int x, const int y, const int w, const int h)
{
    if (am_CompareSystemVersion_p(WindowsVersion::WindowsVista, VersionCompare::Less)) {
        am_Print_p(L"This application cannot be run on such old systems.", true);
        return false;
    }

    if (!am_IsCompositionEnabled_p()) {
        am_Print_p(L"This application could not be started when DWM composition is disabled.", true);
        return false;
    }

    return am_Initialize_p(x, y, w, h);
}

RECT am_GetWindowGeometry()
{
    return am_GetWindowGeometry_p();
}

bool am_SetWindowGeometry(const int x, const int y, const int w, const int h)
{
    return am_SetWindowGeometry_p(x, y, w, h);
}

bool am_MoveWindow(const int x, const int y)
{
    return am_MoveWindow_p(x, y);
}

SIZE am_GetWindowSize()
{
    return am_GetWindowSize_p();
}

bool am_ResizeWindow(const int w, const int h)
{
    return am_ResizeWindow_p(w, h);
}

bool am_CenterWindow()
{
    return am_CenterWindow_p();
}

WindowState am_GetWindowState()
{
    return am_GetWindowState_p();
}

bool am_SetWindowState(const WindowState state)
{
    return am_SetWindowState_p(state);
}

bool am_CloseWindow()
{
    am_Cleanup_p();
    return true;
}

HWND am_GetWindowHandle()
{
    return am_GetWindowHandle_p();
}

SystemTheme am_GetBrushTheme()
{
    return am_GetBrushTheme_p();
}

bool am_SetBrushTheme(const SystemTheme theme)
{
    return am_SwitchAcrylicBrushTheme_p(theme);
}

bool am_GetTintColor(int *r, int *g, int *b, int *a)
{
    return am_GetTintColor_p(r, g, b, a);
}

bool am_SetTintColor(const int r, const int g, const int b, const int a)
{
    return am_SetTintColor_p(r, g, b, a);
}

bool am_GetTintOpacity(double *opacity)
{
    return am_GetTintOpacity_p(opacity);
}

bool am_SetTintOpacity(const double opacity)
{
    return am_SetTintOpacity_p(opacity);
}

bool am_GetTintLuminosityOpacity(double *opacity)
{
    return am_GetTintLuminosityOpacity_p(opacity);
}

bool am_SetTintLuminosityOpacity(const double opacity)
{
    return am_SetTintLuminosityOpacity_p(opacity);
}

bool am_GetFallbackColor(int *r, int *g, int *b, int *a)
{
    return am_GetFallbackColor_p(r, g, b, a);
}

bool am_SetFallbackColor(const int r, const int g, const int b, const int a)
{
    return am_SetFallbackColor_p(r, g, b, a);
}

int am_EventLoopExec()
{
    return am_MainWindowEventLoop_p();
}
