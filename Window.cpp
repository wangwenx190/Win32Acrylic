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

#include "Window.h"
#include "SystemLibraryManager.h"
#include "Utils.h"
#include "Resource.h"
#include <ShellApi.h>
#include <cmath>

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED (0x02E0)
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam) (static_cast<int>(static_cast<short>(LOWORD(lParam))))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam) (static_cast<int>(static_cast<short>(HIWORD(lParam))))
#endif

#ifndef RECT_WIDTH
#define RECT_WIDTH(rect) (std::abs((rect).right - (rect).left))
#endif

#ifndef RECT_HEIGHT
#define RECT_HEIGHT(rect) (std::abs((rect).bottom - (rect).top))
#endif

#ifndef ATOM_TO_STRING
#define ATOM_TO_STRING(atom) (reinterpret_cast<LPCWSTR>(static_cast<WORD>(MAKELONG(atom, 0))))
#endif

// The thickness of an auto-hide taskbar in pixels.
static constexpr UINT g_autoHideTaskbarThickness = 2;

[[nodiscard]] static inline ATOM __RegisterWindowClass(const WNDPROC WndProc) noexcept
{
    USER32_API(LoadCursorW);
    USER32_API(LoadIconW);
    USER32_API(RegisterClassExW);
    if (LoadCursorWFunc && LoadIconWFunc && RegisterClassExWFunc) {
        if (!WndProc) {
            Utils::DisplayErrorDialog(L"Failed to register a window class due to the WindowProc function pointer is null.");
            return INVALID_ATOM;
        }
        const std::wstring guid = Utils::GenerateGUID();
        if (guid.empty()) {
            Utils::DisplayErrorDialog(L"Failed to generate a new GUID.");
            return INVALID_ATOM;
        }
        const HINSTANCE instance = Utils::GetCurrentModuleInstance();
        if (!instance) {
            Utils::DisplayErrorDialog(L"Failed to retrieve the current module instance.");
            return INVALID_ATOM;
        }
        WNDCLASSEXW wcex;
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = instance;
        wcex.lpszClassName = guid.c_str();
        wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
        wcex.hIcon = LoadIconWFunc(instance, MAKEINTRESOURCEW(IDI_APPICON));
        wcex.hIconSm = LoadIconWFunc(instance, MAKEINTRESOURCEW(IDI_APPICON_SMALL));
        const ATOM atom = RegisterClassExWFunc(&wcex);
        if (atom == INVALID_ATOM) {
            PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW, L"Failed to register a window class.")
        }
        return atom;
    } else {
        Utils::DisplayErrorDialog(L"Failed to register a window class due to LoadCursorW(), LoadIconW() and RegisterClassExW() are not available.");
        return INVALID_ATOM;
    }
}

[[nodiscard]] static inline HWND __CreateWindow(const std::wstring &className, void *extraData, const DWORD style, const DWORD extendedStyle, const HWND parentWindow) noexcept
{
    USER32_API(CreateWindowExW);
    if (CreateWindowExWFunc) {
        if (className.empty()) {
            Utils::DisplayErrorDialog(L"Failed to create a window due to the class name is empty.");
            return nullptr;
        }
        const HINSTANCE instance = Utils::GetCurrentModuleInstance();
        if (!instance) {
            Utils::DisplayErrorDialog(L"Failed to retrieve the current module instance.");
            return nullptr;
        }
        const HWND hWnd = CreateWindowExWFunc(
            extendedStyle,     // _In_     DWORD     dwExStyle
            className.c_str(), // _In_opt_ LPCWSTR   lpClassName
            nullptr,           // _In_opt_ LPCWSTR   lpWindowName
            style,             // _In_     DWORD     dwStyle
            CW_USEDEFAULT,     // _In_     int       X
            CW_USEDEFAULT,     // _In_     int       Y
            CW_USEDEFAULT,     // _In_     int       nWidth
            CW_USEDEFAULT,     // _In_     int       nHeight
            parentWindow,      // _In_opt_ HWND      hWndParent
            nullptr,           // _In_opt_ HMENU     hMenu
            instance,          // _In_opt_ HINSTANCE hInstance
            extraData          // _In_opt_ LPVOID    lpParam
        );
        if (!hWnd) {
            PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW, L"Failed to create a window.")
        }
        return hWnd;
    } else {
        Utils::DisplayErrorDialog(L"Failed to create a window due to CreateWindowExW() is not available.");
        return nullptr;
    }
}

