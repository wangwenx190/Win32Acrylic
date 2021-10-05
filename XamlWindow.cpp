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

#include "pch.h"
#include "XamlWindow.h"
#include "SystemLibraryManager.h"
#include "OperationResult.h"
#include "Utils.h"

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam) (static_cast<int>(static_cast<short>(LOWORD(lParam))))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam) (static_cast<int>(static_cast<short>(HIWORD(lParam))))
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

class XamlWindowPrivate
{
public:
    explicit XamlWindowPrivate(XamlWindow *q) noexcept;
    ~XamlWindowPrivate() noexcept;

    [[nodiscard]] bool RefreshWindowBackgroundBrush() noexcept;
    [[nodiscard]] bool SyncXamlIslandWindowGeometry() const noexcept;
    [[nodiscard]] bool SyncDragBarWindowGeometry() const noexcept;
    [[nodiscard]] bool SyncInternalWindowsGeometry() const noexcept;

    [[nodiscard]] bool MessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept;
    [[nodiscard]] bool FilterMessage(const MSG *msg) const noexcept;

private:
    [[nodiscard]] static LRESULT CALLBACK DragBarWindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept;
    [[nodiscard]] bool DragBarMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept;
    [[nodiscard]] bool InitializeXamlIsland() noexcept;
    [[nodiscard]] bool InitializeDragBarWindow() noexcept;

private:
    XamlWindowPrivate(const XamlWindowPrivate &) = delete;
    XamlWindowPrivate &operator=(const XamlWindowPrivate &) = delete;
    XamlWindowPrivate(XamlWindowPrivate &&) = delete;
    XamlWindowPrivate &operator=(XamlWindowPrivate &&) = delete;

private:
    XamlWindow *q_ptr = nullptr;
    std::wstring m_dragBarWindowClassName = {};
    HWND m_dragBarWindow = nullptr;
    HWND m_xamlIslandWindow = nullptr;
    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource m_xamlSource = nullptr;
    winrt::Windows::UI::Xaml::Controls::Grid m_windowRootGrid = nullptr;
    winrt::Windows::UI::Xaml::Media::AcrylicBrush m_windowBackgroundBrush = nullptr;
    winrt::Windows::UI::Xaml::Controls::Button m_windowIconButton = nullptr;
    winrt::Windows::UI::Xaml::Controls::Button m_minimizeButton = nullptr;
    winrt::Windows::UI::Xaml::Controls::Button m_maximizeButton = nullptr;
    winrt::Windows::UI::Xaml::Controls::Button m_closeButton = nullptr;
};

XamlWindowPrivate::XamlWindowPrivate(XamlWindow *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"XamlWindowPrivate's q is null.");
        return;
    }
    q_ptr = q;
    if (InitializeXamlIsland()) {
        if (!InitializeDragBarWindow()) {
            Utils::DisplayErrorDialog(L"Failed to initialize the drag bar window.");
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to initialize the XAML Island.");
    }
}

XamlWindowPrivate::~XamlWindowPrivate() noexcept
{
    if (m_xamlSource != nullptr) {
        m_xamlSource.Close();
        m_xamlSource = nullptr;
    }
    if (q_ptr && m_dragBarWindow && !m_dragBarWindowClassName.empty()) {
        if (q_ptr->CloseChildWindow(m_dragBarWindow, m_dragBarWindowClassName)) {
            m_dragBarWindow = nullptr;
            m_dragBarWindowClassName = {};
        } else {
            Utils::DisplayErrorDialog(L"Failed to close the drag bar window.");
        }
    }
}

