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

#ifndef _DWMAPI_
#define _DWMAPI_
#endif

#include <Unknwn.h> // This header file must be placed before any other header files.
#include <Windows.h>
#include <ShellAPI.h>
#include <VersionHelpers.h>
#include <UxTheme.h>
#include <DWMAPI.h>
#include <WinRT/Windows.UI.Xaml.Hosting.h>
#include <WinRT/Windows.UI.Xaml.Controls.h>
#include <WinRT/Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI 96
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER 92
#endif

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX 0x0000000b
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

#ifndef IsMaximized
#define IsMaximized(window) IsZoomed(window)
#endif

#ifndef IsMinimized
#define IsMinimized(window) IsIconic(window)
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#ifndef RECT_WIDTH
#define RECT_WIDTH(rect) std::abs((rect).right - (rect).left)
#endif

#ifndef RECT_HEIGHT
#define RECT_HEIGHT(rect) std::abs((rect).bottom - (rect).top)
#endif

static LPCWSTR g_windowClassName = L"Win32AcrylicApplicationWindowClass";
static LPCWSTR g_windowTitle = L"Win32 C++ Acrylic Application";

static HINSTANCE g_instance = nullptr;
static HWND g_mainWindowHandle = nullptr;
static HWND g_xamlIslandHandle = nullptr;
static UINT g_currentDPI = USER_DEFAULT_SCREEN_DPI;

// The thickness of an auto-hide taskbar in pixels.
static const int kAutoHideTaskbarThicknessPx = 2;
static const int kAutoHideTaskbarThicknessPy = kAutoHideTaskbarThicknessPx;

static inline void DisplayErrorMessage(LPCWSTR text)
{
    if (!text) {
        return;
    }
    OutputDebugStringW(text);
    MessageBoxW(nullptr, text, L"Error", MB_OK | MB_ICONERROR);
}

[[nodiscard]] static inline int GetResizeBorderThickness(const bool x = true)
{
    // There is no "SM_CYPADDEDBORDER".
    const int result = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, g_currentDPI)
            + GetSystemMetricsForDpi((x ? SM_CXSIZEFRAME : SM_CYSIZEFRAME), g_currentDPI);
    const int preset = std::round(8.0 * static_cast<double>(g_currentDPI) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
    return ((result > 0) ? result : preset);
}

[[nodiscard]] static inline int GetCaptionHeight()
{
    const int result = GetSystemMetricsForDpi(SM_CYCAPTION, g_currentDPI);
    const int preset = std::round(23.0 * static_cast<double>(g_currentDPI) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
    return ((result > 0) ? result : preset);
}

[[nodiscard]] static inline bool IsFullScreened(const HWND hWnd)
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

[[nodiscard]] static inline bool IsNormaled(const HWND hWnd)
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

static inline void TriggerFrameChange(const HWND hWnd)
{
    if (!hWnd) {
        return;
    }
    SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

static inline void UpdateFrameMargins(const HWND hWnd)
{
    if (!hWnd) {
        return;
    }
    MARGINS margins = {0, 0, 0, 0};
    if (IsNormaled(hWnd)) {
        RECT frame = {0, 0, 0, 0};
        AdjustWindowRectExForDpi(&frame,
                                 (static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE)) & ~WS_OVERLAPPED),
                                 FALSE,
                                 static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_EXSTYLE)),
                                 g_currentDPI);
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

