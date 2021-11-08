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
#include "OperationResult.h"
#include "WindowsVersion.h"
#include "Utils.h"
#include "Resource.h"
#include <ComBaseApi.h>
#include <ShellApi.h>
#include <DwmApi.h>
#include <cmath>

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

static constexpr const wchar_t __NEW_LINE[] = L"\r\n";

[[nodiscard]] static inline std::wstring GenerateGUID() noexcept
{
    GUID guid = {};
    const HRESULT hr = CoCreateGuid(&guid);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CoCreateGuid, hr, L"Failed to generate a new GUID.")
        return {};
    }
    wchar_t buf[MAX_PATH] = { L'\0' };
    if (StringFromGUID2(guid, buf, MAX_PATH) == 0) {
        PRINT_WIN32_ERROR_MESSAGE(StringFromGUID2, L"Failed to convert GUID to string.")
        return {};
    }
    return buf;
}

[[nodiscard]] static inline DWORD GetDWORDFromRegistry(const HKEY rootKey, const std::wstring &subKey, const std::wstring &keyName) noexcept
{
    if (!rootKey || subKey.empty() || keyName.empty()) {
        Utils::DisplayErrorDialog(L"Can't query the registry due to invalid parameters are passed.");
        return 0;
    }
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(rootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        PRINT_WIN32_ERROR_MESSAGE(RegOpenKeyExW, L"Failed to open the registry key to read.")
        return 0;
    }
    DWORD dwValue = 0;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(dwValue);
    const bool success = (RegQueryValueExW(hKey, keyName.c_str(), nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwValue), &dwSize) == ERROR_SUCCESS);
    if (!success) {
        PRINT_WIN32_ERROR_MESSAGE(RegQueryValueExW, L"Failed to query the registry key value.")
        // Don't return early here because we have to close the opened registry key later.
    }
    if (RegCloseKey(hKey) != ERROR_SUCCESS) {
        PRINT_WIN32_ERROR_MESSAGE(RegCloseKey, L"Failed to close the registry key.")
        return 0;
    }
    return dwValue;
}

[[nodiscard]] static inline RECT GetWindowFrameGeometry(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return {};
    }
    RECT rect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &rect) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the window frame geometry.")
        return {};
    }
    return rect;
}

[[nodiscard]] static inline bool IsHighContrastModeEnabled() noexcept
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SystemParametersInfoW, L"Failed to retrieve the high contrast mode state.")
        return false;
    }
    return (hc.dwFlags & HCF_HIGHCONTRASTON);
}

[[nodiscard]] static inline bool ShouldAppsUseLightTheme() noexcept
{
    return (GetDWORDFromRegistry(HKEY_CURRENT_USER, PersonalizeRegistryKeyPath, L"AppsUseLightTheme") != 0);
}

[[nodiscard]] static inline DWORD GetGlobalColorizationColor2() noexcept
{
    DWORD color = 0; // The color format of the value is 0xAARRGGBB.
    BOOL opaque = FALSE;
    const HRESULT hr = DwmGetColorizationColor(&color, &opaque);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DwmGetColorizationColor, hr, L"Failed to retrieve the colorization color.")
        return 0;
    }
    return color;
}

[[nodiscard]] static inline WindowColorizationArea GetGlobalColorizationArea2() noexcept
{
    const HKEY rootKey = HKEY_CURRENT_USER;
    const std::wstring keyName = L"ColorPrevalence";
    const DWORD dwmValue = GetDWORDFromRegistry(rootKey, DwmRegistryKeyPath, keyName);
    const DWORD themeValue = GetDWORDFromRegistry(rootKey, PersonalizeRegistryKeyPath, keyName);
    const bool dwm = (dwmValue != 0);
    const bool theme = (themeValue != 0);
    if (dwm && theme) {
        return WindowColorizationArea::All;
    } else if (dwm) {
        return WindowColorizationArea::TitleBar_WindowBorder;
    } else if (theme) {
        return WindowColorizationArea::StartMenu_TaskBar_ActionCenter;
    }
    return WindowColorizationArea::None;
}

[[nodiscard]] static inline WindowTheme GetGlobalApplicationTheme2() noexcept
{
    if (IsHighContrastModeEnabled()) {
        return WindowTheme::HighContrast;
    } else if (ShouldAppsUseLightTheme()) {
        return WindowTheme::Light;
    } else {
        return WindowTheme::Dark;
    }
}

[[nodiscard]] static inline RECT GetScreenGeometry(const HWND hWnd, const bool includeTaskBar) noexcept
{
    if (!hWnd) {
        return {};
    }
    const HMONITOR mon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    if (!mon) {
        PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow, L"Failed to retrieve the corresponding screen.")
        return {};
    }
    MONITORINFO mi;
    SecureZeroMemory(&mi, sizeof(mi));
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(mon, &mi) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(GetMonitorInfoW, L"Failed to retrieve the screen information.")
        return {};
    }
    return (includeTaskBar ? mi.rcMonitor : mi.rcWork);
}

[[nodiscard]] static inline bool ActivateWindow2(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return false;
    }
    const HWND hForegroundWnd = GetForegroundWindow();
    if (!hForegroundWnd) {
        PRINT_WIN32_ERROR_MESSAGE(GetForegroundWindow, L"Failed to retrieve the window handle of the foreground window.")
        return false;
    }
    const DWORD dwForegroundPID = GetWindowThreadProcessId(hForegroundWnd, nullptr);
    if (dwForegroundPID == 0) {
        PRINT_WIN32_ERROR_MESSAGE(GetWindowThreadProcessId, L"Failed to retrieve the foreground process ID.")
        return false;
    }
    const DWORD dwCurrentPID = GetCurrentThreadId();
    if (dwCurrentPID == 0) {
        PRINT_WIN32_ERROR_MESSAGE(GetCurrentThreadId, L"Failed to retrieve the current process ID.")
        return false;
    }
    if (IsWindowVisible(hWnd) == FALSE) {
        SetLastError(ERROR_SUCCESS);
        const BOOL ret = ShowWindow(hWnd, SW_SHOW);
        UNREFERENCED_PARAMETER(ret);
        PRINT_WIN32_ERROR_MESSAGE(ShowWindow, L"Failed to change the window state.")
    }
    if (IsIconic(hWnd) != FALSE) {
        SetLastError(ERROR_SUCCESS);
        const BOOL ret = ShowWindow(hWnd, SW_RESTORE);
        UNREFERENCED_PARAMETER(ret);
        PRINT_WIN32_ERROR_MESSAGE(ShowWindow, L"Failed to change the window state.")
    }
    if (AttachThreadInput(dwCurrentPID, dwForegroundPID, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(AttachThreadInput, L"Failed to attach thread input.")
        return false;
    }
    if (SetForegroundWindow(hWnd) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetForegroundWindow, L"Failed to set the window as the foreground window.")
        return false;
    }
    if (AttachThreadInput(dwCurrentPID, dwForegroundPID, FALSE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(AttachThreadInput, L"Failed to detach thread input.")
        return false;
    }
    return true;
}

