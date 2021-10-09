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

[[nodiscard]] static inline winrt::Windows::UI::Color ToWinRTColor(const Color &value) noexcept
{
    return winrt::Windows::UI::ColorHelper::FromArgb(value.Alpha(), value.Red(), value.Green(), value.Blue());
}

[[nodiscard]] static inline Color ToColor(const winrt::Windows::UI::Color &value) noexcept
{
    return Color::FromRgba(value.R, value.G, value.B, value.A);
}

class XamlWindowPrivate
{
public:
    explicit XamlWindowPrivate(XamlWindow *q) noexcept;
    ~XamlWindowPrivate() noexcept;

    [[nodiscard]] const Color &TintColor() const noexcept;
    void TintColor(const Color &value) noexcept;

    [[nodiscard]] double TintOpacity() const noexcept;
    void TintOpacity(const double value) noexcept;

    [[nodiscard]] double LuminosityOpacity() const noexcept;
    void LuminosityOpacity(const double value) noexcept;

    [[nodiscard]] const Color &FallbackColor() const noexcept;
    void FallbackColor(const Color &value) noexcept;

    [[nodiscard]] double GetDevicePixelRatio() const noexcept;
    [[nodiscard]] SIZE GetPhysicalSize() const noexcept;
    [[nodiscard]] winrt::Windows::Foundation::Size GetLogicalSize(const SIZE physicalSize) const noexcept;
    [[nodiscard]] winrt::Windows::Foundation::Size GetLogicalSize() const noexcept;

    [[nodiscard]] bool RefreshWindowBackgroundBrush() noexcept;
    [[nodiscard]] bool SyncXamlIslandWindowGeometry() const noexcept;
    [[nodiscard]] bool SyncDragBarWindowGeometry() const noexcept;
    [[nodiscard]] bool SyncInternalWindowsGeometry() const noexcept;

    [[nodiscard]] bool MessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept;

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
    HWND m_dragBarWindow = nullptr;
    HWND m_xamlIslandWindow = nullptr;
    std::optional<Color> m_tintColor = std::nullopt;
    std::optional<double> m_tintOpacity = std::nullopt;
    std::optional<double> m_luminosityOpacity = std::nullopt;
    std::optional<Color> m_fallbackColor = std::nullopt;
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
}

const Color &XamlWindowPrivate::TintColor() const noexcept
{
    return m_tintColor.value();
}

void XamlWindowPrivate::TintColor(const Color &value) noexcept
{
    if (m_tintColor.value() != value) {
        m_tintColor = value;
    }
}

double XamlWindowPrivate::TintOpacity() const noexcept
{
    return m_tintOpacity.value();
}

void XamlWindowPrivate::TintOpacity(const double value) noexcept
{
    if (m_tintOpacity.value() != value) {
        m_tintOpacity = value;
    }
}

double XamlWindowPrivate::LuminosityOpacity() const noexcept
{
    return m_luminosityOpacity.value();
}

void XamlWindowPrivate::LuminosityOpacity(const double value) noexcept
{
    if (m_luminosityOpacity.value() != value) {
        m_luminosityOpacity = value;
    }
}

const Color &XamlWindowPrivate::FallbackColor() const noexcept
{
    return m_fallbackColor.value();
}

void XamlWindowPrivate::FallbackColor(const Color &value) noexcept
{
    if (m_fallbackColor.value() != value) {
        m_fallbackColor = value;
    }
}

double XamlWindowPrivate::GetDevicePixelRatio() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't retrieve the device pixel ratio of the window due to q_ptr is null.");
        return 0.0;
    }
    return (static_cast<double>(q_ptr->DotsPerInch()) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
}

SIZE XamlWindowPrivate::GetPhysicalSize() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't retrieve the physical size of the window due to q_ptr is null.");
        return {};
    }
    return {static_cast<LONG>(q_ptr->Width()), static_cast<LONG>(q_ptr->Height())};
}

