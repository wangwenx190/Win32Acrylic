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

// Define these macros first before including their header files to avoid linking
// to their import libraries.
#ifndef _USER32_
#define _USER32_
#endif

#ifndef _UXTHEME_
#define _UXTHEME_
#endif

#ifndef _DWMAPI_
#define _DWMAPI_
#endif

#include <Unknwn.h>

#include <ShellApi.h>
#include <ShellScalingApi.h>
#include <UxTheme.h>
#include <DwmApi.h>

#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

#include "acrylicapplication_global.h"
#include "acrylicapplication.h"
#include "acrylicapplication_p.h"

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

#ifndef BACKGROUND_BRUSH
#define BACKGROUND_BRUSH (reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)))
#endif

#ifndef CURRENT_SCREEN
#define CURRENT_SCREEN(window) (MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST))
#endif

//
#ifdef __cplusplus
EXTERN_C_START
#endif

/////////////////////////////////
/////     User32
/////////////////////////////////

UINT WINAPI
GetDpiForWindow(
    HWND hWnd
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_USER_INT_FUNCTION(GetDpiForWindow, hWnd)
}

UINT WINAPI
GetDpiForSystem()
{
    ACRYLICAPPLICATION_TRY_EXECUTE_USER_INT_FUNCTION(GetDpiForSystem)
}

UINT WINAPI
GetSystemDpiForProcess(
    HANDLE hProcess
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_USER_INT_FUNCTION(GetSystemDpiForProcess, hProcess)
}

int WINAPI
GetSystemMetricsForDpi(
    int  nIndex,
    UINT dpi
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_USER_INT_FUNCTION(GetSystemMetricsForDpi, nIndex, dpi)
}

BOOL WINAPI
AdjustWindowRectExForDpi(
    LPRECT lpRect,
    DWORD  dwStyle,
    BOOL   bMenu,
    DWORD  dwExStyle,
    UINT   dpi
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_USER_FUNCTION(AdjustWindowRectExForDpi, lpRect, dwStyle, bMenu, dwExStyle, dpi)
}

/////////////////////////////////
/////     SHCore
/////////////////////////////////

HRESULT WINAPI
GetDpiForMonitor(
    HMONITOR         hMonitor,
    MONITOR_DPI_TYPE dpiType,
    UINT             *dpiX,
    UINT             *dpiY
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_SHCORE_FUNCTION(GetDpiForMonitor, hMonitor, dpiType, dpiX, dpiY)
}

/////////////////////////////////
/////     UxTheme
/////////////////////////////////

HPAINTBUFFER WINAPI
BeginBufferedPaint(
    HDC             hdcTarget,
    const RECT      *prcTarget,
    BP_BUFFERFORMAT dwFormat,
    BP_PAINTPARAMS  *pPaintParams,
    HDC             *phdc
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_THEME_PTR_FUNCTION(BeginBufferedPaint, hdcTarget, prcTarget, dwFormat, pPaintParams, phdc)
}

HRESULT WINAPI
EndBufferedPaint(
    HPAINTBUFFER hBufferedPaint,
    BOOL         fUpdateTarget
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_THEME_FUNCTION(EndBufferedPaint, hBufferedPaint, fUpdateTarget)
}

HRESULT WINAPI
BufferedPaintSetAlpha(
    HPAINTBUFFER hBufferedPaint,
    const RECT   *prc,
    BYTE         alpha
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_THEME_FUNCTION(BufferedPaintSetAlpha, hBufferedPaint, prc, alpha)
}

/////////////////////////////////
/////     DwmApi
/////////////////////////////////

HRESULT WINAPI
DwmExtendFrameIntoClientArea(
    HWND          hWnd,
    const MARGINS *pMarInset
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_DWM_FUNCTION(DwmExtendFrameIntoClientArea, hWnd, pMarInset)
}

HRESULT WINAPI
DwmSetWindowAttribute(
    HWND    hWnd,
    DWORD   dwAttribute,
    LPCVOID pvAttribute,
    DWORD   cbAttribute
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_DWM_FUNCTION(DwmSetWindowAttribute, hWnd, dwAttribute, pvAttribute, cbAttribute)
}

HRESULT WINAPI
DwmIsCompositionEnabled(
    BOOL *pfEnabled
)
{
    ACRYLICAPPLICATION_TRY_EXECUTE_DWM_FUNCTION(DwmIsCompositionEnabled, pfEnabled)
}

#ifdef __cplusplus
EXTERN_C_END
#endif

// The thickness of an auto-hide taskbar in pixels.
static const int kAutoHideTaskbarThicknessPx = 2;
static const int kAutoHideTaskbarThicknessPy = kAutoHideTaskbarThicknessPx;

//
static AcrylicApplication *instance = nullptr;
static const wchar_t mainWindowClassName[] = L"Win32AcrylicApplicationMainWindowClass";
static const wchar_t dragBarWindowClassName[] = L"Win32AcrylicApplicationDragBarWindowClass";
static const wchar_t mainWindowTitle[] = L"Win32 Native C++ Acrylic Application Main Window";
static const wchar_t dragBarWindowTitle[] = L"";
static UINT mainWindowDpi = USER_DEFAULT_SCREEN_DPI;
static HWND mainWindowHandle = nullptr;
static HWND dragBarWindowHandle = nullptr;
static HWND xamlIslandHandle = nullptr;
static ATOM mainWindowAtom = 0;
static ATOM dragBarWindowAtom = 0;
static wchar_t **arguments = nullptr;
static SystemTheme acrylicBrushTheme = SystemTheme::Invalid;