[[nodiscard]] static inline HWND CreateWindow2(const DWORD style, const DWORD extendedStyle, const HWND parentWindow, void *extraData, const UINT extraDataSize, const HBRUSH backgroundBrush, const WNDPROC wndProc) noexcept
{
    if (!wndProc) {
        Utils::DisplayErrorDialog(L"Failed to register a window class due to the WindowProc function pointer is null.");
        return nullptr;
    }
    const std::wstring guid = GenerateGUID();
    if (guid.empty()) {
        Utils::DisplayErrorDialog(L"Failed to generate a new GUID.");
        return nullptr;
    }
    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = wndProc;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.lpszClassName = guid.c_str();
    if (backgroundBrush) {
        wcex.hbrBackground = backgroundBrush;
    }
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    if (!(style & WS_CHILD)) {
        wcex.hIcon = LoadIconW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_WIN32ACRYLICHELPER_ICON));
        wcex.hIconSm = LoadIconW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_WIN32ACRYLICHELPER_SMALL_ICON));
    }
    if (extraDataSize > 0) {
        wcex.cbWndExtra = extraDataSize;
    }
    const ATOM atom = RegisterClassExW(&wcex);
    if (atom == INVALID_ATOM) {
        PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW, L"Failed to register a window class.")
        return nullptr;
    }
    const HWND hWnd = CreateWindowExW(
        extendedStyle,       // _In_     DWORD     dwExStyle
        guid.c_str(),        // _In_opt_ LPCWSTR   lpClassName
        nullptr,             // _In_opt_ LPCWSTR   lpWindowName
        style,               // _In_     DWORD     dwStyle
        CW_USEDEFAULT,       // _In_     int       X
        CW_USEDEFAULT,       // _In_     int       Y
        CW_USEDEFAULT,       // _In_     int       nWidth
        CW_USEDEFAULT,       // _In_     int       nHeight
        parentWindow,        // _In_opt_ HWND      hWndParent
        nullptr,             // _In_opt_ HMENU     hMenu
        HINST_THISCOMPONENT, // _In_opt_ HINSTANCE hInstance
        extraData            // _In_opt_ LPVOID    lpParam
        );
    if (!hWnd) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW, L"Failed to create a window.")
        return nullptr;
    }
    return hWnd;
}

[[nodiscard]] static inline bool CloseWindow2(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return false;
    }
    wchar_t className[MAX_PATH] = { L'\0' };
    if (GetClassNameW(hWnd, className, MAX_PATH) <= 0) {
        PRINT_WIN32_ERROR_MESSAGE(GetClassNameW, L"Failed to retrieve the class name of the window.")
        return false;
    }
    if (DestroyWindow(hWnd) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(DestroyWindow, L"Failed to destroy the window.")
        return false;
    }
    if (UnregisterClassW(className, HINST_THISCOMPONENT) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW, L"Failed to unregister the window class.")
        return false;
    }
    return true;
}

class WindowPrivate
{
public:
    explicit WindowPrivate(Window *q, const bool NoRedirectionBitmap) noexcept;
    ~WindowPrivate() noexcept;

    template<typename T>
    [[nodiscard]] static T *GetThisFromHandle(const HWND hWnd) noexcept;

    [[nodiscard]] static int MessageLoop() noexcept;

    [[nodiscard]] std::wstring Title() const noexcept;
    void Title(const std::wstring &value) const noexcept;

    [[nodiscard]] int X() const noexcept;
    void X(const int value) const noexcept;

    [[nodiscard]] int Y() const noexcept;
    void Y(const int value) const noexcept;

    [[nodiscard]] UINT Width() const noexcept;
    void Width(const UINT value) const noexcept;

    [[nodiscard]] UINT Height() const noexcept;
    void Height(const UINT value) const noexcept;

    [[nodiscard]] WindowState Visibility() const noexcept;
    void Visibility(const WindowState value) noexcept;

    [[nodiscard]] bool Active() const noexcept;
    void Active(const bool value) noexcept;

    [[nodiscard]] WindowFrameCorner FrameCorner() const noexcept;
    void FrameCorner(const WindowFrameCorner value) noexcept;

    [[nodiscard]] WindowStartupLocation StartupLocation() const noexcept;
    void StartupLocation(const WindowStartupLocation value) noexcept;

    [[nodiscard]] const Color &TitleBarBackgroundColor() const noexcept;
    void TitleBarBackgroundColor(const Color &value) noexcept;

    [[nodiscard]] WindowTheme Theme() const noexcept;

    [[nodiscard]] UINT DotsPerInch() const noexcept;

    [[nodiscard]] const Color &ColorizationColor() const noexcept;

    [[nodiscard]] WindowColorizationArea ColorizationArea() const noexcept;

    [[nodiscard]] HWND CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData, const UINT extraDataSize) const noexcept;
    [[nodiscard]] HWND WindowHandle() const noexcept;
    [[nodiscard]] bool Move(const int x, const int y) const noexcept;
    [[nodiscard]] bool Resize(const UINT w, const UINT h) const noexcept;
    [[nodiscard]] bool SetGeometry(const int x, const int y, const UINT w, const UINT h) const noexcept;

    [[nodiscard]] UINT GetWindowMetrics2(const WindowMetrics metrics) noexcept;

    void TitleChangeHandler(const StrChangeHandlerCallback &cb) noexcept;
    void XChangeHandler(const IntChangeHandlerCallback &cb) noexcept;
    void YChangeHandler(const IntChangeHandlerCallback &cb) noexcept;
    void WidthChangeHandler(const UIntChangeHandlerCallback &cb) noexcept;
    void HeightChangeHandler(const UIntChangeHandlerCallback &cb) noexcept;
    void VisibilityChangeHandler(const WindowStateChangeHandlerCallback &cb) noexcept;
    void ActiveChangeHandler(const BoolChangeHandlerCallback &cb) noexcept;
    void FrameCornerChangeHandler(const WindowFrameCornerChangeHandlerCallback &cb) noexcept;
    void StartupLocationChangeHandler(const WindowStartupLocationChangeHandlerCallback &cb) noexcept;
    void TitleBarBackgroundColorChangeHandler(const ColorChangeHandlerCallback &cb) noexcept;
    void ThemeChangeHandler(const WindowThemeChangeHandlerCallback &cb) noexcept;
    void DotsPerInchChangeHandler(const UIntChangeHandlerCallback &cb) noexcept;
    void ColorizationColorChangeHandler(const ColorChangeHandlerCallback &cb) noexcept;
    void ColorizationAreaChangeHandler(const WindowColorizationAreaChangeHandlerCallback &cb) noexcept;

    [[nodiscard]] bool CustomMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) const noexcept;
    void CustomMessageHandler(const WindowMessageHandlerCallback &cb) noexcept;

    [[nodiscard]] bool WindowMessageFilter(const MSG *message) const noexcept;
    void WindowMessageFilter(const WindowMessageFilterCallback &cb) noexcept;

private:
    WindowPrivate(const WindowPrivate &) = delete;
    WindowPrivate &operator=(const WindowPrivate &) = delete;
    WindowPrivate(WindowPrivate &&) = delete;
    WindowPrivate &operator=(WindowPrivate &&) = delete;

private:
    [[nodiscard]] bool Initialize() noexcept;
    [[nodiscard]] static LRESULT CALLBACK WindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept;
    [[nodiscard]] bool InternalMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept;
    [[nodiscard]] POINT GetWindowPosition2() const noexcept;
    [[nodiscard]] SIZE GetWindowSize2() const noexcept;
    [[nodiscard]] bool TriggerWindowFrameChange2() const noexcept;
    [[nodiscard]] bool RefreshWindowTheme2() const noexcept;
    [[nodiscard]] bool OpenSystemMenu2(const POINT pos) const noexcept;
    [[nodiscard]] bool SetWindowState2(const WindowState state) noexcept;
    [[nodiscard]] UINT GetWindowDPI2() const noexcept;
    [[nodiscard]] UINT GetWindowVisibleFrameBorderThickness2() const noexcept;
    [[nodiscard]] bool UpdateWindowFrameMargins2() noexcept;
    void TitleChangeHandler() const noexcept;
    void XChangeHandler() const noexcept;
    void YChangeHandler() const noexcept;
    void WidthChangeHandler() const noexcept;
    void HeightChangeHandler() const noexcept;
    void VisibilityChangeHandler() const noexcept;
    void ActiveChangeHandler() const noexcept;
    void FrameCornerChangeHandler() const noexcept;
    void StartupLocationChangeHandler() const noexcept;
    void TitleBarBackgroundColorChangeHandler() const noexcept;
    void ThemeChangeHandler() const noexcept;
    void DotsPerInchChangeHandler() const noexcept;
    void ColorizationColorChangeHandler() const noexcept;
    void ColorizationAreaChangeHandler() const noexcept;

