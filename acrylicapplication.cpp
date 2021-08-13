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

#ifndef _USER32_
#define _USER32_
#endif

#ifndef _UXTHEME_
#define _UXTHEME_
#endif

#ifndef _DWMAPI_
#define _DWMAPI_
#endif

#include "acrylicapplication_p.h"
#include "resource.h"
#include <Unknwn.h>
#include <ShellApi.h>
#include <ShellScalingApi.h>
#include <UxTheme.h>
#include <DwmApi.h>
#include <WinRT/Windows.UI.Xaml.Hosting.h>
#include <WinRT/Windows.UI.Xaml.Controls.h>
#include <WinRT/Windows.UI.Xaml.Media.h>
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

namespace Private
{

class DynamicAPIs
{
public:
    explicit DynamicAPIs() = default;
    ~DynamicAPIs() = default;

    static DynamicAPIs &instance() {
        static DynamicAPIs dynamicAPIs = DynamicAPIs{};
        return dynamicAPIs;
    }

    [[nodiscard]] bool ShouldAppsUseDarkMode(bool *ok = nullptr) const
    {
        if (ok) {
            *ok = false;
        }
        static bool tried = false;
        using sig = BOOL(WINAPI *)();
        static sig func = nullptr;
        if (!func) {
            if (tried) {
                return false;
            } else {
                tried = true;
                const HMODULE dll = LoadLibraryExW(L"UxTheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (!dll) {
                    return false;
                }
                func = reinterpret_cast<sig>(GetProcAddress(dll, MAKEINTRESOURCEA(132)));
                if (!func) {
                    return false;
                }
            }
        }
        if (ok) {
            *ok = true;
        }
        return func();
    }

    [[nodiscard]] bool ShouldSystemUseDarkMode(bool *ok = nullptr) const
    {
        if (ok) {
            *ok = false;
        }
        static bool tried = false;
        using sig = BOOL(WINAPI *)();
        static sig func = nullptr;
        if (!func) {
            if (tried) {
                return false;
            } else {
                tried = true;
                const HMODULE dll = LoadLibraryExW(L"UxTheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (!dll) {
                    return false;
                }
                func = reinterpret_cast<sig>(GetProcAddress(dll, MAKEINTRESOURCEA(138)));
                if (!func) {
                    return false;
                }
            }
        }
        if (ok) {
            *ok = true;
        }
        return func();
    }

private:
    DynamicAPIs(const DynamicAPIs &) = delete;
    DynamicAPIs &operator=(const DynamicAPIs &) = delete;
    DynamicAPIs(DynamicAPIs &&) = delete;
    DynamicAPIs &operator=(DynamicAPIs &&) = delete;
};

}

// The thickness of an auto-hide taskbar in pixels.
static const int kAutoHideTaskbarThicknessPx = 2;
static const int kAutoHideTaskbarThicknessPy = kAutoHideTaskbarThicknessPx;

// Initialize global variables.
AcrylicApplication *AcrylicApplication::instance = nullptr;
const std::wstring AcrylicApplicationPrivate::mainWindowClassName = L"Win32AcrylicApplicationMainWindowClass";
const std::wstring AcrylicApplicationPrivate::dragBarWindowClassName = L"Win32AcrylicApplicationDragBarWindowClass";
const std::wstring AcrylicApplicationPrivate::mainWindowTitle = L"Win32 Native C++ Acrylic Application Main Window";
const std::wstring AcrylicApplicationPrivate::dragBarWindowTitle = {};
UINT AcrylicApplicationPrivate::mainWindowDpi = USER_DEFAULT_SCREEN_DPI;
HWND AcrylicApplicationPrivate::mainWindowHandle = nullptr;
HWND AcrylicApplicationPrivate::dragBarWindowHandle = nullptr;
HWND AcrylicApplicationPrivate::xamlIslandHandle = nullptr;
ATOM AcrylicApplicationPrivate::mainWindowAtom = 0;
ATOM AcrylicApplicationPrivate::dragBarWindowAtom = 0;
std::vector<std::wstring> AcrylicApplicationPrivate::arguments = {};
SystemTheme AcrylicApplicationPrivate::acrylicTheme = SystemTheme::Invalid;

// XAML
static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager xamlManager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource xamlSource = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush acrylicBrush = nullptr;