// XAML
static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager xamlManager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource xamlSource = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid xamlGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush xamlBrush = nullptr;

//
static inline void print_p(LPCWSTR text, const bool showUi = false, LPCWSTR title = nullptr)
{
    if (!text) {
        return;
    }
    OutputDebugStringW(text);
    if (showUi) {
        MessageBoxW(nullptr, text, (title ? title : L"Error"), MB_ICONERROR | MB_OK);
    }
}

[[nodiscard]] static inline double getDevicePixelRatio_p(const UINT dpi)
{
    if (dpi == 0) {
        return 1.0;
    }
    return (static_cast<double>(dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
}

UINT getWindowDpi_p(const HWND hWnd)
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

bool isWindowMinimized_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsMinimized(hWnd) != FALSE);
}

bool isWindowMaximized_p(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsMaximized(hWnd) != FALSE);
}

int getResizeBorderThickness(const bool x, const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    // There is no "SM_CYPADDEDBORDER".
    const int result = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, _dpi)
            + GetSystemMetricsForDpi((x ? SM_CXSIZEFRAME : SM_CYSIZEFRAME), _dpi);
    const int preset = std::round(8.0 * getDevicePixelRatio_p(_dpi));
    return ((result > 0) ? result : preset);
}

int getCaptionHeight(const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    const int result = GetSystemMetricsForDpi(SM_CYCAPTION, _dpi);
    const int preset = std::round(23.0 * getDevicePixelRatio_p(_dpi));
    return ((result > 0) ? result : preset);
}

int getTitleBarHeight(const HWND hWnd, const UINT dpi)
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
            print_p(L"AdjustWindowRectExForDpi() failed.");
        } else {
            titleBarHeight = std::abs(frame.top);
        }
    }
    if (titleBarHeight <= 0) {
        titleBarHeight = getResizeBorderThickness(false, _dpi) + getCaptionHeight(_dpi);
        if (titleBarHeight <= 0) {
            titleBarHeight = std::round(31.0 * getDevicePixelRatio_p(_dpi));
        }
    }
    return titleBarHeight;
}

[[nodiscard]] static inline MONITORINFO getMonitorInfo_p(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    const HMONITOR mon = CURRENT_SCREEN(hWnd);
    if (!mon) {
        print_p(L"Failed to retrieve current screen.");
        return {};
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        print_p(L"Failed to retrieve monitor information.");
        return {};
    }
    return mi;
}

RECT getScreenGeometry(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    return getMonitorInfo_p(hWnd).rcMonitor;
}

RECT getScreenAvailableGeometry(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    return getMonitorInfo_p(hWnd).rcWork;
}

bool isCompositionEnabled()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (compareSystemVersion_p(WindowsVersion::Windows8, VersionCompare::GreaterOrEqual)) {
        return true;
    }
    BOOL enabled = FALSE;
    return (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && (enabled != FALSE));
}

bool isWindowFullScreened(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &windowRect) == FALSE) {
        print_p(L"Failed to retrieve window rect of main window.");
        return false;
    }
    const RECT screenRect = getScreenGeometry(hWnd);
    return ((windowRect.left == screenRect.left)
            && (windowRect.right == screenRect.right)
            && (windowRect.top == screenRect.top)
            && (windowRect.bottom == screenRect.bottom));
}

bool isWindowNoState(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    if (GetWindowPlacement(hWnd, &wp) == FALSE) {
        print_p(L"Failed to retrieve window placement of main window.");
        return false;
    }
    return (wp.showCmd == SW_NORMAL);
}

bool isWindowVisible(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsWindowVisible(hWnd) != FALSE);
}

bool triggerFrameChange(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    const BOOL result = SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    return (result != FALSE);
}

bool setWindowTransitionsEnabled(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return false;
    }
    const BOOL disabled = enable ? FALSE : TRUE;
    return SUCCEEDED(DwmSetWindowAttribute(hWnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disabled, sizeof(disabled)));
}

bool openSystemMenu(const HWND hWnd, const POINT pos)
{
    if (!hWnd) {
        return false;
    }
    const HMENU menu = GetSystemMenu(hWnd, FALSE);
    if (!menu) {
        print_p(L"Failed to retrieve system menu of main window.");
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
    const bool isMaximized = isWindowMaximized_p(hWnd);
    if (!setState(SC_RESTORE, isMaximized)) {
        print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_MOVE, !isMaximized)) {
        print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_SIZE, !isMaximized)) {
        print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_MINIMIZE, true)) {
        print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_MAXIMIZE, !isMaximized)) {
        print_p(L"Failed to change menu item state.");
        return false;
    }
    if (!setState(SC_CLOSE, true)) {
        print_p(L"Failed to change menu item state.");
        return false;
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        print_p(L"Failed to set default menu item.");
        return false;
    }
    // ### TODO: support LTR layout.
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, pos.x, pos.y, 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            print_p(L"Failed to post system menu message to main window.");
            return false;
        }
    }
    return true;
}

bool compareSystemVersion(const WindowsVersion ver,
                                                     const VersionCompare comp)
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

[[nodiscard]] static inline int getTopFrameMargin_p(const HWND hWnd, const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    return ((hWnd && isWindowNoState_p(hWnd)) ? std::round(1.0 * getDevicePixelRatio_p(_dpi)) : 0);
}

