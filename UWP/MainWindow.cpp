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
#include "MainWindow.h"
#include "OperationResult.h"
#include "Utils.h"

namespace Constants {
namespace Light {
static constexpr const winrt::Windows::UI::Color TintColor = {255, 252, 252, 252};
static constexpr const double TintOpacity = 0.0;
static constexpr const double LuminosityOpacity = 0.85;
static constexpr const winrt::Windows::UI::Color FallbackColor = {255, 249, 249, 249};
} // namespace Light
namespace Dark {
static constexpr const winrt::Windows::UI::Color TintColor = {255, 44, 44, 44};
static constexpr const double TintOpacity = 0.15;
static constexpr const double LuminosityOpacity = 0.96;
static constexpr const winrt::Windows::UI::Color FallbackColor = {255, 44, 44, 44};
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

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *q) noexcept;
    ~MainWindowPrivate() noexcept;

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

    [[nodiscard]] UINT GetTitleBarHeight() const noexcept;
    [[nodiscard]] UINT GetTopFrameMargin() const noexcept;

private:
    [[nodiscard]] bool MainWindowMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) const noexcept;
    [[nodiscard]] bool MainWindowMessageFilter(const MSG *message) const noexcept;

    void OnWidthChanged(const UINT arg) const noexcept;
    void OnHeightChanged(const UINT arg) const noexcept;
    void OnVisibilityChanged(const WindowState arg) const noexcept;
    void OnThemeChanged(const WindowTheme arg) noexcept;

    [[nodiscard]] static LRESULT CALLBACK DragBarWindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept;
    [[nodiscard]] bool DragBarMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) const noexcept;
    [[nodiscard]] bool InitializeXamlIsland() noexcept;
    [[nodiscard]] bool InitializeDragBarWindow() noexcept;

    [[nodiscard]] bool CreateUWPContents() noexcept;

    [[nodiscard]] static bool SetWindowState(const HWND hWnd, const DWORD extraFlags) noexcept;

private:
    MainWindowPrivate(const MainWindowPrivate &) = delete;
    MainWindowPrivate &operator=(const MainWindowPrivate &) = delete;
    MainWindowPrivate(MainWindowPrivate &&) = delete;
    MainWindowPrivate &operator=(MainWindowPrivate &&) = delete;

private:
    MainWindow *q_ptr = nullptr;
    HWND m_dragBarWindow = nullptr;
    HWND m_xamlIslandWindow = nullptr;
    std::optional<Color> m_tintColor = std::nullopt;
    std::optional<double> m_tintOpacity = std::nullopt;
    std::optional<double> m_luminosityOpacity = std::nullopt;
    std::optional<Color> m_fallbackColor = std::nullopt;
    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource m_xamlSource = nullptr;
    winrt::Windows::UI::Xaml::Controls::Grid m_rootGrid = nullptr;
    winrt::Windows::UI::Xaml::Media::AcrylicBrush m_acrylicBrush = nullptr;
};

