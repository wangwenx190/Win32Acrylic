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

#include "acrylicbrush_winui2.h"
#include "resource.h"
#include "customframe.h"
#include "utils.h"

#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

static const std::wstring g_mainWindowClassNameSuffix = L"@WinUI2MainWindow";
static const std::wstring g_dragBarWindowClassNameSuffix = L"@WinUI2DragBarWindow";
static const std::wstring g_mainWindowTitle = L"AcrylicManager WinUI2 Main Window";
static const std::wstring g_dragBarWindowTitle = nullptr;

static std::wstring g_mainWindowClassName = nullptr;
static std::wstring g_dragBarWindowClassName = nullptr;
static ATOM g_mainWindowAtom = 0;
static ATOM g_dragBarWindowAtom = 0;
static HWND g_mainWindowHandle = nullptr;
static HWND g_XAMLIslandWindowHandle = nullptr;
static HWND g_dragBarWindowHandle = nullptr;
static UINT g_currentDpi = 0;
static double g_currentDpr = 0.0;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_manager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_source = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_backgroundBrush = nullptr;

int AcrylicBrush_WinUI2::m_refCount = 0;

static inline void Cleanup()
{
    if (g_backgroundBrush) {
        g_backgroundBrush = nullptr;
    }
    if (g_rootGrid) {
        g_rootGrid = nullptr;
    }
    if (g_source) {
        g_source.Close();
        g_source = nullptr;
    }
    if (g_manager) {
        g_manager.Close();
        g_manager = nullptr;
    }
    if (g_XAMLIslandWindowHandle) {
        g_XAMLIslandWindowHandle = nullptr;
    }
    if (g_dragBarWindowHandle) {
        DestroyWindow(g_dragBarWindowHandle);
        g_dragBarWindowHandle = nullptr;
    }
    if (g_dragBarWindowAtom != 0) {
        UnregisterClassW(g_dragBarWindowClassName.c_str(), GET_CURRENT_INSTANCE);
        g_dragBarWindowAtom = 0;
        g_dragBarWindowClassName.clear();
    }
    if (g_mainWindowHandle) {
        DestroyWindow(g_mainWindowHandle);
        g_mainWindowHandle = nullptr;
    }
    if (g_mainWindowAtom != 0) {
        UnregisterClassW(g_mainWindowClassName.c_str(), GET_CURRENT_INSTANCE);
        g_mainWindowAtom = 0;
        g_mainWindowClassName.clear();
    }
    if (g_currentDpi != 0) {
        g_currentDpi = 0;
    }
    if (g_currentDpr != 0.0) {
        g_currentDpr = 0.0;
    }
}

