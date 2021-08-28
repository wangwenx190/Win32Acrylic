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

#include "customframe.h"
#include "resource.h"
#include "utils.h"
#include <ShellApi.h>
#include <UxTheme.h>
#include <cmath>

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

static constexpr wchar_t g_classNamePrefix[] = LR"(wangwenx190\AcrylicManager\WindowClasses\)";
static constexpr wchar_t g_windowTitle[] = L"AcrylicManager Background Window";

// The thickness of an auto-hide taskbar in pixels.
static constexpr int g_autoHideTaskbarThickness = 2;

CustomFrame::CustomFrame() = default;

CustomFrame::~CustomFrame() = default;

std::wstring CustomFrame::__RegisterWindowClass(const WNDPROC wndProc) noexcept
{
    if (!Utils::IsCompositionEnabled()) {
        OutputDebugStringW(L"DWM composition is disabled.");
        return nullptr;
    }
    if (!wndProc) {
        OutputDebugStringW(L"The given WindowProc function's address is null.");
        return nullptr;
    }
    const std::wstring className = g_classNamePrefix + Utils::GenerateGUID();
    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wndProc;
    wcex.hInstance = GET_CURRENT_INSTANCE;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIconW(GET_CURRENT_INSTANCE, MAKEINTRESOURCEW(IDI_DEFAULTICON));
    wcex.hIconSm = LoadIconW(GET_CURRENT_INSTANCE, MAKEINTRESOURCEW(IDI_DEFAULTICONSM));
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = className.c_str();
    if (RegisterClassExW(&wcex) == 0) {
        PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW)
        return nullptr;
    }
    return className;
}

HWND CustomFrame::__CreateWindow(const std::wstring &className, const DWORD style,
                                 const DWORD exStyle, const HWND parent,
                                 LPVOID data) noexcept
{
    if (!Utils::IsCompositionEnabled()) {
        OutputDebugStringW(L"DWM composition is disabled.");
        return nullptr;
    }
    if (className.empty()) {
        OutputDebugStringW(L"The given window class name is empty.");
        return nullptr;
    }

    const HWND window = CreateWindowExW(exStyle,
                                        className.c_str(),
                                        g_windowTitle,
                                        style,
                                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                        parent,
                                        nullptr,
                                        GET_CURRENT_INSTANCE,
                                        data);

    if (!window) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW)
        return nullptr;
    }
    return window;
}

std::wstring CustomFrame::__GetWindowClassName() const noexcept
{
    return m_windowClass;
}

void CustomFrame::__SetWindowClassName(const std::wstring &className) noexcept
{
    // todo: guard against empty string?
    m_windowClass = className;
}

void CustomFrame::__SetWindowHandle(const HWND hWnd) noexcept
{
    // todo: guard against null pointer?
    m_window = hWnd;
}

bool CustomFrame::FilterMessage(const MSG *msg) const noexcept
{
    return false;
}

int CustomFrame::MessageLoop() const noexcept
{
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        if (!FilterMessage(&msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}

HWND CustomFrame::GetHandle() const noexcept
{
    return m_window;
}

void CustomFrame::OnNCCreate(const HWND hWnd, const LPARAM lParam) noexcept
{
    const auto cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
    const auto that = static_cast<CustomFrame *>(cs->lpCreateParams);
    that->m_window = hWnd;
    if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
    }
}

void CustomFrame::OnNCDestroy(const HWND hWnd) noexcept
{
    if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0) == 0) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
    }
}