LRESULT CALLBACK XamlWindowPrivate::DragBarWindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept
{
    USER32_API(SetWindowLongPtrW);
    USER32_API(GetWindowLongPtrW);
    USER32_API(DefWindowProcW);
    if (SetWindowLongPtrWFunc && GetWindowLongPtrWFunc && DefWindowProcWFunc) {
        if (message == WM_NCCREATE) {
            const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            const auto that = static_cast<XamlWindowPrivate *>(cs->lpCreateParams);
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) != 0) {
                Utils::DisplayErrorDialog(L"The extra data of the drag bar window has been overwritten.");
            }
        } else if (message == WM_NCDESTROY) {
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, 0) == 0) {
                Utils::DisplayErrorDialog(L"The drag bar window doesn't contain any extra data.");
            }
        }
        if (const auto that = reinterpret_cast<XamlWindowPrivate *>(GetWindowLongPtrWFunc(hWnd, GWLP_USERDATA))) {
            LRESULT result = 0;
            if (that->DragBarMessageHandler(message, wParam, lParam, &result)) {
                return result;
            }
        }
        return DefWindowProcWFunc(hWnd, message, wParam, lParam);
    } else {
        Utils::DisplayErrorDialog(L"Failed to continue the WindowProc function of the drag bar window due to SetWindowLongPtrW(), GetWindowLongPtrW() and DefWindowProcW() are not available.");
        return 0;
    }
}

bool XamlWindowPrivate::DragBarMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept
{
    UNREFERENCED_PARAMETER(wParam); // ### TODO: remove
    if (!result) {
        Utils::DisplayErrorDialog(L"DragBarMessageHandler: the pointer to the result of the WindowProc function is null.");
        return false;
    }
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"DragBarMessageHandler: the q_ptr is null.");
        return false;
    }
    if (!m_dragBarWindow) {
        Utils::DisplayErrorDialog(L"DragBarMessageHandler: the drag bar window has not been created yet.");
        return false;
    }
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
        const HWND mainWindowHandle = q_ptr->WindowHandle();
        if (nonClientMessage.has_value() && mainWindowHandle) {
            USER32_API(ClientToScreen);
            USER32_API(SendMessageW);
            if (ClientToScreenFunc && SendMessageWFunc) {
                POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                if (ClientToScreenFunc(m_dragBarWindow, &pos) == FALSE) {
                    PRINT_WIN32_ERROR_MESSAGE(ClientToScreen, L"Failed to translate from window coordinate to screen coordinate.")
                    return false;
                }
                const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
                // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
                // then pass that click through as an NC click in that location.
                const LRESULT hitTestResult = SendMessageWFunc(mainWindowHandle, WM_NCHITTEST, 0, newLParam);
                SendMessageWFunc(mainWindowHandle, nonClientMessage.value(), hitTestResult, newLParam);
                *result = 0;
                return true;
            } else {
                Utils::DisplayErrorDialog(L"Failed to send hit test message to the main window due to ClientToScreen() and SendMessageW() are not available.");
                return false;
            }
        } else {
            return false;
        }
    } else {
        switch (message) {
        case WM_NCCALCSIZE: {
            *result = WVR_REDRAW;
            return true;
        } break;
        case WM_PAINT: {
            *result = 0;
            return true;
        } break;
        case WM_ERASEBKGND: {
            *result = 1;
            return true;
        } break;
        case WM_CLOSE: {
            if (q_ptr->CloseChildWindow(m_dragBarWindow, m_dragBarWindowClassName)) {
                m_dragBarWindow = nullptr;
                m_dragBarWindowClassName = {};
                *result = 0;
                return true;
            } else {
                Utils::DisplayErrorDialog(L"Failed to close the drag bar window.");
                return false;
            }
        } break;
        case WM_DESTROY: {} break;
        default:
            break;
        }
    }
    return false;
}

bool XamlWindowPrivate::InitializeXamlIsland() noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't initialize the XAML Island due to the q_ptr is null.");
        return false;
    }
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    m_xamlSource = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
    // Get handle to the core window.
    const auto interop = m_xamlSource.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the IDesktopWindowXamlSourceNative.");
        return false;
    }
    // Parent the DesktopWindowXamlSource object to the current window.
    HRESULT hr = interop->AttachToWindow(q_ptr->WindowHandle());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(AttachToWindow, hr, L"Failed to attach the XAML Island window to the main window.")
        return false;
    }
    // Get the new child window's HWND.
    hr = interop->get_WindowHandle(&m_xamlIslandWindow);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(get_WindowHandle, hr, L"Failed to retrieve the XAML Island window's handle.")
        return false;
    }
    if (!m_xamlIslandWindow) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window handle of XAML Island's core window.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    if (!SyncXamlIslandWindowGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
        return false;
    }
    // Create the XAML content.
    m_windowRootGrid = winrt::Windows::UI::Xaml::Controls::Grid();
    m_windowBackgroundBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
    if (!RefreshWindowBackgroundBrush()) {
        Utils::DisplayErrorDialog(L"Failed to refresh the window background brush.");
        return false;
    }
    m_windowBackgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    m_windowRootGrid.Background(m_windowBackgroundBrush);
    //m_windowRootGrid.Children().Clear();
    //m_windowRootGrid.Children().Append(/* some UWP control */);
    //m_windowRootGrid.UpdateLayout();
    m_xamlSource.Content(m_windowRootGrid);
    return true;
}

