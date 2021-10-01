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

#include <SDKDDKVer.h>
#include <Windows.h>
#include <ShellApi.h>
#include <Unknwn.h> // To enable support for non-WinRT interfaces, Unknwn.h must be included before any C++/WinRT headers.

// Avoid collision with WinRT's same name function.
// https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/faq#how-do-i-resolve-ambiguities-with-getcurrenttime-and-or-try-
#pragma push_macro("GetCurrentTime")
#pragma push_macro("TRY")
#undef GetCurrentTime
#undef TRY

#include <WinRT\Base.h>

// Workaround for Windows SDK bug.
// See https://github.com/microsoft/Windows.UI.Composition-Win32-Samples/issues/47
namespace winrt::impl
{
    template <typename Async>
    auto wait_for(Async const& async, Windows::Foundation::TimeSpan const& timeout);
}

#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

// Restore the macros from Win32 headers.
#pragma pop_macro("TRY")
#pragma pop_macro("GetCurrentTime")

#include "AcrylicApplication.h"
#include "Resource.h"
#include "SystemLibraryManager.h"
#include "Utils.h"

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
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

namespace Constants {
namespace Light {
static constexpr winrt::Windows::UI::Color TintColor = {255, 252, 252, 252};
static constexpr double TintOpacity = 0.0;
static constexpr double LuminosityOpacity = 0.85;
static constexpr winrt::Windows::UI::Color FallbackColor = {255, 249, 249, 249};
} // namespace Light
namespace Dark {
static constexpr winrt::Windows::UI::Color TintColor = {255, 44, 44, 44};
static constexpr double TintOpacity = 0.15;
static constexpr double LuminosityOpacity = 0.96;
static constexpr winrt::Windows::UI::Color FallbackColor = {255, 44, 44, 44};
} // namespace Dark
namespace HighContrast {
// ### TO BE IMPLEMENTED
} // namespace HighContrast
} // namespace Constants

// The thickness of an auto-hide taskbar in pixels.
static constexpr UINT g_autoHideTaskbarThickness = 2;

static constexpr wchar_t g_defaultWindowClassNamePrefix[] = LR"(wangwenx190\Win32AcrylicHelper\WindowClasses\)";
static constexpr wchar_t g_defaultWindowTitle[] = L"Win32AcrylicHelper Application Main Window";

static std::wstring g_mainWindowClassName = {};
static std::wstring g_dragBarWindowClassName = {};
static HWND g_mainWindowHandle = nullptr;
static HWND g_dragBarWindowHandle = nullptr;
static HWND g_xamlIslandWindowHandle = nullptr;
static UINT g_mainWindowDPI = 0;
static bool g_comInitialized = false;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_xamlManager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_xamlSource = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_windowBackgroundBrush = nullptr;
static winrt::Windows::UI::Xaml::Controls::Button g_windowIconButton = nullptr;
static winrt::Windows::UI::Xaml::Controls::Button g_minimizeButton = nullptr;
static winrt::Windows::UI::Xaml::Controls::Button g_maximizeButton = nullptr;
static winrt::Windows::UI::Xaml::Controls::Button g_closeButton = nullptr;

[[nodiscard]] static inline bool RefreshBackgroundBrush() noexcept
{
    if (g_windowBackgroundBrush == nullptr) {
        OutputDebugStringW(L"Failed to refresh the background brush due to the brush is null.");
        return false;
    }
    const WindowTheme systemTheme = Utils::GetSystemTheme();
    switch (systemTheme) {
    case WindowTheme::Light: {
        g_windowBackgroundBrush.TintColor(Constants::Light::TintColor);
        g_windowBackgroundBrush.TintOpacity(Constants::Light::TintOpacity);
        g_windowBackgroundBrush.TintLuminosityOpacity(Constants::Light::LuminosityOpacity);
        g_windowBackgroundBrush.FallbackColor(Constants::Light::FallbackColor);
        return true;
    } break;
    case WindowTheme::Dark: {
        g_windowBackgroundBrush.TintColor(Constants::Dark::TintColor);
        g_windowBackgroundBrush.TintOpacity(Constants::Dark::TintOpacity);
        g_windowBackgroundBrush.TintLuminosityOpacity(Constants::Dark::LuminosityOpacity);
        g_windowBackgroundBrush.FallbackColor(Constants::Dark::FallbackColor);
        return true;
    } break;
    case WindowTheme::HighContrast: {
        // ### TODO
    } break;
    }
    return false;
}

