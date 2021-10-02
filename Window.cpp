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

[[nodiscard]] static inline std::tuple<HWND, ATOM> __CreateWindow(const DWORD style, const DWORD extendedStyle, const HWND parentWindow, void *extraData, const WNDPROC wndProc) noexcept
{
    constexpr std::tuple<HWND, ATOM> INVALID_RESULT = std::make_tuple(nullptr, INVALID_ATOM);
    USER32_API(LoadCursorW);
    USER32_API(LoadIconW);
    USER32_API(RegisterClassExW);
    USER32_API(CreateWindowExW);
    if (LoadCursorWFunc && LoadIconWFunc && RegisterClassExWFunc && CreateWindowExWFunc) {
        if (!wndProc) {
            Utils::DisplayErrorDialog(L"Failed to register a window class due to the WindowProc function pointer is null.");
            return INVALID_RESULT;
        }
        const HINSTANCE instance = Utils::GetCurrentModuleInstance();
        if (!instance) {
            Utils::DisplayErrorDialog(L"Failed to retrieve the current module instance.");
            return INVALID_RESULT;
        }
        const std::wstring guid = Utils::GenerateGUID();
        if (guid.empty()) {
            Utils::DisplayErrorDialog(L"Failed to generate a new GUID.");
            return INVALID_RESULT;
        }
        WNDCLASSEXW wcex;
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = wndProc;
        wcex.hInstance = instance;
        wcex.lpszClassName = guid.c_str();
        wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
        wcex.hIcon = LoadIconWFunc(instance, MAKEINTRESOURCEW(IDI_APPICON));
        wcex.hIconSm = LoadIconWFunc(instance, MAKEINTRESOURCEW(IDI_APPICON_SMALL));
        const ATOM atom = RegisterClassExWFunc(&wcex);
        if (atom == INVALID_ATOM) {
            PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW, L"Failed to register a window class.")
            return INVALID_RESULT;
        }
        const HWND hWnd = CreateWindowExWFunc(
            extendedStyle,        // _In_     DWORD     dwExStyle
            ATOM_TO_STRING(atom), // _In_opt_ LPCWSTR   lpClassName
            nullptr,              // _In_opt_ LPCWSTR   lpWindowName
            style,                // _In_     DWORD     dwStyle
            CW_USEDEFAULT,        // _In_     int       X
            CW_USEDEFAULT,        // _In_     int       Y
            CW_USEDEFAULT,        // _In_     int       nWidth
            CW_USEDEFAULT,        // _In_     int       nHeight
            parentWindow,         // _In_opt_ HWND      hWndParent
            nullptr,              // _In_opt_ HMENU     hMenu
            instance,             // _In_opt_ HINSTANCE hInstance
            extraData             // _In_opt_ LPVOID    lpParam
        );
        if (!hWnd) {
            PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW, L"Failed to create a window.")
            return INVALID_RESULT;
        }
        return std::make_tuple(hWnd, atom);
    } else {
        Utils::DisplayErrorDialog(L"Failed to create a window due to LoadCursorW(), LoadIconW(), RegisterClassExW() and CreateWindowExW() are not available.");
        return INVALID_RESULT;
    }
}