[[nodiscard]] static inline bool updateFrameMargins_p(const HWND hWnd, const UINT dpi)
{
    if (!hWnd) {
        return false;
    }
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
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
    const MARGINS margins = {0, 0, (isWindowNoState_p(hWnd) ? getTitleBarHeight_p(hWnd, _dpi) : 0), 0};
    return SUCCEEDED(DwmExtendFrameIntoClientArea(hWnd, &margins));
}

[[nodiscard]] static inline bool showWindowFullScreen_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return false;
    }
    auto style = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE));
    style &= ~(WS_OVERLAPPEDWINDOW); // fixme: check
    style |= WS_POPUP;
    SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    const RECT rect = getScreenGeometry_p(hWnd);
    return (MoveWindow(hWnd, rect.left, rect.top,
                       RECT_WIDTH(rect), RECT_HEIGHT(rect), TRUE) != FALSE);
}

[[nodiscard]] static inline SystemTheme getSystemTheme_p()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        print_p(L"Failed to retrieve high contrast mode state.");
        return SystemTheme::Invalid;
    }
    if (hc.dwFlags & HCF_HIGHCONTRASTON) {
        print_p(L"High contrast mode is on.");
        return SystemTheme::HighContrast;
    }
    // Dark mode was first introduced in Windows 10 1607.
    if (compareSystemVersion_p(WindowsVersion::Windows10_1607, VersionCompare::GreaterOrEqual)) {
        // registry
    } else {
        print_p(L"Current system version doesn't support dark mode.");
    }
    return SystemTheme::Invalid;
}

[[nodiscard]] static inline RECT getWindowGeometry_p()
{
    if (!mainWindowHandle) {
        return {};
    }
    RECT geo = {0, 0, 0, 0};
    if (GetWindowRect(mainWindowHandle, &geo) == FALSE) {
        print_p(L"Failed to retrieve window rect of main window.");
        return {};
    }
    return geo;
}

[[nodiscard]] static inline SIZE getWindowSize_p()
{
    if (!mainWindowHandle) {
        return {};
    }
    const RECT geo = getWindowGeometry_p();
    return {RECT_WIDTH(geo), RECT_HEIGHT(geo)};
}

[[nodiscard]] static inline bool moveWindow_p(const int x, const int y)
{
    if (!mainWindowHandle) {
        return false;
    }
    // The x coordinate and y coordinate can be negative numbers, if so,
    // the window will be moved to another screen.
    const SIZE s = getWindowSize_p();
    return (MoveWindow(mainWindowHandle, x, y, s.cx, s.cy, TRUE) != FALSE);
}

[[nodiscard]] static inline bool resizeWindow_p(const int w, const int h)
{
    if (!mainWindowHandle) {
        return false;
    }
    if ((w <= 0) || (h <= 0)) {
        print_p(L"Can't resize window to empty or negative size.");
        return false;
    }
    const RECT geo = getWindowGeometry_p();
    return (MoveWindow(mainWindowHandle, geo.left, geo.top,
                       RECT_WIDTH(geo), RECT_HEIGHT(geo), TRUE) != FALSE);
}

[[nodiscard]] static inline bool centerWindow_p()
{
    if (!mainWindowHandle) {
        return false;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(mainWindowHandle, &windowRect) == FALSE) {
        print_p(L"Failed to retrieve window rect of main window.");
        return false;
    }
    const int windowWidth = RECT_WIDTH(windowRect);
    const int windowHeight = RECT_HEIGHT(windowRect);
    const RECT screenRect = getScreenGeometry_p(mainWindowHandle);
    const int screenWidth = RECT_WIDTH(screenRect);
    const int screenHeight = RECT_HEIGHT(screenRect);
    const int newX = (screenWidth - windowWidth) / 2;
    const int newY = (screenHeight - windowHeight) / 2;
    return (MoveWindow(mainWindowHandle, newX, newY, windowWidth, windowHeight, TRUE) != FALSE);
}

[[nodiscard]] static inline WindowState getWindowState_p()
{
    if (!mainWindowHandle) {
        return WindowState::Invalid;
    }
    if (isWindowFullScreened_p(mainWindowHandle)) {
        return WindowState::FullScreened;
    } else if (isWindowMaximized_p(mainWindowHandle)) {
        return WindowState::Maximized;
    } else if (isWindowMinimized_p(mainWindowHandle)) {
        return WindowState::Minimized;
    } else if (isWindowNoState_p(mainWindowHandle)) {
        return WindowState::Normal;
    } else if (isWindowVisible_p(mainWindowHandle)) {
        return WindowState::Shown;
    } else if (!isWindowVisible(mainWindowHandle)) {
        return WindowState::Hidden;
    }
    return WindowState::Invalid;
}

[[nodiscard]] static inline bool setWindowState_p(const WindowState state)
{
    if (!mainWindowHandle) {
        return false;
    }
    switch (state) {
    case WindowState::Normal:
        ShowWindow(mainWindowHandle, SW_NORMAL);
        return true;
    case WindowState::Maximized:
        ShowWindow(mainWindowHandle, SW_MAXIMIZE);
        return true;
    case WindowState::Minimized:
        ShowWindow(mainWindowHandle, SW_MINIMIZE);
        return true;
    case WindowState::FullScreened:
        return showWindowFullScreen_p(mainWindowHandle, true);
    case WindowState::Hidden:
        ShowWindow(mainWindowHandle, SW_HIDE);
        return true;
    case WindowState::Shown:
        ShowWindow(mainWindowHandle, SW_SHOW);
        return true;
    default:
        break;
    }
    return false;
}