AcrylicApplicationPrivate::AcrylicApplicationPrivate(const std::vector<std::wstring> &args)
{
    arguments = args;
}

AcrylicApplicationPrivate::~AcrylicApplicationPrivate() = default;

int AcrylicApplicationPrivate::exec()
{
    if (!mainWindowHandle) {
        return -1;
    }

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        if (TranslateMessage(&msg) == FALSE) {
            return -1;
        }
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

void AcrylicApplicationPrivate::print(const MessageType type, const std::wstring &text)
{
    if (text.empty()) {
        return;
    }
    UINT icon = 0;
    std::wstring title = {};
    switch (type) {
    case MessageType::Information: {
        icon = MB_ICONINFORMATION;
        title = L"Information";
    } break;
    case MessageType::Question: {
        icon = MB_ICONQUESTION;
        title = L"Question";
    } break;
    case MessageType::Warning: {
        icon = MB_ICONWARNING;
        title = L"Warning";
    } break;
    case MessageType::Error: {
        icon = MB_ICONERROR;
        title = L"Error";
    } break;
    }
    OutputDebugStringW(text.c_str());
    MessageBoxW(nullptr, text.c_str(), title.c_str(), MB_OK | icon);
}

UINT AcrylicApplicationPrivate::getWindowDpi(const HWND hWnd)
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
        const HDC hScreenDc = GetDC(nullptr);
        const int dpiX = GetDeviceCaps(hScreenDc, LOGPIXELSX);
        const int dpiY = GetDeviceCaps(hScreenDc, LOGPIXELSY);
        ReleaseDC(nullptr, hScreenDc);
        if ((dpiX > 0) && (dpiY > 0)) {
            return std::round(static_cast<double>(dpiX + dpiY) / 2.0);
        }
    }
    return USER_DEFAULT_SCREEN_DPI;
}

bool AcrylicApplicationPrivate::isWindowMinimized(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsMinimized(hWnd) != FALSE);
}

bool AcrylicApplicationPrivate::isWindowMaximized(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsMaximized(hWnd) != FALSE);
}

double AcrylicApplicationPrivate::getDevicePixelRatio(const UINT dpi)
{
    if (dpi == 0) {
        return 1.0;
    }
    return (static_cast<double>(dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
}

int AcrylicApplicationPrivate::getResizeBorderThickness(const bool x, const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    // There is no "SM_CYPADDEDBORDER".
    const int result = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, _dpi)
            + GetSystemMetricsForDpi((x ? SM_CXSIZEFRAME : SM_CYSIZEFRAME), _dpi);
    const int preset = std::round(8.0 * getDevicePixelRatio(_dpi));
    return ((result > 0) ? result : preset);
}

int AcrylicApplicationPrivate::getCaptionHeight(const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    const int result = GetSystemMetricsForDpi(SM_CYCAPTION, _dpi);
    const int preset = std::round(23.0 * getDevicePixelRatio(_dpi));
    return ((result > 0) ? result : preset);
}

int AcrylicApplicationPrivate::getTitleBarHeight(const HWND hWnd, const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    int titleBarHeight = 0;
    if (hWnd) {
        RECT frame = {0, 0, 0, 0};
        AdjustWindowRectExForDpi(&frame,
                                 (static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)) & ~WS_OVERLAPPED),
                                 FALSE,
                                 static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)),
                                 _dpi);
        titleBarHeight = std::abs(frame.top);
    }
    if (titleBarHeight <= 0) {
        titleBarHeight = getResizeBorderThickness(false, _dpi) + getCaptionHeight(_dpi);
        if (titleBarHeight <= 0) {
            titleBarHeight = std::round(31.0 * getDevicePixelRatio(_dpi));
        }
    }
    return titleBarHeight;
}

RECT AcrylicApplicationPrivate::getScreenGeometry(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    return getMonitorInfo(hWnd).rcMonitor;
}

RECT AcrylicApplicationPrivate::getScreenAvailableGeometry(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    return getMonitorInfo(hWnd).rcWork;
}

int AcrylicApplicationPrivate::getTopFrameMargin(const HWND hWnd, const UINT dpi)
{
    const UINT _dpi = (dpi == 0) ? USER_DEFAULT_SCREEN_DPI : dpi;
    return ((hWnd && isWindowNoState(hWnd)) ? std::round(1.0 * getDevicePixelRatio(_dpi)) : 0);
}