bool XamlWindowPrivate::InitializeDragBarWindow() noexcept
{
    // The drag bar window is a child window of the top level window that is put
    // right on top of the drag bar. The XAML island window "steals" our mouse
    // messages which makes it hard to implement a custom drag area. By putting
    // a window on top of it, we prevent it from "stealing" the mouse messages.
    //
    // IMPORTANT NOTE: The WS_EX_LAYERED style is supported for both top-level
    // windows and child windows since Windows 8. Previous Windows versions support
    // WS_EX_LAYERED only for top-level windows.
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't initialize the drag bar window due to the q_ptr is null.");
        return false;
    }
    const auto dragBarWindowCreationData = q_ptr->CreateChildWindow(WS_CHILD, (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP), DragBarWindowProc, this);
    m_dragBarWindow = std::get<0>(dragBarWindowCreationData);
    m_dragBarWindowClassName = std::get<1>(dragBarWindowCreationData);
    if (!m_dragBarWindow || m_dragBarWindowClassName.empty()) {
        Utils::DisplayErrorDialog(L"Failed to create the drag bar window.");
        return false;
    }
    // Layered window won't be visible until we call SetLayeredWindowAttributes()
    // or UpdateLayeredWindow().
    USER32_API(SetLayeredWindowAttributes);
    if (SetLayeredWindowAttributesFunc) {
        if (SetLayeredWindowAttributesFunc(m_dragBarWindow, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetLayeredWindowAttributes, L"Failed to change layered window attributes.")
            return false;
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to initialize the drag bar window due to SetLayeredWindowAttributes() is not available.");
        return false;
    }
    if (!SyncDragBarWindowGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the drag bar window.");
        return false;
    }
    return true;
}

bool XamlWindowPrivate::RefreshWindowBackgroundBrush() noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't refresh the window background brush due to the q_ptr is null.");
        return false;
    }
    if (m_windowBackgroundBrush == nullptr) {
        Utils::DisplayErrorDialog(L"Can't refresh the window background brush due to the brush has not been created yet.");
        return false;
    }
    switch (q_ptr->Theme()) {
    case WindowTheme::Light: {
        m_windowBackgroundBrush.TintColor(Constants::Light::TintColor);
        m_windowBackgroundBrush.TintOpacity(Constants::Light::TintOpacity);
        m_windowBackgroundBrush.TintLuminosityOpacity(Constants::Light::LuminosityOpacity);
        m_windowBackgroundBrush.FallbackColor(Constants::Light::FallbackColor);
        return true;
    } break;
    case WindowTheme::Dark: {
        m_windowBackgroundBrush.TintColor(Constants::Dark::TintColor);
        m_windowBackgroundBrush.TintOpacity(Constants::Dark::TintOpacity);
        m_windowBackgroundBrush.TintLuminosityOpacity(Constants::Dark::LuminosityOpacity);
        m_windowBackgroundBrush.FallbackColor(Constants::Dark::FallbackColor);
        return true;
    } break;
    case WindowTheme::HighContrast: {
        // ### TODO
    } break;
    }
    return false;
}

bool XamlWindowPrivate::SyncXamlIslandWindowGeometry() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't sync the XAML Island window geometry due to the q_ptr is null.");
        return false;
    }
    if (!m_xamlIslandWindow) {
        Utils::DisplayErrorDialog(L"Can't sync the XAML Island window geometry due to it has not been created yet.");
        return false;
    }
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        const UINT windowVisibleFrameBorderThickness = q_ptr->GetWindowMetrics(WindowMetrics::WindowVisibleFrameBorderThickness);
        const UINT actualFrameBorderThickness = ((q_ptr->Visibility() == WindowState::Maximized) ? 0 : windowVisibleFrameBorderThickness);
        if (SetWindowPosFunc(m_xamlIslandWindow, HWND_BOTTOM, 0, actualFrameBorderThickness, q_ptr->Width(), (q_ptr->Height() - actualFrameBorderThickness), (SWP_SHOWWINDOW | SWP_NOOWNERZORDER)) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to sync the XAML Island window geometry.")
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to sync the XAML Island window geometry due to SetWindowPos() is not available.");
        return false;
    }
}