[[nodiscard]] static inline bool destroyWindow_p()
{
    if (dragBarWindowHandle) {
        DestroyWindow(dragBarWindowHandle);
        dragBarWindowHandle = nullptr;
    }
    if (dragBarWindowAtom != static_cast<ATOM>(0)) {
        UnregisterClassW(dragBarWindowClassName, HINST_THISCOMPONENT);
        dragBarWindowAtom = 0;
    }
    if (mainWindowHandle) {
        DestroyWindow(mainWindowHandle);
        mainWindowHandle = nullptr;
    }
    if (mainWindowAtom != static_cast<ATOM>(0)) {
        UnregisterClassW(mainWindowClassName, HINST_THISCOMPONENT);
        mainWindowAtom = 0;
    }
    return true;
}

[[nodiscard]] static inline bool getTintColor_p(int *r, int *g, int *b, int *a)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (!r || !g || !b || !a) {
        print_p(L"Can't retrieve tint color: invalid parameters.");
        return false;
    }
    const winrt::Windows::UI::Color color = xamlBrush.TintColor();
    *r = static_cast<int>(color.R);
    *g = static_cast<int>(color.G);
    *b = static_cast<int>(color.B);
    *a = static_cast<int>(color.A);
    return true;
}

[[nodiscard]] static inline bool setTintColor_p(const int r, const int g, const int b, const int a)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    const auto red = static_cast<uint8_t>(std::clamp(r, 0, 255));
    const auto green = static_cast<uint8_t>(std::clamp(g, 0, 255));
    const auto blue = static_cast<uint8_t>(std::clamp(b, 0, 255));
    const auto alpha = static_cast<uint8_t>(std::clamp(a, 0, 255));
    xamlBrush.TintColor({alpha, red, green, blue}); // ARGB
    return true;
}

[[nodiscard]] static inline bool getTintOpacity_p(double *opacity)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (!opacity) {
        print_p(L"Can't retrieve tint opacity: invalid parameter.");
        return false;
    }
    const double value = xamlBrush.TintOpacity();
    *opacity = value;
    return true;
}

[[nodiscard]] static inline bool setTintOpacity_p(const double opacity)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    const double value = std::clamp(opacity, 0.0, 1.0);
    xamlBrush.TintOpacity(value);
    return true;
}

[[nodiscard]] static inline bool getTintLuminosityOpacity_p(double *opacity)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (!opacity) {
        print_p(L"Can't retrieve tint luminosity opacity: invalid parameter.");
        return false;
    }
    const double value = xamlBrush.TintLuminosityOpacity().GetDouble();
    *opacity = value;
    return true;
}

[[nodiscard]] static inline bool setTintLuminosityOpacity_p(const double opacity)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    const double value = std::clamp(opacity, 0.0, 1.0);
    xamlBrush.TintLuminosityOpacity(value);
    return true;
}

[[nodiscard]] static inline bool getFallbackColor_p(int *r, int *g, int *b, int *a)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (!r || !g || !b || !a) {
        print_p(L"Can't retrieve fallback color: invalid parameters.");
        return false;
    }
    const winrt::Windows::UI::Color color = xamlBrush.FallbackColor();
    *r = static_cast<int>(color.R);
    *g = static_cast<int>(color.G);
    *b = static_cast<int>(color.B);
    *a = static_cast<int>(color.A);
    return true;
}

[[nodiscard]] static inline bool setFallbackColor_p(const int r, const int g, const int b, const int a)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    const auto red = static_cast<uint8_t>(std::clamp(r, 0, 255));
    const auto green = static_cast<uint8_t>(std::clamp(g, 0, 255));
    const auto blue = static_cast<uint8_t>(std::clamp(b, 0, 255));
    const auto alpha = static_cast<uint8_t>(std::clamp(a, 0, 255));
    xamlBrush.FallbackColor({alpha, red, green, blue}); // ARGB
    return true;
}

[[nodiscard]] static inline bool switchAcrylicBrushTheme_p(const SystemTheme theme)
{
    if (!xamlBrush) {
        print_p(L"Acrylic brush is not available.");
        return false;
    }
    if (theme == SystemTheme::Invalid) {
        print_p(L"The given theme type is not valid.");
        return false;
    }
    int tc[4] = {0, 0, 0, 0};
    double to = 0.0;
    double tlo = 0.0;
    int fbc[4] = {0, 0, 0, 0};
    if (theme == SystemTheme::Light) {
        tc[0] = 252; tc[1] = 252; tc[2] = 252; tc[3] = 255; // #FCFCFC
        to = 0.0;
        tlo = 0.85;
        fbc[0] = 249; fbc[1] = 249; fbc[2] = 249; fbc[3] = 255; // #F9F9F9
    } else {
        tc[0] = 44; tc[1] = 44; tc[2] = 44; tc[3] = 255; // #2C2C2C
        to = 0.15;
        tlo = 0.96;
        fbc[0] = 44; fbc[1] = 44; fbc[2] = 44; fbc[3] = 255; // #2C2C2C
    }
    if (!setTintColor_p(tc[0], tc[1], tc[2], tc[3])) {
        print_p(L"Failed to change acrylic brush's tint color.");
        return false;
    }
    if (!setTintOpacity_p(to)) {
        print_p(L"Failed to change acrylic brush's tint opacity.");
        return false;
    }
    if (!setTintLuminosityOpacity_p(tlo)) {
        print_p(L"Failed to change acrylic brush's tint luminosity opacity.");
        return false;
    }
    if (!setFallbackColor_p(fbc[0], fbc[1], fbc[2], fbc[3])) {
        print_p(L"Failed to change acrylic brush's fallback color.");
        return false;
    }
    acrylicBrushTheme = theme;
    return true;
}