bool AcrylicApplicationPrivate::isCompositionEnabled()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (compareSystemVersion(WindowsVersion::Windows8, VersionCompare::GreaterOrEqual)) {
        return true;
    }
    BOOL enabled = FALSE;
    return (SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && (enabled != FALSE));
}

bool AcrylicApplicationPrivate::isWindowFullScreened(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &windowRect) == FALSE) {
        return false;
    }
    const RECT screenRect = getScreenGeometry(hWnd);
    return ((windowRect.left == screenRect.left)
            && (windowRect.right == screenRect.right)
            && (windowRect.top == screenRect.top)
            && (windowRect.bottom == screenRect.bottom));
}

bool AcrylicApplicationPrivate::isWindowNoState(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    if (GetWindowPlacement(hWnd, &wp) == FALSE) {
        return false;
    }
    return (wp.showCmd == SW_NORMAL);
}

bool AcrylicApplicationPrivate::isWindowVisible(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return (IsWindowVisible(hWnd) != FALSE);
}

bool AcrylicApplicationPrivate::isWindowHidden(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    return !isWindowVisible(hWnd);
}

bool AcrylicApplicationPrivate::triggerFrameChange(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    const BOOL result = SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    return (result != FALSE);
}

bool AcrylicApplicationPrivate::updateFrameMargins(const HWND hWnd, const UINT dpi)
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
    const MARGINS margins = {0, 0, (isWindowNoState(hWnd) ? getTitleBarHeight(hWnd, _dpi) : 0), 0};
    return SUCCEEDED(DwmExtendFrameIntoClientArea(hWnd, &margins));
}

MONITORINFO AcrylicApplicationPrivate::getMonitorInfo(const HWND hWnd)
{
    if (!hWnd) {
        return {};
    }
    const HMONITOR mon = CURRENT_SCREEN(hWnd);
    if (!mon) {
        return {};
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        return {};
    }
    return mi;
}

bool AcrylicApplicationPrivate::showWindowFullScreen(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    auto style = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE));
    style &= ~(WS_OVERLAPPEDWINDOW); // fixme: check
    style |= WS_POPUP;
    SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    const RECT rect = getScreenGeometry(hWnd);
    return (MoveWindow(hWnd, rect.left, rect.top,
                       RECT_WIDTH(rect), RECT_HEIGHT(rect), TRUE) != FALSE);
}

bool AcrylicApplicationPrivate::setWindowTransitionsEnabled(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return false;
    }
    const BOOL disabled = enable ? FALSE : TRUE;
    return SUCCEEDED(DwmSetWindowAttribute(hWnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disabled, sizeof(disabled)));
}

bool AcrylicApplicationPrivate::openSystemMenu(const HWND hWnd, const POINT pos)
{
    if (!hWnd) {
        return false;
    }
    const HMENU menu = GetSystemMenu(hWnd, FALSE);
    if (!menu) {
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
    const bool isMaximized = isWindowMaximized(hWnd);
    if (!setState(SC_RESTORE, isMaximized)) {
        return false;
    }
    if (!setState(SC_MOVE, !isMaximized)) {
        return false;
    }
    if (!setState(SC_SIZE, !isMaximized)) {
        return false;
    }
    if (!setState(SC_MINIMIZE, true)) {
        return false;
    }
    if (!setState(SC_MAXIMIZE, !isMaximized)) {
        return false;
    }
    if (!setState(SC_CLOSE, true)) {
        return false;
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        return false;
    }
    // ### TODO: support LTR layout.
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, pos.x, pos.y, 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            return false;
        }
    }
    return true;
}