MainWindowPrivate::MainWindowPrivate(MainWindow *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"MainWindowPrivate's q is null.");
        std::exit(-1);
    }
    q_ptr = q;
    if (InitializeXamlIsland()) {
        if (InitializeDragBarWindow()) {
            q_ptr->CustomMessageHandler(std::bind(&MainWindowPrivate::MainWindowMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            q_ptr->WindowMessageFilter(std::bind(&MainWindowPrivate::MainWindowMessageFilter, this, std::placeholders::_1));
            q_ptr->WidthChangeHandler(std::bind(&MainWindowPrivate::OnWidthChanged, this, std::placeholders::_1));
            q_ptr->HeightChangeHandler(std::bind(&MainWindowPrivate::OnHeightChanged, this, std::placeholders::_1));
            q_ptr->VisibilityChangeHandler(std::bind(&MainWindowPrivate::OnVisibilityChanged, this, std::placeholders::_1));
            q_ptr->ThemeChangeHandler(std::bind(&MainWindowPrivate::OnThemeChanged, this, std::placeholders::_1));
        } else {
            Utils::DisplayErrorDialog(L"Failed to initialize the drag bar window.");
            std::exit(-1);
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to initialize the XAML Island.");
        std::exit(-1);
    }
}

MainWindowPrivate::~MainWindowPrivate() noexcept
{
    if (m_xamlSource != nullptr) {
        m_xamlSource.Close();
        m_xamlSource = nullptr;
    }
}

const Color &MainWindowPrivate::TintColor() const noexcept
{
    return m_tintColor.value();
}

void MainWindowPrivate::TintColor(const Color &value) noexcept
{
    if (m_tintColor.value() != value) {
        m_tintColor = value;
    }
}

double MainWindowPrivate::TintOpacity() const noexcept
{
    return m_tintOpacity.value();
}

void MainWindowPrivate::TintOpacity(const double value) noexcept
{
    if (m_tintOpacity.value() != value) {
        m_tintOpacity = value;
    }
}

double MainWindowPrivate::LuminosityOpacity() const noexcept
{
    return m_luminosityOpacity.value();
}

void MainWindowPrivate::LuminosityOpacity(const double value) noexcept
{
    if (m_luminosityOpacity.value() != value) {
        m_luminosityOpacity = value;
    }
}

const Color &MainWindowPrivate::FallbackColor() const noexcept
{
    return m_fallbackColor.value();
}

void MainWindowPrivate::FallbackColor(const Color &value) noexcept
{
    if (m_fallbackColor.value() != value) {
        m_fallbackColor = value;
    }
}

double MainWindowPrivate::GetDevicePixelRatio() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't retrieve the device pixel ratio of the window due to q_ptr is null.");
        return 0.0;
    }
    return (static_cast<double>(q_ptr->DotsPerInch()) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
}

SIZE MainWindowPrivate::GetPhysicalSize() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't retrieve the physical size of the window due to q_ptr is null.");
        return {};
    }
    const UINT topFrameMargin = GetTopFrameMargin();
    return {static_cast<LONG>(q_ptr->Width()), static_cast<LONG>(q_ptr->Height() - topFrameMargin)};
}

winrt::Windows::Foundation::Size MainWindowPrivate::GetLogicalSize(const SIZE physicalSize) const noexcept
{
    const double scale = GetDevicePixelRatio();
    // 0.5 is to ensure that we pixel snap correctly at the edges, this is necessary with odd DPIs like 1.25, 1.5, 1, .75
    const double logicalWidth = ((physicalSize.cx / scale) + 0.5);
    const double logicalHeight = ((physicalSize.cy / scale) + 0.5);
    return {static_cast<float>(logicalWidth), static_cast<float>(logicalHeight)};
}

winrt::Windows::Foundation::Size MainWindowPrivate::GetLogicalSize() const noexcept
{
    return GetLogicalSize(GetPhysicalSize());
}

LRESULT CALLBACK MainWindowPrivate::DragBarWindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept
{
    if (message == WM_NCCREATE) {
        const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        const auto that = static_cast<MainWindowPrivate *>(cs->lpCreateParams);
        // SetWindowLongPtrW() won't modify the Last Error state on success
        // and it's return value is the previous data so we have to judge
        // the actual operation result from the Last Error state manually.
        SetLastError(ERROR_SUCCESS);
        const LONG_PTR result = SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
        PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to set the extra data to the drag bar window.")
        if (result != 0) {
            Utils::DisplayErrorDialog(L"The extra data of the drag bar window has been overwritten.");
        }
    } else if (message == WM_NCDESTROY) {
        // See the above comments.
        SetLastError(ERROR_SUCCESS);
        const LONG_PTR result = SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
        PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to clear the extra data of the drag bar window.")
        if (result == 0) {
            Utils::DisplayErrorDialog(L"The drag bar window doesn't contain any extra data.");
        }
    }
    if (const auto that = reinterpret_cast<MainWindowPrivate *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
        LRESULT result = 0;
        if (that->DragBarMessageHandler(message, wParam, lParam, &result)) {
            return result;
        }
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

bool MainWindowPrivate::DragBarMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) const noexcept
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
        POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (ClientToScreen(m_dragBarWindow, &pos) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(ClientToScreen, L"Failed to translate from window coordinate to screen coordinate.")
            return false;
        }
        const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
        // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
        // then pass that click through as an NC click in that location.
        const LRESULT hitTestResult = SendMessageW(mainWindowHandle, WM_NCHITTEST, 0, newLParam);
        SendMessageW(mainWindowHandle, nonClientMessage.value(), hitTestResult, newLParam);
        *result = 0;
        return true;
    }
    return false;
}

