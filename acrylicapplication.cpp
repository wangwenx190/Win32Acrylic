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

#include "acrylicapplication.h"
#include "acrylicapplication_p.h"
#include <Unknwn.h>
#include <ShellApi.h>
#include <ShellScalingApi.h>
#include <UxTheme.h>
#include <DwmApi.h>
#include <VersionHelpers.h>
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

// The thickness of an auto-hide taskbar in pixels.
static const int kAutoHideTaskbarThicknessPx = 2;
static const int kAutoHideTaskbarThicknessPy = kAutoHideTaskbarThicknessPx;

const std::wstring AcrylicApplicationPrivate::mainWindowClassName = L"Win32AcrylicApplicationMainWindowClass";
const std::wstring AcrylicApplicationPrivate::mainWindowTitle = L"Win32 Native C++ Acrylic Application Main Window";
UINT AcrylicApplicationPrivate::mainWindowDpi = USER_DEFAULT_SCREEN_DPI;
HWND AcrylicApplicationPrivate::mainWindowHandle = nullptr;
HWND AcrylicApplicationPrivate::xamlIslandHandle = nullptr;
bool AcrylicApplicationPrivate::mainWindowZoomed = false;

[[nodiscard]] static inline bool isWindows10_19H1OrGreater()
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber = 18362;
    DWORDLONG dwlConditionMask = 0;
    const BYTE op = VER_GREATER_EQUAL;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

AcrylicApplicationPrivate::AcrylicApplicationPrivate(const std::vector<std::wstring> &args, AcrylicApplication *q_ptr)
{
    q = q_ptr;
    arguments = args;
    initialize();
}

AcrylicApplicationPrivate::~AcrylicApplicationPrivate()
{
    if (mainWindowAtom != static_cast<ATOM>(0)) {
        UnregisterClassW(mainWindowClassName.c_str(), HINST_THISCOMPONENT);
    }
}