SystemTheme AcrylicApplicationPrivate::getSystemTheme()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, 0, &hc, 0) == FALSE) {
        return SystemTheme::Invalid;
    }
    if (hc.dwFlags & HCF_HIGHCONTRASTON) {
        return SystemTheme::HighContrast;
    }
    // Dark mode was first introduced in Windows 10 1607.
    if (compareSystemVersion(WindowsVersion::Windows10_1607, VersionCompare::GreaterOrEqual)) {
        bool ok = false;
        bool lightModeOn = Private::DynamicAPIs::instance().ShouldAppsUseDarkMode(&ok);
        if (!ok) {
            lightModeOn = Private::DynamicAPIs::instance().ShouldSystemUseDarkMode(&ok);
            if (!ok) {
#if 0
                const QString appKey = QStringLiteral("AppsUseLightTheme");
                const QString sysKey = QStringLiteral("SystemUsesLightTheme");
                const QSettings registry(g_personalizeRegistryKey, QSettings::NativeFormat);
                if (registry.contains(appKey)) {
                    if (registry.value(appKey, 0).toBool()) {
                        return SystemTheme::Light;
                    } else {
                        return SystemTheme::Dark;
                    }
                } else if (registry.contains(sysKey)) {
                    if (registry.value(sysKey, 0).toBool()) {
                        return SystemTheme::Light;
                    } else {
                        return SystemTheme::Dark;
                    }
                } else {
                    return SystemTheme::Light;
                }
#endif
            }
        }
        if (ok) {
            return (lightModeOn ? SystemTheme::Light : SystemTheme::Dark);
        } else {
            return SystemTheme::Invalid;
        }
    } else {
        return SystemTheme::Invalid;
    }
}