winrt::Windows::Foundation::Size XamlWindowPrivate::GetLogicalSize(const SIZE physicalSize) const noexcept
{
    const double scale = GetDevicePixelRatio();
    // 0.5 is to ensure that we pixel snap correctly at the edges, this is necessary with odd DPIs like 1.25, 1.5, 1, .75
    const double logicalWidth = ((physicalSize.cx / scale) + 0.5);
    const double logicalHeight = ((physicalSize.cy / scale) + 0.5);
    return {static_cast<float>(logicalWidth), static_cast<float>(logicalHeight)};
}

winrt::Windows::Foundation::Size XamlWindowPrivate::GetLogicalSize() const noexcept
{
    return GetLogicalSize(GetPhysicalSize());
}

LRESULT CALLBACK XamlWindowPrivate::DragBarWindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept
{
    USER32_API(SetWindowLongPtrW);
    USER32_API(GetWindowLongPtrW);
    USER32_API(DefWindowProcW);
    if (SetWindowLongPtrW_API && GetWindowLongPtrW_API && DefWindowProcW_API) {
        if (message == WM_NCCREATE) {
            const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            const auto that = static_cast<XamlWindowPrivate *>(cs->lpCreateParams);
            // SetWindowLongPtrW() won't modify the Last Error state on success
            // and it's return value is the previous data so we have to judge
            // the actual operation result from the Last Error state manually.
            SetLastError(ERROR_SUCCESS);
            const LONG_PTR result = SetWindowLongPtrW_API(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to set the extra data to the drag bar window.")
            if (result != 0) {
                Utils::DisplayErrorDialog(L"The extra data of the drag bar window has been overwritten.");
            }
        } else if (message == WM_NCDESTROY) {
            // See the above comments.
            SetLastError(ERROR_SUCCESS);
            const LONG_PTR result = SetWindowLongPtrW_API(hWnd, GWLP_USERDATA, 0);
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to clear the extra data of the drag bar window.")
            if (result == 0) {
                Utils::DisplayErrorDialog(L"The drag bar window doesn't contain any extra data.");
            }
        }
        if (const auto that = reinterpret_cast<XamlWindowPrivate *>(GetWindowLongPtrW_API(hWnd, GWLP_USERDATA))) {
            LRESULT result = 0;
            if (that->DragBarMessageHandler(message, wParam, lParam, &result)) {
                return result;
            }
        }
        return DefWindowProcW_API(hWnd, message, wParam, lParam);
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
        //Utils::DisplayErrorDialog(L"DragBarMessageHandler: the drag bar window has not been created yet.");
        return false;
    }
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
        if (ClientToScreen_API && SendMessageW_API) {
            POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (ClientToScreen_API(m_dragBarWindow, &pos) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(ClientToScreen, L"Failed to translate from window coordinate to screen coordinate.")
                return false;
            }
            const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
            // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
            // then pass that click through as an NC click in that location.
            const LRESULT hitTestResult = SendMessageW_API(mainWindowHandle, WM_NCHITTEST, 0, newLParam);
            SendMessageW_API(mainWindowHandle, nonClientMessage.value(), hitTestResult, newLParam);
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Failed to send hit test message to the main window due to ClientToScreen() and SendMessageW() are not available.");
            return false;
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
    const auto interop = m_xamlSource.as<IDesktopWindowXamlSourceNative2>();
    if (!interop) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the IDesktopWindowXamlSourceNative2.");
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
    m_dragBarWindow = q_ptr->CreateChildWindow(WS_CHILD, (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP), DragBarWindowProc, this, sizeof(XamlWindowPrivate *));
    if (!m_dragBarWindow) {
        Utils::DisplayErrorDialog(L"Failed to create the drag bar window.");
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
    winrt::Windows::UI::Color tintColor = {};
    double tintOpacity = 0.0;
    double luminosityOpacity = 0.0;
    winrt::Windows::UI::Color fallbackColor = {};
    switch (q_ptr->Theme()) {
    case WindowTheme::Light: {
        tintColor = Constants::Light::TintColor;
        tintOpacity = Constants::Light::TintOpacity;
        luminosityOpacity = Constants::Light::LuminosityOpacity;
        fallbackColor = Constants::Light::FallbackColor;
    } break;
    case WindowTheme::Dark: {
        tintColor = Constants::Dark::TintColor;
        tintOpacity = Constants::Dark::TintOpacity;
        luminosityOpacity = Constants::Dark::LuminosityOpacity;
        fallbackColor = Constants::Dark::FallbackColor;
    } break;
    case WindowTheme::HighContrast: {
        // ### TODO
    } break;
    }
    if (m_tintColor.has_value()) {
        tintColor = ToWinRTColor(m_tintColor.value());
    }
    if (m_tintOpacity.has_value()) {
        tintOpacity = m_tintOpacity.value();
    }
    if (m_luminosityOpacity.has_value()) {
        luminosityOpacity = m_luminosityOpacity.value();
    }
    if (m_fallbackColor.has_value()) {
        fallbackColor = ToWinRTColor(m_fallbackColor.value());
    }
    m_windowBackgroundBrush.TintColor(tintColor);
    m_windowBackgroundBrush.TintOpacity(tintOpacity);
    m_windowBackgroundBrush.TintLuminosityOpacity(luminosityOpacity);
    m_windowBackgroundBrush.FallbackColor(fallbackColor);
    q_ptr->TitleBarBackgroundColor(ToColor(fallbackColor));
    return true;
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
    if (SetWindowPos_API) {
        const UINT windowVisibleFrameBorderThickness = q_ptr->GetWindowMetrics(WindowMetrics::WindowVisibleFrameBorderThickness);
        const UINT actualFrameBorderThickness = ((q_ptr->Visibility() == WindowState::Maximized) ? 0 : windowVisibleFrameBorderThickness);
        if (SetWindowPos_API(m_xamlIslandWindow, HWND_BOTTOM, 0, actualFrameBorderThickness, q_ptr->Width(), (q_ptr->Height() - actualFrameBorderThickness), (SWP_SHOWWINDOW | SWP_NOOWNERZORDER)) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to sync the XAML Island window geometry.")
            return false;
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to sync the XAML Island window geometry due to SetWindowPos() is not available.");
        return false;
    }
    if (m_windowRootGrid != nullptr) {
        const auto size = GetLogicalSize();
        m_windowRootGrid.Width(size.Width);
        m_windowRootGrid.Height(size.Height);
    }
    return true;
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
    if (SetWindowPos_API) {
        const UINT resizeBorderThicknessY = q_ptr->GetWindowMetrics(WindowMetrics::ResizeBorderThicknessY);
        const UINT captionHeight = q_ptr->GetWindowMetrics(WindowMetrics::CaptionHeight);
        const UINT titleBarHeight = ((q_ptr->Visibility() == WindowState::Maximized) ? captionHeight : (captionHeight + resizeBorderThicknessY));
        if (SetWindowPos_API(m_dragBarWindow, HWND_TOP, 0, 0, q_ptr->Width(), titleBarHeight, (SWP_SHOWWINDOW | SWP_NOOWNERZORDER)) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to sync the drag bar window geometry.")
            return false;
        }
        // Layered window won't be visible until we call SetLayeredWindowAttributes()
        // or UpdateLayeredWindow().
        USER32_API(SetLayeredWindowAttributes);
        if (SetLayeredWindowAttributes_API) {
            if (SetLayeredWindowAttributes_API(m_dragBarWindow, 0, 255, LWA_ALPHA) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetLayeredWindowAttributes, L"Failed to update the drag bar window.")
                return false;
            } else {
                return true;
            }
        } else {
            Utils::DisplayErrorDialog(L"Failed to update the drag bar window due to SetLayeredWindowAttributes() is not available.");
            return false;
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
    UNREFERENCED_PARAMETER(wParam); // ### TODO: remove
    if (!result) {
        Utils::DisplayErrorDialog(L"XamlWindowPrivate::MessageHandler: the pointer to the result of the WindowProc function is null.");
        return false;
    }
    switch (message) {
    case WM_SETFOCUS: {
        if (m_xamlIslandWindow) {
            USER32_API(SetFocus);
            if (SetFocus_API) {
                // Send focus to the XAML Island child window.
                if (SetFocus_API(m_xamlIslandWindow) == nullptr) {
                    PRINT_WIN32_ERROR_MESSAGE(SetFocus, L"Failed to send focus to the XAML Island window.")
                    return false;
                } else {
                    *result = 0;
                    return true;
                }
            } else {
                Utils::DisplayErrorDialog(L"Can't send focus to the XAML Island window due to SetFocus() is not available.");
                return false;
            }
        }
    } break;
    case WM_SETCURSOR: {
        if (q_ptr && q_ptr->WindowHandle()) {
            if (LOWORD(lParam) == HTCLIENT) {
                USER32_API(SendMessageW);
                USER32_API(GetMessagePos);
                USER32_API(SetCursor);
                USER32_API(LoadCursorW);
                if (SendMessageW_API && GetMessagePos_API && SetCursor_API && LoadCursorW_API) {
                    // Get the cursor position from the _last message_ and not from
                    // `GetCursorPos` (which returns the cursor position _at the
                    // moment_) because if we're lagging behind the cursor's position,
                    // we still want to get the cursor position that was associated
                    // with that message at the time it was sent to handle the message
                    // correctly.
                    const LRESULT hitTestResult = SendMessageW_API(q_ptr->WindowHandle(), WM_NCHITTEST, 0, GetMessagePos_API());
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
                        SetCursor_API(LoadCursorW_API(nullptr, IDC_SIZENS));
                    } else {
                        // Reset cursor
                        SetCursor_API(LoadCursorW_API(nullptr, IDC_ARROW));
                    }
                    *result = TRUE;
                    return true;
                } else {
                    Utils::DisplayErrorDialog(L"Can't update the cursor shape due to SendMessageW(), GetMessagePos(), SetCursor() and LoadCursorW() are not available.");
                    return false;
                }
            }
        }
    } break;
    case WM_NCHITTEST: {
        // ### TODO: HTSYSMENU/HTMINBUTTON/HTMAXBUTTON/HTCLOSE
    } break;
    default:
        break;
    }
    return false;
}

XamlWindow::XamlWindow() noexcept
{
    d_ptr = std::make_unique<XamlWindowPrivate>(this);
}

XamlWindow::~XamlWindow() noexcept = default;

const Color &XamlWindow::TintColor() const noexcept
{
    return d_ptr->TintColor();
}

void XamlWindow::TintColor(const Color &value) noexcept
{
    d_ptr->TintColor(value);
}

double XamlWindow::TintOpacity() const noexcept
{
    return d_ptr->TintOpacity();
}

void XamlWindow::TintOpacity(const double value) noexcept
{
    d_ptr->TintOpacity(value);
}

double XamlWindow::LuminosityOpacity() const noexcept
{
    return d_ptr->LuminosityOpacity();
}

void XamlWindow::LuminosityOpacity(const double value) noexcept
{
    d_ptr->LuminosityOpacity(value);
}

const Color &XamlWindow::FallbackColor() const noexcept
{
    return d_ptr->FallbackColor();
}

void XamlWindow::FallbackColor(const Color &value) noexcept
{
    d_ptr->FallbackColor(value);
}

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

void XamlWindow::OnThemeChanged(const WindowTheme arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!d_ptr->RefreshWindowBackgroundBrush()) {
        Utils::DisplayErrorDialog(L"Failed to refresh the window background brush.");
    }
}

bool XamlWindow::MessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept
{
    return d_ptr->MessageHandler(message, wParam, lParam, result);
}