int AcrylicApplicationPrivate::exec()
{
    if (!mainWindowHandle) {
        return -1;
    }

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

void AcrylicApplicationPrivate::print(const MessageType type, const std::wstring &title, const std::wstring &text)
{
    UINT icon = 0;
    std::wstring str = {};
    switch (type) {
    case MessageType::Information: {
        icon = MB_ICONINFORMATION;
        str += L"[INFORMATION] ";
    } break;
    case MessageType::Question: {
        icon = MB_ICONQUESTION;
        str += L"[QUESTION] ";
    } break;
    case MessageType::Warning: {
        icon = MB_ICONWARNING;
        str += L"[WARNING] ";
    } break;
    case MessageType::Error: {
        icon = MB_ICONERROR;
        str += L"[ERROR] ";
    } break;
    }
    str += title + L": " + text;
    OutputDebugStringW(str.c_str());
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
        GetDpiForMonitor(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        if ((dpiX > 0) && (dpiY > 0)) {
            return std::round(static_cast<double>(dpiX + dpiY) / 2.0);
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

double AcrylicApplicationPrivate::getDevicePixelRatio(const UINT dpi)
{
    return (static_cast<double>(dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
}

int AcrylicApplicationPrivate::getResizeBorderThickness(const bool x, const UINT dpi)
{
    // There is no "SM_CYPADDEDBORDER".
    const int result = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi)
            + GetSystemMetricsForDpi((x ? SM_CXSIZEFRAME : SM_CYSIZEFRAME), dpi);
    const int preset = std::round(8.0 * getDevicePixelRatio(dpi));
    return ((result > 0) ? result : preset);
}

int AcrylicApplicationPrivate::getCaptionHeight(const UINT dpi)
{
    const int result = GetSystemMetricsForDpi(SM_CYCAPTION, dpi);
    const int preset = std::round(23.0 * getDevicePixelRatio(dpi));
    return ((result > 0) ? result : preset);
}

bool AcrylicApplicationPrivate::isWindowFullScreened(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    GetMonitorInfoW(MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
    RECT rect = {0, 0, 0, 0};
    GetWindowRect(hWnd, &rect);
    return ((rect.left == mi.rcMonitor.left)
            && (rect.right == mi.rcMonitor.right)
            && (rect.top == mi.rcMonitor.top)
            && (rect.bottom == mi.rcMonitor.bottom));
}

bool AcrylicApplicationPrivate::isWindowNoState(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    GetWindowPlacement(hWnd, &wp);
    return (wp.showCmd == SW_NORMAL);
}

void AcrylicApplicationPrivate::triggerFrameChange(const HWND hWnd)
{
    if (!hWnd) {
        return;
    }
    SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

void AcrylicApplicationPrivate::updateFrameMargins(const HWND hWnd)
{
    if (!hWnd) {
        return;
    }
    MARGINS margins = {0, 0, 0, 0};
    if (isWindowNoState(hWnd)) {
        RECT frame = {0, 0, 0, 0};
        AdjustWindowRectExForDpi(&frame,
                                 (static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)) & ~WS_OVERLAPPED),
                                 FALSE,
                                 static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)),
                                 mainWindowDpi);
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
        margins.cyTopHeight = std::abs(frame.top);
    }
    DwmExtendFrameIntoClientArea(hWnd, &margins);
}

LRESULT CALLBACK AcrylicApplicationPrivate::mainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NCCALCSIZE: {
        if (!wParam) {
            return 0;
        }
        bool nonClientAreaExists = false;
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
        if (IsMaximized(hWnd) && !isWindowFullScreened(hWnd)) {
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
        if (IsMaximized(hWnd) || isWindowFullScreened(hWnd)) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            // First, check if we have an auto-hide taskbar at all:
            if (SHAppBarMessage(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) {
                MONITORINFO mi;
                SecureZeroMemory(&mi, sizeof(mi));
                mi.cbSize = sizeof(mi);
                GetMonitorInfoW(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);
                // This helper can be used to determine if there's a
                // auto-hide taskbar on the given edge of the monitor
                // we're currently on.
                const auto hasAutohideTaskbar = [&mi](const UINT edge) -> bool {
                    APPBARDATA abd2;
                    SecureZeroMemory(&abd2, sizeof(abd2));
                    abd2.cbSize = sizeof(abd2);
                    abd2.uEdge = edge;
                    abd2.rc = mi.rcMonitor;
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
        ScreenToClient(hWnd, &pos);
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
        if (IsMaximized(hWnd)) {
            if ((pos.y >= 0) && (pos.y <= cth)) {
                return HTCAPTION;
            }
        }
        if (isWindowNoState(hWnd) && (pos.y > rbtY) && (pos.y <= (rbtY + cth))) {
            return HTCAPTION;
        }
        return HTCLIENT;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps = {};
        const HDC hdc = BeginPaint(hWnd, &ps);
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
        LONG topBorderHeight = 0;
        if (isWindowNoState(hWnd)) {
            topBorderHeight = 1;
        }
        if (ps.rcPaint.top < topBorderHeight) {
            RECT rcTopBorder = ps.rcPaint;
            rcTopBorder.bottom = topBorderHeight;
            // To show the original top border, we have to paint on top
            // of it with the alpha component set to 0. This page
            // recommends to paint the area in black using the stock
            // BLACK_BRUSH to do this:
            // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
            FillRect(hdc, &rcTopBorder, BACKGROUND_BRUSH);
        }
        if (ps.rcPaint.bottom > topBorderHeight) {
            RECT rcRest = ps.rcPaint;
            rcRest.top = topBorderHeight;
            // To hide the original title bar, we have to paint on top
            // of it with the alpha component set to 255. This is a hack
            // to do it with GDI. See UpdateFrameMarginsForWindow for
            // more information.
            HDC opaqueDc = nullptr;
            BP_PAINTPARAMS params;
            SecureZeroMemory(&params, sizeof(params));
            params.cbSize = sizeof(params);
            params.dwFlags = BPPF_NOCLIP | BPPF_ERASE;
            const HPAINTBUFFER buf = BeginBufferedPaint(hdc, &rcRest, BPBF_TOPDOWNDIB, &params, &opaqueDc);
            FillRect(opaqueDc, &rcRest, reinterpret_cast<HBRUSH>(GetClassLongPtrW(hWnd, GCLP_HBRBACKGROUND)));
            BufferedPaintSetAlpha(buf, nullptr, 255);
            EndBufferedPaint(buf, TRUE);
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_DPICHANGED: {
        const double x = LOWORD(wParam);
        const double y = HIWORD(wParam);
        mainWindowDpi = std::round((x + y) / 2.0);
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        SetWindowPos(hWnd, nullptr, prcNewWindow->left, prcNewWindow->top,
                     RECT_WIDTH(*prcNewWindow), RECT_HEIGHT(*prcNewWindow),
                     SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        return 0;
    }
    case WM_SIZE: {
        bool shouldUpdateFrameMargins = false;
        if (IsMaximized(hWnd) || isWindowFullScreened(hWnd)) {
            mainWindowZoomed = true;
            shouldUpdateFrameMargins = true;
        }
        if (mainWindowZoomed && isWindowNoState(hWnd)) {
            mainWindowZoomed = false;
            shouldUpdateFrameMargins = true;
        }
        if (shouldUpdateFrameMargins) {
            updateFrameMargins(hWnd);
        }
        if (xamlIslandHandle) {
            RECT rect = {0, 0, 0, 0};
            GetClientRect(hWnd, &rect);
            SetWindowPos(xamlIslandHandle, nullptr, 0, 0, rect.right, rect.bottom,
                         SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    } break;
    case WM_CLOSE: {
        DestroyWindow(hWnd);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

bool AcrylicApplicationPrivate::registerMainWindowClass()
{
    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = mainWindowProc;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = BACKGROUND_BRUSH;
    wcex.lpszClassName = mainWindowClassName.c_str();
    mainWindowAtom = RegisterClassExW(&wcex);

    return (mainWindowAtom != static_cast<ATOM>(0));
}

bool AcrylicApplicationPrivate::createMainWindow() const
{
    if (mainWindowAtom == static_cast<ATOM>(0)) {
        return false;
    }

    const DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    const DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;
    const int x = CW_USEDEFAULT;
    const int y = CW_USEDEFAULT;
    const int width = CW_USEDEFAULT;
    const int height = CW_USEDEFAULT;
    mainWindowHandle = CreateWindowExW(exStyle, mainWindowClassName.c_str(), mainWindowTitle.c_str(),
                             style, x, y, width, height, nullptr, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!mainWindowHandle) {
        return false;
    }

    ShowWindow(mainWindowHandle, SW_SHOW);
    UpdateWindow(mainWindowHandle);

    updateFrameMargins(mainWindowHandle);
    triggerFrameChange(mainWindowHandle);

    return true;
}

bool AcrylicApplicationPrivate::createXAMLIsland() const
{
    if (!mainWindowHandle) {
        return false;
    }

    // XAML Island section:
    // The call to winrt::init_apartment initializes COM; by default, in a multithreaded apartment.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // Initialize the XAML framework's core window for the current thread.
    // We need this manager live through out the whole application's life-cycle,
    // don't remove it, otherwise the application will crash.
    static const auto manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource source = {};
    // Get handle to the core window.
    const auto interop = source.as<IDesktopWindowXamlSourceNative>();
    // Parent the DesktopWindowXamlSource object to the current window.
    winrt::check_hresult(interop->AttachToWindow(mainWindowHandle));
    // Get the new child window's HWND.
    interop->get_WindowHandle(&xamlIslandHandle);
    if (!xamlIslandHandle) {
        print(MessageType::Error, L"Error", L"Failed to retrieve XAML Island window handle.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    GetClientRect(mainWindowHandle, &rect);
    SetWindowPos(xamlIslandHandle, nullptr, 0, 0, rect.right, rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    // Create the XAML content.
    static winrt::Windows::UI::Xaml::Controls::Grid grid = {};
    static winrt::Windows::UI::Xaml::Media::AcrylicBrush brush = {};
    brush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    grid.Background(brush);
    //grid.Children().Clear();
    //grid.Children().Append(/* some UWP control */);
    //grid.UpdateLayout();
    source.Content(grid);
    // End XAML Island section.

    ShowWindow(xamlIslandHandle, SW_SHOW);
    UpdateWindow(xamlIslandHandle);

    return true;
}

void AcrylicApplicationPrivate::initialize()
{
    static bool inited = false;
    if (inited) {
        return;
    }
    inited = true;

    if (!registerMainWindowClass()) {
        print(MessageType::Error, L"Error", L"Failed to register main window class.");
        return;
    }
    if (!createMainWindow()) {
        print(MessageType::Error, L"Error", L"Failed to create main window.");
        return;
    }
    mainWindowDpi = getWindowDpi(mainWindowHandle);
    if (IsWindows10OrGreater()) {
        if (isWindows10_19H1OrGreater()) {
            if (!createXAMLIsland()) {
                print(MessageType::Error, L"Error", L"Failed to create XAML Island.");
            }
        } else {
            print(MessageType::Error, L"Error", L"XAML Island applications are only supported from Windows 10 19H1.");
        }
    } else {
        print(MessageType::Warning, L"Warning", L"This application only supports Windows 10 and onwards.");
    }
}

AcrylicApplication::AcrylicApplication(const int argc, wchar_t *argv[])
{
    std::vector<std::wstring> args = {};
    for (int i = 0; i != argc; ++i) {
        args.push_back(argv[i]);
    }
    d = std::make_unique<AcrylicApplicationPrivate>(args, this);
}

AcrylicApplication::~AcrylicApplication() = default;

int AcrylicApplication::exec()
{
    return AcrylicApplicationPrivate::exec();
}
