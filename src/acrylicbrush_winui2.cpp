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
#include "customframe.h"
#include "utils.h"

#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

static bool g_winRTInitialized = false;
static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_manager = nullptr;

class AcrylicBrushWinUI2Private
{
    ACRYLICMANAGER_DISABLE_COPY_MOVE(AcrylicBrushWinUI2Private)

public:
    explicit AcrylicBrushWinUI2Private(AcrylicBrushWinUI2 *q);
    ~AcrylicBrushWinUI2Private();

    static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK DragBarWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    [[nodiscard]] bool CreateMainWindow();
    [[nodiscard]] bool CreateDragBarWindow();
    [[nodiscard]] bool CreateXAMLIsland();
    [[nodiscard]] HWND GetMainWindowHandle() const;
    [[nodiscard]] int EventLoop() const;
    void ReloadBrushParameters();
    void Cleanup();

private:
    [[nodiscard]] LRESULT MainWindowMessageHandler(UINT message, WPARAM wParam, LPARAM lParam);
    [[nodiscard]] LRESULT DragBarWindowMessageHandler(UINT message, WPARAM wParam, LPARAM lParam);

private:
    AcrylicBrushWinUI2 *q_ptr = nullptr;

    std::wstring m_mainWindowClassName = nullptr;
    std::wstring m_dragBarWindowClassName = nullptr;
    HWND m_mainWindowHandle = nullptr;
    HWND m_XAMLIslandWindowHandle = nullptr;
    HWND m_dragBarWindowHandle = nullptr;
    UINT m_currentDpi = 0;
    double m_currentDpr = 0.0;

    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource m_source = nullptr;
    winrt::Windows::UI::Xaml::Controls::Grid m_rootGrid = nullptr;
    winrt::Windows::UI::Xaml::Media::AcrylicBrush m_backgroundBrush = nullptr;
};

AcrylicBrushWinUI2Private::AcrylicBrushWinUI2Private(AcrylicBrushWinUI2 *q)
{
    q_ptr = q;
}

AcrylicBrushWinUI2Private::~AcrylicBrushWinUI2Private()
{
}