private:
    Window *q_ptr = nullptr;
    HWND m_window = nullptr;
    std::wstring m_title = {};
    int m_x = 0;
    int m_y = 0;
    UINT m_width = 0;
    UINT m_height = 0;
    WindowState m_visibility = WindowState::Hidden;
    bool m_active = false;
    WindowFrameCorner m_frameCorner = WindowFrameCorner::Square;
    WindowStartupLocation m_startupLocation = WindowStartupLocation::Default;
    Color m_titleBarBackgroundColor = Color();
    WindowTheme m_theme = WindowTheme::Light;
    Color m_colorizationColor = Color();
    WindowColorizationArea m_colorizationArea = WindowColorizationArea::None;
    UINT m_dpi = 0;
    HBRUSH m_windowBackgroundBrush = nullptr;
    bool m_exposed = false;
    UINT m_resizeBorderThicknessX = 0;
    UINT m_resizeBorderThicknessY = 0;
    UINT m_windowVisibleFrameBorderThickness = 0;
    UINT m_captionHeight = 0;
    UINT m_windowIconWidth = 0;
    UINT m_windowIconHeight = 0;
    UINT m_windowSmallIconWidth = 0;
    UINT m_windowSmallIconHeight = 0;
    HBRUSH m_titleBarBackgroundBrush = nullptr;
    StrChangeHandlerCallback m_titleChangeHandlerCallback = nullptr;
    IntChangeHandlerCallback m_xChangeHandlerCallback = nullptr;
    IntChangeHandlerCallback m_yChangeHandlerCallback = nullptr;
    UIntChangeHandlerCallback m_widthChangeHandlerCallback = nullptr;
    UIntChangeHandlerCallback m_heightChangeHandlerCallback = nullptr;
    WindowStateChangeHandlerCallback m_visibilityChangeHandlerCallback = nullptr;
    BoolChangeHandlerCallback m_activeChangeHandlerCallback = nullptr;
    WindowFrameCornerChangeHandlerCallback m_frameCornerChangeHandlerCallback = nullptr;
    WindowStartupLocationChangeHandlerCallback m_startupLocationChangeHandlerCallback = nullptr;
    ColorChangeHandlerCallback m_titleBarBackgroundColorChangeHandlerCallback = nullptr;
    WindowThemeChangeHandlerCallback m_themeChangeHandlerCallback = nullptr;
    UIntChangeHandlerCallback m_dotsPerInchChangeHandlerCallback = nullptr;
    ColorChangeHandlerCallback m_colorizationColorChangeHandlerCallback = nullptr;
    WindowColorizationAreaChangeHandlerCallback m_colorizationAreaChangeHandlerCallback = nullptr;
    WindowMessageHandlerCallback m_customMessageHandlerCallback = nullptr;
    WindowMessageFilterCallback m_windowMessageFilterCallback = nullptr;
    bool m_bNoRedirectionBitmap = false;
};

template<typename T>
T *WindowPrivate::GetThisFromHandle(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return nullptr;
    }
    SetLastError(ERROR_SUCCESS);
    const LONG_PTR result = GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    PRINT_WIN32_ERROR_MESSAGE(GetWindowLongPtrW, L"Failed to retrieve the user data of the window.")
    return ((result == 0) ? nullptr : reinterpret_cast<T *>(result));
}

POINT WindowPrivate::GetWindowPosition2() const noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Can't retrieve the window position due to the window has not been created yet.");
        return {};
    }
    const RECT frameGeometry = GetWindowFrameGeometry(m_window);
    return {frameGeometry.left, frameGeometry.top};
}

SIZE WindowPrivate::GetWindowSize2() const noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window size due to the window has not been created yet.");
        return {};
    }
    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(m_window, &rect) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(GetClientRect, L"Failed to retrieve the window client area size.")
        return {};
    }
    return {RECT_WIDTH(rect), RECT_HEIGHT(rect)};
}

bool WindowPrivate::TriggerWindowFrameChange2() const noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to trigger a window frame change event due to the window has not been created yet.");
        return false;
    }
    constexpr const UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (SetWindowPos(m_window, nullptr, 0, 0, 0, 0, flags) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to trigger a window frame change event for the window.")
        return false;
    }
    return true;
}

bool WindowPrivate::RefreshWindowTheme2() const noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to refresh the window theme due to the window has not been created yet.");
        return false;
    }
    BOOL enableDarkFrame = FALSE;
    std::wstring themeName = {};
    switch (m_theme) {
    case WindowTheme::Light: {
        enableDarkFrame = FALSE;
        themeName = {};
    } break;
    case WindowTheme::Dark: {
        enableDarkFrame = TRUE;
        themeName = L"Dark_Explorer";
    } break;
    case WindowTheme::HighContrast: {
        // ### TODO
    } break;
    }
    const HRESULT hr1 = DwmSetWindowAttribute(m_window, _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &enableDarkFrame, sizeof(enableDarkFrame));
    const HRESULT hr2 = DwmSetWindowAttribute(m_window, _DWMWA_USE_IMMERSIVE_DARK_MODE, &enableDarkFrame, sizeof(enableDarkFrame));
    const HRESULT hr3 = SetWindowTheme(m_window, themeName.c_str(), nullptr);
    if (FAILED(hr1) && FAILED(hr2)) {
        PRINT_HR_ERROR_MESSAGE(DwmSetWindowAttribute, hr2, L"Failed to change the window dark mode state.")
        return false;
    }
    if (FAILED(hr3)) {
        PRINT_HR_ERROR_MESSAGE(SetWindowTheme, hr3, L"Failed to change the window theme.")
        return false;
    }
    return true;
}

bool WindowPrivate::OpenSystemMenu2(const POINT pos) const noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to open the system menu due to the window has not been created yet.");
        return false;
    }
    const HMENU menu = GetSystemMenu(m_window, FALSE);
    if (!menu) {
        PRINT_WIN32_ERROR_MESSAGE(GetSystemMenu, L"Failed to retrieve the system menu of the window.")
        return false;
    }
    // Update the options based on window state.
    MENUITEMINFOW mii;
    SecureZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fType = MFT_STRING;
    const auto setState = [&mii, menu](const UINT item, const bool enabled) -> bool {
        mii.fState = (enabled ? MF_ENABLED : MF_DISABLED);
        if (SetMenuItemInfoW(menu, item, FALSE, &mii) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW, L"Failed to set menu item information.")
            return false;
        }
        return true;
    };
    const bool max = (m_visibility == WindowState::Maximized);
    if (!setState(SC_RESTORE, max)) {
        return false;
    }
    if (!setState(SC_MOVE, !max)) {
        return false;
    }
    if (!setState(SC_SIZE, !max)) {
        return false;
    }
    if (!setState(SC_MINIMIZE, true)) {
        return false;
    }
    if (!setState(SC_MAXIMIZE, !max)) {
        return false;
    }
    if (!setState(SC_CLOSE, true)) {
        return false;
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetMenuDefaultItem, L"Failed to set default menu item.")
        return false;
    }
    // ### TODO: support RTL layout: TPM_LAYOUTRTL
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, pos.x, pos.y, 0, m_window, nullptr);
    if (ret != 0) {
        if (PostMessageW(m_window, WM_SYSCOMMAND, ret, 0) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(PostMessageW, L"Failed to post message.")
            return false;
        }
    }
    return true;
}

bool WindowPrivate::SetWindowState2(const WindowState state) noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to change the window state due to the window has not been created yet.");
        return false;
    }
    int nCmdShow = SW_SHOW;
    switch (state) {
    case WindowState::Minimized: {
        nCmdShow = SW_MINIMIZE;
    } break;
    case WindowState::Windowed: {
        if (m_visibility == WindowState::Hidden) {
            if (m_exposed) {
                nCmdShow = SW_SHOW;
            } else {
                m_exposed = true;
                nCmdShow = SW_NORMAL;
            }
        } else {
            nCmdShow = SW_RESTORE;
        }
    } break;
    case WindowState::Maximized: {
        nCmdShow = SW_MAXIMIZE;
    } break;
    case WindowState::Hidden: {
        nCmdShow = SW_HIDE;
    } break;
    }
    // Don't check ShowWindow()'s result because it returns the previous window state rather than
    // the operation result of itself.
    SetLastError(ERROR_SUCCESS);
    const BOOL previousState = ShowWindow(m_window, nCmdShow);
    UNREFERENCED_PARAMETER(previousState);
    PRINT_WIN32_ERROR_MESSAGE(ShowWindow, L"Failed to change the window state.")
    if ((state == WindowState::Windowed) || (state == WindowState::Maximized)) {
        if (UpdateWindow(m_window) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UpdateWindow, L"Failed to update the window.")
            return false;
        }
    }
    return true;
}