bool AcrylicApplicationPrivate::compareSystemVersion(const WindowsVersion ver,
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

LRESULT CALLBACK AcrylicApplicationPrivate::mainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
        if (isWindowMaximized(hWnd) && !isWindowFullScreened(hWnd)) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            clientRect->top += getResizeBorderThickness(false, mainWindowDpi);
            nonClientAreaExists = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (isWindowMaximized(hWnd) || isWindowFullScreened(hWnd)) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            // First, check if we have an auto-hide taskbar at all:
            if (SHAppBarMessage(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) {
                const RECT screenRect = getScreenGeometry(hWnd);
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
            break;
        }
        const int rbtY = getResizeBorderThickness(false, mainWindowDpi);
        // At this point, we know that the cursor is inside the client area
        // so it has to be either the little border at the top of our custom
        // title bar or the drag bar. Apparently, it must be the drag bar or
        // the little border at the top which the user can use to move or
        // resize the window.
        if (isWindowNoState(hWnd) && (pos.y <= rbtY)) {
            return HTTOP;
        }
        const int cth = getCaptionHeight(mainWindowDpi);
        if (isWindowMaximized(hWnd) && (pos.y >= 0) && (pos.y <= cth)) {
            return HTCAPTION;
        }
        if (isWindowNoState(hWnd) && (pos.y > rbtY) && (pos.y <= (rbtY + cth))) {
            return HTCAPTION;
        }
        return HTCLIENT;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps = {};
        const HDC hdc = BeginPaint(hWnd, &ps);
        if (!hdc) {
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
        const LONG topBorderHeight = getTopFrameMargin(hWnd, mainWindowDpi);
        if (ps.rcPaint.top < topBorderHeight) {
            RECT rcTopBorder = ps.rcPaint;
            rcTopBorder.bottom = topBorderHeight;
            // To show the original top border, we have to paint on top
            // of it with the alpha component set to 0. This page
            // recommends to paint the area in black using the stock
            // BLACK_BRUSH to do this:
            // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
            if (FillRect(hdc, &rcTopBorder, BACKGROUND_BRUSH) == 0) {
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
                break;
            }
            if (FillRect(opaqueDc, &rcRest,
                         reinterpret_cast<HBRUSH>(GetClassLongPtrW(hWnd, GCLP_HBRBACKGROUND))) == 0) {
                break;
            }
            if (FAILED(BufferedPaintSetAlpha(buf, nullptr, 255))) {
                break;
            }
            if (FAILED(EndBufferedPaint(buf, TRUE))) {
                break;
            }
        }
        if (EndPaint(hWnd, &ps) == FALSE) {
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
            break;
        }
        return 0;
    }
    case WM_SIZE: {
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)
                || isWindowFullScreened(mainWindowHandle)) {
            if (!updateFrameMargins(hWnd, mainWindowDpi)) {
                break;
            }
        }
        const auto width = LOWORD(lParam);
        const UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER;
        if (xamlIslandHandle) {
            // Give enough space to our thin homemade top border.
            if (SetWindowPos(xamlIslandHandle, HWND_BOTTOM, 0, getTopFrameMargin(hWnd, mainWindowDpi),
                         width, HIWORD(lParam), flags) == FALSE) {
                break;
            }
        }
        if (dragBarWindowHandle) {
            if (SetWindowPos(dragBarWindowHandle, HWND_TOP, 0, 0, width,
                         getTitleBarHeight(hWnd, mainWindowDpi), flags) == FALSE) {
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
            if (!openSystemMenu(hWnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) {
                break;
            }
        }
    } break;
    case WM_CLOSE: {
        if (dragBarWindowHandle) {
            if (DestroyWindow(dragBarWindowHandle) == FALSE) {
                break;
            }
            dragBarWindowHandle = nullptr;
        }
#if 0
        if (xamlIslandHandle) {
            if (DestroyWindow(xamlIslandHandle) == FALSE) {
                break;
            }
            xamlIslandHandle = nullptr;
        }
#endif
        if (DestroyWindow(hWnd) == FALSE) {
            break;
        }
        mainWindowHandle = nullptr;
        return 0;
    }
    case WM_DESTROY: {
        if (dragBarWindowAtom != static_cast<ATOM>(0)) {
            if (UnregisterClassW(dragBarWindowClassName.c_str(), HINST_THISCOMPONENT) == FALSE) {
                break;
            }
            dragBarWindowAtom = 0;
        }
        if (mainWindowAtom != static_cast<ATOM>(0)) {
            if (UnregisterClassW(mainWindowClassName.c_str(), HINST_THISCOMPONENT) == FALSE) {
                break;
            }
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
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK AcrylicApplicationPrivate::dragBarWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

bool AcrylicApplicationPrivate::registerMainWindowClass() const
{
    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = mainWindowProc;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = mainWindowClassName.c_str();
    wcex.hIcon = LoadIconW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_ICON1));

    mainWindowAtom = RegisterClassExW(&wcex);

    return (mainWindowAtom != static_cast<ATOM>(0));
}

bool AcrylicApplicationPrivate::registerDragBarWindowClass() const
{
    if (compareSystemVersion(WindowsVersion::Windows8, VersionCompare::Less)) {
        return false;
    }
    if (!mainWindowHandle) {
        return false;
    }

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = dragBarWindowProc;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = BACKGROUND_BRUSH;
    wcex.lpszClassName = dragBarWindowClassName.c_str();

    dragBarWindowAtom = RegisterClassExW(&wcex);

    return (dragBarWindowAtom != static_cast<ATOM>(0));
}

bool AcrylicApplicationPrivate::createMainWindow(const int x, const int y, const int w, const int h) const
{
    if (mainWindowAtom == static_cast<ATOM>(0)) {
        return false;
    }

    mainWindowHandle = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                                       mainWindowClassName.c_str(), mainWindowTitle.c_str(),
                                       WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                       ((x > 0) ? x : CW_USEDEFAULT),
                                       ((y > 0) ? y : CW_USEDEFAULT),
                                       ((w > 0) ? w : CW_USEDEFAULT),
                                       ((h > 0) ? h : CW_USEDEFAULT),
                                       nullptr, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!mainWindowHandle) {
        return false;
    }

    mainWindowDpi = getWindowDpi(mainWindowHandle);

    const auto cleanup = []() {
        if (mainWindowHandle) {
            DestroyWindow(mainWindowHandle);
            mainWindowHandle = nullptr;
        }
        if (mainWindowAtom != static_cast<ATOM>(0)) {
            UnregisterClassW(mainWindowClassName.c_str(), HINST_THISCOMPONENT);
            mainWindowAtom = 0;
        }
    };

    ShowWindow(mainWindowHandle, SW_SHOW);
    if (UpdateWindow(mainWindowHandle) == FALSE) {
        cleanup();
        return false;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (!updateFrameMargins(mainWindowHandle, mainWindowDpi)) {
        cleanup();
        return false;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (!triggerFrameChange(mainWindowHandle)) {
        cleanup();
        return false;
    }
    // Ensure our window still has window transitions.
    if (!setWindowTransitionsEnabled(mainWindowHandle, true)) {
        cleanup();
        return false;
    }

    if (RedrawWindow(mainWindowHandle, nullptr, nullptr,
                 RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW) == FALSE) {
        cleanup();
        return false;
    }

    return true;
}

bool AcrylicApplicationPrivate::createDragBarWindow() const
{
    // Please refer to the "IMPORTANT NOTE" section below.
    if (compareSystemVersion(WindowsVersion::Windows8, VersionCompare::Less)) {
        return false;
    }
    if (dragBarWindowAtom == static_cast<ATOM>(0)) {
        return false;
    }
    if (!mainWindowHandle) {
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
                                          dragBarWindowClassName.c_str(), dragBarWindowTitle.c_str(),
                                          WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                          mainWindowHandle, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!dragBarWindowHandle) {
        return false;
    }

    const auto cleanup = []() {
        if (dragBarWindowHandle) {
            DestroyWindow(dragBarWindowHandle);
            dragBarWindowHandle = nullptr;
        }
        if (dragBarWindowAtom != static_cast<ATOM>(0)) {
            UnregisterClassW(dragBarWindowClassName.c_str(), HINST_THISCOMPONENT);
            dragBarWindowAtom = 0;
        }
    };

    // Layered window won't be visible until we call SetLayeredWindowAttributes()
    // or UpdateLayeredWindow().
    if (SetLayeredWindowAttributes(dragBarWindowHandle, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
        cleanup();
        return false;
    }

    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(mainWindowHandle, &rect) == FALSE) {
        cleanup();
        return false;
    }
    if (SetWindowPos(dragBarWindowHandle, HWND_TOP, 0, 0, rect.right,
                 getTitleBarHeight(mainWindowHandle, mainWindowDpi),
                 SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        cleanup();
        return false;
    }

    ShowWindow(dragBarWindowHandle, SW_SHOW);
    if (UpdateWindow(dragBarWindowHandle) == FALSE) {
        cleanup();
        return false;
    }

    return true;
}

bool AcrylicApplicationPrivate::createXAMLIsland() const
{
    // XAML Island is only supported on Windows 10 19H1 and onwards.
    if (compareSystemVersion(WindowsVersion::Windows10_19H1, VersionCompare::Less)) {
        return false;
    }
    if (!mainWindowHandle) {
        return false;
    }
    const SystemTheme systemTheme = getSystemTheme();
    if (systemTheme == SystemTheme::Invalid) {
        return false;
    }

    const auto cleanup = []() {
        if (rootGrid) {
            rootGrid.Background(nullptr);
        }
        if (xamlSource) {
            xamlSource.Content(nullptr);
        }
        acrylicBrush = nullptr;
        rootGrid = nullptr;
        xamlSource = nullptr;
        xamlManager = nullptr;
    };

    winrt::init_apartment(winrt::apartment_type::single_threaded);
    xamlManager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    if (!xamlManager) {
        cleanup();
        return false;
    }
    const auto interop = xamlSource.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        cleanup();
        return false;
    }
    winrt::check_hresult(interop->AttachToWindow(mainWindowHandle));
    winrt::check_hresult(interop->get_WindowHandle(&xamlIslandHandle));
    if (!xamlIslandHandle) {
        cleanup();
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(mainWindowHandle, &rect) == FALSE) {
        cleanup();
        return false;
    }
    // Give enough space to our thin homemade top border.
    if (SetWindowPos(xamlIslandHandle, HWND_BOTTOM, 0,
                 getTopFrameMargin(mainWindowHandle, mainWindowDpi), rect.right, rect.bottom,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        cleanup();
        return false;
    }
    acrylicBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    if (!setTheme(systemTheme)) {
        cleanup();
        return false;
    }
    rootGrid.Background(acrylicBrush);
    //rootGrid.Children().Clear();
    //rootGrid.Children().Append(/* some UWP control */);
    //rootGrid.UpdateLayout();
    xamlSource.Content(rootGrid);

    ShowWindow(xamlIslandHandle, SW_SHOW);
    if (UpdateWindow(xamlIslandHandle) == FALSE) {
        cleanup();
        return false;
    }

    return true;
}

bool AcrylicApplicationPrivate::createWindow(const int x, const int y, const int w, const int h) const
{
    static bool tried = false;
    if (tried) {
        return false;
    }
    tried = true;

    if (!registerMainWindowClass()) {
        print(MessageType::Error, L"Failed to register main window class.");
        return false;
    }
    if (!createMainWindow(x, y, w, h)) {
        print(MessageType::Error, L"Failed to create main window.");
        return false;
    }
    if (compareSystemVersion(WindowsVersion::Windows10, VersionCompare::GreaterOrEqual)) {
        if (compareSystemVersion(WindowsVersion::Windows10_19H1, VersionCompare::GreaterOrEqual)) {
            if (createXAMLIsland()) {
                if (registerDragBarWindowClass()) {
                    if (!createDragBarWindow()) {
                        print(MessageType::Error, L"Failed to create drag bar window.");
                        return false;
                    }
                } else {
                    print(MessageType::Error, L"Failed to register drag bar window class.");
                    return false;
                }
            } else {
                print(MessageType::Error, L"Failed to create XAML Island.");
                return false;
            }
        } else {
            print(MessageType::Warning, L"XAML Island applications are only supported from Windows 10 19H1.");
            //return false;
        }
    } else {
        print(MessageType::Warning, L"This application only supports Windows 10 and onwards.");
        //return false;
    }

    return true;
}

RECT AcrylicApplicationPrivate::getWindowGeometry() const
{
    if (!mainWindowHandle) {
        return {};
    }
    RECT geo = {0, 0, 0, 0};
    if (GetWindowRect(mainWindowHandle, &geo) == FALSE) {
        return {};
    }
    return geo;
}

bool AcrylicApplicationPrivate::moveWindow(const int x, const int y) const
{
    if (!mainWindowHandle) {
        return false;
    }
    // The x coordinate and y coordinate can be negative numbers, if so,
    // the window will be moved to another screen.
    const SIZE s = getWindowSize();
    return (MoveWindow(mainWindowHandle, x, y, s.cx, s.cy, TRUE) != FALSE);
}

SIZE AcrylicApplicationPrivate::getWindowSize() const
{
    if (!mainWindowHandle) {
        return {};
    }
    const RECT geo = getWindowGeometry();
    return {RECT_WIDTH(geo), RECT_HEIGHT(geo)};
}

bool AcrylicApplicationPrivate::resizeWindow(const int w, const int h) const
{
    if (!mainWindowHandle) {
        return false;
    }
    if ((w <= 0) || (h <= 0)) {
        return false;
    }
    const RECT geo = getWindowGeometry();
    return (MoveWindow(mainWindowHandle, geo.left, geo.top,
                       RECT_WIDTH(geo), RECT_HEIGHT(geo), TRUE) != FALSE);
}

bool AcrylicApplicationPrivate::centerWindow() const
{
    if (!mainWindowHandle) {
        return false;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(mainWindowHandle, &windowRect) == FALSE) {
        return false;
    }
    const int windowWidth = RECT_WIDTH(windowRect);
    const int windowHeight = RECT_HEIGHT(windowRect);
    const RECT screenRect = getScreenGeometry(mainWindowHandle);
    const int screenWidth = RECT_WIDTH(screenRect);
    const int screenHeight = RECT_HEIGHT(screenRect);
    const int newX = (screenWidth - windowWidth) / 2;
    const int newY = (screenHeight - windowHeight) / 2;
    return (MoveWindow(mainWindowHandle, newX, newY, windowWidth, windowHeight, TRUE) != FALSE);
}

WindowState AcrylicApplicationPrivate::getWindowState() const
{
    if (!mainWindowHandle) {
        return WindowState::Invalid;
    }
    if (isWindowFullScreened(mainWindowHandle)) {
        return WindowState::FullScreened;
    } else if (isWindowMaximized(mainWindowHandle)) {
        return WindowState::Maximized;
    } else if (isWindowMinimized(mainWindowHandle)) {
        return WindowState::Minimized;
    } else if (isWindowNoState(mainWindowHandle)) {
        return WindowState::Normal;
    } else if (isWindowVisible(mainWindowHandle)) {
        return WindowState::Shown;
    } else if (isWindowHidden(mainWindowHandle)) {
        return WindowState::Hidden;
    }
    return WindowState::Invalid;
}

bool AcrylicApplicationPrivate::setWindowState(const WindowState state) const
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
        return showWindowFullScreen(mainWindowHandle);
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

bool AcrylicApplicationPrivate::destroyWindow() const
{
    if (dragBarWindowHandle) {
        DestroyWindow(dragBarWindowHandle);
        dragBarWindowHandle = nullptr;
    }
    if (dragBarWindowAtom != static_cast<ATOM>(0)) {
        UnregisterClassW(dragBarWindowClassName.c_str(), HINST_THISCOMPONENT);
        dragBarWindowAtom = 0;
    }
    if (mainWindowHandle) {
        DestroyWindow(mainWindowHandle);
        mainWindowHandle = nullptr;
    }
    if (mainWindowAtom != static_cast<ATOM>(0)) {
        UnregisterClassW(mainWindowClassName.c_str(), HINST_THISCOMPONENT);
        mainWindowAtom = 0;
    }
    return true;
}

HWND AcrylicApplicationPrivate::getHandle() const
{
    return mainWindowHandle;
}

SystemTheme AcrylicApplicationPrivate::getTheme() const
{
    return acrylicTheme;
}

bool AcrylicApplicationPrivate::setTheme(const SystemTheme theme) const
{
    if (!acrylicBrush) {
        return false;
    }
    if (theme == SystemTheme::Invalid) {
        return false;
    }
    if (theme == SystemTheme::Light) {
        acrylicBrush.TintColor(L"#FCFCFC");
        acrylicBrush.TintOpacity(0.0);
        acrylicBrush.TintLuminosityOpacity(0.85);
        acrylicBrush.FallbackColor(L"#F9F9F9");
        acrylicTheme = SystemTheme::Light;
    } else {
        acrylicBrush.TintColor(L"#2C2C2C");
        acrylicBrush.TintOpacity(0.15);
        acrylicBrush.TintLuminosityOpacity(0.96);
        acrylicBrush.FallbackColor(L"#2C2C2C");
        acrylicTheme = SystemTheme::Dark;
    }
    return true;
}

AcrylicApplication::AcrylicApplication(const int argc, wchar_t *argv[])
{
    if (AcrylicApplicationPrivate::compareSystemVersion(WindowsVersion::WindowsVista, VersionCompare::Less)) {
        AcrylicApplicationPrivate::print(MessageType::Error,
                                         L"This application cannot be run on such old systems.");
        std::exit(-1);
    }

    if (!AcrylicApplicationPrivate::isCompositionEnabled()) {
        AcrylicApplicationPrivate::print(MessageType::Error,
                                         L"This application could not be started when DWM composition is disabled.");
        std::exit(-1);
    }

    if (instance) {
        AcrylicApplicationPrivate::print(MessageType::Error,
                                         L"There could only be one AcrylicApplication instance.");
        std::exit(-1);
    } else {
        instance = this;
    }

    std::vector<std::wstring> args = {};
    for (int i = 0; i != argc; ++i) {
        args.push_back(argv[i]);
    }

    d = std::make_unique<const AcrylicApplicationPrivate>(args);
}

AcrylicApplication::~AcrylicApplication() = default;

bool AcrylicApplication::createWindow(const int x, const int y, const int w, const int h) const
{
    return d->createWindow(x, y, w, h);
}

RECT AcrylicApplication::getWindowGeometry() const
{
    return d->getWindowGeometry();
}

bool AcrylicApplication::moveWindow(const int x, const int y) const
{
    return d->moveWindow(x, y);
}

SIZE AcrylicApplication::getWindowSize() const
{
    return d->getWindowSize();
}

bool AcrylicApplication::resizeWindow(const int w, const int h) const
{
    return d->resizeWindow(w, h);
}

bool AcrylicApplication::centerWindow() const
{
    return d->centerWindow();
}

WindowState AcrylicApplication::getWindowState() const
{
    return d->getWindowState();
}

bool AcrylicApplication::setWindowState(const WindowState state) const
{
    return d->setWindowState(state);
}

bool AcrylicApplication::destroyWindow() const
{
    return d->destroyWindow();
}

HWND AcrylicApplication::getHandle() const
{
    return d->getHandle();
}

SystemTheme AcrylicApplication::getTheme() const
{
    return d->getTheme();
}

bool AcrylicApplication::setTheme(const SystemTheme theme) const
{
    return d->setTheme(theme);
}

int AcrylicApplication::exec()
{
    return AcrylicApplicationPrivate::exec();
}