LRESULT CALLBACK AcrylicBrushWinUI2Private::MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE) {
        const auto cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        const auto that = static_cast<AcrylicBrushWinUI2Private *>(cs->lpCreateParams);
        if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
        }
    } else if (message == WM_NCDESTROY) {
        if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
        }
    } else if (const auto that = reinterpret_cast<AcrylicBrushWinUI2Private *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
        return that->MainWindowMessageHandler(message, wParam, lParam);
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK AcrylicBrushWinUI2Private::DragBarWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE) {
        const auto cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        const auto that = static_cast<AcrylicBrushWinUI2Private *>(cs->lpCreateParams);
        if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
        }
    } else if (message == WM_NCDESTROY) {
        if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
        }
    } else if (const auto that = reinterpret_cast<AcrylicBrushWinUI2Private *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
        return that->DragBarWindowMessageHandler(message, wParam, lParam);
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

HWND AcrylicBrushWinUI2Private::GetMainWindowHandle() const
{
    return m_mainWindowHandle;
}

int AcrylicBrushWinUI2Private::EventLoop() const
{
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        BOOL filtered = FALSE;
        if (m_source) {
            const auto interop = m_source.as<IDesktopWindowXamlSourceNative2>();
            if (interop) {
                winrt::check_hresult(interop->PreTranslateMessage(&msg, &filtered));
            } else {
                OutputDebugStringW(L"Failed to retrieve IDesktopWindowXamlSourceNative2.");
            }
        }
        if (filtered == FALSE) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}

void AcrylicBrushWinUI2Private::ReloadBrushParameters()
{
    if (!m_backgroundBrush) {
        return;
    }
    m_backgroundBrush.TintColor(q_ptr->GetEffectiveTintColor());
    m_backgroundBrush.TintOpacity(q_ptr->GetTintOpacity());
    m_backgroundBrush.TintLuminosityOpacity(q_ptr->GetLuminosityOpacity());
    m_backgroundBrush.FallbackColor(q_ptr->GetFallbackColor());
}

void AcrylicBrushWinUI2Private::Cleanup()
{
    if (m_backgroundBrush) {
        m_backgroundBrush = nullptr;
    }
    if (m_rootGrid) {
        m_rootGrid = nullptr;
    }
    if (m_source) {
        m_source.Close();
        m_source = nullptr;
    }
    if (m_XAMLIslandWindowHandle) {
        m_XAMLIslandWindowHandle = nullptr;
    }
    if (m_dragBarWindowHandle) {
        DestroyWindow(m_dragBarWindowHandle);
        m_dragBarWindowHandle = nullptr;
    }
    if (!m_dragBarWindowClassName.empty()) {
        UnregisterClassW(m_dragBarWindowClassName.c_str(), GET_CURRENT_INSTANCE);
        m_dragBarWindowClassName.clear();
    }
    if (m_mainWindowHandle) {
        DestroyWindow(m_mainWindowHandle);
        m_mainWindowHandle = nullptr;
    }
    if (!m_mainWindowClassName.empty()) {
        UnregisterClassW(m_mainWindowClassName.c_str(), GET_CURRENT_INSTANCE);
        m_mainWindowClassName.clear();
    }
    if (m_currentDpi != 0) {
        m_currentDpi = 0;
    }
    if (m_currentDpr != 0.0) {
        m_currentDpr = 0.0;
    }
}

bool AcrylicBrushWinUI2Private::CreateMainWindow()
{
    m_mainWindowClassName = Utils::RegisterWindowClass(MainWindowProc);
    if (m_mainWindowClassName.empty()) {
        OutputDebugStringW(L"Failed to register the main window class.");
        return false;
    }

    m_mainWindowHandle = CreateWindowExW(0L,
                                         m_mainWindowClassName.c_str(),
                                         g_mainWindowTitle,
                                         WS_OVERLAPPEDWINDOW,
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                         nullptr,
                                         nullptr,
                                         GET_CURRENT_INSTANCE,
                                         this);

    if (!m_mainWindowHandle) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW)
        return false;
    }

    m_currentDpi = Utils::GetDotsPerInchForWindow(m_mainWindowHandle);
    if (m_currentDpi <= 0) {
        m_currentDpi = USER_DEFAULT_SCREEN_DPI;
    }
    m_currentDpr = Utils::GetDevicePixelRatioForWindow(m_mainWindowHandle);
    if (m_currentDpr <= 0.0) {
        m_currentDpr = 1.0;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (!Utils::UpdateFrameMargins(m_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to update main window's frame margins.");
        return false;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (!Utils::TriggerFrameChangeForWindow(m_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to trigger frame change event for main window.");
        return false;
    }
    // Ensure our window still has window transitions.
    if (!Utils::SetWindowTransitionsEnabled(m_mainWindowHandle, true)) {
        OutputDebugStringW(L"Failed to enable window transitions for main window.");
        return false;
    }

    return true;
}

bool AcrylicBrushWinUI2Private::CreateDragBarWindow()
{
    // Please refer to the "IMPORTANT NOTE" section below.
    if (!Utils::IsWindows8OrGreater()) {
        OutputDebugStringW(L"Drag bar window is only available on Windows 8 and onwards.");
        return false;
    }

    m_dragBarWindowClassName = Utils::RegisterWindowClass(DragBarWindowProc);
    if (m_dragBarWindowClassName.empty()) {
        OutputDebugStringW(L"Failed to register the drag bar window class.");
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
    m_dragBarWindowHandle = CreateWindowExW(WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP,
                                            m_dragBarWindowClassName.c_str(),
                                            g_dragBarWindowTitle,
                                            WS_CHILD,
                                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                            m_mainWindowHandle,
                                            nullptr,
                                            GET_CURRENT_INSTANCE,
                                            this);

    if (!m_dragBarWindowHandle) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW)
        return false;
    }

    // Layered window won't be visible until we call SetLayeredWindowAttributes()
    // or UpdateLayeredWindow().
    if (SetLayeredWindowAttributes(m_dragBarWindowHandle, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetLayeredWindowAttributes)
        return false;
    }

    const SIZE size = GET_WINDOW_CLIENT_SIZE(m_mainWindowHandle);
    const int titleBarHeight = Utils::GetTitleBarHeight(m_mainWindowHandle);
    if (SetWindowPos(m_dragBarWindowHandle, HWND_TOP, 0, 0, size.cx, titleBarHeight,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
        return false;
    }

    return true;
}

bool AcrylicBrushWinUI2Private::CreateXAMLIsland()
{
    // XAML Island is only supported on Windows 10 RS5 and onwards.
    if (!Utils::IsWindows10RS5OrGreater()) {
        OutputDebugStringW(L"XAML Island is only supported on Windows 10 RS5 and onwards.");
        return false;
    }

    if (Utils::IsHighContrastModeEnabled()) {
        OutputDebugStringW(L"AcrylicManager won't be functional when high contrast mode is on.");
        return false;
    }

    if (!g_winRTInitialized) {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
    }
    if (!g_manager) {
        g_manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    }

    m_source = {};
    const auto interop = m_source.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        OutputDebugStringW(L"Failed to retrieve IDesktopWindowXamlSourceNative.");
        return false;
    }
    winrt::check_hresult(interop->AttachToWindow(m_mainWindowHandle));
    winrt::check_hresult(interop->get_WindowHandle(&m_XAMLIslandWindowHandle));
    if (!m_XAMLIslandWindowHandle) {
        OutputDebugStringW(L"Failed to retrieve XAML Island window handle.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    // And give enough space to our thin homemade top border.
    const int borderThickness = Utils::GetWindowVisibleFrameBorderThickness(m_mainWindowHandle);
    const SIZE size = GET_WINDOW_CLIENT_SIZE(m_mainWindowHandle);
    if (SetWindowPos(m_XAMLIslandWindowHandle, HWND_BOTTOM, 0,
                     borderThickness, size.cx, (size.cy - borderThickness),
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
        return false;
    }
    m_backgroundBrush = {};
    m_backgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    ReloadBrushParameters();
    m_rootGrid = {};
    m_rootGrid.Background(m_backgroundBrush);
    //m_rootGrid.Children().Clear();
    //m_rootGrid.Children().Append(/* some UWP control */);
    //m_rootGrid.UpdateLayout();
    m_source.Content(m_rootGrid);

    return true;
}

LRESULT AcrylicBrushWinUI2Private::MainWindowMessageHandler(UINT message, WPARAM wParam, LPARAM lParam)
{
    bool themeChanged = false;

    switch (message) {
    case WM_SIZE: {
        const auto width = LOWORD(lParam);
        const UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER;
        if (m_XAMLIslandWindowHandle) {
            // Give enough space to our thin homemade top border.
            const int borderThickness = Utils::GetWindowVisibleFrameBorderThickness(m_mainWindowHandle);
            const int height = (HIWORD(lParam) - borderThickness);
            if (SetWindowPos(m_XAMLIslandWindowHandle, HWND_BOTTOM, 0, borderThickness,
                             width, height, flags) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
                break;
            }
        }
        if (m_dragBarWindowHandle) {
            const int titleBarHeight = Utils::GetTitleBarHeight(m_mainWindowHandle);
            if (SetWindowPos(m_dragBarWindowHandle, HWND_TOP, 0, 0, width, titleBarHeight, flags) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
                break;
            }
        }
    } break;
    case WM_SETFOCUS: {
        if (m_XAMLIslandWindowHandle) {
            // Send focus to the XAML Island child window.
            if (SetFocus(m_XAMLIslandWindowHandle) == nullptr) {
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
            const LRESULT hitTestResult = SendMessageW(m_mainWindowHandle, WM_NCHITTEST, 0, GetMessagePos());
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
    default:
        break;
    }

    if (themeChanged && m_backgroundBrush && !Utils::IsHighContrastModeEnabled()) {
        ReloadBrushParameters();
    }

    return DefWindowProcW(m_mainWindowHandle, message, wParam, lParam);
}

LRESULT AcrylicBrushWinUI2Private::DragBarWindowMessageHandler(UINT message, WPARAM wParam, LPARAM lParam)
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
    if (nonClientMessage.has_value() && m_mainWindowHandle) {
        POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (ClientToScreen(m_dragBarWindowHandle, &pos) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(ClientToScreen)
            return 0;
        }
        const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
        // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
        // then pass that click through as an NC click in that location.
        const LRESULT hitTestResult = SendMessageW(m_mainWindowHandle, WM_NCHITTEST, 0, newLParam);
        SendMessageW(m_mainWindowHandle, nonClientMessage.value(), hitTestResult, newLParam);
        return 0;
    }
    return DefWindowProcW(m_dragBarWindowHandle, message, wParam, lParam);
}

AcrylicBrushWinUI2::AcrylicBrushWinUI2()
{
    d_ptr = std::make_unique<AcrylicBrushWinUI2Private>(this);
}

AcrylicBrushWinUI2::~AcrylicBrushWinUI2()
{
}

bool AcrylicBrushWinUI2::IsSupportedByCurrentOS() const
{
    static const bool result = Utils::IsWindows10RS5OrGreater();
    return result;
}

bool AcrylicBrushWinUI2::Initialize() const
{
    if (!d_ptr->CreateMainWindow()) {
        OutputDebugStringW(L"Failed to create main window.");
        return false;
    }
    if (!d_ptr->CreateDragBarWindow()) {
        OutputDebugStringW(L"Failed to create drag bar window.");
        return false;
    }
    if (!d_ptr->CreateXAMLIsland()) {
        OutputDebugStringW(L"Failed to create XAML Island.");
        return false;
    }
    return true;
}

bool AcrylicBrushWinUI2::RefreshBrush() const
{
    d_ptr->ReloadBrushParameters();
    return true;
}

HWND AcrylicBrushWinUI2::GetWindowHandle() const
{
    return d_ptr->GetWindowHandle();
}

int AcrylicBrushWinUI2::MessageLoop() const
{
    return d_ptr->GetMessageLoopResult();
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