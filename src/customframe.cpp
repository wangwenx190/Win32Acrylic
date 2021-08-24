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
#include "utils.h"
#include <ShellApi.h>
#include <UxTheme.h>
#include <cmath>

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

// The thickness of an auto-hide taskbar in pixels.
static constexpr int g_autoHideTaskbarThickness = 2;

bool CustomFrame::HandleWindowProc(const MSG *msg, LRESULT *result)
{
    if (!msg || !result) {
        return false;
    }
    switch (msg->message)
    {
    case WM_NCCALCSIZE: {
        if (!msg->wParam) {
            *result = 0;
            return true;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(msg->lParam)->rgrc[0]);
        if (Utils::IsWindows10OrGreater()) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
            if (ret != 0) {
                *result = ret;
                return true;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        }
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        bool nonClientAreaExists = false;
        const bool max = IsMaximized(msg->hwnd);
        const bool full = IsFullScreened(msg->hwnd);
        if (max && !full) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            const int resizeBorderThickness = Utils::GetResizeBorderThickness(msg->hwnd);
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
                    const RECT screenRect = GET_SCREEN_GEOMETRY(msg->hwnd);
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
                        const HMONITOR windowMonitor = GET_CURRENT_SCREEN(msg->hwnd);
                        if (!windowMonitor) {
                            PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow)
                            break;
                        }
                        const HMONITOR taskbarMonitor = GET_PRIMARY_SCREEN(_abd.hWnd);
                        if (!taskbarMonitor) {
                            PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow)
                            break;
                        }
                        if (taskbarMonitor == windowMonitor) {
                            SHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
                            edge = _abd.uEdge;
                        }
                    } else {
                        PRINT_WIN32_ERROR_MESSAGE(FindWindowW)
                        break;
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
        *result = (nonClientAreaExists ? 0 : WVR_REDRAW);
        return true;
    } break;
    case WM_NCHITTEST: {
        const POINT globalPos = {GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
        POINT localPos = globalPos;
        if (ScreenToClient(msg->hwnd, &localPos) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(ScreenToClient)
            break;
        }
        const bool max = IsMaximized(msg->hwnd);
        const bool full = IsFullScreened(msg->hwnd);
        const bool normal = IsWindowNoState(msg->hwnd);
        const SIZE windowSize = GET_WINDOW_SIZE(msg->hwnd);
        const LONG windowWidth = windowSize.cx;
        const int resizeBorderThickness = Utils::GetResizeBorderThickness(msg->hwnd);
        const int captionHeight = Utils::GetCaptionHeight(msg->hwnd);
        bool isTitleBar = false;
        if (max || full) {
            isTitleBar = ((localPos.y >= 0) && (localPos.y <= captionHeight)
                          && (localPos.x >= 0) && (localPos.x <= windowWidth));
        } else if (normal) {
            isTitleBar = ((localPos.y > resizeBorderThickness)
                          && (localPos.y <= (resizeBorderThickness + captionHeight))
                          && (localPos.x > resizeBorderThickness)
                          && (localPos.x < (windowWidth - resizeBorderThickness)));
        }
        const bool isTop = (normal ? (localPos.y <= resizeBorderThickness) : false);
        if (Utils::IsWindows10OrGreater()) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcW(msg->hwnd, msg->message, msg->wParam, msg->lParam);
            if (originalRet != HTCLIENT) {
                *result = originalRet;
                return true;
            }
            // At this point, we know that the cursor is inside the client area
            // so it has to be either the little border at the top of our custom
            // title bar or the drag bar. Apparently, it must be the drag bar or
            // the little border at the top which the user can use to move or
            // resize the window.
            if (isTop) {
                *result = HTTOP;
                return true;
            }
            if (isTitleBar) {
                *result = HTCAPTION;
                return true;
            }
            *result = HTCLIENT;
            return true;
        } else {
            if (max || full) {
                if (isTitleBar) {
                    *result = HTCAPTION;
                    return true;
                }
                *result = HTCLIENT;
                return true;
            }
            const LONG windowHeight = windowSize.cy;
            const bool isBottom = (normal ? (localPos.y >= (windowHeight - resizeBorderThickness)) : false);
            // Make the border a little wider to let the user easy to resize on corners.
            const double factor = (normal ? ((isTop || isBottom) ? 2.0 : 1.0) : 0.0);
            const bool isLeft = (normal ? (localPos.x <= std::round(static_cast<double>(resizeBorderThickness) * factor)) : false);
            const bool isRight = (normal ? (localPos.x >= (windowWidth - std::round(static_cast<double>(resizeBorderThickness) * factor))) : false);
            if (isTop) {
                if (isLeft) {
                    *result = HTTOPLEFT;
                    return true;
                }
                if (isRight) {
                    *result = HTTOPRIGHT;
                    return true;
                }
                *result = HTTOP;
                return true;
            }
            if (isBottom) {
                if (isLeft) {
                    *result = HTBOTTOMLEFT;
                    return true;
                }
                if (isRight) {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }
                *result = HTBOTTOM;
                return true;
            }
            if (isLeft) {
                *result = HTLEFT;
                return true;
            }
            if (isRight) {
                *result = HTRIGHT;
                return true;
            }
            if (isTitleBar) {
                *result = HTCAPTION;
                return true;
            }
            *result = HTCLIENT;
            return true;
        }
        *result = HTNOWHERE;
        return true;
    } break;
    case WM_PAINT: {
        if (Utils::IsWindows10OrGreater()) {
            PAINTSTRUCT ps = {};
            const HDC hdc = BeginPaint(msg->hwnd, &ps);
            if (!hdc) {
                PRINT_WIN32_ERROR_MESSAGE(BeginPaint)
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
            const LONG borderThickness = Utils::GetWindowVisibleFrameBorderThickness(msg->hwnd);
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
                    PRINT_WIN32_ERROR_MESSAGE(BeginBufferedPaint)
                    break;
                }
                if (FillRect(opaqueDc, &rcPaint,
                             reinterpret_cast<HBRUSH>(GetClassLongPtrW(msg->hwnd, GCLP_HBRBACKGROUND))) == 0) {
                    PRINT_WIN32_ERROR_MESSAGE(FillRect)
                    break;
                }
                HRESULT hr = BufferedPaintSetAlpha(buf, nullptr, 255);
                if (FAILED(hr)) {
                    PRINT_HR_ERROR_MESSAGE(BufferedPaintSetAlpha, hr)
                    break;
                }
                hr = EndBufferedPaint(buf, TRUE);
                if (FAILED(hr)) {
                    PRINT_HR_ERROR_MESSAGE(EndBufferedPaint, hr)
                    break;
                }
            }
            if (EndPaint(msg->hwnd, &ps) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(EndPaint)
                break;
            }
            *result = 0;
            return true;
        }
    } break;
    case WM_NCRBUTTONUP: {
        // The `DefWindowProc` function doesn't open the system menu for some
        // reason so we have to do it ourselves.
        if (msg->wParam == HTCAPTION) {
            if (!Utils::OpenSystemMenu(msg->hwnd, {GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)})) {
                break;
            }
            *result = 0;
            return true;
        }
    } break;
    default:
        break;
    }
    return false;
}