UINT WindowPrivate::GetWindowDPI2() const noexcept
{
    return (m_window ? GetDpiForWindow(m_window) : USER_DEFAULT_SCREEN_DPI);
}

UINT WindowPrivate::GetWindowVisibleFrameBorderThickness2() const noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window visible frame border thickness due to the window has not been created yet.");
        return DefaultWindowVisibleFrameBorderThickness;
    }
    UINT value = 0;
    const HRESULT hr = DwmGetWindowAttribute(m_window, _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
    if (SUCCEEDED(hr)) {
        // The returned value is already scaled to the DPI automatically.
        // Don't double scale it!!!
        return value;
    } else {
        // We just eat this error because this enumeration value is only available
        // on Windows 11 and onwards, so querying it's value will always result in
        // a "parameter error" (error code: 87) on older systems.
        const auto dpr = (static_cast<double>(m_dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
        return static_cast<UINT>(std::round(static_cast<double>(DefaultWindowVisibleFrameBorderThickness) * dpr));
    }
}

bool WindowPrivate::Initialize() noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to initialize WindowPrivate due to this window has not been created.");
        return false;
    }
    m_dpi = GetWindowDPI2();
    const std::wstring dpiDbgMsg = std::wstring(L"Current window's dots-per-inch (DPI): ") + std::to_wstring(m_dpi) + std::wstring(__NEW_LINE);
    OutputDebugStringW(dpiDbgMsg.c_str());
    if (!UpdateWindowFrameMargins2()) {
        Utils::DisplayErrorDialog(L"Failed to update the window frame margins.");
        return false;
    }
    if (!TriggerWindowFrameChange2()) {
        Utils::DisplayErrorDialog(L"Failed to trigger a window frame change event for the window.");
        return false;
    }
    m_theme = GetGlobalApplicationTheme2();
    if (!RefreshWindowTheme2()) {
        Utils::DisplayErrorDialog(L"Failed to change the window theme.");
        return false;
    }
    m_colorizationColor = Color(GetGlobalColorizationColor2());
    m_colorizationArea = GetGlobalColorizationArea2();
    m_visibility = WindowState::Hidden;
    m_active = false;
    const POINT windowPosition = GetWindowPosition2();
    const SIZE windowSize = GetWindowSize2();
    m_x = windowPosition.x;
    m_y = windowPosition.y;
    m_width = windowSize.cx;
    m_height = windowSize.cy;
    m_title = {};
    m_frameCorner = ((WindowsVersion::CurrentVersion() >= WindowsVersion::Windows11) ? WindowFrameCorner::Round : WindowFrameCorner::Square);
    m_startupLocation = WindowStartupLocation::Default;
    return true;
}

WindowPrivate::WindowPrivate(Window *q, const bool NoRedirectionBitmap) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"WindowPrivate's q pointer is null.");
        std::exit(-1);
    }
    q_ptr = q;
    m_bNoRedirectionBitmap = NoRedirectionBitmap;
    if (!m_bNoRedirectionBitmap) {
        // We need this window background brush in WM_PAINT, so create it early.
        m_windowBackgroundBrush = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        if (!m_windowBackgroundBrush) {
            PRINT_WIN32_ERROR_MESSAGE(GetStockObject, L"Failed to retrieve the black brush.")
            std::exit(-1);
        }
        // Create the title bar background brush early, we'll need it in WM_PAINT.
        TitleBarBackgroundColor(Color::FromRgba(0, 0, 0));
    }
    m_window = CreateWindow2(WS_OVERLAPPEDWINDOW, (m_bNoRedirectionBitmap ? WS_EX_NOREDIRECTIONBITMAP : 0L), nullptr, this, sizeof(WindowPrivate *), m_windowBackgroundBrush, WindowProc);
    if (m_window) {
        if (!Initialize()) {
            Utils::DisplayErrorDialog(L"Failed to initialize WindowPrivate.");
            std::exit(-1);
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to create this window.");
        std::exit(-1);
    }
}

WindowPrivate::~WindowPrivate() noexcept
{
    // According to MSDN, it's not needed to delete the window background brush
    // due to it's retrieved through GetStockObject(). But calling DeleteObject()
    // on it is harmless.
    if (m_titleBarBackgroundBrush) {
        if (DeleteObject(m_titleBarBackgroundBrush) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DeleteObject, L"Failed to delete the title bar background brush.")
        } else {
            m_titleBarBackgroundBrush = nullptr;
        }
    }
    if (m_window) {
        if (CloseWindow2(m_window)) {
            m_window = nullptr;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close this window.");
        }
    }
}

int WindowPrivate::MessageLoop() noexcept
{
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
#if 0
        const auto that = GetThisFromHandle<WindowPrivate>(msg.hwnd);
        const bool filtered = (that && that->WindowMessageFilter(&msg));
#else
        static constexpr const bool filtered = false;
#endif
        if (!filtered) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}

std::wstring WindowPrivate::Title() const noexcept
{
    return m_title;
}

void WindowPrivate::Title(const std::wstring &value) const noexcept
{
    if (m_title != value) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Failed to change the window title due to the window has not been created yet.");
            return;
        }
        if (SetWindowTextW(m_window, value.c_str()) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowTextW, L"Failed to change the window title.")
        }
    }
}

int WindowPrivate::X() const noexcept
{
    return m_x;
}

void WindowPrivate::X(const int value) const noexcept
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

void WindowPrivate::Y(const int value) const noexcept
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

void WindowPrivate::Width(const UINT value) const noexcept
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

void WindowPrivate::Height(const UINT value) const noexcept
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
        if (!SetWindowState2(value)) {
            Utils::DisplayErrorDialog(L"Failed to update the Visibility property of this window.");
        }
    }
}

bool WindowPrivate::Active() const noexcept
{
    return m_active;
}

void WindowPrivate::Active(const bool value) noexcept
{
    if (m_active != value) {
        if (!ActivateWindow2(m_window)) {
            Utils::DisplayErrorDialog(L"Failed to activate this window.");
        }
    }
}

WindowFrameCorner WindowPrivate::FrameCorner() const noexcept
{
    return m_frameCorner;
}

void WindowPrivate::FrameCorner(const WindowFrameCorner value) noexcept
{
    if (m_frameCorner != value) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Can't change the window frame corner style due to the window has not been created yet.");
            return;
        }
        const DwmWindowCornerPreference wcp = ((value == WindowFrameCorner::Round) ? DwmWindowCornerPreference::Round : DwmWindowCornerPreference::DoNotRound);
        const HRESULT hr = DwmSetWindowAttribute(m_window, _DWMWA_WINDOW_CORNER_PREFERENCE, &wcp, sizeof(wcp));
        if (SUCCEEDED(hr)) {
            m_frameCorner = value;
        } else {
            // ### TODO: SetWindowRgn
        }
    }
}

WindowStartupLocation WindowPrivate::StartupLocation() const noexcept
{
    return m_startupLocation;
}

void WindowPrivate::StartupLocation(const WindowStartupLocation value) noexcept
{
    if (m_startupLocation != value) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Can't set the window startup location due to the window has not been created yet.");
            return;
        }
        m_startupLocation = value;
        StartupLocationChangeHandler();
        RECT rect = {0, 0, 0, 0};
        if (m_startupLocation == WindowStartupLocation::Default) {
            return;
        } else if (m_startupLocation == WindowStartupLocation::OwnerCenter) {
            // ### TODO
            return;
        } else if (m_startupLocation == WindowStartupLocation::DesktopCenter) {
            rect = GetScreenGeometry(m_window, false);
        } else if (m_startupLocation == WindowStartupLocation::ScreenCenter) {
            rect = GetScreenGeometry(m_window, true);
        }
        const UINT rectWidth = RECT_WIDTH(rect);
        const UINT rectHeight = RECT_HEIGHT(rect);
        const auto newX = static_cast<int>(std::round(static_cast<double>(rectWidth - m_width) / 2.0));
        const auto newY = static_cast<int>(std::round(static_cast<double>(rectHeight - m_height) / 2.0));
        if (!Move(newX, newY)) {
            Utils::DisplayErrorDialog(L"Failed to change the window geometry.");
        }
    }
}