[[nodiscard]] static inline bool RefreshWindowTheme(const HWND hWnd) noexcept
{
    if (!hWnd) {
        OutputDebugStringW(L"Failed to refresh the window theme due to the window handle is null.");
        return false;
    }
    const WindowTheme systemTheme = Utils::GetSystemTheme();
    return Utils::SetWindowTheme(hWnd, systemTheme);
}

[[nodiscard]] static inline bool SyncXAMLIslandPosition(const UINT width, const UINT height) noexcept
{
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to the main window handle is null.");
            return false;
        }
        if (!g_xamlIslandWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
            return false;
        }
        if ((width == 0) || (height == 0)) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to the main window width and height are invalid.");
            return false;
        }
        const UINT borderThickness = Utils::GetWindowMetrics(g_mainWindowHandle, WindowMetrics::FrameBorderThickness);
        if (SetWindowPosFunc(g_xamlIslandWindowHandle, HWND_BOTTOM, 0, borderThickness, width, height, SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to sync the geometry of the XAML Island window.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"SetWindowPos() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool SyncXAMLIslandPosition() noexcept
{
    if (!g_mainWindowHandle) {
        OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to the main window handle is null.");
        return false;
    }
    if (!g_xamlIslandWindowHandle) {
        OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
        return false;
    }
    const UINT width = Utils::GetWindowMetrics(g_mainWindowHandle, WindowMetrics::Width);
    const UINT height = Utils::GetWindowMetrics(g_mainWindowHandle, WindowMetrics::Height);
    return SyncXAMLIslandPosition(width, height);
}

[[nodiscard]] static inline bool SyncDragBarPosition(const UINT width) noexcept
{
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the drag bar window due to the main window handle is null.");
            return false;
        }
        if (!g_dragBarWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the drag bar window due to the drag bar window handle is null.");
            return false;
        }
        if (width == 0) {
            OutputDebugStringW(L"Failed to sync the geometry of the drag bar window due to the main window width is invalid.");
            return false;
        }
        const UINT titleBarHeight = Utils::GetWindowMetrics(g_mainWindowHandle, WindowMetrics::TitleBarHeight);
        if (SetWindowPosFunc(g_dragBarWindowHandle, HWND_TOP, 0, 0, width, titleBarHeight, SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to sync the geometry of the drag bar window.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"SetWindowPos() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool SyncDragBarPosition() noexcept
{
    if (!g_mainWindowHandle) {
        OutputDebugStringW(L"Failed to sync the geometry of the drag bar window due to the main window handle is null.");
        return false;
    }
    if (!g_dragBarWindowHandle) {
        OutputDebugStringW(L"Failed to sync the geometry of the drag bar window due to the drag bar window handle is null.");
        return false;
    }
    const UINT width = Utils::GetWindowMetrics(g_mainWindowHandle, WindowMetrics::Width);
    return SyncDragBarPosition(width);
}

[[nodiscard]] static inline LRESULT CALLBACK MainWindowMessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (message) {
    case WM_NCCREATE: {
        USER32_API(EnableNonClientDpiScaling);
        if (EnableNonClientDpiScalingFunc) {
            if (EnableNonClientDpiScalingFunc(hWnd) == FALSE) {
                OutputDebugStringW(L"Failed to enable non-client area DPI auto-scaling.");
            }
        } else {
            OutputDebugStringW(L"EnableNonClientDpiScaling() is not available.");
        }
    } break;
    case WM_NCCALCSIZE: {
        if (wParam == FALSE) {
            return 0;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        USER32_API(DefWindowProcW);
        if (DefWindowProcWFunc) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcWFunc(hWnd, WM_NCCALCSIZE, TRUE, lParam);
            if (ret != 0) {
                return ret;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        } else {
            OutputDebugStringW(L"DefWindowProcW() is not available.");
            break;
        }
        bool nonClientAreaExists = false;
        const WindowState windowState = Utils::GetWindowState(hWnd);
        const bool max = (windowState == WindowState::Maximized);
        const bool full = (windowState == WindowState::FullScreen);
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
            const UINT resizeBorderThicknessY = Utils::GetWindowMetrics(hWnd, WindowMetrics::ResizeBorderThicknessY);
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
                    const RECT screenRect = Utils::GetScreenGeometry(hWnd);
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
            OutputDebugStringW(L"SHAppBarMessage() is not available.");
            break;
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
        const POINT screenPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT windowPos = screenPos;
        USER32_API(ScreenToClient);
        if (ScreenToClientFunc) {
            if (ScreenToClientFunc(hWnd, &windowPos) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(ScreenToClient, L"Failed to translate from screen coordinate to window coordinate.")
                break;
            }
        } else {
            OutputDebugStringW(L"ScreenToClient() is not available.");
            break;
        }
        const WindowState windowState = Utils::GetWindowState(hWnd);
        const auto resizeBorderThicknessY = static_cast<LONG>(Utils::GetWindowMetrics(hWnd, WindowMetrics::ResizeBorderThicknessY));
        const auto titleBarHeight = static_cast<LONG>(Utils::GetWindowMetrics(hWnd, WindowMetrics::TitleBarHeight));
        const bool isTitleBar = ((windowState != WindowState::Minimized) ? (windowPos.y <= titleBarHeight) : false);
        const bool isTop = ((windowState == WindowState::Normal) ? (windowPos.y <= resizeBorderThicknessY) : false);
        USER32_API(DefWindowProcW);
        if (DefWindowProcWFunc) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcWFunc(hWnd, WM_NCHITTEST, 0, lParam);
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
            OutputDebugStringW(L"DefWindowProcW() is not available.");
            break;
        }
    } break;
    case WM_SETFOCUS: {
        if (g_xamlIslandWindowHandle) {
            USER32_API(SetFocus);
            if (SetFocusFunc) {
                // Send focus to the XAML Island child window.
                if (SetFocusFunc(g_xamlIslandWindowHandle) == nullptr) {
                    PRINT_WIN32_ERROR_MESSAGE(SetFocus, L"Failed to send focus to the XAML Island window.")
                    break;
                }
                return 0;
            } else {
                OutputDebugStringW(L"SetFocus() is not available.");
            }
        }
    } break;
    case WM_SETCURSOR: {
        if (LOWORD(lParam) == HTCLIENT) {
            USER32_API(SendMessageW);
            USER32_API(GetMessagePos);
            USER32_API(SetCursor);
            USER32_API(LoadCursorW);
            if (SendMessageWFunc && GetMessagePosFunc && SetCursorFunc && LoadCursorWFunc) {
                // Get the cursor position from the _last message_ and not from
                // `GetCursorPos` (which returns the cursor position _at the
                // moment_) because if we're lagging behind the cursor's position,
                // we still want to get the cursor position that was associated
                // with that message at the time it was sent to handle the message
                // correctly.
                const LRESULT hitTestResult = SendMessageWFunc(hWnd, WM_NCHITTEST, 0, GetMessagePosFunc());
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
                    SetCursorFunc(LoadCursorWFunc(nullptr, IDC_SIZENS));
                } else {
                    // Reset cursor
                    SetCursorFunc(LoadCursorWFunc(nullptr, IDC_ARROW));
                }
                return TRUE;
            } else {
                OutputDebugStringW(L"SendMessageW(), GetMessagePos(), SetCursor() and LoadCursorW() are not available.");
            }
        }
    } break;
    case WM_NCRBUTTONUP: {
        if (wParam == HTCAPTION) {
            if (Utils::OpenSystemMenu(hWnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)})) {
                return 0;
            } else {
                OutputDebugStringW(L"Failed to open the system menu for the main window.");
            }
        }
    } break;
    case WM_CREATE: {
        g_mainWindowDPI = Utils::GetWindowMetrics(hWnd, WindowMetrics::DotsPerInch);
        const std::wstring dpiMsg = L"Main window dots-per-inch (DPI): " + Utils::IntegerToString(g_mainWindowDPI, 10);
        OutputDebugStringW(dpiMsg.c_str());
        if (!Utils::UpdateFrameMargins(hWnd)) {
            OutputDebugStringW(L"Failed to update the frame margins for the main window.");
            break;
        }
        USER32_API(SetWindowPos);
        if (SetWindowPosFunc) {
            if (SetWindowPosFunc(hWnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to trigger a frame change event for the main window.")
                break;
            }
        } else {
            OutputDebugStringW(L"SetWindowPos() is not available.");
            break;
        }
        if (!RefreshWindowTheme(hWnd)) {
            Utils::DisplayErrorDialog(L"Failed to refresh the window theme.");
        }
    } break;
    case WM_SIZE: {
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED) || (Utils::GetWindowState(hWnd) == WindowState::FullScreen)) {
            if (!Utils::UpdateFrameMargins(hWnd)) {
                OutputDebugStringW(L"Failed to update the frame margins for the main window.");
                break;
            }
            const UINT width = LOWORD(lParam);
            if (g_xamlIslandWindowHandle) {
                const UINT height = HIWORD(lParam);
                if (!SyncXAMLIslandPosition(width, height)) {
                    Utils::DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
                    break;
                }
            }
            if (g_dragBarWindowHandle) {
                if (!SyncDragBarPosition(width)) {
                    Utils::DisplayErrorDialog(L"Failed to sync the geometry of the drag bar window.");
                }
            }
        }
    } break;
    case WM_SETTINGCHANGE: {
        // wParam == 0: User-wide setting change
        // wParam == 1: System-wide setting change
        // ### TODO: how to detect high contrast theme here
        if (((wParam == 0) || (wParam == 1)) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            if (!RefreshWindowTheme(hWnd)) {
                Utils::DisplayErrorDialog(L"Failed to refresh the window theme.");
                break;
            }
            if (!RefreshBackgroundBrush()) {
                Utils::DisplayErrorDialog(L"Failed to refresh the background brush.");
            }
        }
    } break;
    case WM_DPICHANGED: {
        const UINT oldMainWindowDPI = g_mainWindowDPI;
        const UINT dpiX = LOWORD(wParam);
        const UINT dpiY = HIWORD(wParam);
        g_mainWindowDPI = static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
        const std::wstring dpiMsg = L"The dots-per-inch (DPI) of main window has changed. " + Utils::IntegerToString(oldMainWindowDPI, 10) + L" --> " + Utils::IntegerToString(g_mainWindowDPI, 10) + L".";
        OutputDebugStringW(dpiMsg.c_str());
        USER32_API(SetWindowPos);
        if (SetWindowPosFunc) {
            const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
            if (SetWindowPosFunc(hWnd, nullptr,
                                 prcNewWindow->left, prcNewWindow->top,
                                 std::abs(prcNewWindow->right - prcNewWindow->left),
                                 std::abs(prcNewWindow->bottom - prcNewWindow->top),
                                 SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to update the geometry of the main window.")
                break;
            }
            return 0;
        } else {
            OutputDebugStringW(L"SetWindowPos() is not available.");
        }
    } break;
    case WM_DWMCOLORIZATIONCOLORCHANGED: {
        // The color format is 0xAARRGGBB.
        const auto color = static_cast<DWORD>(wParam);
        static_cast<void>(color); // ### TODO
    } break;
    case WM_PAINT: {
        return 0;
    } break;
    case WM_ERASEBKGND: {
        return 1;
    } break;
    case WM_CLOSE: {
        if (Utils::CloseWindow(hWnd, g_mainWindowClassName)) {
            const std::wstring dbgMsg = L"Main window class \"" + g_mainWindowClassName + L"\" has been unregistered.";
            OutputDebugStringW(dbgMsg.c_str());
            g_mainWindowHandle = nullptr;
            g_mainWindowClassName = {};
            g_mainWindowDPI = 0;
            return 0;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close the main window.");
        }
    } break;
    case WM_DESTROY: {
        g_windowBackgroundBrush = nullptr;
        g_rootGrid = nullptr;
        if (g_xamlSource != nullptr) {
            g_xamlSource.Close();
            g_xamlSource = nullptr;
        }
        if (g_xamlManager != nullptr) {
            g_xamlManager.Close();
            g_xamlManager = nullptr;
        }
        g_xamlIslandWindowHandle = nullptr;
        USER32_API(PostQuitMessage);
        if (PostQuitMessageFunc) {
            PostQuitMessageFunc(0);
            return 0;
        } else {
            OutputDebugStringW(L"PostQuitMessage() is not available.");
        }
    } break;
    default:
        break;
    }
    USER32_API(DefWindowProcW);
    if (DefWindowProcWFunc) {
        return DefWindowProcWFunc(hWnd, message, wParam, lParam);
    } else {
        OutputDebugStringW(L"DefWindowProcW() is not available.");
        return 0;
    }
}