bool MainWindowPrivate::InitializeXamlIsland() noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't initialize the XAML Island due to the q_ptr is null.");
        return false;
    }
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    m_xamlSource = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
    if (m_xamlSource == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the DesktopWindowXamlSource object.");
        return false;
    }
    // Get handle to the core window.
    const auto interop = m_xamlSource.as<IDesktopWindowXamlSourceNative2>();
    if (!interop) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the core window.");
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
    // Create the UWP content.
    if (!CreateUWPContents()) {
        Utils::DisplayErrorDialog(L"Failed to create the UWP contents.");
        return false;
    }
    m_xamlSource.Content(m_rootGrid);
    return true;
}

bool MainWindowPrivate::InitializeDragBarWindow() noexcept
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
    m_dragBarWindow = q_ptr->CreateChildWindow(WS_CHILD, (WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP), DragBarWindowProc, this, sizeof(MainWindowPrivate *));
    if (!m_dragBarWindow) {
        Utils::DisplayErrorDialog(L"Failed to create the drag bar window.");
        return false;
    }
    // Layered window won't be visible until we call SetLayeredWindowAttributes() or UpdateLayeredWindow().
    if (SetLayeredWindowAttributes(m_dragBarWindow, 0, 255, LWA_ALPHA) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetLayeredWindowAttributes, L"Failed to update the drag bar window.")
        return false;
    }
    if (!SyncDragBarWindowGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the drag bar window.");
        return false;
    }
    return true;
}

UINT MainWindowPrivate::GetTopFrameMargin() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't retrieve the top frame margin due to q_ptr is null.");
        return 0;
    }
    if (q_ptr->Visibility() == WindowState::Maximized) {
        return 0;
    } else {
        return q_ptr->GetWindowMetrics(WindowMetrics::WindowVisibleFrameBorderThickness);
    }
}

bool MainWindowPrivate::CreateUWPContents() noexcept
{
    m_rootGrid = winrt::Windows::UI::Xaml::Controls::Grid();
    if (m_rootGrid == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the root Grid.");
        return false;
    }

    m_acrylicBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
    if (m_acrylicBrush == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the AcrylicBrush.");
        return false;
    }
    m_acrylicBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    if (!RefreshWindowBackgroundBrush()) {
        Utils::DisplayErrorDialog(L"Failed to refresh the window background brush.");
        return false;
    }

    m_rootGrid.Background(m_acrylicBrush);

    return true;
}

bool MainWindowPrivate::SetWindowState(const HWND hWnd, const DWORD extraFlags) noexcept
{
    if (!hWnd) {
        return false;
    }
    POINT pos = {};
    if (GetCursorPos(&pos) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(GetCursorPos, L"Failed to retrieve the cursor position.")
        return false;
    }
    const LPARAM lParam = MAKELPARAM(pos.x, pos.y);
    WINDOWPLACEMENT wp;
    SecureZeroMemory(&wp, sizeof(wp));
    wp.length = sizeof(wp);
    if (GetWindowPlacement(hWnd, &wp) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(GetWindowPlacement, L"Failed to retrieve the window placement.")
        return false;
    }
    DWORD flags = extraFlags;
    if ((wp.showCmd == SW_NORMAL) || (wp.showCmd == SW_RESTORE)) {
        flags |= SC_MAXIMIZE;
    } else if (wp.showCmd == SW_MAXIMIZE) {
        flags |= SC_RESTORE;
    }
    if (PostMessageW(hWnd, WM_SYSCOMMAND, static_cast<WPARAM>(flags), lParam) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(PostMessageW, L"Failed to post message to the window.")
        return false;
    }
    return true;
}