bool XamlWindowPrivate::SyncDragBarWindowGeometry() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't sync the drag bar window geometry due to the q_ptr is null.");
        return false;
    }
    if (!m_dragBarWindow) {
        Utils::DisplayErrorDialog(L"Can't sync the drag bar window geometry due to it has not been created yet.");
        return false;
    }
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        const UINT resizeBorderThicknessY = q_ptr->GetWindowMetrics(WindowMetrics::ResizeBorderThicknessY);
        const UINT captionHeight = q_ptr->GetWindowMetrics(WindowMetrics::CaptionHeight);
        const UINT titleBarHeight = ((q_ptr->Visibility() == WindowState::Maximized) ? captionHeight : (captionHeight + resizeBorderThicknessY));
        if (SetWindowPosFunc(m_dragBarWindow, HWND_TOP, 0, 0, q_ptr->Width(), titleBarHeight, (SWP_SHOWWINDOW | SWP_NOOWNERZORDER)) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to sync the drag bar window geometry.")
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to sync the drag bar window geometry due to SetWindowPos() is not available.");
        return false;
    }
}

bool XamlWindowPrivate::SyncInternalWindowsGeometry() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't sync the internal windows geometry due to the q_ptr is null.");
        return false;
    }
    if (!m_xamlIslandWindow) {
        Utils::DisplayErrorDialog(L"Can't sync the internal windows geometry due to the XAML Island window has not been created yet.");
        return false;
    }
    if (!m_dragBarWindow) {
        Utils::DisplayErrorDialog(L"Can't sync the internal windows geometry due to the drag bar window has not been created yet.");
        return false;
    }
    if (!SyncXamlIslandWindowGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the XAML Island window geometry.");
        return false;
    }
    if (!SyncDragBarWindowGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the drag bar window geometry.");
        return false;
    }
    return true;
}

bool XamlWindowPrivate::MessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(result);
    return false;
}

bool XamlWindowPrivate::FilterMessage(const MSG *msg) const noexcept
{
    if (!msg) {
        Utils::DisplayErrorDialog(L"Can't filter Win32 messages due to the pointer to it is null.");
        return false;
    }
    if (m_xamlSource == nullptr) {
        Utils::DisplayErrorDialog(L"Can't filter Win32 messages due to the XAML source has not been created yet.");
        return false;
    }
    const auto interop2 = m_xamlSource.as<IDesktopWindowXamlSourceNative2>();
    if (interop2) {
        BOOL filtered = FALSE;
        const HRESULT hr = interop2->PreTranslateMessage(msg, &filtered);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(PreTranslateMessage, hr, L"Failed to pre-translate Win32 messages.")
            return false;
        } else {
            return (filtered != FALSE);
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to retrieve the IDesktopWindowXamlSourceNative2.");
        return false;
    }
}

XamlWindow::XamlWindow() noexcept
{
    d_ptr = std::make_unique<XamlWindowPrivate>(this);
}

XamlWindow::~XamlWindow() noexcept = default;

void XamlWindow::OnWidthChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!d_ptr->SyncInternalWindowsGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the internal windows geometry.");
    }
}

void XamlWindow::OnHeightChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!d_ptr->SyncInternalWindowsGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the internal windows geometry.");
    }
}

void XamlWindow::OnVisibilityChanged(const WindowState arg) noexcept
{
    if ((arg == WindowState::Hidden) || (arg == WindowState::Minimized)) {
        return;
    }
    if (!d_ptr->SyncInternalWindowsGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the internal windows geometry.");
    }
}

bool XamlWindow::MessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept
{
    return d_ptr->MessageHandler(message, wParam, lParam, result);
}

bool XamlWindow::FilterMessage(const MSG *msg) const noexcept
{
    return d_ptr->FilterMessage(msg);
}