[[nodiscard]] static inline LRESULT CALLBACK DragBarMessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    if ((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST)) {
        std::optional<UINT> nonClientMessage = std::nullopt;
        switch (message)
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
        if (nonClientMessage.has_value() && g_mainWindowHandle) {
            USER32_API(ClientToScreen);
            USER32_API(SendMessageW);
            if (ClientToScreenFunc && SendMessageWFunc) {
                POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                if (ClientToScreenFunc(hWnd, &pos) == FALSE) {
                    PRINT_WIN32_ERROR_MESSAGE(ClientToScreen, L"Failed to translate from window coordinate to screen coordinate.")
                    return 0;
                }
                const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
                // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
                // then pass that click through as an NC click in that location.
                const LRESULT hitTestResult = SendMessageWFunc(g_mainWindowHandle, WM_NCHITTEST, 0, newLParam);
                SendMessageWFunc(g_mainWindowHandle, nonClientMessage.value(), hitTestResult, newLParam);
                return 0;
            } else {
                OutputDebugStringW(L"ClientToScreen() and SendMessageW() are not available.");
            }
        }
    } else {
        switch (message) {
        case WM_NCCALCSIZE: {
            return WVR_REDRAW;
        } break;
        case WM_PAINT: {
            return 0;
        } break;
        case WM_ERASEBKGND: {
            return 1;
        } break;
        case WM_CLOSE: {
            if (Utils::CloseWindow(hWnd, g_dragBarWindowClassName)) {
                const std::wstring dbgMsg = L"Drag bar window class \"" + g_dragBarWindowClassName + L"\" has been unregistered.";
                OutputDebugStringW(dbgMsg.c_str());
                g_dragBarWindowHandle = nullptr;
                g_dragBarWindowClassName = {};
                return 0;
            } else {
                Utils::DisplayErrorDialog(L"Failed to close the drag bar window.");
            }
        } break;
        default:
            break;
        }
    }
    USER32_API(DefWindowProcW);
    if (DefWindowProcWFunc) {
        return DefWindowProcWFunc(hWnd, message, wParam, lParam);
    } else {
        OutputDebugStringW(L"DefWindowProcW() is not available.");
        return 0;
    }
}

