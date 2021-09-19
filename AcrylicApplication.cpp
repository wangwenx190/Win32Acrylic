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
#include <Unknwn.h>
#pragma push_macro("GetCurrentTime")
#pragma push_macro("TRY")
#undef GetCurrentTime
#undef TRY
#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>
#pragma pop_macro("TRY")
#pragma pop_macro("GetCurrentTime")
#include "AcrylicApplication.h"
#include "Resource.h"
#include "WindowsVersion.h"
#include "SystemLibraryManager.h"
#include "Utils.h"

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

static constexpr UINT g_autoHideTaskbarThickness = 2;

static constexpr wchar_t g_defaultWindowTitle[] = L"Win32AcrylicHelper Application Main Window";

static ATOM g_mainWindowAtom = INVALID_ATOM;
static HWND g_mainWindowHandle = nullptr;
static HWND g_xamlIslandWindowHandle = nullptr;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_manager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_source = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_backgroundBrush = nullptr;

[[nodiscard]] static inline bool RefreshBackgroundBrush() noexcept
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    if (g_backgroundBrush == nullptr) {
        OutputDebugStringW(L"Failed to refresh the background brush due to the brush is null.");
        return false;
    }
    if (Utils::IsHighContrastModeEnabled()) {
        // ### TO BE IMPLEMENTED
    } else if (Utils::ShouldAppsUseDarkMode()) {
        g_backgroundBrush.TintColor(Constants::Dark::TintColor);
        g_backgroundBrush.TintOpacity(Constants::Dark::TintOpacity);
        g_backgroundBrush.TintLuminosityOpacity(Constants::Dark::LuminosityOpacity);
        g_backgroundBrush.FallbackColor(Constants::Dark::FallbackColor);
    } else {
        g_backgroundBrush.TintColor(Constants::Light::TintColor);
        g_backgroundBrush.TintOpacity(Constants::Light::TintOpacity);
        g_backgroundBrush.TintLuminosityOpacity(Constants::Light::LuminosityOpacity);
        g_backgroundBrush.FallbackColor(Constants::Light::FallbackColor);
    }
    return true;
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
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to invalid width and height.");
            return false;
        }
        const UINT borderThickness = Utils::GetFrameBorderThickness(g_mainWindowHandle);
        if (SetWindowPosFunc(g_xamlIslandWindowHandle, nullptr, 0, borderThickness, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
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
    USER32_API(GetClientRect);
    if (GetClientRectFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to the main window handle is null.");
            return false;
        }
        if (!g_xamlIslandWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
            return false;
        }
        RECT clientRect = {0, 0, 0, 0};
        if (GetClientRectFunc(g_mainWindowHandle, &clientRect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetClientRect, L"Failed to retrieve the client rect of the window.")
            return false;
        }
        return SyncXAMLIslandPosition(clientRect.right, clientRect.bottom);
    } else {
        OutputDebugStringW(L"GetClientRect() is not available.");
        return false;
    }
}