bool MainWindowPrivate::RefreshWindowBackgroundBrush() noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't refresh the window background brush due to the q_ptr is null.");
        return false;
    }
    if (m_acrylicBrush == nullptr) {
        Utils::DisplayErrorDialog(L"Can't refresh the window background brush due to the brush has not been created yet.");
        return false;
    }
    auto tintColor = winrt::Windows::UI::Color();
    double tintOpacity = 0.0;
    double luminosityOpacity = 0.0;
    auto fallbackColor = winrt::Windows::UI::Color();
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
    m_acrylicBrush.TintColor(tintColor);
    m_acrylicBrush.TintOpacity(tintOpacity);
    m_acrylicBrush.TintLuminosityOpacity(luminosityOpacity);
    m_acrylicBrush.FallbackColor(fallbackColor);
    q_ptr->TitleBarBackgroundColor(ToColor(fallbackColor));
    return true;
}

bool MainWindowPrivate::SyncXamlIslandWindowGeometry() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't sync the XAML Island window geometry due to the q_ptr is null.");
        return false;
    }
    if (!m_xamlIslandWindow) {
        Utils::DisplayErrorDialog(L"Can't sync the XAML Island window geometry due to it has not been created yet.");
        return false;
    }
    const UINT topFrameMargin = GetTopFrameMargin();
    const UINT actualHeight = (q_ptr->Height() - topFrameMargin);
    if (SetWindowPos(m_xamlIslandWindow, HWND_BOTTOM, 0, topFrameMargin, q_ptr->Width(), actualHeight, (SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER)) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to sync the XAML Island window geometry.")
        return false;
    }
    if (m_rootGrid != nullptr) {
        const winrt::Windows::Foundation::Size size = GetLogicalSize();
        m_rootGrid.Width(size.Width);
        m_rootGrid.Height(size.Height);
    }
    return true;
}

bool MainWindowPrivate::SyncDragBarWindowGeometry() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't sync the drag bar window geometry due to the q_ptr is null.");
        return false;
    }
    if (!m_dragBarWindow) {
        Utils::DisplayErrorDialog(L"Can't sync the drag bar window geometry due to it has not been created yet.");
        return false;
    }
    const UINT titleBarHeight = GetTitleBarHeight();
    if (SetWindowPos(m_dragBarWindow, HWND_TOP, 0, 0, q_ptr->Width(), titleBarHeight, (SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER)) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to sync the drag bar window geometry.")
        return false;
    } else {
        return true;
    }
}

bool MainWindowPrivate::SyncInternalWindowsGeometry() const noexcept
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
    if (q_ptr->Visibility() == WindowState::Minimized) {
        // When the window is minimized, the x/y/width/height of it will become negative
        // because Windows actually move it out of the current screen. Passing a negative
        // number to our change handler is meaningless and thus we should ignore it.
        // But we should not ignore it when the window is hidden because they will always
        // be valid no matter the window itself is visible or not.
        return true;
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

UINT MainWindowPrivate::GetTitleBarHeight() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't retrieve the title bar height due to the q_ptr is null.");
        return 0;
    }
    const UINT resizeBorderThicknessY = q_ptr->GetWindowMetrics(WindowMetrics::ResizeBorderThicknessY);
    const UINT captionHeight = q_ptr->GetWindowMetrics(WindowMetrics::CaptionHeight);
    const UINT titleBarHeight = ((q_ptr->Visibility() == WindowState::Maximized) ? captionHeight : (captionHeight + resizeBorderThicknessY));
    return titleBarHeight;
}