[[nodiscard]] static inline bool __CloseWindow(const HWND hWnd, const std::wstring &className) noexcept
{
    USER32_API(DestroyWindow);
    USER32_API(UnregisterClassW);
    if (DestroyWindowFunc && UnregisterClassWFunc) {
        if (!hWnd) {
            Utils::DisplayErrorDialog(L"Failed to destroy the window due to the window handle is null.");
            return false;
        }
        if (className.empty()) {
            Utils::DisplayErrorDialog(L"Failed to destroy the window due to the class name is empty.");
            return false;
        }
        const HINSTANCE instance = Utils::GetCurrentModuleInstance();
        if (!instance) {
            Utils::DisplayErrorDialog(L"Failed to retrieve the current module instance.");
            return false;
        }
        if (DestroyWindowFunc(hWnd) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow, L"Failed to destroy the window.")
            return false;
        }
        if (UnregisterClassWFunc(className.c_str(), instance) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW, L"Failed to unregister the window class.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to destroy the window due to DestroyWindow() and UnregisterClassW() are not available.");
        return false;
    }
}

class WindowPrivate
{
public:
    explicit WindowPrivate(Window *q) noexcept;
    ~WindowPrivate() noexcept;

    [[nodiscard]] std::wstring Title() const noexcept;
    void Title(const std::wstring &value) noexcept;

    [[nodiscard]] int Icon() const noexcept;
    void Icon(const int value) noexcept;

    [[nodiscard]] int X() const noexcept;
    void X(const int value) noexcept;

    [[nodiscard]] int Y() const noexcept;
    void Y(const int value) noexcept;

    [[nodiscard]] UINT Width() const noexcept;
    void Width(const UINT value) noexcept;

    [[nodiscard]] UINT Height() const noexcept;
    void Height(const UINT value) noexcept;

    [[nodiscard]] WindowState Visibility() const noexcept;
    void Visibility(const WindowState value) noexcept;

    [[nodiscard]] WindowTheme Theme() const noexcept;
    void Theme(const WindowTheme value) noexcept;

    [[nodiscard]] UINT DotsPerInch() const noexcept;

    [[nodiscard]] HWND CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC WndProc, void *extraData) const noexcept;
    [[nodiscard]] HWND WindowHandle() const noexcept;
    [[nodiscard]] int MessageLoop() const noexcept;
    [[nodiscard]] bool Move(const int x, const int y) noexcept;
    [[nodiscard]] bool Resize(const UINT w, const UINT h) noexcept;
    [[nodiscard]] bool SetGeometry(const int x, const int y, const UINT w, const UINT h) noexcept;

private:
    WindowPrivate(const WindowPrivate &) = delete;
    WindowPrivate &operator=(const WindowPrivate &) = delete;
    WindowPrivate(WindowPrivate &&) = delete;
    WindowPrivate &operator=(WindowPrivate &&) = delete;

private:
    [[nodiscard]] static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    [[nodiscard]] LRESULT DefaultMessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept;

private:
    Window *q_ptr = nullptr;
    HWND m_window = nullptr;
    ATOM m_atom = INVALID_ATOM;
    int m_icon = 0;
    std::wstring m_title = {};
    int m_x = 0;
    int m_y = 0;
    UINT m_width = 0;
    UINT m_height = 0;
    WindowState m_visibility = WindowState::Normal;
    WindowTheme m_theme = WindowTheme::Light;
    UINT m_dpi = 0;
};

WindowPrivate::WindowPrivate(Window *q) noexcept
{
    q_ptr = q;
    m_atom = __RegisterWindowClass(WindowProc);
    if (m_atom == INVALID_ATOM) {
        Utils::DisplayErrorDialog(L"Failed to register the window class for this window.");
        return false;
    }
    m_window = __CreateWindow(ATOM_TO_STRING(m_atom), this, (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS), WS_EX_NOREDIRECTIONBITMAP, nullptr);
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to create this window.");
        return false;
    }
    return true;
}

WindowPrivate::~WindowPrivate() noexcept
{
    if (m_window && (m_atom != INVALID_ATOM)) {
        if (__CloseWindow(m_window, ATOM_TO_STRING(m_atom))) {
            m_window = nullptr;
            m_atom = INVALID_ATOM;
        } else {
            Utils::DisplayErrorDialog(L"Failed to destroy this window.");
        }
    }
}

std::wstring WindowPrivate::Title() const noexcept
{
    return m_title;
}

void WindowPrivate::Title(const std::wstring &value) noexcept
{

}