static inline LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
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
        const LRESULT ret = DefWindowProcW(hWnd, message, wParam, lParam);
        if (ret != 0) {
            return ret;
        }
        // Re-apply the original top from before the size of the default frame was applied.
        clientRect->top = originalTop;
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        if (IsMaximized(hWnd) && !IsFullScreened(hWnd)) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            clientRect->top += GetResizeBorderThickness(false);
            nonClientAreaExists = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (IsMaximized(hWnd) || IsFullScreened(hWnd)) {
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
        const LRESULT originalRet = DefWindowProcW(hWnd, message, wParam, lParam);
        if (originalRet != HTCLIENT) {
            return originalRet;
        }
        POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        ScreenToClient(hWnd, &pos);
        const int rbtY = GetResizeBorderThickness(false);
        // At this point, we know that the cursor is inside the client area
        // so it has to be either the little border at the top of our custom
        // title bar or the drag bar. Apparently, it must be the drag bar or
        // the little border at the top which the user can use to move or
        // resize the window.
        if (!IsMaximized(hWnd) && (pos.y <= rbtY)) {
            return HTTOP;
        }
        if ((pos.y > rbtY) && (pos.y <= (rbtY + GetCaptionHeight()))) {
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
        if (IsNormaled(hWnd)) {
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
            FillRect(hdc, &rcTopBorder, reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
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
#if 0
    case WM_ERASEBKGND:
        return 1;
#endif
    case WM_DPICHANGED: {
        const double x = LOWORD(wParam);
        const double y = HIWORD(wParam);
        g_currentDPI = std::round((x + y) / 2.0);
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        SetWindowPos(hWnd, nullptr, prcNewWindow->left, prcNewWindow->top,
                     RECT_WIDTH(*prcNewWindow), RECT_HEIGHT(*prcNewWindow),
                     SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        return 0;
    }
    case WM_SIZE: {
        if (g_xamlIslandHandle) {
            RECT rect = {0, 0, 0, 0};
            GetClientRect(hWnd, &rect);
            SetWindowPos(g_xamlIslandHandle, nullptr, 0, 0, rect.right, rect.bottom,
                         SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    } break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

EXTERN_C int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (!IsWindows10OrGreater()) {
        DisplayErrorMessage(L"This application only supports Windows 10 and onwards.");
        return -1;
    }

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wcex.lpszClassName = g_windowClassName;

    if (!RegisterClassExW(&wcex)) {
        DisplayErrorMessage(L"Failed to register the window class.");
        return -1;
    }

    const HWND mainWindowHandle = CreateWindowExW(
        0L, g_windowClassName, g_windowTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr
    );
    if (!mainWindowHandle) {
        DisplayErrorMessage(L"Failed to create the main window.");
        return -1;
    }

    g_instance = hInstance;
    g_mainWindowHandle = mainWindowHandle;

    // XAML Island section:
    // The call to winrt::init_apartment initializes COM; by default, in a multithreaded apartment.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // Initialize the XAML framework's core window for the current thread.
    // We need this "windowsXamlManager" live through out the whole application's life-cycle,
    // don't remove it, otherwise the application will crash.
    const auto windowsXamlManager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource desktopWindowXamlSource = {};
    // Get handle to the core window.
    const auto interop = desktopWindowXamlSource.as<IDesktopWindowXamlSourceNative>();
    // Parent the DesktopWindowXamlSource object to the current window.
    winrt::check_hresult(interop->AttachToWindow(mainWindowHandle));
    // Get the new child window's HWND.
    HWND xamlIslandHandle = nullptr;
    interop->get_WindowHandle(&xamlIslandHandle);
    if (!xamlIslandHandle) {
        DisplayErrorMessage(L"Failed to retrieve the XAML Island window handle.");
        return -1;
    }
    g_xamlIslandHandle = xamlIslandHandle;
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    GetClientRect(mainWindowHandle, &rect);
    SetWindowPos(xamlIslandHandle, nullptr, 0, 0, rect.right, rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    // Create the XAML content.
    winrt::Windows::UI::Xaml::Controls::Grid xamlGrid = {};
    winrt::Windows::UI::Xaml::Media::AcrylicBrush acrylicBrush = {};
    acrylicBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    xamlGrid.Background(acrylicBrush);
    //xamlGrid.Children().Append(/* some UWP control */);
    //xamlGrid.UpdateLayout();
    desktopWindowXamlSource.Content(xamlGrid);
    // End XAML Island section.

    ShowWindow(xamlIslandHandle, nCmdShow);
    UpdateWindow(xamlIslandHandle);

    // Message loop:
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