LRESULT CustomFrame::OnNCCalcSize(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept
{
    if (wParam == FALSE) {
        return 0;
    }
    const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
    if (Utils::IsWindows10OrGreater()) {
        // Store the original top before the default window proc applies the default frame.
        const LONG originalTop = clientRect->top;
        // Apply the default frame
        const LRESULT ret = DefWindowProcW(hWnd, WM_NCCALCSIZE, TRUE, lParam);
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
    const bool max = IsMaximized(hWnd);
    const bool full = IsFullScreened(hWnd);
    if (max && !full) {
        // When a window is maximized, its size is actually a little bit more
        // than the monitor's work area. The window is positioned and sized in
        // such a way that the resize handles are outside of the monitor and
        // then the window is clipped to the monitor so that the resize handle
        // do not appear because you don't need them (because you can't resize
        // a window when it's maximized unless you restore it).
        const int resizeBorderThickness = Utils::GetResizeBorderThickness(hWnd);
        clientRect->top += resizeBorderThickness;
        if (!Utils::IsWindows10OrGreater()) {
            clientRect->bottom -= resizeBorderThickness;
            clientRect->left += resizeBorderThickness;
            clientRect->right -= resizeBorderThickness;
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
            if (Utils::IsWindows8OrGreater()) {
                const RECT screenRect = GET_SCREEN_GEOMETRY(GET_CURRENT_SCREEN(hWnd));
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
                    const HMONITOR windowMonitor = GET_CURRENT_SCREEN(hWnd);
                    if (!windowMonitor) {
                        PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow)
                        return WVR_REDRAW;
                    }
                    const HMONITOR taskbarMonitor = GET_PRIMARY_SCREEN(_abd.hWnd);
                    if (!taskbarMonitor) {
                        PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow)
                        return WVR_REDRAW;
                    }
                    if (taskbarMonitor == windowMonitor) {
                        SHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
                        edge = _abd.uEdge;
                    }
                } else {
                    PRINT_WIN32_ERROR_MESSAGE(FindWindowW)
                    return WVR_REDRAW;
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
                clientRect->top += g_autoHideTaskbarThickness;
                nonClientAreaExists = true;
            } else if (bottom) {
                clientRect->bottom -= g_autoHideTaskbarThickness;
                nonClientAreaExists = true;
            } else if (left) {
                clientRect->left += g_autoHideTaskbarThickness;
                nonClientAreaExists = true;
            } else if (right) {
                clientRect->right -= g_autoHideTaskbarThickness;
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
    return (nonClientAreaExists ? 0 : WVR_REDRAW);
}

LRESULT CustomFrame::OnNCHitTest(const HWND hWnd, const LPARAM lParam) noexcept
{
    const POINT screenPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    POINT windowPos = screenPos;
    if (ScreenToClient(hWnd, &windowPos) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(ScreenToClient)
        return HTERROR;
    }
    const bool max = IsMaximized(hWnd);
    const bool full = IsFullScreened(hWnd);
    const bool normal = IsWindowNoState(hWnd);
    const SIZE windowSize = GET_WINDOW_SIZE(hWnd);
    const LONG windowWidth = windowSize.cx;
    const int resizeBorderThickness = Utils::GetResizeBorderThickness(hWnd);
    const int captionHeight = Utils::GetCaptionHeight(hWnd);
    bool isTitleBar = false;
    if (max || full) {
        isTitleBar = ((windowPos.y >= 0) && (windowPos.y <= captionHeight)
                      && (windowPos.x >= 0) && (windowPos.x <= windowWidth));
    } else if (normal) {
        isTitleBar = ((windowPos.y > resizeBorderThickness)
                      && (windowPos.y <= (resizeBorderThickness + captionHeight))
                      && (windowPos.x > resizeBorderThickness)
                      && (windowPos.x < (windowWidth - resizeBorderThickness)));
    }
    const bool isTop = (normal ? (windowPos.y <= resizeBorderThickness) : false);
    if (Utils::IsWindows10OrGreater()) {
        // This will handle the left, right and bottom parts of the frame
        // because we didn't change them.
        const LRESULT originalRet = DefWindowProcW(hWnd, WM_NCHITTEST, 0, lParam);
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
        if (max || full) {
            if (isTitleBar) {
                return HTCAPTION;
            }
            return HTCLIENT;
        }
        const LONG windowHeight = windowSize.cy;
        const bool isBottom = (normal ? (windowPos.y >= (windowHeight - resizeBorderThickness)) : false);
        // Make the border a little wider to let the user easy to resize on corners.
        const double factor = (normal ? ((isTop || isBottom) ? 2.0 : 1.0) : 0.0);
        const bool isLeft = (normal ? (windowPos.x <= std::round(static_cast<double>(resizeBorderThickness) * factor)) : false);
        const bool isRight = (normal ? (windowPos.x >= (windowWidth - std::round(static_cast<double>(resizeBorderThickness) * factor))) : false);
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

void CustomFrame::OnPaint(const HWND hWnd) noexcept
{
    if (Utils::IsWindows10OrGreater()) {
        PAINTSTRUCT ps = {};
        const HDC hdc = BeginPaint(hWnd, &ps);
        if (!hdc) {
            PRINT_WIN32_ERROR_MESSAGE(BeginPaint)
            return;
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
        const LONG borderThickness = Utils::GetWindowVisibleFrameBorderThickness(hWnd);
        if (ps.rcPaint.top < borderThickness) {
            RECT rcPaint = ps.rcPaint;
            rcPaint.bottom = borderThickness;
            // To show the original top border, we have to paint on top
            // of it with the alpha component set to 0. This page
            // recommends to paint the area in black using the stock
            // BLACK_BRUSH to do this:
            // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
            if (FillRect(hdc, &rcPaint, GET_BLACK_BRUSH) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(FillRect)
                return;
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
                PRINT_WIN32_ERROR_MESSAGE(BeginBufferedPaint)
                return;
            }
            if (FillRect(opaqueDc, &rcPaint, reinterpret_cast<HBRUSH>(GetClassLongPtrW(hWnd, GCLP_HBRBACKGROUND))) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(FillRect)
                return;
            }
            HRESULT hr = BufferedPaintSetAlpha(buf, nullptr, 255);
            if (FAILED(hr)) {
                PRINT_HR_ERROR_MESSAGE(BufferedPaintSetAlpha, hr)
                return;
            }
            hr = EndBufferedPaint(buf, TRUE);
            if (FAILED(hr)) {
                PRINT_HR_ERROR_MESSAGE(EndBufferedPaint, hr)
                return;
            }
        }
        if (EndPaint(hWnd, &ps) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(EndPaint)
            return;
        }
    }
}

void CustomFrame::OnNCRButtonUp(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept
{
    if (wParam == HTCAPTION) {
        if (!Utils::OpenSystemMenu(hWnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) {
            OutputDebugStringW(L"Failed to open system menu.");
        }
    }
}

void CustomFrame::OnCreate(const HWND hWnd, const LPARAM lParam, UINT *dpi) noexcept
{
    if (!Utils::SetDpiAwarenessForWindow(hWnd, DpiAwareness::PerMonitorV2)) {
        // Setting the DPI awareness for window may fail in many different reasons,
        // no need to print any debug messages for it.
        // PerMonitorV2 implies the non-client area dpi scaling so we only have to
        // enable it if we cannot enable PerMonitorV2.
        EnableNonClientDpiScaling(hWnd);
    }
    if (!Utils::UpdateFrameMargins(hWnd)) {
        OutputDebugStringW(L"Failed to update frame margins.");
    }
    constexpr UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, flags) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
    }
    if (dpi) {
        *dpi = Utils::GetDotsPerInchForWindow(hWnd);
    }
}

void CustomFrame::OnSize(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept
{
    if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED) || IsFullScreened(hWnd)) {
        if (!Utils::UpdateFrameMargins(hWnd)) {
            OutputDebugStringW(L"Failed to update frame margins.");
        }
    }
}

void CustomFrame::OnSettingChange(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept
{
    if (!Utils::IsWindows10OrGreater()) {
        return;
    }
    if ((wParam == 0) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
        if (Utils::IsWindows10RS1OrGreater() && !Utils::IsHighContrastModeEnabled()) {
            if (!Utils::SetWindowDarkFrameBorderEnabled(hWnd, Utils::ShouldAppsUseDarkMode())) {
                OutputDebugStringW(L"Failed to toggle the visibility of window's dark frame border.");
            }
        }
    }
}

void CustomFrame::OnDwmCompositionChanged() noexcept
{
    if (!Utils::IsCompositionEnabled()) {
        OutputDebugStringW(L"AcrylicManager won't be functional when DWM composition is disabled.");
        //std::exit(-1);
    }
}

void CustomFrame::OnDPIChanged(const HWND hWnd, const WPARAM wParam, const LPARAM lParam, UINT *newDpi) noexcept
{
    if (newDpi) {
        const auto x = static_cast<double>(LOWORD(wParam));
        const auto y = static_cast<double>(HIWORD(wParam));
        *newDpi = static_cast<int>(std::round((x + y) / 2.0));
    }
    const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
    if (MoveWindow(hWnd, prcNewWindow->left, prcNewWindow->top,
                   GET_RECT_WIDTH(*prcNewWindow), GET_RECT_HEIGHT(*prcNewWindow), TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(MoveWindow)
    }
}

void CustomFrame::OnClose(const HWND hWnd, const std::wstring &className) noexcept
{
    if (DestroyWindow(hWnd) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(DestroyWindow)
    }
    if (!className.empty()) {
        if (UnregisterClassW(className.c_str(), GET_CURRENT_INSTANCE) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW)
        }
    }
}

void CustomFrame::OnDestroy() noexcept
{
    PostQuitMessage(0);
}