[[nodiscard]] static inline bool __CloseWindow(const HWND hWnd, const std::wstring &className) noexcept
{
    USER32_API(DestroyWindow);
    USER32_API(UnregisterClassW);
    if (DestroyWindowFunc && UnregisterClassWFunc) {
        if (!hWnd) {
            Utils::DisplayErrorDialog(L"Failed to close the window due to the window handle is null.");
            return false;
        }
        if (className.empty()) {
            Utils::DisplayErrorDialog(L"Failed to close the window due to the class name is empty.");
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
        Utils::DisplayErrorDialog(L"Failed to close the window due to DestroyWindow() and UnregisterClassW() are not available.");
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

    [[nodiscard]] UINT DotsPerInch() const noexcept;

    [[nodiscard]] COLORREF ColorizationColor() const noexcept;

    [[nodiscard]] HWND CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData) const noexcept;
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
    [[nodiscard]] bool DefaultMessageHandler(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result) noexcept;

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
    COLORREF m_colorizationColor = 0;
    UINT m_dpi = 0;
};

WindowPrivate::WindowPrivate(Window *q) noexcept
{
    q_ptr = q;
    const auto result = __CreateWindow((WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS), WS_EX_NOREDIRECTIONBITMAP, nullptr, this, WindowProc);
    m_window = std::get<0>(result);
    m_atom = std::get<1>(result);
    if (!m_window || (m_atom == INVALID_ATOM)) {
        Utils::DisplayErrorDialog(L"Failed to create this window.");
    }
}

WindowPrivate::~WindowPrivate() noexcept
{
    if (m_window && (m_atom != INVALID_ATOM)) {
        if (__CloseWindow(m_window, ATOM_TO_STRING(m_atom))) {
            m_window = nullptr;
            m_atom = INVALID_ATOM;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close this window.");
        }
    }
}

std::wstring WindowPrivate::Title() const noexcept
{
    return m_title;
}

void WindowPrivate::Title(const std::wstring &value) noexcept
{
    USER32_API(SetWindowTextW);
    if (SetWindowTextWFunc) {
        if (SetWindowTextWFunc(m_window, value.c_str()) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowTextW, L"Failed to change the window title.")
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to change the window title due to SetWindowTextW() is not available.");
    }
}

int WindowPrivate::Icon() const noexcept
{
    return m_icon;
}

void WindowPrivate::Icon(const int value) noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(value);
}

int WindowPrivate::X() const noexcept
{
    return m_x;
}

void WindowPrivate::X(const int value) noexcept
{
    if (m_x != value) {
        if (!Move(value, m_y)) {
            Utils::DisplayErrorDialog(L"Failed to update the X property of this window.");
        }
    }
}

int WindowPrivate::Y() const noexcept
{
    return m_y;
}

void WindowPrivate::Y(const int value) noexcept
{
    if (m_y != value) {
        if (!Move(m_x, value)) {
            Utils::DisplayErrorDialog(L"Failed to update the Y property of this window.");
        }
    }
}

UINT WindowPrivate::Width() const noexcept
{
    return m_width;
}

void WindowPrivate::Width(const UINT value) noexcept
{
    if (m_width != value) {
        if (!Resize(value, m_height)) {
            Utils::DisplayErrorDialog(L"Failed to update the Width property of this window.");
        }
    }
}

UINT WindowPrivate::Height() const noexcept
{
    return m_height;
}

void WindowPrivate::Height(const UINT value) noexcept
{
    if (m_height != value) {
        if (!Resize(m_width, value)) {
            Utils::DisplayErrorDialog(L"Failed to update the Height property of this window.");
        }
    }
}

WindowState WindowPrivate::Visibility() const noexcept
{
    return m_visibility;
}

void WindowPrivate::Visibility(const WindowState value) noexcept
{
    if (m_visibility != value) {
        if (!Utils::SetWindowState(m_window, value)) {
            Utils::DisplayErrorDialog(L"Failed to update the Visibility property of this window.");
        }
    }
}

WindowTheme WindowPrivate::Theme() const noexcept
{
    return m_theme;
}

UINT WindowPrivate::DotsPerInch() const noexcept
{
    return m_dpi;
}

COLORREF WindowPrivate::ColorizationColor() const noexcept
{
    return m_colorizationColor;
}

HWND WindowPrivate::CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData) const noexcept
{
    const auto result = __CreateWindow(style, (extendedStyle | WS_CHILD), m_window, extraData, wndProc);
    const HWND hWnd = std::get<0>(result);
    if (!hWnd) {
        Utils::DisplayErrorDialog(L"Failed to create child window.");
        return nullptr;
    }
    return hWnd;
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
            if (!q_ptr->FilterMessage(&msg)) {
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
            const auto that = static_cast<WindowPrivate *>(cs->lpCreateParams);
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to set the window extra data.")
            }
        } else if (message == WM_NCDESTROY) {
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, 0) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to clear the window extra data.")
            }
        }
        const auto that = reinterpret_cast<WindowPrivate *>(GetWindowLongPtrWFunc(hWnd, GWLP_USERDATA));
        if (that) {
            LRESULT result = 0;
            if (that->DefaultMessageHandler(message, wParam, lParam, &result)) {
                return result;
            } else {
                return that->q_ptr->MessageHandler(message, wParam, lParam);
            }
        } else {
            return DefWindowProcWFunc(hWnd, message, wParam, lParam);
        }
    } else {
        Utils::DisplayErrorDialog(L"SetWindowLongPtrW(), GetWindowLongPtrW() and DefWindowProcW() are not available.");
        return 0;
    }
}