EXTERN_C LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    SecureZeroMemory(&msg, sizeof(msg));
    msg.hwnd = hWnd;
    msg.message = message;
    msg.wParam = wParam;
    msg.lParam = lParam;
    const DWORD pos = GetMessagePos();
    msg.pt = {GET_X_LPARAM(pos), GET_Y_LPARAM(pos)};
    msg.time = GetMessageTime();
    LRESULT result = 0;
    if (CustomFrame::HandleWindowProc(&msg, &result)) {
        return result;
    }

    bool themeChanged = false;

    switch (message) {
    case WM_DPICHANGED: {
        const auto x = static_cast<double>(LOWORD(wParam));
        const auto y = static_cast<double>(HIWORD(wParam));
        g_currentDpi = std::round((x + y) / 2.0);
        g_currentDpr = Utils::GetDevicePixelRatioForWindow(hWnd);
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (MoveWindow(hWnd, prcNewWindow->left, prcNewWindow->top,
                       GET_RECT_WIDTH(*prcNewWindow), GET_RECT_HEIGHT(*prcNewWindow), TRUE) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(MoveWindow)
            break;
        }
        return 0;
    } break;
    case WM_SIZE: {
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED) || IsFullScreened(hWnd)) {
            if (!Utils::UpdateFrameMargins(hWnd)) {
                OutputDebugStringW(L"WM_SIZE: Failed to update frame margins.");
                break;
            }
        }
        const auto width = LOWORD(lParam);
        const UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER;
        if (g_XAMLIslandWindowHandle) {
            // Give enough space to our thin homemade top border.
            const int borderThickness = Utils::GetWindowVisibleFrameBorderThickness(hWnd);
            const int height = (HIWORD(lParam) - borderThickness);
            if (SetWindowPos(g_XAMLIslandWindowHandle, HWND_BOTTOM, 0, borderThickness,
                             width, height, flags) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
                break;
            }
        }
        if (g_dragBarWindowHandle) {
            const int titleBarHeight = Utils::GetTitleBarHeight(hWnd);
            if (SetWindowPos(g_dragBarWindowHandle, HWND_TOP, 0, 0, width, titleBarHeight, flags) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
                break;
            }
        }
    } break;
    case WM_SETFOCUS: {
        if (g_XAMLIslandWindowHandle) {
            // Send focus to the XAML Island child window.
            if (SetFocus(g_XAMLIslandWindowHandle) == nullptr) {
                PRINT_WIN32_ERROR_MESSAGE(SetFocus)
                break;
            }
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
    case WM_SETTINGCHANGE: {
        if ((wParam == 0) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            themeChanged = true;
        }
    } break;
    case WM_THEMECHANGED:
    case WM_DWMCOLORIZATIONCOLORCHANGED:
        themeChanged = true;
        break;
    case WM_DWMCOMPOSITIONCHANGED: {
        if (!Utils::IsCompositionEnabled()) {
            OutputDebugStringW(L"AcrylicManager won't be functional when DWM composition is disabled.");
            std::exit(-1);
        }
    } break;
    case WM_CLOSE: {
        Cleanup();
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }

    if (themeChanged && g_backgroundBrush && !Utils::IsHighContrastModeEnabled()) {
        const auto window = reinterpret_cast<AcrylicBrush_WinUI2 *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        if (window) {
            window->ReloadBlurParameters();
        }
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

EXTERN_C LRESULT CALLBACK DragBarWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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

    if (nonClientMessage.has_value() && g_mainWindowHandle)
    {
        POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (ClientToScreen(hWnd, &pos) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(ClientToScreen)
            return 0;
        }
        const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
        // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
        // then pass that click through as an NC click in that location.
        const LRESULT hitTestResult = SendMessageW(g_mainWindowHandle, WM_NCHITTEST, 0, newLParam);
        SendMessageW(g_mainWindowHandle, nonClientMessage.value(), hitTestResult, newLParam);
        return 0;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

bool AcrylicBrush_WinUI2::RegisterMainWindowClass() const
{
    g_mainWindowClassName = m_windowClassNamePrefix + Utils::GenerateGUID() + g_mainWindowClassNameSuffix;

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindowProc;
    wcex.hInstance = GET_CURRENT_INSTANCE;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIconW(GET_CURRENT_INSTANCE, MAKEINTRESOURCEW(IDI_DEFAULTICON));
    wcex.hIconSm = LoadIconW(GET_CURRENT_INSTANCE, MAKEINTRESOURCEW(IDI_DEFAULTICONSM));
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = g_mainWindowClassName.c_str();

    g_mainWindowAtom = RegisterClassExW(&wcex);

    if (g_mainWindowAtom == 0) {
        PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW)
        return false;
    }

    return true;
}

bool AcrylicBrush_WinUI2::CreateMainWindow() const
{
    g_mainWindowHandle = CreateWindowExW(0L,
                                         g_mainWindowClassName.c_str(),
                                         g_mainWindowTitle.c_str(),
                                         WS_OVERLAPPEDWINDOW,
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                         nullptr, nullptr, GET_CURRENT_INSTANCE, nullptr);

    if (!g_mainWindowHandle) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW)
        return false;
    }

    g_currentDpi = Utils::GetDotsPerInchForWindow(g_mainWindowHandle);
    if (g_currentDpi <= 0) {
        g_currentDpi = USER_DEFAULT_SCREEN_DPI;
    }
    g_currentDpr = Utils::GetDevicePixelRatioForWindow(g_mainWindowHandle);
    if (g_currentDpr <= 0.0) {
        g_currentDpr = 1.0;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (!Utils::UpdateFrameMargins(g_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to update main window's frame margins.");
        return false;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (!Utils::TriggerFrameChangeForWindow(g_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to trigger frame change event for main window.");
        return false;
    }
    // Ensure our window still has window transitions.
    if (!Utils::SetWindowTransitionsEnabled(g_mainWindowHandle, true)) {
        OutputDebugStringW(L"Failed to enable window transitions for main window.");
        return false;
    }

    return true;
}

bool AcrylicBrush_WinUI2::RegisterDragBarWindowClass() const
{
    if (!Utils::IsWindows8OrGreater()) {
        OutputDebugStringW(L"Drag bar window is only available on Windows 8 and onwards.");
        return false;
    }

    g_dragBarWindowClassName = m_windowClassNamePrefix + Utils::GenerateGUID() + g_dragBarWindowClassNameSuffix;

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = DragBarWindowProc;
    wcex.hInstance = GET_CURRENT_INSTANCE;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = GET_BLACK_BRUSH;
    wcex.lpszClassName = g_dragBarWindowClassName.c_str();

    g_dragBarWindowAtom = RegisterClassExW(&wcex);

    if (g_dragBarWindowAtom == 0) {
        PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW)
        return false;
    }

    return true;
}

bool AcrylicBrush_WinUI2::CreateDragBarWindow() const
{
    // Please refer to the "IMPORTANT NOTE" section below.
    if (!Utils::IsWindows8OrGreater()) {
        OutputDebugStringW(L"Drag bar window is only available on Windows 8 and onwards.");
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
    g_dragBarWindowHandle = CreateWindowExW(WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP,
                                            g_dragBarWindowClassName.c_str(),
                                            g_dragBarWindowTitle.c_str(),
                                            WS_CHILD,
                                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                            g_mainWindowHandle, nullptr, GET_CURRENT_INSTANCE, nullptr);

    if (!g_dragBarWindowHandle) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW)
        return false;
    }

    // Layered window won't be visible until we call SetLayeredWindowAttributes()
    // or UpdateLayeredWindow().
    if (SetLayeredWindowAttributes(g_dragBarWindowHandle, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetLayeredWindowAttributes)
        return false;
    }

    const SIZE size = GET_WINDOW_CLIENT_SIZE(g_mainWindowHandle);
    const int titleBarHeight = Utils::GetTitleBarHeight(g_mainWindowHandle);
    if (SetWindowPos(g_dragBarWindowHandle, HWND_TOP, 0, 0, size.cx, titleBarHeight,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
        return false;
    }

    return true;
}

bool AcrylicBrush_WinUI2::CreateXAMLIsland() const
{
    // XAML Island is only supported on Windows 10 19H1 and onwards.
    if (!Utils::IsWindows1019H1OrGreater()) {
        OutputDebugStringW(L"XAML Island is only supported on Windows 10 19H1 and onwards.");
        return false;
    }

    if (Utils::IsHighContrastModeEnabled()) {
        OutputDebugStringW(L"AcrylicManager won't be functional when high contrast mode is on.");
        return false;
    }

    winrt::init_apartment(winrt::apartment_type::single_threaded);
    g_manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();

    g_source = {};
    const auto interop = g_source.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        OutputDebugStringW(L"Failed to retrieve IDesktopWindowXamlSourceNative.");
        return false;
    }
    winrt::check_hresult(interop->AttachToWindow(g_mainWindowHandle));
    winrt::check_hresult(interop->get_WindowHandle(&g_XAMLIslandWindowHandle));
    if (!g_XAMLIslandWindowHandle) {
        OutputDebugStringW(L"Failed to retrieve XAML Island window handle.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    // And give enough space to our thin homemade top border.
    const int borderThickness = Utils::GetWindowVisibleFrameBorderThickness(g_mainWindowHandle);
    const SIZE size = GET_WINDOW_CLIENT_SIZE(g_mainWindowHandle);
    if (SetWindowPos(g_XAMLIslandWindowHandle, HWND_BOTTOM, 0,
                     borderThickness, size.cx, (size.cy - borderThickness),
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
        return false;
    }
    g_backgroundBrush = {};
    g_backgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    ReloadBlurParameters();
    g_rootGrid = {};
    g_rootGrid.Background(g_backgroundBrush);
    //g_rootGrid.Children().Clear();
    //g_rootGrid.Children().Append(/* some UWP control */);
    //g_rootGrid.UpdateLayout();
    g_source.Content(g_rootGrid);

    return true;
}

AcrylicBrush_WinUI2::AcrylicBrush_WinUI2()
{
    ++m_refCount;
}

AcrylicBrush_WinUI2::~AcrylicBrush_WinUI2()
{
    --m_refCount;
}

bool AcrylicBrush_WinUI2::IsSupportedByCurrentOS() const
{
    static const bool result = Utils::IsWindows1019H1OrGreater();
    return result;
}

bool AcrylicBrush_WinUI2::Create() const
{
    if (!RegisterMainWindowClass()) {
        return false;
    }
    if (!CreateMainWindow()) {
        return false;
    }
    if (!RegisterDragBarWindowClass()) {
        return false;
    }
    if (!CreateDragBarWindow()) {
        return false;
    }
    if (!CreateXAMLIsland()) {
        return false;
    }
    return true;
}

void AcrylicBrush_WinUI2::ReloadBlurParameters() const
{
    if (!g_backgroundBrush) {
        return;
    }
    g_backgroundBrush.TintColor(GetTintColor());
    g_backgroundBrush.TintOpacity(GetTintOpacity());
    g_backgroundBrush.TintLuminosityOpacity(GetLuminosityOpacity());
    g_backgroundBrush.FallbackColor(GetFallbackColor());
}

HWND AcrylicBrush_WinUI2::GetWindowHandle() const
{
    return g_mainWindowHandle;
}

int AcrylicBrush_WinUI2::EventLoopExec() const
{
    MSG msg = {};

    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        BOOL filtered = FALSE;
        if (g_source) {
            const auto interop = g_source.as<IDesktopWindowXamlSourceNative2>();
            if (interop) {
                winrt::check_hresult(interop->PreTranslateMessage(&msg, &filtered));
            }
        }
        if (filtered == FALSE) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

void AcrylicBrush_WinUI2::Release()
{
    --m_refCount;
    if (m_refCount <= 0) {
        m_refCount = 0;
        delete this;
    }
}

#if 0
static const auto invalidReason = static_cast<winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason>(-1);

[[nodiscard]] static inline HRESULT am_GetReasonFromKey_p(const WPARAM key,
                       winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason *result)
{
    if (!key || !result) {
        return E_INVALIDARG;
    }
    auto reason = invalidReason;
    if (key == VK_TAB) {
        byte keyboardState[256];
        SecureZeroMemory(keyboardState, sizeof(keyboardState));
        if (GetKeyboardState(keyboardState) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetKeyboardState)
        }
        reason = ((keyboardState[VK_SHIFT] & 0x80) ?
            winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Last :
            winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::First);
    } else if (key == VK_LEFT) {
        reason = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Left;
    } else if (key == VK_RIGHT) {
        reason = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Right;
    } else if (key == VK_UP) {
        reason = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Up;
    } else if (key == VK_DOWN) {
        reason = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Down;
    }
    *result = reason;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetNextFocusedIsland_p(const MSG *msg,
            const std::vector<winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource> &sources,
                                winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource *result)
{
    if (!msg || sources.empty() || !result) {
        return E_INVALIDARG;
    }
    if (msg->message == WM_KEYDOWN) {
        winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason reason = invalidReason;
        if (FAILED(am_GetReasonFromKey_p(msg->wParam, &reason))) {
            return E_FAIL;
        }
        if (reason != invalidReason) {
            const BOOL previous =
                ((reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::First) ||
                    (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Down) ||
                    (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Right)) ? FALSE : TRUE;
            const HWND currentFocusedWindow = GetFocus();
            const HWND nextElement = GetNextDlgTabItem(g_am_MainWindowHandle_p, currentFocusedWindow, previous);
            for (auto &&source : std::as_const(sources)) {
                HWND islandWnd = nullptr;
                const auto interop = source.as<IDesktopWindowXamlSourceNative>();
                if (!interop) {
                    return E_FAIL;
                }
                winrt::check_hresult(interop->get_WindowHandle(&islandWnd));
                if (nextElement == islandWnd) {
                    *result = source;
                    return S_OK;
                }
            }
        }
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_GetFocusedIsland_p(
        const std::vector<winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource> &sources,
                                winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource *result)
{
    if (sources.empty() || !result) {
        return E_INVALIDARG;
    }
    for (auto &&source : std::as_const(sources)) {
        if (source.HasFocus()) {
            *result = source;
            return S_OK;
        }
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_NavigateFocus_p(MSG *msg, bool *ret)
{
    if (const auto nextFocusedIsland = am_GetNextFocusedIsland_p(msg)) {
        WINRT_VERIFY(!nextFocusedIsland.HasFocus());
        const HWND previousFocusedWindow = GetFocus();
        RECT rect = {};
        WINRT_VERIFY(GetWindowRect(previousFocusedWindow, &rect));
        const auto nativeIsland = nextFocusedIsland.as<IDesktopWindowXamlSourceNative>();
        HWND islandWnd{};
        winrt::check_hresult(nativeIsland->get_WindowHandle(&islandWnd));
        POINT pt = { rect.left, rect.top };
        SIZE size = { rect.right - rect.left, rect.bottom - rect.top };
        ::ScreenToClient(islandWnd, &pt);
        const auto hintRect = winrt::Windows::Foundation::Rect({ static_cast<float>(pt.x), static_cast<float>(pt.y), static_cast<float>(size.cx), static_cast<float>(size.cy) });
        const auto reason = am_GetReasonFromKey_p(msg->wParam);
        const auto request = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationRequest(reason, hintRect);
        m_lastFocusRequestId = request.CorrelationId();
        const auto result = nextFocusedIsland.NavigateFocus(request);
        *ret = result.WasFocusMoved();
        return S_OK;
    } else {
        const bool islandIsFocused = am_GetFocusedIsland()_p != nullptr;
        byte keyboardState[256];
        SecureZeroMemory(keyboardState, sizeof(keyboardState));
        WINRT_VERIFY(GetKeyboardState(keyboardState));
        const bool isMenuModifier = (keyboardState[VK_MENU] & 0x80);
        if (islandIsFocused && !isMenuModifier) {
            *ret = false;
            return S_OK;
        }
        const bool isDialogMessage = !!IsDialogMessageW(g_am_MainWindowHandle_p, msg);
        *ret = isDialogMessage;
        return S_OK;
    }
    return E_FAIL;
}

static const WPARAM invalidKey = (WPARAM)-1;

WPARAM GetKeyFromReason(winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason reason)
{
    auto key = invalidKey;
    if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Last || reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::First)
    {
        key = VK_TAB;
    }
    else if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Left)
    {
        key = VK_LEFT;
    }
    else if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Right)
    {
        key = VK_RIGHT;
    }
    else if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Up)
    {
        key = VK_UP;
    }
    else if (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Down)
    {
        key = VK_DOWN;
    }
    return key;
}

void DesktopWindow::OnTakeFocusRequested(winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource const& sender, winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSourceTakeFocusRequestedEventArgs const& args)
{
    if (args.Request().CorrelationId() != m_lastFocusRequestId)
    {
        const auto reason = args.Request().Reason();
        const BOOL previous =
            (reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::First ||
                reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Down ||
                reason == winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Right) ? false : true;

        const auto nativeXamlSource = sender.as<IDesktopWindowXamlSourceNative>();
        HWND senderHwnd = nullptr;
        winrt::check_hresult(nativeXamlSource->get_WindowHandle(&senderHwnd));

        MSG msg = {};
        msg.hwnd = senderHwnd;
        msg.message = WM_KEYDOWN;
        msg.wParam = GetKeyFromReason(reason);
        if (!NavigateFocus(&msg))
        {
            const auto nextElement = ::GetNextDlgTabItem(m_window.get(), senderHwnd, previous);
            ::SetFocus(nextElement);
        }
    }
    else
    {
        const auto request = winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationRequest(winrt::Windows::UI::Xaml::Hosting::XamlSourceFocusNavigationReason::Restore);
        m_lastFocusRequestId = request.CorrelationId();
        sender.NavigateFocus(request);
    }
}

winrt::Windows::UI::Xaml::UIElement LoadXamlControl(uint32_t id)
{
    auto rc = ::FindResourceW(nullptr, MAKEINTRESOURCE(id), MAKEINTRESOURCE(XAMLRESOURCE));
    THROW_LAST_ERROR_IF(!rc);

    HGLOBAL rcData = ::LoadResource(nullptr, rc);
    THROW_LAST_ERROR_IF(!rcData);

    auto data = static_cast<wchar_t*>(::LockResource(rcData));
    auto content = winrt::Windows::UI::Xaml::Markup::XamlReader::Load(data);
    return content.as<winrt::Windows::UI::Xaml::UIElement>();
}
#endif