[[nodiscard]] static inline LRESULT CALLBACK MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (message) {
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
            return WVR_REDRAW;
        }
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        bool nonClientAreaExists = false;
        const bool max = Utils::IsWindowMaximized(hWnd);
        const bool full = Utils::IsWindowFullScreen(hWnd);
        if (max && !full) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            const UINT resizeBorderThicknessY = Utils::GetResizeBorderThickness(hWnd, false);
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
                    const HMONITOR mon = Utils::GetWindowScreen(hWnd, true);
                    if (mon) {
                        USER32_API(GetMonitorInfoW);
                        if (GetMonitorInfoWFunc) {
                            MONITORINFO mi;
                            SecureZeroMemory(&mi, sizeof(mi));
                            mi.cbSize = sizeof(mi);
                            if (GetMonitorInfoWFunc(mon, &mi) == FALSE) {
                                PRINT_WIN32_ERROR_MESSAGE(GetMonitorInfoW, L"Failed to retrieve the screen information.")
                                return WVR_REDRAW;
                            } else {
                                const RECT screenRect = mi.rcMonitor;
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
                        } else {
                            OutputDebugStringW(L"GetMonitorInfoW() is not available.");
                            return WVR_REDRAW;
                        }
                    } else {
                        OutputDebugStringW(L"Failed to retrieve the window's corresponding screen.");
                        return WVR_REDRAW;
                    }
                }
            }
        } else {
            OutputDebugStringW(L"SHAppBarMessage() is not available.");
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
        const POINT screenPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT windowPos = screenPos;
        USER32_API(ScreenToClient);
        if (ScreenToClientFunc) {
            if (ScreenToClientFunc(hWnd, &windowPos) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(ScreenToClient, L"Failed to translate from screen coordinate to window coordinate.")
                return HTNOWHERE;
            }
        } else {
            OutputDebugStringW(L"ScreenToClient() is not available.");
            return HTNOWHERE;
        }
        const bool max = Utils::IsWindowMaximized(hWnd);
        const bool full = Utils::IsWindowFullScreen(hWnd);
        const bool normal = Utils::IsWindowNoState(hWnd);
        RECT windowRect = {0, 0, 0, 0};
        USER32_API(GetWindowRect);
        if (GetWindowRectFunc) {
            if (GetWindowRectFunc(hWnd, &windowRect) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the window geometry.")
                return HTNOWHERE;
            }
        } else {
            OutputDebugStringW(L"GetWindowRect() is not available.");
            return HTNOWHERE;
        }
        const auto windowWidth = static_cast<LONG>(std::abs(windowRect.right - windowRect.left));
        const auto resizeBorderThicknessX = static_cast<LONG>(Utils::GetResizeBorderThickness(hWnd, true));
        const auto resizeBorderThicknessY = static_cast<LONG>(Utils::GetResizeBorderThickness(hWnd, false));
        const auto captionHeight = static_cast<LONG>(Utils::GetCaptionHeight(hWnd));
        bool isTitleBar = false;
        if (max || full) {
            isTitleBar = ((windowPos.y >= 0) && (windowPos.y <= captionHeight)
                          && (windowPos.x >= 0) && (windowPos.x <= windowWidth));
        } else if (normal) {
            isTitleBar = ((windowPos.y > resizeBorderThicknessY)
                          && (windowPos.y <= (resizeBorderThicknessY + captionHeight))
                          && (windowPos.x > resizeBorderThicknessX)
                          && (windowPos.x < (windowWidth - resizeBorderThicknessX)));
        }
        const bool isTop = (normal ? (windowPos.y <= resizeBorderThicknessY) : false);
        // This will handle the left, right and bottom parts of the frame
        // because we didn't change them.
        USER32_API(DefWindowProcW);
        if (DefWindowProcWFunc) {
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
            return HTNOWHERE;
        }
    } break;
    case WM_CLOSE: {
        if (Utils::CloseWindow(g_mainWindowHandle, g_mainWindowAtom)) {
            g_mainWindowHandle = nullptr;
            g_mainWindowAtom = INVALID_ATOM;
            return 0;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close the main window.");
        }
    } break;
    case WM_DESTROY: {
        g_backgroundBrush = nullptr;
        g_rootGrid = nullptr;
        if (g_source != nullptr) {
            g_source.Close();
            g_source = nullptr;
        }
        if (g_manager != nullptr) {
            g_manager.Close();
            g_manager = nullptr;
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
    case WM_SIZE: {
        if (g_xamlIslandWindowHandle) {
            const UINT width = LOWORD(lParam);
            const UINT height = HIWORD(lParam);
            if (!SyncXAMLIslandPosition(width, height)) {
                Utils::DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
            }
        }
    } break;
    case WM_SETTINGCHANGE: {
        if (!IsWindows10RS1OrGreater()) {
            break;
        }
        // wParam == 0: User-wide setting change
        // wParam == 1: System-wide setting change
        if (((wParam == 0) || (wParam == 1)) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            if (!Utils::RefreshWindowTheme(hWnd)) {
                Utils::DisplayErrorDialog(L"Failed to refresh the main window theme.");
            }
            if (!RefreshBackgroundBrush()) {
                Utils::DisplayErrorDialog(L"Failed to refresh the background brush.");
            }
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

[[nodiscard]] static inline bool RegisterMainWindowClass(LPCWSTR name) noexcept
{
    USER32_API(LoadCursorW);
    USER32_API(LoadIconW);
    USER32_API(RegisterClassExW);
    USER32_API(GetClassInfoExW);
    if (LoadCursorWFunc && LoadIconWFunc && RegisterClassExWFunc && GetClassInfoExWFunc) {
        bool exists = false;
        WNDCLASSEXW wcex = {};
        if (name) {
            exists = (GetClassInfoExWFunc(Utils::GetCurrentInstance(), name, &wcex) != FALSE);
        }
        if (!exists) {
            if (g_mainWindowAtom != INVALID_ATOM) {
                exists = (GetClassInfoExWFunc(Utils::GetCurrentInstance(), Utils::GetWindowClassName(g_mainWindowAtom), &wcex) != FALSE);
            }
        }
        LPCWSTR guid = nullptr;
        if (!exists) {
            guid = Utils::GenerateGUID();
            if (!guid) {
                OutputDebugStringW(L"Failed to register the window class due to can't generate a new GUID.");
                return false;
            }
            SecureZeroMemory(&wcex, sizeof(wcex));
            wcex.cbSize = sizeof(wcex);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = MessageHandler;
            wcex.hInstance = Utils::GetCurrentInstance();
            wcex.lpszClassName = (name ? name : guid);
            wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
            wcex.hIcon = LoadIconWFunc(Utils::GetCurrentInstance(), MAKEINTRESOURCEW(IDI_APPICON));
            wcex.hIconSm = LoadIconWFunc(Utils::GetCurrentInstance(), MAKEINTRESOURCEW(IDI_APPICON_SMALL));
        }
        g_mainWindowAtom = RegisterClassExWFunc(&wcex);
        if (guid) {
            delete [] guid;
            guid = nullptr;
        }
        if (g_mainWindowAtom == INVALID_ATOM) {
            PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW, L"Failed to register the window class for the main window.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"LoadCursorW(), LoadIconW(), RegisterClassExW() and GetClassInfoExW() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool CreateMainWindow(LPCWSTR title) noexcept
{
    USER32_API(CreateWindowExW);
    if (CreateWindowExWFunc) {
        if (g_mainWindowAtom == INVALID_ATOM) {
            OutputDebugStringW(L"Failed to create main window due to the main window ATOM is invalid.");
            return false;
        }
        g_mainWindowHandle = CreateWindowExWFunc(
            WS_EX_NOREDIRECTIONBITMAP,
            Utils::GetWindowClassName(g_mainWindowAtom),
            (title ? title : g_defaultWindowTitle),
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            nullptr, nullptr, Utils::GetCurrentInstance(), nullptr);
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

[[nodiscard]] static inline bool ShowMainWindow(const int nCmdShow) noexcept
{
    USER32_API(ShowWindow);
    USER32_API(UpdateWindow);
    if (ShowWindowFunc && UpdateWindowFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to show the main window due to the main window handle is null.");
            return false;
        }
        ShowWindowFunc(g_mainWindowHandle, nCmdShow);
        if (UpdateWindowFunc(g_mainWindowHandle) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UpdateWindow, L"Failed to update the main window.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"ShowWindow() and UpdateWindow() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool InitializeXAMLIsland() noexcept
{
    // The call to winrt::init_apartment() initializes COM. By default, in a multithreaded apartment.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // Initialize the XAML framework's core window for the current thread.
    g_manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    if (g_manager == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to initialize the Windows XAML Manager for the current thread.");
        return false;
    }
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    g_source = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
    if (g_source == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the Desktop Window XAML Source.");
        return false;
    }
    // Get handle to the core window.
    const auto interop = g_source.as<IDesktopWindowXamlSourceNative>();
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
    if (g_rootGrid == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the XAML Grid.");
        return false;
    }
    g_backgroundBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
    if (g_backgroundBrush == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the XAML AcrylicBrush");
        return false;
    }
    if (!RefreshBackgroundBrush()) {
        Utils::DisplayErrorDialog(L"Failed to refresh the background brush.");
        return false;
    }
    g_backgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_rootGrid.Background(g_backgroundBrush);
    //g_rootGrid.Children().Clear();
    //g_rootGrid.Children().Append(/* some UWP control */);
    //g_rootGrid.UpdateLayout();
    g_source.Content(g_rootGrid);
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
            if (g_source != nullptr) {
                const auto interop2 = g_source.as<IDesktopWindowXamlSourceNative2>();
                if (interop2) {
                    const HRESULT hr = interop2->PreTranslateMessage(&msg, &filtered);
                    if (FAILED(hr)) {
                        PRINT_HR_ERROR_MESSAGE(PreTranslateMessage, hr, L"Failed to pre-translate win32 messages.")
                    }
                } else {
                    OutputDebugStringW(L"Failed to retrieve the IDesktopWindowXamlSourceNative2.");
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

int AcrylicApplication::Main(const int nCmdShow) noexcept
{
    if (!IsWindows1019H1OrGreater()) {
        Utils::DisplayErrorDialog(L"This application only supports Windows 10 19H1 and onwards.");
        return -1;
    }

    if (!RegisterMainWindowClass(nullptr)) {
        Utils::DisplayErrorDialog(L"Failed to register the main window class.");
        return -1;
    }

    if (!CreateMainWindow(nullptr)) {
        Utils::DisplayErrorDialog(L"Failed to create the main window.");
        return -1;
    }

    if (!Utils::RefreshWindowTheme(g_mainWindowHandle)) {
        Utils::DisplayErrorDialog(L"Failed to refresh the main window theme.");
        return -1;
    }

    if (!InitializeXAMLIsland()) {
        Utils::DisplayErrorDialog(L"Failed to initialize the XAML Island.");
        return -1;
    }

    if (!ShowMainWindow(nCmdShow)) {
        Utils::DisplayErrorDialog(L"Failed to show the main window.");
        return -1;
    }

    return MessageLoop();
}