const Color &WindowPrivate::TitleBarBackgroundColor() const noexcept
{
    return m_titleBarBackgroundColor;
}

void WindowPrivate::TitleBarBackgroundColor(const Color &value) noexcept
{
    if (m_titleBarBackgroundColor != value) {
        if (m_titleBarBackgroundBrush) {
            if (DeleteObject(m_titleBarBackgroundBrush) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(DeleteObject, L"Failed to delete the previous title bar background brush.")
                return;
            }
            m_titleBarBackgroundBrush = nullptr;
        }
        m_titleBarBackgroundBrush = CreateSolidBrush(value.ToWin32());
        if (m_titleBarBackgroundBrush) {
            m_titleBarBackgroundColor = value;
            TitleBarBackgroundColorChangeHandler();
        } else {
            PRINT_WIN32_ERROR_MESSAGE(CreateSolidBrush, L"Failed to create the title bar background brush.")
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

const Color &WindowPrivate::ColorizationColor() const noexcept
{
    return m_colorizationColor;
}

WindowColorizationArea WindowPrivate::ColorizationArea() const noexcept
{
    return m_colorizationArea;
}

HWND WindowPrivate::CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData, const UINT extraDataSize) const noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Can't create the child window due to the parent window has not been created yet.");
        return nullptr;
    }
    return CreateWindow2((style | WS_CHILD), extendedStyle, m_window, extraData, extraDataSize, m_windowBackgroundBrush, wndProc);
}

HWND WindowPrivate::WindowHandle() const noexcept
{
    return m_window;
}

bool WindowPrivate::Move(const int x, const int y) const noexcept
{
    return SetGeometry(x, y, m_width, m_height);
}

bool WindowPrivate::Resize(const UINT w, const UINT h) const noexcept
{
    return SetGeometry(m_x, m_y, w, h);
}

bool WindowPrivate::SetGeometry(const int x, const int y, const UINT w, const UINT h) const noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to change the window geometry due to the window has not been created yet.");
        return false;
    }
    constexpr const UINT flags = (SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    if (SetWindowPos(m_window, nullptr, x, y, w, h, flags) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to change the window geometry.")
        return false;
    }
    return true;
}

UINT WindowPrivate::GetWindowMetrics2(const WindowMetrics metrics) noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window metrics due to the window has not been created yet.");
        return 0;
    }
    switch (metrics) {
    case WindowMetrics::ResizeBorderThicknessX: {
        if (m_resizeBorderThicknessX == 0) {
            const int paddedBorderThicknessX = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, m_dpi);
            const int sizeFrameThicknessX = GetSystemMetricsForDpi(SM_CXSIZEFRAME, m_dpi);
            m_resizeBorderThicknessX = (paddedBorderThicknessX + sizeFrameThicknessX);
        }
        return m_resizeBorderThicknessX;
    } break;
    case WindowMetrics::ResizeBorderThicknessY: {
        if (m_resizeBorderThicknessY == 0) {
            const int paddedBorderThicknessY = GetSystemMetricsForDpi(SM_CYPADDEDBORDER, m_dpi);
            const int sizeFrameThicknessY = GetSystemMetricsForDpi(SM_CYSIZEFRAME, m_dpi);
            m_resizeBorderThicknessY = (paddedBorderThicknessY + sizeFrameThicknessY);
        }
        return m_resizeBorderThicknessY;
    } break;
    case WindowMetrics::WindowVisibleFrameBorderThickness: {
        if (m_windowVisibleFrameBorderThickness == 0) {
            m_windowVisibleFrameBorderThickness = GetWindowVisibleFrameBorderThickness2();
        }
        return m_windowVisibleFrameBorderThickness;
    } break;
    case WindowMetrics::CaptionHeight: {
        if (m_captionHeight == 0) {
            m_captionHeight = GetSystemMetricsForDpi(SM_CYCAPTION, m_dpi);
        }
        return m_captionHeight;
    } break;
    case WindowMetrics::WindowIconWidth: {
        if (m_windowIconWidth == 0) {
            m_windowIconWidth = GetSystemMetricsForDpi(SM_CXICON, m_dpi);
        }
        return m_windowIconWidth;
    } break;
    case WindowMetrics::WindowIconHeight: {
        if (m_windowIconHeight == 0) {
            m_windowIconHeight = GetSystemMetricsForDpi(SM_CYICON, m_dpi);
        }
        return m_windowIconHeight;
    } break;
    case WindowMetrics::WindowSmallIconWidth: {
        if (m_windowSmallIconWidth == 0) {
            m_windowSmallIconWidth = GetSystemMetricsForDpi(SM_CXSMICON, m_dpi);
        }
        return m_windowSmallIconWidth;
    } break;
    case WindowMetrics::WindowSmallIconHeight: {
        if (m_windowSmallIconHeight == 0) {
            m_windowSmallIconHeight = GetSystemMetricsForDpi(SM_CYSMICON, m_dpi);
        }
        return m_windowSmallIconHeight;
    } break;
    }
    return 0;
}

void WindowPrivate::TitleChangeHandler(const StrChangeHandlerCallback &cb) noexcept
{
    m_titleChangeHandlerCallback = cb;
}

void WindowPrivate::XChangeHandler(const IntChangeHandlerCallback &cb) noexcept
{
    m_xChangeHandlerCallback = cb;
}

void WindowPrivate::YChangeHandler(const IntChangeHandlerCallback &cb) noexcept
{
    m_yChangeHandlerCallback = cb;
}

void WindowPrivate::WidthChangeHandler(const UIntChangeHandlerCallback &cb) noexcept
{
    m_widthChangeHandlerCallback = cb;
}

void WindowPrivate::HeightChangeHandler(const UIntChangeHandlerCallback &cb) noexcept
{
    m_heightChangeHandlerCallback = cb;
}

void WindowPrivate::VisibilityChangeHandler(const WindowStateChangeHandlerCallback &cb) noexcept
{
    m_visibilityChangeHandlerCallback = cb;
}

void WindowPrivate::ActiveChangeHandler(const BoolChangeHandlerCallback &cb) noexcept
{
    m_activeChangeHandlerCallback = cb;
}

void WindowPrivate::FrameCornerChangeHandler(const WindowFrameCornerChangeHandlerCallback &cb) noexcept
{
    m_frameCornerChangeHandlerCallback = cb;
}

void WindowPrivate::StartupLocationChangeHandler(const WindowStartupLocationChangeHandlerCallback &cb) noexcept
{
    m_startupLocationChangeHandlerCallback = cb;
}

void WindowPrivate::TitleBarBackgroundColorChangeHandler(const ColorChangeHandlerCallback &cb) noexcept
{
    m_titleBarBackgroundColorChangeHandlerCallback = cb;
}

void WindowPrivate::ThemeChangeHandler(const WindowThemeChangeHandlerCallback &cb) noexcept
{
    m_themeChangeHandlerCallback = cb;
}

void WindowPrivate::DotsPerInchChangeHandler(const UIntChangeHandlerCallback &cb) noexcept
{
    m_dotsPerInchChangeHandlerCallback = cb;
}

void WindowPrivate::ColorizationColorChangeHandler(const ColorChangeHandlerCallback &cb) noexcept
{
    m_colorizationColorChangeHandlerCallback = cb;
}

void WindowPrivate::ColorizationAreaChangeHandler(const WindowColorizationAreaChangeHandlerCallback &cb) noexcept
{
    m_colorizationAreaChangeHandlerCallback = cb;
}

bool WindowPrivate::CustomMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) const noexcept
{
    return (m_customMessageHandlerCallback ? m_customMessageHandlerCallback(message, wParam, lParam, result) : false);
}

void WindowPrivate::CustomMessageHandler(const WindowMessageHandlerCallback &cb) noexcept
{
    m_customMessageHandlerCallback = cb;
}