[[nodiscard]] static inline LRESULT CALLBACK mainWindowProc_p(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool systemThemeChanged = false;
    switch (uMsg)
    {
    case WM_NCCALCSIZE: {
        if (!wParam) {
            return 0;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        // Store the original top before the default window proc applies the default frame.
        const LONG originalTop = clientRect->top;
        // Apply the default frame
        const LRESULT ret = DefWindowProcW(hWnd, uMsg, wParam, lParam);
        if (ret != 0) {
            return ret;
        }
        // Re-apply the original top from before the size of the default frame was applied.
        clientRect->top = originalTop;
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        bool nonClientAreaExists = false;
        if (isWindowMaximized_p(hWnd) && !isWindowFullScreened_p(hWnd)) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            clientRect->top += getResizeBorderThickness_p(false, mainWindowDpi);
            nonClientAreaExists = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (isWindowMaximized_p(hWnd) || isWindowFullScreened_p(hWnd)) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            // First, check if we have an auto-hide taskbar at all:
            if (SHAppBarMessage(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) {
                const RECT screenRect = getScreenGeometry_p(hWnd);
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
                if (hasAutohideTaskbar(ABE_TOP)) {
                    // Peculiarly, when we're fullscreen,
                    clientRect->top += kAutoHideTaskbarThicknessPy;
                    nonClientAreaExists = true;
                } else if (hasAutohideTaskbar(ABE_BOTTOM)) {
                    clientRect->bottom -= kAutoHideTaskbarThicknessPy;
                    nonClientAreaExists = true;
                } else if (hasAutohideTaskbar(ABE_LEFT)) {
                    clientRect->left += kAutoHideTaskbarThicknessPx;
                    nonClientAreaExists = true;
                } else if (hasAutohideTaskbar(ABE_RIGHT)) {
                    clientRect->right -= kAutoHideTaskbarThicknessPx;
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
        // This will handle the left, right and bottom parts of the frame
        // because we didn't change them.
        const LRESULT originalRet = DefWindowProcW(hWnd, uMsg, wParam, lParam);
        if (originalRet != HTCLIENT) {
            return originalRet;
        }
        POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (ScreenToClient(hWnd, &pos) == FALSE) {
            print_p(L"WM_NCHITTEST: Failed to translate screen coordinates to client coordinates.");
            break;
        }
        const int rbtY = getResizeBorderThickness_p(false, mainWindowDpi);
        // At this point, we know that the cursor is inside the client area
        // so it has to be either the little border at the top of our custom
        // title bar or the drag bar. Apparently, it must be the drag bar or
        // the little border at the top which the user can use to move or
        // resize the window.
        if (isWindowNoState_p(hWnd) && (pos.y <= rbtY)) {
            return HTTOP;
        }
        const int cth = getCaptionHeight_p(mainWindowDpi);
        if (isWindowMaximized_p(hWnd) && (pos.y >= 0) && (pos.y <= cth)) {
            return HTCAPTION;
        }
        if (isWindowNoState_p(hWnd) && (pos.y > rbtY) && (pos.y <= (rbtY + cth))) {
            return HTCAPTION;
        }
        return HTCLIENT;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps = {};
        const HDC hdc = BeginPaint(hWnd, &ps);
        if (!hdc) {
            print_p(L"WM_PAINT: BeginPaint() returns null.");
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
        const LONG topBorderHeight = getTopFrameMargin_p(hWnd, mainWindowDpi);
        if (ps.rcPaint.top < topBorderHeight) {
            RECT rcTopBorder = ps.rcPaint;
            rcTopBorder.bottom = topBorderHeight;
            // To show the original top border, we have to paint on top
            // of it with the alpha component set to 0. This page
            // recommends to paint the area in black using the stock
            // BLACK_BRUSH to do this:
            // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
            if (FillRect(hdc, &rcTopBorder, BACKGROUND_BRUSH) == 0) {
                print_p(L"WM_PAINT: FillRect() returns zero.");
                break;
            }
        }
        if (ps.rcPaint.bottom > topBorderHeight) {
            RECT rcRest = ps.rcPaint;
            rcRest.top = topBorderHeight;
            // To hide the original title bar, we have to paint on top
            // of it with the alpha component set to 255. This is a hack
            // to do it with GDI. See updateFrameMargins() for more information.
            HDC opaqueDc = nullptr;
            BP_PAINTPARAMS params;
            SecureZeroMemory(&params, sizeof(params));
            params.cbSize = sizeof(params);
            params.dwFlags = BPPF_NOCLIP | BPPF_ERASE;
            const HPAINTBUFFER buf = BeginBufferedPaint(hdc, &rcRest, BPBF_TOPDOWNDIB, &params, &opaqueDc);
            if (!buf) {
                print_p(L"WM_PAINT: BeginBufferedPaint() returns null.");
                break;
            }
            if (FillRect(opaqueDc, &rcRest,
                         reinterpret_cast<HBRUSH>(GetClassLongPtrW(hWnd, GCLP_HBRBACKGROUND))) == 0) {
                print_p(L"WM_PAINT: FillRect() returns zero.");
                break;
            }
            if (FAILED(BufferedPaintSetAlpha(buf, nullptr, 255))) {
                print_p(L"WM_PAINT: BufferedPaintSetAlpha() failed.");
                break;
            }
            if (FAILED(EndBufferedPaint(buf, TRUE))) {
                print_p(L"WM_PAINT: EndBufferedPaint() failed.");
                break;
            }
        }
        if (EndPaint(hWnd, &ps) == FALSE) {
            print_p(L"WM_PAINT: EndPaint() returns FALSE.");
            break;
        }
        return 0;
    }
    case WM_DPICHANGED: {
        const double x = LOWORD(wParam);
        const double y = HIWORD(wParam);
        mainWindowDpi = std::round((x + y) / 2.0);
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (MoveWindow(hWnd, prcNewWindow->left, prcNewWindow->top,
                   RECT_WIDTH(*prcNewWindow), RECT_HEIGHT(*prcNewWindow), TRUE) == FALSE) {
            print_p(L"WM_DPICHANGED: Failed to move window to new position and size.");
            break;
        }
        return 0;
    }
    case WM_SIZE: {
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)
                || isWindowFullScreened_p(mainWindowHandle)) {
            if (!updateFrameMargins_p(hWnd, mainWindowDpi)) {
                print_p(L"WM_SIZE: Failed to update frame margins.");
                break;
            }
        }
        const auto width = LOWORD(lParam);
        const UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER;
        if (xamlIslandHandle) {
            // Give enough space to our thin homemade top border.
            const int topMargin = getTopFrameMargin_p(hWnd, mainWindowDpi);
            const int height = (HIWORD(lParam) - topMargin);
            if (SetWindowPos(xamlIslandHandle, HWND_BOTTOM, 0, topMargin,
                         width, height, flags) == FALSE) {
                print_p(L"WM_SIZE: Failed to move XAML Island window.");
                break;
            }
        }
        if (dragBarWindowHandle) {
            if (SetWindowPos(dragBarWindowHandle, HWND_TOP, 0, 0, width,
                         getTitleBarHeight_p(hWnd, mainWindowDpi), flags) == FALSE) {
                print_p(L"WM_SIZE: Failed to move drag bar window.");
                break;
            }
        }
    } break;
    case WM_SETFOCUS: {
        if (xamlIslandHandle) {
            // Send focus to the XAML Island child window.
            SetFocus(xamlIslandHandle);
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
            if (!openSystemMenu_p(hWnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) {
                print_p(L"WM_NCRBUTTONUP: Failed to open the system menu.");
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
        if (!isCompositionEnabled_p()) {
            print_p(L"This application can't continue running when DWM composition is disabled.", true);
            std::exit(-1);
        }
    } break;
    case WM_CLOSE: {
        if (dragBarWindowHandle) {
            DestroyWindow(dragBarWindowHandle);
            dragBarWindowHandle = nullptr;
        }
        DestroyWindow(hWnd);
        mainWindowHandle = nullptr;
        return 0;
    }
    case WM_DESTROY: {
        if (dragBarWindowAtom != static_cast<ATOM>(0)) {
            UnregisterClassW(dragBarWindowClassName, HINST_THISCOMPONENT);
            dragBarWindowAtom = 0;
        }
        if (mainWindowAtom != static_cast<ATOM>(0)) {
            UnregisterClassW(mainWindowClassName, HINST_THISCOMPONENT);
            mainWindowAtom = 0;
        }
#if 0
        PostQuitMessage(0);
        return 0;
#else
        std::exit(0);
#endif
    }
    default:
        break;
    }
    if (xamlBrush && (acrylicBrushTheme == SystemTheme::Auto) && systemThemeChanged) {
        const SystemTheme systemTheme = getSystemTheme_p();
        if (systemTheme != SystemTheme::Invalid) {
            if (switchAcrylicBrushTheme_p(systemTheme)) {
                acrylicBrushTheme = SystemTheme::Auto;
            } else {
                print_p(L"Failed to switch acrylic brush theme.");
            }
        } else {
            print_p(L"Failed to retrieve system theme or high contrast mode is on.");
        }
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

[[nodiscard]] static inline LRESULT CALLBACK dragBarWindowProc_p(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    if (nonClientMessage.has_value() && mainWindowHandle)
    {
        POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (ClientToScreen(hWnd, &pos) == FALSE) {
            print_p(L"dragBarWindowProc: Failed to translate client coordinates to screen coordinates.");
            return 0;
        }
        const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
        // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
        // then pass that click through as an NC click in that location.
        const LRESULT hitTestResult = SendMessageW(mainWindowHandle, WM_NCHITTEST, 0, newLParam);
        SendMessageW(mainWindowHandle, nonClientMessage.value(), hitTestResult, newLParam);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

[[nodiscard]] static inline bool registerMainWindowClass_p()
{
    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = mainWindowProc_p;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = mainWindowClassName;

    mainWindowAtom = RegisterClassExW(&wcex);

    return (mainWindowAtom != static_cast<ATOM>(0));
}

[[nodiscard]] static inline bool registerDragBarWindowClass_p()
{
    if (compareSystemVersion_p(WindowsVersion::Windows8, VersionCompare::Less)) {
        print_p(L"Drag bar window is only available on Windows 8 and onwards.");
        return false;
    }
    if (!mainWindowHandle) {
        print_p(L"Main window has not been created.");
        return false;
    }

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = dragBarWindowProc_p;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = BACKGROUND_BRUSH;
    wcex.lpszClassName = dragBarWindowClassName;

    dragBarWindowAtom = RegisterClassExW(&wcex);

    return (dragBarWindowAtom != static_cast<ATOM>(0));
}

[[nodiscard]] static inline bool createMainWindow_p(const int x, const int y, const int w, const int h)
{
    if (mainWindowAtom == static_cast<ATOM>(0)) {
        print_p(L"Main window class has not been registered.");
        return false;
    }

    mainWindowHandle = CreateWindowExW(0L,
                                       mainWindowClassName, mainWindowTitle,
                                       WS_OVERLAPPEDWINDOW,
                                       ((x > 0) ? x : CW_USEDEFAULT),
                                       ((y > 0) ? y : CW_USEDEFAULT),
                                       ((w > 0) ? w : CW_USEDEFAULT),
                                       ((h > 0) ? h : CW_USEDEFAULT),
                                       nullptr, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!mainWindowHandle) {
        print_p(L"Failed to create main window.");
        return false;
    }

    mainWindowDpi = getWindowDpi_p(mainWindowHandle);
    if (mainWindowDpi == 0) {
        mainWindowDpi = USER_DEFAULT_SCREEN_DPI;
    }

    const auto cleanup = []() {
        if (mainWindowHandle) {
            DestroyWindow(mainWindowHandle);
            mainWindowHandle = nullptr;
        }
        if (mainWindowAtom != static_cast<ATOM>(0)) {
            UnregisterClassW(mainWindowClassName, HINST_THISCOMPONENT);
            mainWindowAtom = 0;
        }
    };

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (!updateFrameMargins_p(mainWindowHandle, mainWindowDpi)) {
        print_p(L"Failed to update main window's frame margins.");
        cleanup();
        return false;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (!triggerFrameChange_p(mainWindowHandle)) {
        print_p(L"Failed to trigger frame change event for main window.");
        cleanup();
        return false;
    }
    // Ensure our window still has window transitions.
    if (!setWindowTransitionsEnabled_p(mainWindowHandle, true)) {
        print_p(L"Failed to enable window transitions for main window.");
        cleanup();
        return false;
    }

    return true;
}

[[nodiscard]] static inline bool createDragBarWindow_p()
{
    // Please refer to the "IMPORTANT NOTE" section below.
    if (compareSystemVersion_p(WindowsVersion::Windows8, VersionCompare::Less)) {
        print_p(L"Drag bar window is only available on Windows 8 and onwards.");
        return false;
    }
    if (dragBarWindowAtom == static_cast<ATOM>(0)) {
        print_p(L"Drag bar window class has not been created.");
        return false;
    }
    if (!mainWindowHandle) {
        print_p(L"Main window has not been created.");
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
    dragBarWindowHandle = CreateWindowExW(WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP,
                                          dragBarWindowClassName, dragBarWindowTitle,
                                          WS_CHILD,
                                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                          mainWindowHandle, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!dragBarWindowHandle) {
        print_p(L"Failed to create drag bar window.");
        return false;
    }

    const auto cleanup = []() {
        if (dragBarWindowHandle) {
            DestroyWindow(dragBarWindowHandle);
            dragBarWindowHandle = nullptr;
        }
        if (dragBarWindowAtom != static_cast<ATOM>(0)) {
            UnregisterClassW(dragBarWindowClassName, HINST_THISCOMPONENT);
            dragBarWindowAtom = 0;
        }
    };

    // Layered window won't be visible until we call SetLayeredWindowAttributes()
    // or UpdateLayeredWindow().
    if (SetLayeredWindowAttributes(dragBarWindowHandle, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
        print_p(L"SetLayeredWindowAttributes() failed.");
        cleanup();
        return false;
    }

    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(mainWindowHandle, &rect) == FALSE) {
        print_p(L"Failed to retrieve client rect of main window.");
        cleanup();
        return false;
    }
    if (SetWindowPos(dragBarWindowHandle, HWND_TOP, 0, 0, rect.right,
                 getTitleBarHeight_p(mainWindowHandle, mainWindowDpi),
                 SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        print_p(L"Failed to move drag bar window.");
        cleanup();
        return false;
    }

    return true;
}

[[nodiscard]] static inline bool createXAMLIsland_p()
{
    // XAML Island is only supported on Windows 10 19H1 and onwards.
    if (compareSystemVersion_p(WindowsVersion::Windows10_19H1, VersionCompare::Less)) {
        print_p(L"XAML Island is only supported on Windows 10 19H1 and onwards.");
        return false;
    }
    if (!mainWindowHandle) {
        print_p(L"Main window has not been created.");
        return false;
    }
    const SystemTheme systemTheme = getSystemTheme_p();
    if (systemTheme == SystemTheme::Invalid) {
        print_p(L"Failed to retrieve system theme or high contrast mode is on.");
        return false;
    }

    winrt::init_apartment(winrt::apartment_type::single_threaded);
    xamlManager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    xamlSource = {};
    const auto interop = xamlSource.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        print_p(L"Failed to retrieve IDesktopWindowXamlSourceNative.");
        return false;
    }
    winrt::check_hresult(interop->AttachToWindow(mainWindowHandle));
    winrt::check_hresult(interop->get_WindowHandle(&xamlIslandHandle));
    if (!xamlIslandHandle) {
        print_p(L"Failed to retrieve XAML Island window handle.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(mainWindowHandle, &rect) == FALSE) {
        print_p(L"Failed to retrieve client rect of main window.");
        return false;
    }
    // Give enough space to our thin homemade top border.
    const int topMargin = getTopFrameMargin_p(mainWindowHandle, mainWindowDpi);
    if (SetWindowPos(xamlIslandHandle, HWND_BOTTOM, 0,
                 topMargin, rect.right, (rect.bottom - topMargin),
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        print_p(L"Failed to move XAML Island window.");
        return false;
    }
    xamlBrush = {};
    if (!switchAcrylicBrushTheme_p((systemTheme == SystemTheme::Auto) ? SystemTheme::Default : systemTheme)) {
        print_p(L"Failed to change acrylic brush's theme.");
        return false;
    }
    acrylicBrushTheme = SystemTheme::Auto;
    xamlBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    xamlGrid = {};
    xamlGrid.Background(xamlBrush);
    //xamlGrid.Children().Clear();
    //xamlGrid.Children().Append(/* some UWP control */);
    //xamlGrid.UpdateLayout();
    xamlSource.Content(xamlGrid);

    return true;
}

[[nodiscard]] static inline bool initialize_p(const int x, const int y, const int w, const int h)
{
    static bool tried = false;
    if (tried) {
        print_p(L"Acrylic application has been initialized already.");
        return false;
    }
    tried = true;

    if (!registerMainWindowClass_p()) {
        print_p(L"Failed to register main window class.", true);
        return false;
    }
    if (!createMainWindow_p(x, y, w, h)) {
        print_p(L"Failed to create main window.", true);
        return false;
    }
    if (compareSystemVersion_p(WindowsVersion::Windows10, VersionCompare::GreaterOrEqual)) {
        if (compareSystemVersion_p(WindowsVersion::Windows10_19H1, VersionCompare::GreaterOrEqual)) {
            if (createXAMLIsland_p()) {
                if (registerDragBarWindowClass_p()) {
                    if (!createDragBarWindow_p()) {
                        print_p(L"Failed to create drag bar window.", true);
                        return false;
                    }
                } else {
                    print_p(L"Failed to register drag bar window class.", true);
                    return false;
                }
            } else {
                print_p(L"Failed to create XAML Island.", true);
                return false;
            }
        } else {
            print_p(L"XAML Island is only supported on Windows 10 19H1 and onwards.", true);
            //return false;
        }
    } else {
        print_p(L"This application only supports Windows 10 and onwards.", true);
        //return false;
    }

    return true;
}

[[nodiscard]] static inline int mainWindowEventLoop_p()
{
    if (!mainWindowHandle) {
        return -1;
    }

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

AcrylicApplication::AcrylicApplication(const int argc, wchar_t *argv[])
{
    if (compareSystemVersion_p(WindowsVersion::WindowsVista, VersionCompare::Less)) {
        print_p(L"This application cannot be run on such old systems.", true);
        std::exit(-1);
    }

    if (!isCompositionEnabled_p()) {
        print_p(L"This application could not be started when DWM composition is disabled.", true);
        std::exit(-1);
    }

    if (instance) {
        print_p(L"There could only be one AcrylicApplication instance.", true);
        std::exit(-1);
    } else {
        instance = this;
    }

    arguments = argv;
}

AcrylicApplication::~AcrylicApplication() = default;

bool AcrylicApplication::createWindow(const int x, const int y, const int w, const int h) const
{
    return initialize_p(x, y, w, h);
}

RECT AcrylicApplication::getWindowGeometry() const
{
    return getWindowGeometry_p();
}

bool AcrylicApplication::moveWindow(const int x, const int y) const
{
    return moveWindow_p(x, y);
}

SIZE AcrylicApplication::getWindowSize() const
{
    return getWindowSize_p();
}

bool AcrylicApplication::resizeWindow(const int w, const int h) const
{
    return resizeWindow_p(w, h);
}

bool AcrylicApplication::centerWindow() const
{
    return centerWindow_p();
}

WindowState AcrylicApplication::getWindowState() const
{
    return getWindowState_p();
}

bool AcrylicApplication::setWindowState(const WindowState state) const
{
    return setWindowState_p(state);
}

bool AcrylicApplication::destroyWindow() const
{
    return destroyWindow_p();
}

HWND AcrylicApplication::getHandle() const
{
    return mainWindowHandle;
}

SystemTheme AcrylicApplication::getAcrylicBrushTheme() const
{
    return acrylicBrushTheme;
}

bool AcrylicApplication::setAcrylicBrushTheme(const SystemTheme theme) const
{
    return switchAcrylicBrushTheme_p(theme);
}

bool AcrylicApplication::getTintColor(int *r, int *g, int *b, int *a) const
{
    return getTintColor_p(r, g, b, a);
}

bool AcrylicApplication::setTintColor(const int r, const int g, const int b, const int a) const
{
    return setTintColor_p(r, g, b, a);
}

bool AcrylicApplication::getTintOpacity(double *opacity) const
{
    return getTintOpacity_p(opacity);
}

bool AcrylicApplication::setTintOpacity(const double opacity) const
{
    return setTintOpacity_p(opacity);
}

bool AcrylicApplication::getTintLuminosityOpacity(double *opacity) const
{
    return getTintLuminosityOpacity_p(opacity);
}

bool AcrylicApplication::setTintLuminosityOpacity(const double opacity) const
{
    return setTintLuminosityOpacity_p(opacity);
}

bool AcrylicApplication::getFallbackColor(int *r, int *g, int *b, int *a) const
{
    return getFallbackColor_p(r, g, b, a);
}

bool AcrylicApplication::setFallbackColor(const int r, const int g, const int b, const int a) const
{
    return setFallbackColor_p(r, g, b, a);
}

int AcrylicApplication::exec()
{
    return mainWindowEventLoop_p();
}