int WindowPrivate::Icon() const noexcept
{
    return m_icon;
}

void WindowPrivate::Icon(const int value) noexcept
{

}

int WindowPrivate::X() const noexcept
{
    return m_x;
}

void WindowPrivate::X(const int value) noexcept
{

}

int WindowPrivate::Y() const noexcept
{
    return m_y;
}

void WindowPrivate::Y(const int value) noexcept
{

}

UINT WindowPrivate::Width() const noexcept
{
    return m_width;
}

void WindowPrivate::Width(const UINT value) noexcept
{

}

UINT WindowPrivate::Height() const noexcept
{
    return m_height;
}

void WindowPrivate::Height(const UINT value) noexcept
{

}

WindowState WindowPrivate::Visibility() const noexcept
{
    return m_visibility;
}

void WindowPrivate::Visibility(const WindowState value) noexcept
{

}

WindowTheme WindowPrivate::Theme() const noexcept
{
    return m_theme;
}

void WindowPrivate::Theme(const WindowTheme value) noexcept
{

}

UINT WindowPrivate::DotsPerInch() const noexcept
{
    return m_dpi;
}

HWND WindowPrivate::WindowHandle() const noexcept
{
    return m_window;
}

int WindowPrivate::MessageLoop() const noexcept
{
    USER32_API(GetMessageW);
    USER32_API(TranslateMessage);
    USER32_API(DispatchMessageW);
    if (GetMessageWFunc && TranslateMessageFunc && DispatchMessageWFunc) {
        MSG msg = {};
        while (GetMessageWFunc(&msg, nullptr, 0, 0) != FALSE) {
            if (!FilterMessage(&msg)) {
                TranslateMessageFunc(&msg);
                DispatchMessageWFunc(&msg);
            }
        }
        return static_cast<int>(msg.wParam);
    } else {
        Utils::DisplayErrorDialog(L"Failed to execute the message loop due to GetMessageW(), TranslateMessage() and DispatchMessageW() are not available.");
        return -1;
    }
}

bool WindowPrivate::Move(const int x, const int y) noexcept
{
    return SetGeometry(x, y, m_width, m_height);
}

bool WindowPrivate::Resize(const UINT w, const UINT h) noexcept
{
    return SetGeometry(m_x, m_y, w, h);
}

bool WindowPrivate::SetGeometry(const int x, const int y, const UINT w, const UINT h) noexcept
{
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        constexpr UINT flags = (SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER);
        if (SetWindowPosFunc(m_window, nullptr, x, y, w, h, flags) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to change the window geometry.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"SetWindowPos() is not available.");
        return false;
    }
}

LRESULT CALLBACK WindowPrivate::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    USER32_API(SetWindowLongPtrW);
    USER32_API(GetWindowLongPtrW);
    USER32_API(DefWindowProcW);
    if (SetWindowLongPtrWFunc && GetWindowLongPtrWFunc && DefWindowProcWFunc) {
        if (message == WM_NCCREATE) {
            const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            const auto that = static_cast<Window *>(cs->lpCreateParams);
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to set the window extra data.")
            }
        } else if (message == WM_NCDESTROY) {
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, 0) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to clear the window extra data.")
            }
        }
        const auto that = reinterpret_cast<Window *>(GetWindowLongPtrWFunc(hWnd, GWLP_USERDATA));
        if (that) {
            const LRESULT result = that->DefaultMessageHandler(message, wParam, lParam);
            return ((result == 0) ? that->MessageHandler(message, wParam, lParam) : result);
        } else {
            return DefWindowProcWFunc(hWnd, message, wParam, lParam);
        }
    } else {
        Utils::DisplayErrorDialog(L"SetWindowLongPtrW(), GetWindowLongPtrW() and DefWindowProcW() are not available.");
        return 0;
    }
}