[[nodiscard]] static inline bool RegisterMainWindowClass(const std::wstring &name) noexcept
{
    USER32_API(LoadCursorW);
    USER32_API(LoadIconW);
    USER32_API(RegisterClassExW);
    USER32_API(GetClassInfoExW);
    if (LoadCursorWFunc && LoadIconWFunc && RegisterClassExWFunc && GetClassInfoExWFunc) {
        bool exists = false;
        WNDCLASSEXW wcex = {};
        if (!name.empty()) {
            exists = (GetClassInfoExWFunc(Utils::GetCurrentModuleInstance(), name.c_str(), &wcex) != FALSE);
        }
        if (!exists) {
            if (!g_mainWindowClassName.empty()) {
                exists = (GetClassInfoExWFunc(Utils::GetCurrentModuleInstance(), g_mainWindowClassName.c_str(), &wcex) != FALSE);
            }
        }
        const std::wstring guid = Utils::GenerateGUID();
        if (guid.empty()) {
            OutputDebugStringW(L"Failed to generate a new GUID.");
            return false;
        }
        const std::wstring defaultWindowClassName = g_defaultWindowClassNamePrefix + guid + L"@MainWindow";
        g_mainWindowClassName = (name.empty() ? defaultWindowClassName : name);
        if (!exists) {
            SecureZeroMemory(&wcex, sizeof(wcex));
            wcex.cbSize = sizeof(wcex);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = MainWindowMessageHandler;
            wcex.hInstance = Utils::GetCurrentModuleInstance();
            wcex.lpszClassName = g_mainWindowClassName.c_str();
            wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
            wcex.hIcon = LoadIconWFunc(Utils::GetCurrentModuleInstance(), MAKEINTRESOURCEW(IDI_APPICON));
            wcex.hIconSm = LoadIconWFunc(Utils::GetCurrentModuleInstance(), MAKEINTRESOURCEW(IDI_APPICON_SMALL));
        }
        const ATOM atom = RegisterClassExWFunc(&wcex);
        if (atom == INVALID_ATOM) {
            PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW, L"Failed to register the main window class.")
            return false;
        }
        const std::wstring dbgMsg = L"Main window class \"" + g_mainWindowClassName + L"\" has been registered.";
        OutputDebugStringW(dbgMsg.c_str());
        return true;
    } else {
        OutputDebugStringW(L"LoadCursorW(), LoadIconW(), RegisterClassExW() and GetClassInfoExW() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool RegisterDragBarWindowClass() noexcept
{
    USER32_API(LoadCursorW);
    USER32_API(RegisterClassExW);
    if (LoadCursorWFunc && RegisterClassExWFunc) {
        const std::wstring guid = Utils::GenerateGUID();
        if (guid.empty()) {
            OutputDebugStringW(L"Failed to generate a new GUID.");
            return false;
        }
        g_dragBarWindowClassName = g_defaultWindowClassNamePrefix + guid + L"@DragBarWindow";
        WNDCLASSEXW wcex;
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wcex.lpfnWndProc = DragBarMessageHandler;
        wcex.hInstance = Utils::GetCurrentModuleInstance();
        wcex.lpszClassName = g_dragBarWindowClassName.c_str();
        wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
        const ATOM atom = RegisterClassExWFunc(&wcex);
        if (atom == INVALID_ATOM) {
            PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW, L"Failed to register the drag bar window class.")
            return false;
        }
        const std::wstring dbgMsg = L"Drag bar window class \"" + g_dragBarWindowClassName + L"\" has been registered.";
        OutputDebugStringW(dbgMsg.c_str());
        return true;
    } else {
        OutputDebugStringW(L"LoadCursorW() and RegisterClassExW() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool CreateMainWindow(const std::wstring &title) noexcept
{
    USER32_API(CreateWindowExW);
    if (CreateWindowExWFunc) {
        if (g_mainWindowClassName.empty()) {
            OutputDebugStringW(L"Failed to create the main window due to the main window class is not registered.");
            return false;
        }
        g_mainWindowHandle = CreateWindowExWFunc(
            WS_EX_NOREDIRECTIONBITMAP,
            g_mainWindowClassName.c_str(),
            (title.empty() ? g_defaultWindowTitle : title.c_str()),
            (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS),
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            nullptr, nullptr, Utils::GetCurrentModuleInstance(), nullptr);
        if (!g_mainWindowHandle) {
            PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW, L"Failed to create the main window.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"CreateWindowExW() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool CreateDragBarWindow() noexcept
{
    // The drag bar window is a child window of the top level window that is put
    // right on top of the drag bar. The XAML island window "steals" our mouse
    // messages which makes it hard to implement a custom drag area. By putting
    // a window on top of it, we prevent it from "stealing" the mouse messages.
    //
    // IMPORTANT NOTE: The WS_EX_LAYERED style is supported for both top-level
    // windows and child windows since Windows 8. Previous Windows versions support
    // WS_EX_LAYERED only for top-level windows.
    USER32_API(CreateWindowExW);
    USER32_API(SetLayeredWindowAttributes);
    if (CreateWindowExWFunc && SetLayeredWindowAttributesFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to create the drag bar window due to the main window has not been created.");
            return false;
        }
        if (g_dragBarWindowClassName.empty()) {
            OutputDebugStringW(L"Failed to create the drag bar window due to the drag bar window class is not registered.");
            return false;
        }
        g_dragBarWindowHandle = CreateWindowExWFunc(
            (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP),
            g_dragBarWindowClassName.c_str(),
            nullptr, WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            g_mainWindowHandle, nullptr, Utils::GetCurrentModuleInstance(), nullptr);
        if (!g_dragBarWindowHandle) {
            PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW, L"Failed to create the drag bar window.")
            return false;
        }
        // Layered window won't be visible until we call SetLayeredWindowAttributes()
        // or UpdateLayeredWindow().
        if (SetLayeredWindowAttributesFunc(g_dragBarWindowHandle, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetLayeredWindowAttributes, L"Failed to set layered window attributes.")
            return false;
        }
        if (!SyncDragBarPosition()) {
            OutputDebugStringW(L"Failed to sync the geometry of the drag bar window.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"CreateWindowExW() and SetLayeredWindowAttributes() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool InitializeXAMLIsland() noexcept
{
    // The call to winrt::init_apartment() initializes COM. By default, in a multithreaded apartment.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    g_comInitialized = true;
    // Initialize the XAML framework's core window for the current thread.
    g_xamlManager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    if (g_xamlManager == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to initialize the Windows XAML Manager for the current thread.");
        return false;
    }
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    g_xamlSource = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
    // Get handle to the core window.
    const auto interop = g_xamlSource.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the IDesktopWindowXamlSourceNative.");
        return false;
    }
    // Parent the DesktopWindowXamlSource object to the current window.
    HRESULT hr = interop->AttachToWindow(g_mainWindowHandle);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(AttachToWindow, hr, L"Failed to attach the XAML Island window to the main window.")
        return false;
    }
    // Get the new child window's HWND.
    hr = interop->get_WindowHandle(&g_xamlIslandWindowHandle);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(get_WindowHandle, hr, L"Failed to retrieve the XAML Island window's handle.")
        return false;
    }
    if (!g_xamlIslandWindowHandle) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window handle of XAML Island's core window.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    if (!SyncXAMLIslandPosition()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
        return false;
    }
    // Create the XAML content.
    g_rootGrid = winrt::Windows::UI::Xaml::Controls::Grid();
    g_windowBackgroundBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
    if (!RefreshBackgroundBrush()) {
        Utils::DisplayErrorDialog(L"Failed to refresh the background brush.");
        return false;
    }
    g_windowBackgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_rootGrid.Background(g_windowBackgroundBrush);
    //g_rootGrid.Children().Clear();
    //g_rootGrid.Children().Append(/* some UWP control */);
    //g_rootGrid.UpdateLayout();
    g_xamlSource.Content(g_rootGrid);
    return true;
}

[[nodiscard]] static inline int MessageLoop() noexcept
{
    USER32_API(GetMessageW);
    USER32_API(TranslateMessage);
    USER32_API(DispatchMessageW);
    if (GetMessageWFunc && TranslateMessageFunc && DispatchMessageWFunc) {
        MSG msg = {};
        while (GetMessageWFunc(&msg, nullptr, 0, 0) != FALSE) {
            BOOL filtered = FALSE;
            if (g_xamlSource != nullptr) {
                const auto interop2 = g_xamlSource.as<IDesktopWindowXamlSourceNative2>();
                if (interop2) {
                    const HRESULT hr = interop2->PreTranslateMessage(&msg, &filtered);
                    if (FAILED(hr)) {
                        PRINT_HR_ERROR_MESSAGE(PreTranslateMessage, hr, L"Failed to pre-translate win32 messages.")
                    }
                } else {
                    Utils::DisplayErrorDialog(L"Failed to retrieve the IDesktopWindowXamlSourceNative2.");
                }
            }
            if (filtered == FALSE) {
                TranslateMessageFunc(&msg);
                DispatchMessageWFunc(&msg);
            }
        }
        return static_cast<int>(msg.wParam);
    } else {
        OutputDebugStringW(L"GetMessageW(), TranslateMessage() and DispatchMessageW() are not available.");
        return -1;
    }
}

int AcrylicApplication::Main() noexcept
{
    // XAML Island is only supported on Windows 10 19H1 and onwards.
    if (!IsWindows1019H1OrGreater()) {
        Utils::DisplayErrorDialog(L"This application only supports Windows 10 19H1 and onwards.");
        return -1;
    }

    if (!Utils::SetProcessDPIAwareness(DPIAwareness::PerMonitorV2)) {
        if (!Utils::SetProcessDPIAwareness(DPIAwareness::PerMonitor)) {
            if (!Utils::SetProcessDPIAwareness(DPIAwareness::System)) {
                if (!Utils::SetProcessDPIAwareness(DPIAwareness::GdiScaled)) {
                    Utils::DisplayErrorDialog(L"Failed to set the DPI awareness for the current process.");
                    return -1;
                }
            }
        }
    }

    if (!RegisterMainWindowClass({})) {
        Utils::DisplayErrorDialog(L"Failed to register the main window class.");
        return -1;
    }

    if (!CreateMainWindow({})) {
        Utils::DisplayErrorDialog(L"Failed to create the main window.");
        return -1;
    }

    if (!InitializeXAMLIsland()) {
        Utils::DisplayErrorDialog(L"Failed to initialize the XAML Island.");
        return -1;
    }

    if (!RegisterDragBarWindowClass()) {
        Utils::DisplayErrorDialog(L"Failed to register the drag bar window class.");
        return -1;
    }

    if (!CreateDragBarWindow()) {
        Utils::DisplayErrorDialog(L"Failed to create the drag bar window.");
        return -1;
    }

    if (!Utils::SetWindowState(g_mainWindowHandle, WindowState::Shown)) {
        Utils::DisplayErrorDialog(L"Failed to show the main window.");
        return -1;
    }

    return MessageLoop();
}