bool WindowPrivate::WindowMessageFilter(const MSG *message) const noexcept
{
    return (m_windowMessageFilterCallback ? m_windowMessageFilterCallback(message) : false);
}

void WindowPrivate::WindowMessageFilter(const WindowMessageFilterCallback &cb) noexcept
{
    m_windowMessageFilterCallback = cb;
}

LRESULT CALLBACK WindowPrivate::WindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept
{
    if (message == WM_NCCREATE) {
        const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        const auto that = static_cast<WindowPrivate *>(cs->lpCreateParams);
        // SetWindowLongPtrW() won't modify the Last Error state on success
        // and it's return value is the previous data so we have to judge
        // the actual operation result from the Last Error state manually.
        SetLastError(ERROR_SUCCESS);
        const LONG_PTR result = SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
        PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to set the extra data to the window.")
        if (result != 0) {
            Utils::DisplayErrorDialog(L"The extra data of this window has been overwritten.");
        }
    } else if (message == WM_NCDESTROY) {
        // See the above comments.
        SetLastError(ERROR_SUCCESS);
        const LONG_PTR result = SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
        PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to clear the extra data of the window.")
        if (result == 0) {
            Utils::DisplayErrorDialog(L"This window doesn't contain any extra data.");
        }
    }
    if (const auto that = GetThisFromHandle<WindowPrivate>(hWnd)) {
        LRESULT result = 0;
        if (that->CustomMessageHandler(message, wParam, lParam, &result)) {
            return result;
        }
        if (that->InternalMessageHandler(message, wParam, lParam, &result)) {
            return result;
        }
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

bool WindowPrivate::InternalMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept
{
    if (!result) {
        Utils::DisplayErrorDialog(L"InternalMessageHandler: the pointer to the result of the WindowProc function is null.");
        return false;
    }
    if (!m_window) {
        //Utils::DisplayErrorDialog(L"InternalMessageHandler: this window has not been created yet.");
        return false;
    }
    switch (message) {
    case WM_MOVE: {
        m_x = GET_X_LPARAM(lParam);
        m_y = GET_Y_LPARAM(lParam);
        XChangeHandler();
        YChangeHandler();
    } break;
    case WM_SIZE: {
        bool visibilityChanged = false;
        if (wParam == SIZE_RESTORED) {
            if (m_visibility != WindowState::Windowed) {
                m_visibility = WindowState::Windowed;
                visibilityChanged = true;
            }
        } else if (wParam == SIZE_MINIMIZED) {
            m_visibility = WindowState::Minimized;
            visibilityChanged = true;
        } else if (wParam == SIZE_MAXIMIZED) {
            m_visibility = WindowState::Maximized;
            visibilityChanged = true;
        }
        m_width = LOWORD(lParam);
        m_height = HIWORD(lParam);
        if (visibilityChanged) {
            VisibilityChangeHandler();
        }
        WidthChangeHandler();
        HeightChangeHandler();
        if (visibilityChanged && (m_visibility != WindowState::Minimized)) {
            if (!UpdateWindowFrameMargins2()) {
                Utils::DisplayErrorDialog(L"Failed to update the window frame margins.");
                return false;
            }
            if (!TriggerWindowFrameChange2()) {
                Utils::DisplayErrorDialog(L"Failed to trigger a window frame change event for the window.");
                return false;
            }
        }
    } break;
    case WM_SETTINGCHANGE: {
        // wParam == 0: User-wide setting change
        // wParam == 1: System-wide setting change
        // ### TODO: how to detect high contrast theme here
        if (((wParam == 0) || (wParam == 1)) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            m_colorizationArea = GetGlobalColorizationArea2();
            m_theme = GetGlobalApplicationTheme2();
            ColorizationAreaChangeHandler();
            ThemeChangeHandler();
            if (!RefreshWindowTheme2()) {
                Utils::DisplayErrorDialog(L"Failed to refresh the window theme.");
                return false;
            }
        }
    } break;
    case WM_DPICHANGED: {
        const UINT oldDPI = m_dpi;
        const UINT dpiX = LOWORD(wParam);
        const UINT dpiY = HIWORD(wParam);
        m_dpi = static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
        DotsPerInchChangeHandler();
        const std::wstring dpiDbgMsg = std::wstring(L"Current window's dots-per-inch (DPI) has changed from ") + std::to_wstring(oldDPI) + std::wstring(L" to ") + std::to_wstring(m_dpi) + std::wstring(L".") + std::wstring(__NEW_LINE);
        OutputDebugStringW(dpiDbgMsg.c_str());
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (SetGeometry(prcNewWindow->left, prcNewWindow->top, RECT_WIDTH(*prcNewWindow), RECT_HEIGHT(*prcNewWindow))) {
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Failed to scale the window size according to the new DPI.");
            return false;
        }
    } break;
    case WM_DWMCOLORIZATIONCOLORCHANGED: {
        m_colorizationColor = Color(static_cast<COLORREF>(wParam)); // The color format is 0xAARRGGBB.
        ColorizationColorChangeHandler();
    } break;
    case WM_PAINT: {
        if (m_bNoRedirectionBitmap) {
            break;
        }
        if (!m_windowBackgroundBrush || !m_titleBarBackgroundBrush) {
            Utils::DisplayErrorDialog(L"Can't paint the window when the window background brush and/or the title bar background brush is null.");
            return false;
        }
        PAINTSTRUCT ps;
        SecureZeroMemory(&ps, sizeof(ps));
        const HDC hDC = BeginPaint(m_window, &ps);
        if (!hDC) {
            PRINT_WIN32_ERROR_MESSAGE(BeginPaint, L"Failed to start painting.")
            return false;
        }
        const LONG topBorderHeight = ((m_visibility == WindowState::Maximized) ? 0 : GetWindowVisibleFrameBorderThickness2());
        if (ps.rcPaint.top < topBorderHeight) {
            RECT rcTopBorder = ps.rcPaint;
            rcTopBorder.bottom = topBorderHeight;
            // To show the original top border, we have to paint on top of it with
            // the alpha component set to 0. This page recommends to paint the area
            // in black using the stock BLACK_BRUSH to do this:
            // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
            if (FillRect(hDC, &rcTopBorder, m_windowBackgroundBrush) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(FillRect, L"Failed to paint the window background.")
                return false;
            }
        }
        if (ps.rcPaint.bottom > topBorderHeight) {
            RECT rcRest = ps.rcPaint;
            rcRest.top = topBorderHeight;
            // To hide the original title bar, we have to paint on top of it with
            // the alpha component set to 255. This is a hack to do it with GDI.
            // See UpdateWindowFrameMargins2() for more information.
            HDC opaqueDc = nullptr;
            BP_PAINTPARAMS params;
            SecureZeroMemory(&params, sizeof(params));
            params.cbSize = sizeof(params);
            params.dwFlags = (BPPF_NOCLIP | BPPF_ERASE);
            const HPAINTBUFFER buf = BeginBufferedPaint(hDC, &rcRest, BPBF_TOPDOWNDIB, &params, &opaqueDc);
            if (!buf || !opaqueDc) {
                PRINT_WIN32_ERROR_MESSAGE(BeginBufferedPaint, L"Failed to begin the buffered paint.")
                return false;
            }
            if (FillRect(opaqueDc, &rcRest, m_titleBarBackgroundBrush) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(FillRect, L"Failed to paint the window background.")
                return false;
            }
            HRESULT hr = BufferedPaintSetAlpha(buf, nullptr, 255);
            if (FAILED(hr)) {
                PRINT_HR_ERROR_MESSAGE(BufferedPaintSetAlpha, hr, L"Failed to change alpha of the buffered paint.")
                return false;
            }
            hr = EndBufferedPaint(buf, TRUE);
            if (FAILED(hr)) {
                PRINT_HR_ERROR_MESSAGE(EndBufferedPaint, hr, L"Failed to finish the buffered paint.")
                return false;
            }
        }
        if (EndPaint(m_window, &ps) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(EndPaint, L"Failed to end painting.")
            return false;
        }
        *result = 0;
        return true;
    } break;
    case WM_SETTEXT: {
        const auto title = reinterpret_cast<LPCWSTR>(lParam);
        if (title) {
            m_title = title;
        } else {
            m_title = {};
        }
        TitleChangeHandler();
    } break;
    case WM_SETICON: {} break;
    case WM_CLOSE: {
        if (CloseWindow2(m_window)) {
            m_window = nullptr;
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close this window.");
            return false;
        }
    } break;
    case WM_DESTROY: {
        PostQuitMessage(0);
        *result = 0;
        return true;
    } break;
    case WM_NCCREATE: {
        if (EnableNonClientDpiScaling(m_window) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(EnableNonClientDpiScaling, L"Failed to enable window non-client area automatic DPI scaling.")
        }
    } break;
    case WM_NCCALCSIZE: {
        if (static_cast<BOOL>(wParam) == FALSE) {
            *result = 0;
            return true;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        // Store the original top before the default window proc applies the default frame.
        const LONG originalTop = clientRect->top;
        // Apply the default frame
        const LRESULT ret = DefWindowProcW(m_window, WM_NCCALCSIZE, TRUE, lParam);
        if (ret != 0) {
            *result = ret;
            return true;
        }
        // Re-apply the original top from before the size of the default frame was applied.
        clientRect->top = originalTop;
        bool nonClientAreaExists = false;
        const bool max = (m_visibility == WindowState::Maximized);
        const bool full = false; // ### TODO
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
            const UINT resizeBorderThicknessY = GetWindowMetrics2(WindowMetrics::ResizeBorderThicknessY);
            clientRect->top += resizeBorderThicknessY;
            nonClientAreaExists = true;
        }
        // Attempt to detect if there's an autohide taskbar, and if
        // there is, reduce our size a bit on the side with the taskbar,
        // so the user can still mouse-over the taskbar to reveal it.
        // Make sure to use MONITOR_DEFAULTTONEAREST, so that this will
        // still find the right monitor even when we're restoring from
        // minimized.
        if (max || full) {
            APPBARDATA abd;
            SecureZeroMemory(&abd, sizeof(abd));
            abd.cbSize = sizeof(abd);
            // First, check if we have an auto-hide taskbar at all:
            if (SHAppBarMessage(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) {
                const HMONITOR mon = MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
                if (mon) {
                    MONITORINFO mi;
                    SecureZeroMemory(&mi, sizeof(mi));
                    mi.cbSize = sizeof(mi);
                    if (GetMonitorInfoW(mon, &mi) == FALSE) {
                        PRINT_WIN32_ERROR_MESSAGE(GetMonitorInfoW, L"Failed to retrieve the screen information.")
                        return false;
                    }
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
                        return (reinterpret_cast<HWND>(SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &abd2)) != nullptr);
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
                        clientRect->top += DefaultAutoHideTaskBarThicknessY;
                        nonClientAreaExists = true;
                    } else if (hasAutohideTaskbar(ABE_BOTTOM)) {
                        clientRect->bottom -= DefaultAutoHideTaskBarThicknessY;
                        nonClientAreaExists = true;
                    } else if (hasAutohideTaskbar(ABE_LEFT)) {
                        clientRect->left += DefaultAutoHideTaskBarThicknessX;
                        nonClientAreaExists = true;
                    } else if (hasAutohideTaskbar(ABE_RIGHT)) {
                        clientRect->right -= DefaultAutoHideTaskBarThicknessX;
                        nonClientAreaExists = true;
                    }
                } else {
                    PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow, L"Failed to retrieve the corresponding screen.")
                    return false;
                }
            }
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
        const POINT globalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT localPos = globalPos;
        if (ScreenToClient(m_window, &localPos) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(ScreenToClient, L"Failed to translate from screen coordinate to window coordinate.")
            return false;
        }
        const auto resizeBorderThicknessY = static_cast<LONG>(GetWindowMetrics2(WindowMetrics::ResizeBorderThicknessY));
        const UINT captionHeight = GetWindowMetrics2(WindowMetrics::CaptionHeight);
        const LONG titleBarHeight = ((m_visibility == WindowState::Hidden) ? 0 : ((m_visibility == WindowState::Maximized) ? captionHeight : (captionHeight + resizeBorderThicknessY)));
        const bool isTitleBar = (((m_visibility == WindowState::Windowed) || (m_visibility == WindowState::Maximized)) ? (localPos.y <= titleBarHeight) : false);
        const bool isTop = ((m_visibility == WindowState::Windowed) ? (localPos.y <= resizeBorderThicknessY) : false);
        // This will handle the left, right and bottom parts of the frame
        // because we didn't change them.
        const LRESULT originalRet = DefWindowProcW(m_window, WM_NCHITTEST, 0, lParam);
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
    } break;
    case WM_NCRBUTTONUP: {
        if (wParam == HTCAPTION) {
            const POINT mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (OpenSystemMenu2(mousePos)) {
                *result = 0;
                return true;
            } else {
                Utils::DisplayErrorDialog(L"Failed to open the system menu for this window.");
                return false;
            }
        }
    } break;
    case WM_ACTIVATE: {
        m_active = ((wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE));
        ActiveChangeHandler();
    } break;
    case WM_WINDOWPOSCHANGING: {
        // Tell Windows to discard the entire contents of the client area, as re-using
        // parts of the client area would lead to jitter during resize.
        const auto windowPos = reinterpret_cast<LPWINDOWPOS>(lParam);
        windowPos->flags |= SWP_NOCOPYBITS;
    } break;
    default:
        break;
    }
    return false;
}

bool WindowPrivate::UpdateWindowFrameMargins2() noexcept
{
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to update the window frame margins due to the window has not been created yet.");
        return false;
    }
    // We removed the whole top part of the frame (see handling of
    // WM_NCCALCSIZE) so the top border is missing now. We add it back here.
    // Note #1: You might wonder why we don't remove just the title bar instead
    //  of removing the whole top part of the frame and then adding the little
    //  top border back. I tried to do this but it didn't work: DWM drew the
    //  whole title bar anyways on top of the window. It seems that DWM only
    //  wants to draw either nothing or the whole top part of the frame.
    // Note #2: For some reason if you try to set the top margin to just the
    //  top border height (what we want to do), then there is a transparency
    //  bug when the window is inactive, so I've decided to add the whole top
    //  part of the frame instead and then we will hide everything that we
    //  don't need (that is, the whole thing but the little 1 pixel wide border
    //  at the top) in the WM_PAINT handler. This eliminates the transparency
    //  bug and it's what a lot of Win32 apps that customize the title bar do
    //  so it should work fine.
    const UINT frameBorderThickness = GetWindowMetrics2(WindowMetrics::WindowVisibleFrameBorderThickness);
    const UINT titleBarHeight = (GetWindowMetrics2(WindowMetrics::ResizeBorderThicknessY) + GetWindowMetrics2(WindowMetrics::CaptionHeight));
    const UINT topFrameMargin = ((m_visibility == WindowState::Maximized) ? 0 : (m_bNoRedirectionBitmap ? frameBorderThickness : titleBarHeight));
    const MARGINS margins = {0, 0, static_cast<int>(topFrameMargin), 0};
    const HRESULT hr = DwmExtendFrameIntoClientArea(m_window, &margins);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DwmExtendFrameIntoClientArea, hr, L"Failed to update the window frame margins for the window.")
        return false;
    }
    return true;
}