bool MainWindowPrivate::MainWindowMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) const noexcept
{
    UNREFERENCED_PARAMETER(wParam); // ### TODO: remove
    if (!result) {
        Utils::DisplayErrorDialog(L"MainWindowPrivate::MessageHandler: the pointer to the result of the WindowProc function is null.");
        return false;
    }
    switch (message) {
    case WM_SETFOCUS: {
        if (m_xamlIslandWindow) {
            // Send focus to the XAML Island child window.
            if (SetFocus(m_xamlIslandWindow) == nullptr) {
                PRINT_WIN32_ERROR_MESSAGE(SetFocus, L"Failed to send focus to the XAML Island window.")
                return false;
            } else {
                *result = 0;
                return true;
            }
        }
    } break;
    case WM_SETCURSOR: {
        if (q_ptr && q_ptr->WindowHandle()) {
            if (LOWORD(lParam) == HTCLIENT) {
                // Get the cursor position from the _last message_ and not from
                // `GetCursorPos` (which returns the cursor position _at the
                // moment_) because if we're lagging behind the cursor's position,
                // we still want to get the cursor position that was associated
                // with that message at the time it was sent to handle the message
                // correctly.
                const LRESULT hitTestResult = SendMessageW(q_ptr->WindowHandle(), WM_NCHITTEST, 0, GetMessagePos());
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
                *result = TRUE;
                return true;
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

bool MainWindowPrivate::MainWindowMessageFilter(const MSG *message) const noexcept
{
    if (m_xamlSource == nullptr) {
        return false;
    }
    if (!message) {
        return false;
    }
    BOOL filtered = FALSE;
    const auto interop = m_xamlSource.as<IDesktopWindowXamlSourceNative2>();
    if (!interop) {
        Utils::DisplayErrorDialog(L"Can't retrieve the core window of the XAML Source.");
        return false;
    }
    const HRESULT hr = interop->PreTranslateMessage(message, &filtered);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(PreTranslateMessage, hr, L"Failed to pre-translate the window messages.")
        return false;
    }
    return (filtered != FALSE);
}

void MainWindowPrivate::OnWidthChanged(const UINT arg) const noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!SyncInternalWindowsGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the internal windows geometry.");
    }
}

void MainWindowPrivate::OnHeightChanged(const UINT arg) const noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!SyncInternalWindowsGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the internal windows geometry.");
    }
}

void MainWindowPrivate::OnVisibilityChanged(const WindowState arg) const noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!SyncInternalWindowsGeometry()) {
        Utils::DisplayErrorDialog(L"Failed to sync the internal windows geometry.");
    }
}

void MainWindowPrivate::OnThemeChanged(const WindowTheme arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!RefreshWindowBackgroundBrush()) {
        Utils::DisplayErrorDialog(L"Failed to refresh the window background brush.");
    }
}

MainWindow::MainWindow() noexcept : Window(false)
{
    d_ptr = std::make_unique<MainWindowPrivate>(this);
}

MainWindow::~MainWindow() noexcept = default;

const Color &MainWindow::TintColor() const noexcept
{
    return d_ptr->TintColor();
}

void MainWindow::TintColor(const Color &value) noexcept
{
    d_ptr->TintColor(value);
}

double MainWindow::TintOpacity() const noexcept
{
    return d_ptr->TintOpacity();
}

void MainWindow::TintOpacity(const double value) noexcept
{
    d_ptr->TintOpacity(value);
}

double MainWindow::LuminosityOpacity() const noexcept
{
    return d_ptr->LuminosityOpacity();
}

void MainWindow::LuminosityOpacity(const double value) noexcept
{
    d_ptr->LuminosityOpacity(value);
}

const Color &MainWindow::FallbackColor() const noexcept
{
    return d_ptr->FallbackColor();
}

void MainWindow::FallbackColor(const Color &value) noexcept
{
    d_ptr->FallbackColor(value);
}