bool WindowPrivate::DefaultMessageHandler(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result) noexcept
{
    if (!result) {
        Utils::DisplayErrorDialog(L"DefaultMessageHandler: the pointer to the result of the WindowProc function is null.");
        return false;
    }
    switch (message) {
    case WM_CREATE: {
        m_dpi = Utils::GetWindowMetrics(m_window, WindowMetrics::DotsPerInch);
        q_ptr->OnDotsPerInchChanged(m_dpi);
        const std::wstring dpiMsg = L"Current window's dots-per-inch (DPI): " + Utils::IntegerToString(m_dpi, 10);
        OutputDebugStringW(dpiMsg.c_str());
        if (!Utils::UpdateFrameMargins(m_window)) {
            Utils::DisplayErrorDialog(L"Failed to update the window frame margins.");
            return false;
        }
        USER32_API(SetWindowPos);
        if (SetWindowPosFunc) {
            constexpr UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
            if (SetWindowPosFunc(m_window, nullptr, 0, 0, 0, 0, flags) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to trigger a frame change event for the window.")
                return false;
            }
        } else {
            Utils::DisplayErrorDialog(L"SetWindowPos() is not available.");
            return false;
        }
        m_theme = Utils::GetSystemTheme();
        q_ptr->OnThemeChanged(m_theme);
        const std::wstring themeMsg = L"Current window's theme: " + Utils::ThemeToString(m_theme);
        OutputDebugStringW(themeMsg.c_str());
        if (!Utils::SetWindowTheme(m_window, m_theme)) {
            Utils::DisplayErrorDialog(L"Failed to set the window theme.");
            return false;
        }
        m_colorizationColor = L"";
        q_ptr->OnColorizationColorChanged(m_colorizationColor);
        m_visibility = WindowState::Hidden;
        q_ptr->OnVisibilityChanged(m_visibility);
        const auto cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        if ((cs->x == CW_USEDEFAULT) || (cs->y == CW_USEDEFAULT) || (cs->cx == CW_USEDEFAULT) || (cs->cy == CW_USEDEFAULT)) {
            USER32_API(GetWindowRect);
            if (GetWindowRectFunc) {
                RECT rect = {0, 0, 0, 0};
                if (GetWindowRectFunc(m_window, &rect) == FALSE) {
                    PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the window geometry.")
                    return false;
                } else {
                    m_x = rect.left;
                    m_y = rect.top;
                    m_width = RECT_WIDTH(rect);
                    m_height = RECT_HEIGHT(rect);
                }
            } else {
                Utils::DisplayErrorDialog(L"GetWindowRect() is not available.");
                return false;
            }
        } else {
            m_x = cs->x;
            m_y = cs->y;
            m_width = cs->cx;
            m_height = cs->cy;
        }
        q_ptr->OnXChanged(m_x);
        q_ptr->OnYChanged(m_y);
        q_ptr->OnWidthChanged(m_width);
        q_ptr->OnHeightChanged(m_height);
        LPCWSTR title = cs->lpszName;
        if (title) {
            m_title = title;
        } else {
            m_title = {};
        }
        q_ptr->OnTitleChanged(m_title);
    } break;
    case WM_MOVE: {
        m_x = GET_X_LPARAM(lParam);
        q_ptr->OnXChanged(m_x);
        m_y = GET_Y_LPARAM(lParam);
        q_ptr->OnYChanged(m_y);
    } break;
    case WM_SIZE: {
        bool needNotify = false;
        if (wParam == SIZE_RESTORED) {
            if ((m_visibility == WindowState::Minimized) || (m_visibility == WindowState::Maximized)) {
                m_visibility = WindowState::Normal;
                needNotify = true;
            }
        } else if (wParam == SIZE_MINIMIZED) {
            m_visibility = WindowState::Minimized;
            needNotify = true;
        } else if (wParam == SIZE_MAXIMIZED) {
            m_visibility = WindowState::Maximized;
            needNotify = true;
        }
        if (needNotify) {
            q_ptr->OnVisibilityChanged(m_visibility);
        }
        m_width = LOWORD(lParam);
        q_ptr->OnWidthChanged(m_width);
        m_height = HIWORD(lParam);
        q_ptr->OnHeightChanged(m_height);
    } break;
    case WM_SETTINGCHANGE: {
        // wParam == 0: User-wide setting change
        // wParam == 1: System-wide setting change
        // ### TODO: how to detect high contrast theme here
        if (((wParam == 0) || (wParam == 1)) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            m_theme = Utils::GetSystemTheme();
            q_ptr->OnThemeChanged(m_theme);
            if (!Utils::SetWindowTheme(m_window, m_theme)) {
                Utils::DisplayErrorDialog(L"Failed to set the window theme.");
                return false;
            }
        }
    } break;
    case WM_DPICHANGED: {
        const UINT oldDPI = m_dpi;
        const UINT dpiX = LOWORD(wParam);
        const UINT dpiY = HIWORD(wParam);
        m_dpi = static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
        q_ptr->OnDotsPerInchChanged(m_dpi);
        const std::wstring dpiMsg = L"Current window's dots-per-inch (DPI) has changed from " + Utils::IntegerToString(oldDPI, 10) + L" to " + Utils::IntegerToString(m_dpi, 10) + L".";
        OutputDebugStringW(dpiMsg.c_str());
        USER32_API(SetWindowPos);
        if (SetWindowPosFunc) {
            const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
            constexpr UINT flags = (SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER);
            if (SetWindowPosFunc(m_window, nullptr, prcNewWindow->left, prcNewWindow->top, RECT_WIDTH(*prcNewWindow), RECT_HEIGHT(*prcNewWindow), flags) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to update the geometry of the main window.")
                return false;
            } else {
                *result = 0;
                return true;
            }
        } else {
            Utils::DisplayErrorDialog(L"SetWindowPos() is not available.");
            return false;
        }
    } break;
    case WM_PAINT: {
        *result = 0;
        return true;
    } break;
    case WM_ERASEBKGND: {
        *result = 1;
        return true;
    } break;
    case WM_SETTEXT: {
        const auto title = reinterpret_cast<LPCWSTR>(lParam);
        if (title) {
            m_title = title;
        } else {
            m_title = {};
        }
        q_ptr->OnTitleChanged(m_title);
    } break;
    case WM_SETICON: {} break;
    case WM_CLOSE: {
        if (__CloseWindow(m_window, ATOM_TO_STRING(m_atom))) {
            m_window = nullptr;
            m_atom = INVALID_ATOM;
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close this window.");
            return false;
        }
    } break;
    case WM_DESTROY: {
        USER32_API(PostQuitMessage);
        if (PostQuitMessageFunc) {
            PostQuitMessageFunc(0);
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Failed to destroy this window due to PostQuitMessage() is not available.");
            return false;
        }
    } break;
    case WM_NCCREATE: {
        USER32_API(EnableNonClientDpiScaling);
        if (EnableNonClientDpiScalingFunc) {
            if (EnableNonClientDpiScalingFunc(m_window) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(EnableNonClientDpiScaling, L"Failed to enable non-client area DPI auto scaling.")
                return false;
            }
        } else {
            Utils::DisplayErrorDialog(L"Failed to enable non-client area DPI auto scaling due to EnableNonClientDpiScaling() is not available.");
            return false;
        }
    } break;
    case WM_NCCALCSIZE: {
        if (static_cast<BOOL>(wParam) == FALSE) {
            *result = 0;
            return true;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        USER32_API(DefWindowProcW);
        if (DefWindowProcWFunc) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcWFunc(m_window, WM_NCCALCSIZE, TRUE, lParam);
            if (ret != 0) {
                *result = ret;
                return true;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        } else {
            Utils::DisplayErrorDialog(L"DefWindowProcW() is not available.");
            return false;
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
            return false;
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
        // ### TODO: HTSYSMENU/HTMINBUTTON/HTMAXBUTTON/HTCLOSE
        const POINT globalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT localPos = globalPos;
        USER32_API(ScreenToClient);
        if (ScreenToClientFunc) {
            if (ScreenToClientFunc(m_window, &localPos) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(ScreenToClient, L"Failed to translate from screen coordinate to window coordinate.")
                return false;
            }
        } else {
            Utils::DisplayErrorDialog(L"ScreenToClient() is not available.");
            return false;
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
            Utils::DisplayErrorDialog(L"DefWindowProcW() is not available.");
            return false;
        }
    } break;
    case WM_NCRBUTTONUP: {
        if (wParam == HTCAPTION) {
            const POINT mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (Utils::OpenSystemMenu(m_window, mousePos)) {
                *result = 0;
                return true;
            } else {
                Utils::DisplayErrorDialog(L"Failed to open the system menu for the main window.");
                return false;
            }
        }
    } break;
    default:
        break;
    }
    return false;
}

Window::Window() noexcept
{
    d_ptr = std::make_unique<WindowPrivate>(this);
}

Window::~Window() noexcept = default;

std::wstring Window::Title() const noexcept
{
    return d_ptr->Title();
}

void Window::Title(const std::wstring &value) noexcept
{
    d_ptr->Title(value);
}

void Window::OnTitleChanged(const std::wstring &arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

int Window::Icon() const noexcept
{
    return d_ptr->Icon();
}

void Window::Icon(const int value) noexcept
{
    d_ptr->Icon(value);
}

void Window::OnIconChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

int Window::X() const noexcept
{
    return d_ptr->X();
}

void Window::X(const int value) noexcept
{
    d_ptr->X(value);
}

void Window::OnXChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

int Window::Y() const noexcept
{
    return d_ptr->Y();
}

void Window::Y(const int value) noexcept
{
    d_ptr->Y(value);
}

void Window::OnYChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

UINT Window::Width() const noexcept
{
    return d_ptr->Width();
}

void Window::Width(const UINT value) noexcept
{
    d_ptr->Width(value);
}

void Window::OnWidthChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

UINT Window::Height() const noexcept
{
    return d_ptr->Height();
}

void Window::Height(const UINT value) noexcept
{
    d_ptr->Height(value);
}

void Window::OnHeightChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

WindowState Window::Visibility() const noexcept
{
    return d_ptr->Visibility();
}

void Window::Visibility(const WindowState value) noexcept
{
    d_ptr->Visibility(value);
}

void Window::OnVisibilityChanged(const WindowState arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

WindowTheme Window::Theme() const noexcept
{
    return d_ptr->Theme();
}

void Window::OnThemeChanged(const WindowTheme arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

UINT Window::DotsPerInch() const noexcept
{
    return d_ptr->DotsPerInch();
}

void Window::OnDotsPerInchChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

COLORREF Window::ColorizationColor() const noexcept
{
    return d_ptr->ColorizationColor();
}

void Window::OnColorizationColorChanged(const COLORREF arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

HWND Window::CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData) const noexcept
{
    return d_ptr->CreateChildWindow(style, extendedStyle, wndProc, extraData);
}

HWND Window::WindowHandle() const noexcept
{
    return d_ptr->WindowHandle();
}

int Window::MessageLoop() const noexcept
{
    return d_ptr->MessageLoop();
}

bool Window::Move(const int x, const int y) noexcept
{
    return d_ptr->Move(x, y);
}

bool Window::Resize(const UINT w, const UINT h) noexcept
{
    return d_ptr->Resize(w, h);
}

bool Window::SetGeometry(const int x, const int y, const UINT w, const UINT h) noexcept
{
    return d_ptr->SetGeometry(x, y, w, h);
}