void WindowPrivate::TitleChangeHandler() const noexcept
{
    if (m_titleChangeHandlerCallback) {
        m_titleChangeHandlerCallback(m_title);
    }
}

void WindowPrivate::XChangeHandler() const noexcept
{
    if (m_xChangeHandlerCallback) {
        m_xChangeHandlerCallback(m_x);
    }
}

void WindowPrivate::YChangeHandler() const noexcept
{
    if (m_yChangeHandlerCallback) {
        m_yChangeHandlerCallback(m_y);
    }
}

void WindowPrivate::WidthChangeHandler() const noexcept
{
    if (m_widthChangeHandlerCallback) {
        m_widthChangeHandlerCallback(m_width);
    }
}

void WindowPrivate::HeightChangeHandler() const noexcept
{
    if (m_heightChangeHandlerCallback) {
        m_heightChangeHandlerCallback(m_height);
    }
}

void WindowPrivate::VisibilityChangeHandler() const noexcept
{
    if (m_visibilityChangeHandlerCallback) {
        m_visibilityChangeHandlerCallback(m_visibility);
    }
}

void WindowPrivate::ActiveChangeHandler() const noexcept
{
    if (m_activeChangeHandlerCallback) {
        m_activeChangeHandlerCallback(m_active);
    }
}