LRESULT WindowPrivate::DefaultMessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (message) {
    case WM_CREATE: {
        m_dpi = Utils::GetWindowMetrics(m_window, WindowMetrics::DotsPerInch);
        OnDPIChanged(m_dpi);
        const std::wstring dpiMsg = L"Current window's dots-per-inch (DPI): " + Utils::IntegerToString(m_dpi, 10);
        OutputDebugStringW(dpiMsg.c_str());
        if (!Utils::UpdateFrameMargins(m_window)) {
            Utils::DisplayErrorDialog(L"Failed to update the window frame margins.");
        }
        USER32_API(SetWindowPos);
        if (SetWindowPosFunc) {
            constexpr UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
            if (SetWindowPosFunc(m_window, nullptr, 0, 0, 0, 0, flags) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to trigger a frame change event for the window.")
            }
        } else {
            Utils::DisplayErrorDialog(L"SetWindowPos() is not available.");
        }
        m_theme = Utils::GetSystemTheme();
        OnThemeChanged(m_theme);
        std::wstring themeName = L"Unknown";
        switch (m_theme) {
        case WindowTheme::Light: {
            themeName = L"Light";
        } break;
        case WindowTheme::Dark: {
            themeName = L"Dark";
        } break;
        case WindowTheme::HighContrast: {
            themeName = L"High Contrast";
        } break;
        }
        const std::wstring themeMsg = L"Current window's theme: " + themeName;
        OutputDebugStringW(themeMsg.c_str());
        if (!Utils::SetWindowTheme(m_window, m_theme)) {
            Utils::DisplayErrorDialog(L"Failed to set the window theme.");
        }
        const auto cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        m_x = cs->x;
        OnXChanged(m_x);
        m_y = cs->y;
        OnYChanged(m_y);
        m_width = cs->cx;
        OnWidthChanged(m_width);
        m_height = cs->cy;
        OnHeightChanged(m_height);
    } break;
    case WM_MOVE: {
        m_x = GET_X_LPARAM(lParam);
        OnXChanged(m_x);
        m_y = GET_Y_LPARAM(lParam);
        OnYChanged(m_y);
    } break;
    case WM_SIZE: {
        if (wParam == SIZE_RESTORED) {
            if ((m_visibility == WindowState::Minimized) || (m_visibility == WindowState::Maximized)) {
                m_visibility = WindowState::Normal;
                OnVisibilityChanged(m_visibility);
            }
        } else if (wParam == SIZE_MINIMIZED) {
            m_visibility = WindowState::Minimized;
            OnVisibilityChanged(m_visibility);
        } else if (wParam == SIZE_MAXIMIZED) {
            m_visibility = WindowState::Maximized;
            OnVisibilityChanged(m_visibility);
        }
        m_width = LOWORD(lParam);
        OnWidthChanged(m_width);
        m_height = HIWORD(lParam);
        OnHeightChanged(m_height);
    } break;
    case WM_SETTINGCHANGE: {
        // wParam == 0: User-wide setting change
        // wParam == 1: System-wide setting change
        // ### TODO: how to detect high contrast theme here
        if (((wParam == 0) || (wParam == 1)) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            m_theme = Utils::GetSystemTheme();
            OnThemeChanged(m_theme);
            if (!Utils::SetWindowTheme(m_window, m_theme)) {
                Utils::DisplayErrorDialog(L"Failed to set the window theme.");
            }
        }
    } break;
    case WM_DPICHANGED: {
        const UINT oldDPI = m_dpi;
        const UINT dpiX = LOWORD(wParam);
        const UINT dpiY = HIWORD(wParam);
        m_dpi = static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
        OnDotsPerInchChanged(m_dpi);
        const std::wstring dpiMsg = L"Current window's dots-per-inch (DPI) has changed from " + Utils::IntegerToString(oldDPI, 10) + L" to " + Utils::IntegerToString(m_dpi, 10) + L".";
        OutputDebugStringW(dpiMsg.c_str());
        USER32_API(SetWindowPos);
        if (SetWindowPosFunc) {
            const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
            constexpr UINT flags = (SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER);
            if (SetWindowPosFunc(m_window, nullptr, prcNewWindow->left, prcNewWindow->top, RECT_WIDTH(*prcNewWindow), RECT_HEIGHT(*prcNewWindow), flags) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to update the geometry of the main window.")
            } else {
                return 0;
            }
        } else {
            Utils::DisplayErrorDialog(L"SetWindowPos() is not available.");
        }
    } break;
    case WM_PAINT: {
        return 0;
    } break;
    case WM_ERASEBKGND: {
        return 1;
    } break;
    case WM_CLOSE: {} break;
    case WM_DESTROY: {} break;
    case WM_NCCREATE: {
        USER32_API(EnableNonClientDpiScaling);
        if (EnableNonClientDpiScalingFunc) {
            if (EnableNonClientDpiScalingFunc(m_window) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(EnableNonClientDpiScaling, L"Failed to enable non-client area DPI auto scaling.")
            }
        } else {
            Utils::DisplayErrorDialog(L"Failed to enable non-client area DPI auto scaling due to EnableNonClientDpiScaling() is not available.");
        }
    } break;
    case WM_NCCALCSIZE: {
        if (static_cast<BOOL>(wParam) == FALSE) {
            return 0;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        USER32_API(DefWindowProcW);
        if (DefWindowProcWFunc) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcWFunc(m_window, WM_NCCALCSIZE, TRUE, lParam);
            if (ret != 0) {
                return ret;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        } else {
            Utils::DisplayErrorDialog(L"DefWindowProcW() is not available.");
            return WVR_REDRAW;
        }
        bool nonClientAreaExists = false;
        const bool max = (m_visibility == WindowState::Maximized);
        const bool full = Utils::IsWindowFullScreen(m_window);
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        if (max && !full) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            const UINT resizeBorderThicknessY = Utils::GetWindowMetrics(m_window, WindowMetrics::ResizeBorderThicknessY);
            clientRect->top += resizeBorderThicknessY;
            nonClientAreaExists = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        SHELL32_API(SHAppBarMessage);
        if (SHAppBarMessageFunc) {
            if (max || full) {
                APPBARDATA abd;
                SecureZeroMemory(&abd, sizeof(abd));
                abd.cbSize = sizeof(abd);
                // First, check if we have an auto-hide taskbar at all:
                if (SHAppBarMessageFunc(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) {
                    const RECT screenRect = Utils::GetScreenGeometry(m_window);
                    // This helper can be used to determine if there's a
                    // auto-hide taskbar on the given edge of the monitor
                    // we're currently on.
                    const auto hasAutohideTaskbar = [&screenRect](const UINT edge) -> bool {
                        APPBARDATA abd2;
                        SecureZeroMemory(&abd2, sizeof(abd2));
                        abd2.cbSize = sizeof(abd2);
                        abd2.uEdge = edge;
                        abd2.rc = screenRect;
                        return (reinterpret_cast<HWND>(SHAppBarMessageFunc(ABM_GETAUTOHIDEBAREX, &abd2)) != nullptr);
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
                        clientRect->top += g_autoHideTaskbarThickness;
                        nonClientAreaExists = true;
                    } else if (hasAutohideTaskbar(ABE_BOTTOM)) {
                        clientRect->bottom -= g_autoHideTaskbarThickness;
                        nonClientAreaExists = true;
                    } else if (hasAutohideTaskbar(ABE_LEFT)) {
                        clientRect->left += g_autoHideTaskbarThickness;
                        nonClientAreaExists = true;
                    } else if (hasAutohideTaskbar(ABE_RIGHT)) {
                        clientRect->right -= g_autoHideTaskbarThickness;
                        nonClientAreaExists = true;
                    }
                }
            }
        } else {
            Utils::DisplayErrorDialog(L"SHAppBarMessage() is not available.");
            return WVR_REDRAW;
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
    } break;
    case WM_NCHITTEST: {
        const POINT globalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT localPos = globalPos;
        USER32_API(ScreenToClient);
        if (ScreenToClientFunc) {
            if (ScreenToClientFunc(m_window, &localPos) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(ScreenToClient, L"Failed to translate from screen coordinate to window coordinate.")
            }
        } else {
            Utils::DisplayErrorDialog(L"ScreenToClient() is not available.");
            return HTCLIENT;
        }
        const auto resizeBorderThicknessY = static_cast<LONG>(Utils::GetWindowMetrics(m_window, WindowMetrics::ResizeBorderThicknessY));
        const auto titleBarHeight = static_cast<LONG>(Utils::GetWindowMetrics(m_window, WindowMetrics::TitleBarHeight));
        const bool isTitleBar = ((m_visibility != WindowState::Minimized) ? (localPos.y <= titleBarHeight) : false);
        const bool isTop = ((m_visibility == WindowState::Normal) ? (localPos.y <= resizeBorderThicknessY) : false);
        USER32_API(DefWindowProcW);
        if (DefWindowProcWFunc) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcWFunc(m_window, WM_NCHITTEST, 0, lParam);
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
            Utils::DisplayErrorDialog(L"DefWindowProcW() is not available.");
            return HTCLIENT;
        }
    } break;
    case WM_NCRBUTTONUP: {
        if (wParam == HTCAPTION) {
            const POINT mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (Utils::OpenSystemMenu(m_window, mousePos)) {
                return 0;
            } else {
                Utils::DisplayErrorDialog(L"Failed to open the system menu for the main window.");
            }
        }
    } break;
    default:
        break;
    }
    return 0;
}
