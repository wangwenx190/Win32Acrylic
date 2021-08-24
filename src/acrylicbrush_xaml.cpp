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

#include "acrylicbrush.h"
#include <Unknwn.h>
#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_am_XAMLManager_p = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_am_XAMLSource_p = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_am_RootGrid_p = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_am_BackgroundBrush_p = nullptr;

[[nodiscard]] static inline HRESULT am_CreateXAMLIslandHelper_p()
{
    if (!g_am_MainWindowHandle_p) {
        PRINT_AND_SAFE_RETURN(L"The main window has not been created.")
    }

    // XAML Island is only supported on Windows 10 19H1 and onwards.
    if (!g_am_IsXAMLIslandAvailable_p) {
        PRINT_AND_SAFE_RETURN(L"XAML Island is only supported on Windows 10 19H1 and onwards.")
    }
    SystemTheme systemTheme = SystemTheme::Invalid;
    if (FAILED(am_GetSystemThemeHelper_p(&systemTheme)) || (systemTheme == SystemTheme::Invalid)) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the system theme.")
    }
    if (systemTheme == SystemTheme::HighContrast) {
        PRINT_AND_SAFE_RETURN(L"AcrylicManager won't be functional when high contrast mode is on.")
    }

    winrt::init_apartment(winrt::apartment_type::single_threaded);
    g_am_XAMLManager_p = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();

    g_am_XAMLSource_p = {};
    const auto interop = g_am_XAMLSource_p.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve IDesktopWindowXamlSourceNative.")
    }
    winrt::check_hresult(interop->AttachToWindow(g_am_MainWindowHandle_p));
    winrt::check_hresult(interop->get_WindowHandle(&g_am_XAMLIslandWindowHandle_p));
    if (!g_am_XAMLIslandWindowHandle_p) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve XAML Island window handle.")
    }
    // Update the XAML Island window size because initially it is 0x0.
    // Give enough space to our thin homemade top border.
    int borderThickness = 0;
    if (FAILED(am_GetWindowVisibleFrameBorderThickness_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p, &borderThickness))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the window visible frame border thickness.")
    }
    SIZE size = {0, 0};
    if (FAILED(am_GetWindowClientSize_p(g_am_MainWindowHandle_p, &size))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the client area size of main window.")
    }
    if (SetWindowPos(g_am_XAMLIslandWindowHandle_p, HWND_BOTTOM, 0,
                 borderThickness, size.cx, (size.cy - borderThickness),
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN(SetWindowPos)
    }
    g_am_BackgroundBrush_p = {};
    if (FAILED(am_SwitchAcrylicBrushThemeHelper_p((systemTheme == SystemTheme::Auto) ? SystemTheme::Dark : systemTheme))) {
        PRINT_AND_SAFE_RETURN(L"Failed to change acrylic brush's theme.")
    }
    g_am_BrushTheme_p = SystemTheme::Auto;
    g_am_BackgroundBrush_p.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_am_RootGrid_p = {};
    g_am_RootGrid_p.Background(g_am_BackgroundBrush_p);
    //g_am_RootGrid_p.Children().Clear();
    //g_am_RootGrid_p.Children().Append(/* some UWP control */);
    //g_am_RootGrid_p.UpdateLayout();
    g_am_XAMLSource_p.Content(g_am_RootGrid_p);

    // Retrieve initial parameters of the acrylic brush.
    g_am_TintColor_p = g_am_BackgroundBrush_p.TintColor();
    g_am_TintOpacity_p = g_am_BackgroundBrush_p.TintOpacity();
    g_am_TintLuminosityOpacity_p = g_am_BackgroundBrush_p.TintLuminosityOpacity();
    g_am_FallbackColor_p = g_am_BackgroundBrush_p.FallbackColor();

    return S_OK;
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