void WindowPrivate::FrameCornerChangeHandler() const noexcept
{
    if (m_frameCornerChangeHandlerCallback) {
        m_frameCornerChangeHandlerCallback(m_frameCorner);
    }
}

void WindowPrivate::StartupLocationChangeHandler() const noexcept
{
    if (m_startupLocationChangeHandlerCallback) {
        m_startupLocationChangeHandlerCallback(m_startupLocation);
    }
}

void WindowPrivate::TitleBarBackgroundColorChangeHandler() const noexcept
{
    if (m_titleBarBackgroundColorChangeHandlerCallback) {
        m_titleBarBackgroundColorChangeHandlerCallback(m_titleBarBackgroundColor);
    }
}

void WindowPrivate::ThemeChangeHandler() const noexcept
{
    if (m_themeChangeHandlerCallback) {
        m_themeChangeHandlerCallback(m_theme);
    }
}

void WindowPrivate::DotsPerInchChangeHandler() const noexcept
{
    if (m_dotsPerInchChangeHandlerCallback) {
        m_dotsPerInchChangeHandlerCallback(m_dpi);
    }
}

void WindowPrivate::ColorizationColorChangeHandler() const noexcept
{
    if (m_colorizationColorChangeHandlerCallback) {
        m_colorizationColorChangeHandlerCallback(m_colorizationColor);
    }
}

void WindowPrivate::ColorizationAreaChangeHandler() const noexcept
{
    if (m_colorizationAreaChangeHandlerCallback) {
        m_colorizationAreaChangeHandlerCallback(m_colorizationArea);
    }
}

Window::Window(const bool NoRedirectionBitmap) noexcept
{
    d_ptr = std::make_unique<WindowPrivate>(this, NoRedirectionBitmap);
}

Window::~Window() noexcept = default;

std::wstring Window::Title() const noexcept
{
    return d_ptr->Title();
}

int Window::MessageLoop() noexcept
{
    return WindowPrivate::MessageLoop();
}

void Window::Title(const std::wstring &value) noexcept
{
    d_ptr->Title(value);
}

int Window::X() const noexcept
{
    return d_ptr->X();
}

void Window::X(const int value) noexcept
{
    d_ptr->X(value);
}

int Window::Y() const noexcept
{
    return d_ptr->Y();
}

void Window::Y(const int value) noexcept
{
    d_ptr->Y(value);
}

UINT Window::Width() const noexcept
{
    return d_ptr->Width();
}

void Window::Width(const UINT value) noexcept
{
    d_ptr->Width(value);
}

UINT Window::Height() const noexcept
{
    return d_ptr->Height();
}

void Window::Height(const UINT value) noexcept
{
    d_ptr->Height(value);
}

WindowState Window::Visibility() const noexcept
{
    return d_ptr->Visibility();
}

void Window::Visibility(const WindowState value) noexcept
{
    d_ptr->Visibility(value);
}

bool Window::Active() const noexcept
{
    return d_ptr->Active();
}

void Window::Active(const bool value) noexcept
{
    d_ptr->Active(value);
}

WindowFrameCorner Window::FrameCorner() const noexcept
{
    return d_ptr->FrameCorner();
}

void Window::FrameCorner(const WindowFrameCorner value) noexcept
{
    d_ptr->FrameCorner(value);
}

WindowStartupLocation Window::StartupLocation() const noexcept
{
    return d_ptr->StartupLocation();
}

void Window::StartupLocation(const WindowStartupLocation value) noexcept
{
    d_ptr->StartupLocation(value);
}

const Color &Window::TitleBarBackgroundColor() const noexcept
{
    return d_ptr->TitleBarBackgroundColor();
}

void Window::TitleBarBackgroundColor(const Color &value) noexcept
{
    d_ptr->TitleBarBackgroundColor(value);
}

WindowTheme Window::Theme() const noexcept
{
    return d_ptr->Theme();
}

UINT Window::DotsPerInch() const noexcept
{
    return d_ptr->DotsPerInch();
}

const Color &Window::ColorizationColor() const noexcept
{
    return d_ptr->ColorizationColor();
}

WindowColorizationArea Window::ColorizationArea() const noexcept
{
    return d_ptr->ColorizationArea();
}

HWND Window::CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData, const UINT extraDataSize) const noexcept
{
    return d_ptr->CreateChildWindow(style, extendedStyle, wndProc, extraData, extraDataSize);
}

HWND Window::WindowHandle() const noexcept
{
    return d_ptr->WindowHandle();
}

bool Window::Move(const int x, const int y) const noexcept
{
    return d_ptr->Move(x, y);
}

bool Window::Resize(const UINT w, const UINT h) const noexcept
{
    return d_ptr->Resize(w, h);
}

bool Window::SetGeometry(const int x, const int y, const UINT w, const UINT h) const noexcept
{
    return d_ptr->SetGeometry(x, y, w, h);
}

void Window::TitleChangeHandler(const StrChangeHandlerCallback &cb) noexcept
{
    d_ptr->TitleChangeHandler(cb);
}

void Window::XChangeHandler(const IntChangeHandlerCallback &cb) noexcept
{
    d_ptr->XChangeHandler(cb);
}

void Window::YChangeHandler(const IntChangeHandlerCallback &cb) noexcept
{
    d_ptr->YChangeHandler(cb);
}

void Window::WidthChangeHandler(const UIntChangeHandlerCallback &cb) noexcept
{
    d_ptr->WidthChangeHandler(cb);
}

void Window::HeightChangeHandler(const UIntChangeHandlerCallback &cb) noexcept
{
    d_ptr->HeightChangeHandler(cb);
}

void Window::VisibilityChangeHandler(const WindowStateChangeHandlerCallback &cb) noexcept
{
    d_ptr->VisibilityChangeHandler(cb);
}

void Window::ActiveChangeHandler(const BoolChangeHandlerCallback &cb) noexcept
{
    d_ptr->ActiveChangeHandler(cb);
}

void Window::FrameCornerChangeHandler(const WindowFrameCornerChangeHandlerCallback &cb) noexcept
{
    d_ptr->FrameCornerChangeHandler(cb);
}

void Window::StartupLocationChangeHandler(const WindowStartupLocationChangeHandlerCallback &cb) noexcept
{
    d_ptr->StartupLocationChangeHandler(cb);
}

void Window::TitleBarBackgroundColorChangeHandler(const ColorChangeHandlerCallback &cb) noexcept
{
    d_ptr->TitleBarBackgroundColorChangeHandler(cb);
}

void Window::ThemeChangeHandler(const WindowThemeChangeHandlerCallback &cb) noexcept
{
    d_ptr->ThemeChangeHandler(cb);
}

void Window::DotsPerInchChangeHandler(const UIntChangeHandlerCallback &cb) noexcept
{
    d_ptr->DotsPerInchChangeHandler(cb);
}

void Window::ColorizationColorChangeHandler(const ColorChangeHandlerCallback &cb) noexcept
{
    d_ptr->ColorizationColorChangeHandler(cb);
}

void Window::ColorizationAreaChangeHandler(const WindowColorizationAreaChangeHandlerCallback &cb) noexcept
{
    d_ptr->ColorizationAreaChangeHandler(cb);
}

void Window::CustomMessageHandler(const WindowMessageHandlerCallback &cb) noexcept
{
    d_ptr->CustomMessageHandler(cb);
}

void Window::WindowMessageFilter(const WindowMessageFilterCallback &cb) noexcept
{
    d_ptr->WindowMessageFilter(cb);
}

UINT Window::GetWindowMetrics(const WindowMetrics metrics) const noexcept
{
    return d_ptr->GetWindowMetrics2(metrics);
}
