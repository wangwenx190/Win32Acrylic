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
#include "OperationResult.h"
#include "WindowsVersion.h"
#include "Utils.h"
#include "Resource.h"
#include <ShellApi.h>
#include <ShellScalingApi.h>
#include <DwmApi.h>
#include <cmath>

#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

[[nodiscard]] static inline DWORD GetDWORDFromRegistry(const HKEY rootKey, const std::wstring &subKey, const std::wstring &keyName) noexcept
{
    ADVAPI32_API(RegOpenKeyExW);
    ADVAPI32_API(RegQueryValueExW);
    ADVAPI32_API(RegCloseKey);
    if (RegOpenKeyExW_API && RegQueryValueExW_API && RegCloseKey_API) {
        if (!rootKey || subKey.empty() || keyName.empty()) {
            Utils::DisplayErrorDialog(L"Can't query the registry due to invalid parameters are passed.");
            return 0;
        }
        HKEY hKey = nullptr;
        if (RegOpenKeyExW_API(rootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            PRINT_WIN32_ERROR_MESSAGE(RegOpenKeyExW, L"Failed to open the registry key to read.")
            return 0;
        }
        DWORD dwValue = 0;
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(dwValue);
        const bool success = (RegQueryValueExW_API(hKey, keyName.c_str(), nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwValue), &dwSize) == ERROR_SUCCESS);
        if (!success) {
            PRINT_WIN32_ERROR_MESSAGE(RegQueryValueExW, L"Failed to query the registry key value.")
            // Don't return early here because we have to close the opened registry key later.
        }
        if (RegCloseKey_API(hKey) != ERROR_SUCCESS) {
            PRINT_WIN32_ERROR_MESSAGE(RegCloseKey, L"Failed to close the registry key.")
            return 0;
        }
        return dwValue;
    } else {
        Utils::DisplayErrorDialog(L"Failed to read DWORD from registry due to RegOpenKeyExW(), RegQueryValueExW() and RegCloseKey() are not available.");
        return 0;
    }
}

[[nodiscard]] static inline RECT GetWindowFrameGeometry(const HWND hWnd) noexcept
{
    USER32_API(GetWindowRect);
    if (GetWindowRect_API) {
        if (!hWnd) {
            return {};
        }
        RECT rect = {0, 0, 0, 0};
        if (GetWindowRect_API(hWnd, &rect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the window frame geometry.")
            return {};
        }
        return rect;
    } else {
        Utils::DisplayErrorDialog(L"Can't retrieve the window frame geometry due to GetWindowRect() is not available.");
        return {};
    }
}

[[maybe_unused]] [[nodiscard]] static inline bool IsVisible(const HWND hWnd) noexcept
{
    USER32_API(IsWindowVisible);
    if (IsWindowVisible_API) {
        if (!hWnd) {
            return false;
        }
        return (IsWindowVisible_API(hWnd) != FALSE);
    } else {
        Utils::DisplayErrorDialog(L"IsWindowVisible() is not available.");
        return false;
    }
}

[[maybe_unused]] [[nodiscard]] static inline bool IsMinimized(const HWND hWnd) noexcept
{
    USER32_API(IsIconic);
    if (IsIconic_API) {
        if (!hWnd) {
            return false;
        }
        return (IsIconic_API(hWnd) != FALSE);
    } else {
        Utils::DisplayErrorDialog(L"IsIconic() is not available.");
        return false;
    }
}

[[maybe_unused]] [[nodiscard]] static inline bool IsMaximized(const HWND hWnd) noexcept
{
    USER32_API(IsZoomed);
    if (IsZoomed_API) {
        if (!hWnd) {
            return false;
        }
        return (IsZoomed_API(hWnd) != FALSE);
    } else {
        Utils::DisplayErrorDialog(L"IsZoomed() is not available.");
        return false;
    }
}

[[maybe_unused]] [[nodiscard]] static inline bool IsWindowed(const HWND hWnd) noexcept
{
    USER32_API(GetWindowPlacement);
    if (GetWindowPlacement_API) {
        if (!hWnd) {
            return false;
        }
        WINDOWPLACEMENT wp;
        SecureZeroMemory(&wp, sizeof(wp));
        wp.length = sizeof(wp);
        if (GetWindowPlacement_API(hWnd, &wp) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowPlacement, L"Failed to retrieve the window state.")
            return false;
        }
        return ((wp.showCmd == SW_NORMAL) || (wp.showCmd == SW_RESTORE));
    } else {
        Utils::DisplayErrorDialog(L"GetWindowPlacement() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool IsHighContrastModeEnabled() noexcept
{
    USER32_API(SystemParametersInfoW);
    if (SystemParametersInfoW_API) {
        HIGHCONTRASTW hc;
        SecureZeroMemory(&hc, sizeof(hc));
        hc.cbSize = sizeof(hc);
        if (SystemParametersInfoW_API(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SystemParametersInfoW, L"Failed to retrieve the high contrast mode state.")
            return false;
        }
        return (hc.dwFlags & HCF_HIGHCONTRASTON);
    } else {
        Utils::DisplayErrorDialog(L"Failed to query the high contrast mode state due to SystemParametersInfoW() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool ShouldAppsUseLightTheme() noexcept
{
    return (GetDWORDFromRegistry(HKEY_CURRENT_USER, PersonalizeRegistryKeyPath, L"AppsUseLightTheme") != 0);
}

[[nodiscard]] static inline DWORD GetGlobalColorizationColor2() noexcept
{
    DWM_API(DwmGetColorizationColor);
    if (DwmGetColorizationColor_API) {
        DWORD color = 0; // The color format of the value is 0xAARRGGBB.
        BOOL opaque = FALSE;
        const HRESULT hr = DwmGetColorizationColor_API(&color, &opaque);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmGetColorizationColor, hr, L"Failed to retrieve the colorization color.")
            return 0;
        }
        return color;
    } else {
        Utils::DisplayErrorDialog(L"Failed to retrieve the colorization color due to DwmGetColorizationColor() is not available.");
        return 0;
    }
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
    USER32_API(MonitorFromWindow);
    USER32_API(GetMonitorInfoW);
    if (MonitorFromWindow_API && GetMonitorInfoW_API) {
        if (!hWnd) {
            return {};
        }
        const HMONITOR mon = MonitorFromWindow_API(hWnd, MONITOR_DEFAULTTONEAREST);
        if (!mon) {
            PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow, L"Failed to retrieve the corresponding screen.")
            return {};
        }
        MONITORINFO mi;
        SecureZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        if (GetMonitorInfoW_API(mon, &mi) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetMonitorInfoW, L"Failed to retrieve the screen information.")
            return {};
        }
        return (includeTaskBar ? mi.rcMonitor : mi.rcWork);
    } else {
        Utils::DisplayErrorDialog(L"Can't retrieve the screen geometry due to MonitorFromWindow() and GetMonitorInfoW() are not available.");
        return {};
    }
}

[[nodiscard]] static inline HWND CreateWindow2(const DWORD style, const DWORD extendedStyle, const HWND parentWindow, void *extraData, const UINT extraDataSize, const HBRUSH backgroundBrush, const WNDPROC wndProc) noexcept
{
    USER32_API(LoadCursorW);
    USER32_API(LoadIconW);
    USER32_API(RegisterClassExW);
    USER32_API(CreateWindowExW);
    if (LoadCursorW_API && LoadIconW_API && RegisterClassExW_API && CreateWindowExW_API) {
        if (!wndProc) {
            Utils::DisplayErrorDialog(L"Failed to register a window class due to the WindowProc function pointer is null.");
            return nullptr;
        }
        const std::wstring guid = Utils::GenerateGUID();
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
        wcex.hCursor = LoadCursorW_API(nullptr, IDC_ARROW);
        if (!(style & WS_CHILD)) {
            wcex.hIcon = LoadIconW_API(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_MYAPPLICATION));
            wcex.hIconSm = LoadIconW_API(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_MYAPPLICATION_SMALL));
        }
        if (extraDataSize > 0) {
            wcex.cbWndExtra = extraDataSize;
        }
        const ATOM atom = RegisterClassExW_API(&wcex);
        if (atom == INVALID_ATOM) {
            PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW, L"Failed to register a window class.")
            return nullptr;
        }
        const HWND hWnd = CreateWindowExW_API(
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
    } else {
        Utils::DisplayErrorDialog(L"Failed to create a window due to LoadCursorW(), LoadIconW(), RegisterClassExW() and CreateWindowExW() are not available.");
        return nullptr;
    }
}

[[nodiscard]] static inline bool CloseWindow2(const HWND hWnd) noexcept
{
    USER32_API(DestroyWindow);
    USER32_API(GetClassNameW);
    USER32_API(UnregisterClassW);
    if (DestroyWindow_API && GetClassNameW_API && UnregisterClassW_API) {
        if (!hWnd) {
            return false;
        }
        wchar_t className[MAX_PATH] = { L'\0' };
        if (GetClassNameW_API(hWnd, className, MAX_PATH) <= 0) {
            PRINT_WIN32_ERROR_MESSAGE(GetClassNameW, L"Failed to retrieve the class name of the window.")
            return false;
        }
        if (DestroyWindow_API(hWnd) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow, L"Failed to destroy the window.")
            return false;
        }
        if (UnregisterClassW_API(className, HINST_THISCOMPONENT) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW, L"Failed to unregister the window class.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to close the window due to DestroyWindow(), GetClassNameW() and UnregisterClassW() are not available.");
        return false;
    }
}

class WindowPrivate
{
public:
    explicit WindowPrivate(Window *q) noexcept;
    ~WindowPrivate() noexcept;

    [[nodiscard]] static int MessageLoop() noexcept;

    [[nodiscard]] std::wstring Title() const noexcept;
    void Title(const std::wstring &value) const noexcept;

    [[nodiscard]] int Icon() const noexcept;
    void Icon(const int value) const noexcept;

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
    [[nodiscard]] bool EnsureNonClientAreaRendering2() const noexcept;

private:
    Window *q_ptr = nullptr;
    HWND m_window = nullptr;
    int m_icon = 0;
    std::wstring m_title = {};
    int m_x = 0;
    int m_y = 0;
    UINT m_width = 0;
    UINT m_height = 0;
    WindowState m_visibility = WindowState::Hidden;
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
};

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
    USER32_API(GetClientRect);
    if (GetClientRect_API) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Failed to retrieve the window size due to the window has not been created yet.");
            return {};
        }
        RECT rect = {0, 0, 0, 0};
        if (GetClientRect_API(m_window, &rect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetClientRect, L"Failed to retrieve the window client area size.")
            return {};
        }
        return {RECT_WIDTH(rect), RECT_HEIGHT(rect)};
    } else {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window size due to GetClientRect() is not available.");
        return {};
    }
}

bool WindowPrivate::TriggerWindowFrameChange2() const noexcept
{
    USER32_API(SetWindowPos);
    if (SetWindowPos_API) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Failed to trigger a window frame change event due to the window has not been created yet.");
            return false;
        }
        constexpr UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        if (SetWindowPos_API(m_window, nullptr, 0, 0, 0, 0, flags) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to trigger a window frame change event for the window.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to trigger a window frame change event for the window due to SetWindowPos() is not available.");
        return false;
    }
}

bool WindowPrivate::RefreshWindowTheme2() const noexcept
{
    DWM_API(DwmSetWindowAttribute);
    UXTHEME_API(SetWindowTheme);
    if (DwmSetWindowAttribute_API && SetWindowTheme_API) {
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
        const HRESULT hr1 = DwmSetWindowAttribute_API(m_window, _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &enableDarkFrame, sizeof(enableDarkFrame));
        const HRESULT hr2 = DwmSetWindowAttribute_API(m_window, _DWMWA_USE_IMMERSIVE_DARK_MODE, &enableDarkFrame, sizeof(enableDarkFrame));
        const HRESULT hr3 = SetWindowTheme_API(m_window, themeName.c_str(), nullptr);
        if (FAILED(hr1) && FAILED(hr2)) {
            PRINT_HR_ERROR_MESSAGE(DwmSetWindowAttribute, hr2, L"Failed to change the window dark mode state.")
            return false;
        }
        if (FAILED(hr3)) {
            PRINT_HR_ERROR_MESSAGE(SetWindowTheme, hr3, L"Failed to change the window theme.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to refresh the window theme due to DwmSetWindowAttribute() and SetWindowTheme() are not available.");
        return false;
    }
}

bool WindowPrivate::OpenSystemMenu2(const POINT pos) const noexcept
{
    USER32_API(GetSystemMenu);
    USER32_API(SetMenuItemInfoW);
    USER32_API(SetMenuDefaultItem);
    USER32_API(TrackPopupMenu);
    USER32_API(PostMessageW);
    if (GetSystemMenu_API && SetMenuItemInfoW_API && SetMenuDefaultItem_API && TrackPopupMenu_API && PostMessageW_API) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Failed to open the system menu due to the window has not been created yet.");
            return false;
        }
        const HMENU menu = GetSystemMenu_API(m_window, FALSE);
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
            if (SetMenuItemInfoW_API(menu, item, FALSE, &mii) == FALSE) {
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
        if (SetMenuDefaultItem_API(menu, UINT_MAX, FALSE) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetMenuDefaultItem, L"Failed to set default menu item.")
            return false;
        }
        // ### TODO: support RTL layout: TPM_LAYOUTRTL
        const auto ret = TrackPopupMenu_API(menu, TPM_RETURNCMD, pos.x, pos.y, 0, m_window, nullptr);
        if (ret != 0) {
            if (PostMessageW_API(m_window, WM_SYSCOMMAND, ret, 0) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(PostMessageW, L"Failed to post message.")
                return false;
            }
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to open the system menu due to GetSystemMenu(), SetMenuItemInfoW(), SetMenuDefaultItem(), TrackPopupMenu() and PostMessageW() are not available.");
        return false;
    }
}

bool WindowPrivate::SetWindowState2(const WindowState state) noexcept
{
    USER32_API(ShowWindow);
    USER32_API(UpdateWindow);
    if (ShowWindow_API && UpdateWindow_API) {
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
        const BOOL previousState = ShowWindow_API(m_window, nCmdShow);
        UNREFERENCED_PARAMETER(previousState);
        PRINT_WIN32_ERROR_MESSAGE(ShowWindow, L"Failed to change the window state.")
        if ((state == WindowState::Windowed) || (state == WindowState::Maximized)) {
            if (UpdateWindow_API(m_window) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(UpdateWindow, L"Failed to update the window.")
                return false;
            }
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to change the window state due to ShowWindow() and UpdateWindow() are not available.");
        return false;
    }
}

UINT WindowPrivate::GetWindowDPI2() const noexcept
{
    if (!m_window) {
        return USER_DEFAULT_SCREEN_DPI;
    }
    USER32_API(GetDpiForWindow);
    if (GetDpiForWindow_API) {
        return GetDpiForWindow_API(m_window);
    } else {
        OutputDebugStringW(L"GetDpiForWindow() is not available.");
        USER32_API(GetSystemDpiForProcess);
        if (GetSystemDpiForProcess_API) {
            return GetSystemDpiForProcess_API(GetCurrentProcess());
        } else {
            OutputDebugStringW(L"GetSystemDpiForProcess() is not available.");
            USER32_API(GetDpiForSystem);
            if (GetDpiForSystem_API) {
                return GetDpiForSystem_API();
            } else {
                OutputDebugStringW(L"GetDpiForSystem() is not available.");
                SHCORE_API(GetDpiForMonitor);
                if (GetDpiForMonitor_API) {
                    USER32_API(MonitorFromWindow);
                    if (MonitorFromWindow_API) {
                        const HMONITOR mon = MonitorFromWindow_API(m_window, MONITOR_DEFAULTTONEAREST);
                        if (mon) {
                            UINT dpiX = 0, dpiY = 0;
                            const HRESULT hr = GetDpiForMonitor_API(mon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
                            if (SUCCEEDED(hr)) {
                                return static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
                            } else {
                                PRINT_HR_ERROR_MESSAGE(GetDpiForMonitor, hr, L"Failed to retrieve the screen's DPI.")
                                return USER_DEFAULT_SCREEN_DPI;
                            }
                        } else {
                            PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow, L"Failed to retrieve the corresponding screen.")
                            return USER_DEFAULT_SCREEN_DPI;
                        }
                    } else {
                        Utils::DisplayErrorDialog(L"Failed to retrieve the screen's DPI due to MonitorFromWindow() is not available.");
                        return USER_DEFAULT_SCREEN_DPI;
                    }
                } else {
                    OutputDebugStringW(L"GetDpiForMonitor() is not available.");
                    USER32_API(GetDC);
                    USER32_API(ReleaseDC);
                    GDI32_API(GetDeviceCaps);
                    if (GetDC_API && GetDeviceCaps_API && ReleaseDC_API) {
                        const HDC hdc = GetDC_API(nullptr);
                        if (hdc) {
                            const int dpiX = GetDeviceCaps_API(hdc, LOGPIXELSX);
                            const int dpiY = GetDeviceCaps_API(hdc, LOGPIXELSY);
                            if (ReleaseDC_API(nullptr, hdc) == 0) {
                                PRINT_WIN32_ERROR_MESSAGE(ReleaseDC, L"Failed to release the desktop window's DC.")
                                return USER_DEFAULT_SCREEN_DPI;
                            } else {
                                return static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
                            }
                        } else {
                            PRINT_WIN32_ERROR_MESSAGE(GetDC, L"Failed to retrieve the desktop window's DC.")
                            return USER_DEFAULT_SCREEN_DPI;
                        }
                    } else {
                        Utils::DisplayErrorDialog(L"Failed to retrieve the DPI of this window due to GetDC(), ReleaseDC() and GetDeviceCaps() are not available.");
                        return USER_DEFAULT_SCREEN_DPI;
                    }
                }
            }
        }
    }
}

UINT WindowPrivate::GetWindowVisibleFrameBorderThickness2() const noexcept
{
    DWM_API(DwmGetWindowAttribute);
    if (DwmGetWindowAttribute_API) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Failed to retrieve the window visible frame border thickness due to the window has not been created yet.");
            return DefaultWindowVisibleFrameBorderThickness;
        }
        UINT value = 0;
        const HRESULT hr = DwmGetWindowAttribute_API(m_window, _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
        if (SUCCEEDED(hr)) {
            // The returned value is already scaled to the DPI automatically.
            // Don't double scale it!!!
            return value;
        } else {
            // We just eat this error because this enumeration value is only available
            // on Windows 10 21H2 and onwards, so querying it's value will always result in
            // a "parameter error" (error code: 87) on older systems.
            const auto dpr = (static_cast<double>(m_dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
            return static_cast<UINT>(std::round(static_cast<double>(DefaultWindowVisibleFrameBorderThickness) * dpr));
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window visible frame border thickness due to DwmGetWindowAttribute() is not available.");
        return DefaultWindowVisibleFrameBorderThickness;
    }
}

bool WindowPrivate::Initialize() noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Failed to initialize WindowPrivate due to q_ptr is null.");
        return false;
    }
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to initialize WindowPrivate due to this window has not been created.");
        return false;
    }
    m_dpi = GetWindowDPI2();
    const std::wstring dpiDbgMsg = L"Current window's dots-per-inch (DPI): " + Utils::IntegerToString(m_dpi, 10);
    OutputDebugStringW(dpiDbgMsg.c_str());
    if (!EnsureNonClientAreaRendering2()) {
        Utils::DisplayErrorDialog(L"Failed to enable window non-client area rendering.");
        return false;
    }
    if (!UpdateWindowFrameMargins2()) {
        Utils::DisplayErrorDialog(L"Failed to update the window frame margins.");
        return false;
    }
    if (!TriggerWindowFrameChange2()) {
        Utils::DisplayErrorDialog(L"Failed to trigger a window frame change event for the window.");
        return false;
    }
    m_theme = GetGlobalApplicationTheme2();
    const std::wstring themeDbgMsg = L"Current window's theme: " + Utils::ThemeToString(m_theme);
    OutputDebugStringW(themeDbgMsg.c_str());
    if (!RefreshWindowTheme2()) {
        Utils::DisplayErrorDialog(L"Failed to change the window theme.");
        return false;
    }
    m_colorizationColor = Color(GetGlobalColorizationColor2());
    m_colorizationArea = GetGlobalColorizationArea2();
    m_visibility = WindowState::Hidden;
    const POINT windowPosition = GetWindowPosition2();
    const SIZE windowSize = GetWindowSize2();
    m_x = windowPosition.x;
    m_y = windowPosition.y;
    m_width = windowSize.cx;
    m_height = windowSize.cy;
    m_title = {};
    m_frameCorner = ((WindowsVersion::CurrentVersion() >= WindowsVersion::Windows10_21Half2) ? WindowFrameCorner::Round : WindowFrameCorner::Square);
    m_startupLocation = WindowStartupLocation::Default;

    q_ptr->OnXChanged(m_x);
    q_ptr->OnYChanged(m_y);
    q_ptr->OnWidthChanged(m_width);
    q_ptr->OnHeightChanged(m_height);
    q_ptr->OnVisibilityChanged(m_visibility);
    q_ptr->OnTitleChanged(m_title);
    q_ptr->OnIconChanged(m_icon);
    q_ptr->OnDotsPerInchChanged(m_dpi);
    q_ptr->OnThemeChanged(m_theme);
    q_ptr->OnColorizationColorChanged(m_colorizationColor);
    q_ptr->OnColorizationAreaChanged(m_colorizationArea);
    q_ptr->OnFrameCornerChanged(m_frameCorner);
    q_ptr->OnStartupLocationChanged(m_startupLocation);

    return true;
}

WindowPrivate::WindowPrivate(Window *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"WindowPrivate's q pointer is null.");
        return;
    }
    q_ptr = q;
    GDI32_API(GetStockObject);
    if (GetStockObject_API) {
        // We need this window background brush in WM_PAINT, so create it early.
        m_windowBackgroundBrush = static_cast<HBRUSH>(GetStockObject_API(BLACK_BRUSH));
        if (!m_windowBackgroundBrush) {
            PRINT_WIN32_ERROR_MESSAGE(GetStockObject, L"Failed to retrieve the black brush.")
            return;
        }
    } else {
        Utils::DisplayErrorDialog(L"Can't retrieve the window background brush due to GetStockObject() is not available.");
        return;
    }
    // Create the title bar background brush early, we'll need it in WM_PAINT.
    TitleBarBackgroundColor(Color::FromRgba(0, 0, 0));
    m_window = CreateWindow2(WS_OVERLAPPEDWINDOW, WS_EX_NOREDIRECTIONBITMAP, nullptr, this, sizeof(WindowPrivate *), m_windowBackgroundBrush, WindowProc);
    if (m_window) {
        if (!Initialize()) {
            Utils::DisplayErrorDialog(L"Failed to initialize WindowPrivate.");
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to create this window.");
    }
}

WindowPrivate::~WindowPrivate() noexcept
{
    // According to MSDN, it's not needed to delete the window background brush
    // due to it's retrieved through GetStockObject(). But calling DeleteObject()
    // on it is harmless.
    if (m_titleBarBackgroundBrush) {
        GDI32_API(DeleteObject);
        if (DeleteObject_API) {
            if (DeleteObject_API(m_titleBarBackgroundBrush) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(DeleteObject, L"Failed to delete the title bar background brush.")
            } else {
                m_titleBarBackgroundBrush = nullptr;
            }
        } else {
            Utils::DisplayErrorDialog(L"Can't delete the title bar background brush due to DeleteObject() is not available.");
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
    USER32_API(GetMessageW);
    USER32_API(TranslateMessage);
    USER32_API(DispatchMessageW);
    if (GetMessageW_API && TranslateMessage_API && DispatchMessageW_API) {
        MSG msg = {};
        while (GetMessageW_API(&msg, nullptr, 0, 0) != FALSE) {
            TranslateMessage_API(&msg);
            DispatchMessageW_API(&msg);
        }
        return static_cast<int>(msg.wParam);
    } else {
        Utils::DisplayErrorDialog(L"Can't continue the message loop due to GetMessageW(), TranslateMessage() and DispatchMessageW() are not available.");
        return -1;
    }
}

std::wstring WindowPrivate::Title() const noexcept
{
    return m_title;
}

void WindowPrivate::Title(const std::wstring &value) const noexcept
{
    if (m_title != value) {
        USER32_API(SetWindowTextW);
        if (SetWindowTextW_API) {
            if (!m_window) {
                Utils::DisplayErrorDialog(L"Failed to change the window title due to the window has not been created yet.");
                return;
            }
            if (SetWindowTextW_API(m_window, value.c_str()) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowTextW, L"Failed to change the window title.")
            }
        } else {
            Utils::DisplayErrorDialog(L"Failed to change the window title due to SetWindowTextW() is not available.");
        }
    }
}

int WindowPrivate::Icon() const noexcept
{
    return m_icon;
}

void WindowPrivate::Icon(const int value) const noexcept
{
    if (m_icon != value) {
        // ### TODO
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
        DWM_API(DwmSetWindowAttribute);
        USER32_API(SetWindowRgn);
        if (DwmSetWindowAttribute_API && SetWindowRgn_API) {
            const DwmWindowCornerPreference wcp = ((value == WindowFrameCorner::Round) ? DwmWindowCornerPreference::Round : DwmWindowCornerPreference::DoNotRound);
            const HRESULT hr = DwmSetWindowAttribute_API(m_window, _DWMWA_WINDOW_CORNER_PREFERENCE, &wcp, sizeof(wcp));
            if (SUCCEEDED(hr)) {
                m_frameCorner = value;
            } else {
                // ### TODO: SetWindowRgn
            }
        } else {
            Utils::DisplayErrorDialog(L"Can't change the window frame corner style due to DwmSetWindowAttribute() and SetWindowRgn() are not available.");
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
        if (!q_ptr) {
            Utils::DisplayErrorDialog(L"Can't set the window startup location due to q_ptr is null.");
            return;
        }
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Can't set the window startup location due to the window has not been created yet.");
            return;
        }
        m_startupLocation = value;
        q_ptr->OnStartupLocationChanged(m_startupLocation);
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
        if (!q_ptr) {
            Utils::DisplayErrorDialog(L"Can't change the title bar background brush due to q_ptr is null.");
            return;
        }
        GDI32_API(CreateSolidBrush);
        GDI32_API(DeleteObject);
        if (CreateSolidBrush_API && DeleteObject_API) {
            if (m_titleBarBackgroundBrush) {
                if (DeleteObject_API(m_titleBarBackgroundBrush) == FALSE) {
                    PRINT_WIN32_ERROR_MESSAGE(DeleteObject, L"Failed to delete the previous title bar background brush.")
                    return;
                }
                m_titleBarBackgroundBrush = nullptr;
            }
            m_titleBarBackgroundBrush = CreateSolidBrush_API(value.ToWin32());
            if (m_titleBarBackgroundBrush) {
                m_titleBarBackgroundColor = value;
                q_ptr->OnTitleBarBackgroundColorChanged(m_titleBarBackgroundColor);
            } else {
                PRINT_WIN32_ERROR_MESSAGE(CreateSolidBrush, L"Failed to create the title bar background brush.")
            }
        } else {
            Utils::DisplayErrorDialog(L"Can't change the title bar background brush due to CreateSolidBrush() and DeleteObject() are not available.");
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
    USER32_API(SetWindowPos);
    if (SetWindowPos_API) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Failed to change the window geometry due to the window has not been created yet.");
            return false;
        }
        constexpr UINT flags = (SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER);
        if (SetWindowPos_API(m_window, nullptr, x, y, w, h, flags) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to change the window geometry.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to change the window geometry due to SetWindowPos() is not available.");
        return false;
    }
}

UINT WindowPrivate::GetWindowMetrics2(const WindowMetrics metrics) noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window metrics due to q_ptr is null.");
        return 0;
    }
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window metrics due to the window has not been created yet.");
        return 0;
    }
    USER32_API(GetSystemMetricsForDpi);
    if (GetSystemMetricsForDpi_API) {
        switch (metrics) {
        case WindowMetrics::ResizeBorderThicknessX: {
            if (m_resizeBorderThicknessX == 0) {
                const int paddedBorderThicknessX = GetSystemMetricsForDpi_API(SM_CXPADDEDBORDER, m_dpi);
                const int sizeFrameThicknessX = GetSystemMetricsForDpi_API(SM_CXSIZEFRAME, m_dpi);
                m_resizeBorderThicknessX = (paddedBorderThicknessX + sizeFrameThicknessX);
            }
            return m_resizeBorderThicknessX;
        } break;
        case WindowMetrics::ResizeBorderThicknessY: {
            if (m_resizeBorderThicknessY == 0) {
                const int paddedBorderThicknessY = GetSystemMetricsForDpi_API(SM_CYPADDEDBORDER, m_dpi);
                const int sizeFrameThicknessY = GetSystemMetricsForDpi_API(SM_CYSIZEFRAME, m_dpi);
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
                m_captionHeight = GetSystemMetricsForDpi_API(SM_CYCAPTION, m_dpi);
            }
            return m_captionHeight;
        } break;
        case WindowMetrics::WindowIconWidth: {
            if (m_windowIconWidth == 0) {
                m_windowIconWidth = GetSystemMetricsForDpi_API(SM_CXICON, m_dpi);
            }
            return m_windowIconWidth;
        } break;
        case WindowMetrics::WindowIconHeight: {
            if (m_windowIconHeight == 0) {
                m_windowIconHeight = GetSystemMetricsForDpi_API(SM_CYICON, m_dpi);
            }
            return m_windowIconHeight;
        } break;
        case WindowMetrics::WindowSmallIconWidth: {
            if (m_windowSmallIconWidth == 0) {
                m_windowSmallIconWidth = GetSystemMetricsForDpi_API(SM_CXSMICON, m_dpi);
            }
            return m_windowSmallIconWidth;
        } break;
        case WindowMetrics::WindowSmallIconHeight: {
            if (m_windowSmallIconHeight == 0) {
                m_windowSmallIconHeight = GetSystemMetricsForDpi_API(SM_CYSMICON, m_dpi);
            }
            return m_windowSmallIconHeight;
        } break;
        }
        return 0;
    } else {
        OutputDebugStringW(L"GetSystemMetricsForDpi() is not available.");
        USER32_API(GetSystemMetrics);
        if (GetSystemMetrics_API) {
            // GetSystemMetrics() will return an automatically scaled value
            // based on the DPI awareness of the current process.
            switch (metrics) {
            case WindowMetrics::ResizeBorderThicknessX: {
                if (m_resizeBorderThicknessX == 0) {
                    const int paddedBorderThicknessX = GetSystemMetrics_API(SM_CXPADDEDBORDER);
                    const int sizeFrameThicknessX = GetSystemMetrics_API(SM_CXSIZEFRAME);
                    m_resizeBorderThicknessX = (paddedBorderThicknessX + sizeFrameThicknessX);
                }
                return m_resizeBorderThicknessX;
            } break;
            case WindowMetrics::ResizeBorderThicknessY: {
                if (m_resizeBorderThicknessY == 0) {
                    const int paddedBorderThicknessY = GetSystemMetrics_API(SM_CYPADDEDBORDER);
                    const int sizeFrameThicknessY = GetSystemMetrics_API(SM_CYSIZEFRAME);
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
                    m_captionHeight = GetSystemMetrics_API(SM_CYCAPTION);
                }
                return m_captionHeight;
            } break;
            case WindowMetrics::WindowIconWidth: {
                if (m_windowIconWidth == 0) {
                    m_windowIconWidth = GetSystemMetrics_API(SM_CXICON);
                }
                return m_windowIconWidth;
            } break;
            case WindowMetrics::WindowIconHeight: {
                if (m_windowIconHeight == 0) {
                    m_windowIconHeight = GetSystemMetrics_API(SM_CYICON);
                }
                return m_windowIconHeight;
            } break;
            case WindowMetrics::WindowSmallIconWidth: {
                if (m_windowSmallIconWidth == 0) {
                    m_windowSmallIconWidth = GetSystemMetrics_API(SM_CXSMICON);
                }
                return m_windowSmallIconWidth;
            } break;
            case WindowMetrics::WindowSmallIconHeight: {
                if (m_windowSmallIconHeight == 0) {
                    m_windowSmallIconHeight = GetSystemMetrics_API(SM_CYSMICON);
                }
                return m_windowSmallIconHeight;
            } break;
            }
            return 0;
        } else {
            Utils::DisplayErrorDialog(L"Failed to retrieve the window metrics due to GetSystemMetricsForDpi() and GetSystemMetrics() are not available.");
            return 0;
        }
    }
}

LRESULT CALLBACK WindowPrivate::WindowProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept
{
    USER32_API(SetWindowLongPtrW);
    USER32_API(GetWindowLongPtrW);
    USER32_API(DefWindowProcW);
    if (SetWindowLongPtrW_API && GetWindowLongPtrW_API && DefWindowProcW_API) {
        if (message == WM_NCCREATE) {
            const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            const auto that = static_cast<WindowPrivate *>(cs->lpCreateParams);
            // SetWindowLongPtrW() won't modify the Last Error state on success
            // and it's return value is the previous data so we have to judge
            // the actual operation result from the Last Error state manually.
            SetLastError(ERROR_SUCCESS);
            const LONG_PTR result = SetWindowLongPtrW_API(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to set the extra data to the window.")
            if (result != 0) {
                Utils::DisplayErrorDialog(L"The extra data of this window has been overwritten.");
            }
        } else if (message == WM_NCDESTROY) {
            // See the above comments.
            SetLastError(ERROR_SUCCESS);
            const LONG_PTR result = SetWindowLongPtrW_API(hWnd, GWLP_USERDATA, 0);
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to clear the extra data of the window.")
            if (result == 0) {
                Utils::DisplayErrorDialog(L"This window doesn't contain any extra data.");
            }
        }
        if (const auto that = reinterpret_cast<WindowPrivate *>(GetWindowLongPtrW_API(hWnd, GWLP_USERDATA))) {
            LRESULT result = 0;
            if (that->q_ptr) {
                if (that->q_ptr->MessageHandler(message, wParam, lParam, &result)) {
                    return result;
                }
            }
            if (that->InternalMessageHandler(message, wParam, lParam, &result)) {
                return result;
            }
        }
        return DefWindowProcW_API(hWnd, message, wParam, lParam);
    } else {
        Utils::DisplayErrorDialog(L"Failed to continue the WindowProc function due to SetWindowLongPtrW(), GetWindowLongPtrW() and DefWindowProcW() are not available.");
        return 0;
    }
}

bool WindowPrivate::InternalMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept
{
    if (!result) {
        Utils::DisplayErrorDialog(L"InternalMessageHandler: the pointer to the result of the WindowProc function is null.");
        return false;
    }
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"InternalMessageHandler: the q_ptr is null.");
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
        q_ptr->OnXChanged(m_x);
        q_ptr->OnYChanged(m_y);
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
            q_ptr->OnVisibilityChanged(m_visibility);
        }
        q_ptr->OnWidthChanged(m_width);
        q_ptr->OnHeightChanged(m_height);
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
            m_theme = GetGlobalApplicationTheme2();
            if (!RefreshWindowTheme2()) {
                Utils::DisplayErrorDialog(L"Failed to refresh the window theme.");
                return false;
            }
            q_ptr->OnThemeChanged(m_theme);
        }
    } break;
    case WM_DPICHANGED: {
        const UINT oldDPI = m_dpi;
        const UINT dpiX = LOWORD(wParam);
        const UINT dpiY = HIWORD(wParam);
        m_dpi = static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
        const std::wstring dpiDbgMsg = L"Current window's dots-per-inch (DPI) has changed from " + Utils::IntegerToString(oldDPI, 10) + L" to " + Utils::IntegerToString(m_dpi, 10) + L".";
        OutputDebugStringW(dpiDbgMsg.c_str());
        q_ptr->OnDotsPerInchChanged(m_dpi);
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
        q_ptr->OnColorizationColorChanged(m_colorizationColor);
    } break;
    case WM_PAINT: {
        if (!m_windowBackgroundBrush || !m_titleBarBackgroundBrush) {
            Utils::DisplayErrorDialog(L"Can't paint the window when the window background brush and/or the title bar background brush is null.");
            return false;
        }
        USER32_API(BeginPaint);
        USER32_API(FillRect);
        USER32_API(EndPaint);
        UXTHEME_API(BeginBufferedPaint);
        UXTHEME_API(BufferedPaintSetAlpha);
        UXTHEME_API(EndBufferedPaint);
        if (BeginPaint_API && FillRect_API && EndPaint_API && BeginBufferedPaint_API && BufferedPaintSetAlpha_API && EndBufferedPaint_API) {
            PAINTSTRUCT ps;
            SecureZeroMemory(&ps, sizeof(ps));
            const HDC hDC = BeginPaint_API(m_window, &ps);
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
                if (FillRect_API(hDC, &rcTopBorder, m_windowBackgroundBrush) == 0) {
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
                const HPAINTBUFFER buf = BeginBufferedPaint_API(hDC, &rcRest, BPBF_TOPDOWNDIB, &params, &opaqueDc);
                if (!buf || !opaqueDc) {
                    PRINT_WIN32_ERROR_MESSAGE(BeginBufferedPaint, L"Failed to begin the buffered paint.")
                    return false;
                }
                if (FillRect_API(opaqueDc, &rcRest, m_titleBarBackgroundBrush) == 0) {
                    PRINT_WIN32_ERROR_MESSAGE(FillRect, L"Failed to paint the window background.")
                    return false;
                }
                HRESULT hr = BufferedPaintSetAlpha_API(buf, nullptr, 255);
                if (FAILED(hr)) {
                    PRINT_HR_ERROR_MESSAGE(BufferedPaintSetAlpha, hr, L"Failed to change alpha of the buffered paint.")
                    return false;
                }
                hr = EndBufferedPaint_API(buf, TRUE);
                if (FAILED(hr)) {
                    PRINT_HR_ERROR_MESSAGE(EndBufferedPaint, hr, L"Failed to finish the buffered paint.")
                    return false;
                }
            }
            if (EndPaint_API(m_window, &ps) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(EndPaint, L"Failed to end painting.")
                return false;
            }
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Can't paint the window due to BeginPaint(), FillRect(), EndPaint(), BeginBufferedPaint(), BufferedPaintSetAlpha() and EndBufferedPaint() are not available.");
            return false;
        }
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
        USER32_API(PostQuitMessage);
        if (PostQuitMessage_API) {
            PostQuitMessage_API(0);
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Can't quit application due to PostQuitMessage() is not available.");
            return false;
        }
    } break;
    case WM_NCCREATE: {
        USER32_API(EnableNonClientDpiScaling);
        if (EnableNonClientDpiScaling_API) {
            if (EnableNonClientDpiScaling_API(m_window) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(EnableNonClientDpiScaling, L"Failed to enable window non-client area automatic DPI scaling.")
            }
        } else {
            Utils::DisplayErrorDialog(L"Can't enable window non-client area automatic DPI scaling due to EnableNonClientDpiScaling() is not available.");
        }
    } break;
    case WM_NCCALCSIZE: {
        if (static_cast<BOOL>(wParam) == FALSE) {
            *result = 0;
            return true;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        USER32_API(DefWindowProcW);
        if (DefWindowProcW_API) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcW_API(m_window, WM_NCCALCSIZE, TRUE, lParam);
            if (ret != 0) {
                *result = ret;
                return true;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        } else {
            Utils::DisplayErrorDialog(L"Error occurred when processing WM_NCCALCSIZE due to DefWindowProcW() is not available.");
            return false;
        }
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
        SHELL32_API(SHAppBarMessage);
        if (SHAppBarMessage_API) {
            if (max || full) {
                APPBARDATA abd;
                SecureZeroMemory(&abd, sizeof(abd));
                abd.cbSize = sizeof(abd);
                // First, check if we have an auto-hide taskbar at all:
                if (SHAppBarMessage_API(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) {
                    USER32_API(MonitorFromWindow);
                    USER32_API(GetMonitorInfoW);
                    if (MonitorFromWindow_API && GetMonitorInfoW_API) {
                        const HMONITOR mon = MonitorFromWindow_API(m_window, MONITOR_DEFAULTTONEAREST);
                        if (mon) {
                            MONITORINFO mi;
                            SecureZeroMemory(&mi, sizeof(mi));
                            mi.cbSize = sizeof(mi);
                            if (GetMonitorInfoW_API(mon, &mi) == FALSE) {
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
                                return (reinterpret_cast<HWND>(SHAppBarMessage_API(ABM_GETAUTOHIDEBAREX, &abd2)) != nullptr);
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
                    } else {
                        Utils::DisplayErrorDialog(L"Error occurred when processing WM_NCCALCSIZE due to MonitorFromWindow() and GetMonitorInfoW() are not available.");
                        return false;
                    }
                }
            }
        } else {
            Utils::DisplayErrorDialog(L"Error occurred when processing WM_NCCALCSIZE due to SHAppBarMessage() is not available.");
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
        const POINT globalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT localPos = globalPos;
        USER32_API(ScreenToClient);
        if (ScreenToClient_API) {
            if (ScreenToClient_API(m_window, &localPos) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(ScreenToClient, L"Failed to translate from screen coordinate to window coordinate.")
                return false;
            }
        } else {
            Utils::DisplayErrorDialog(L"Error occurred when processing WM_NCHITTEST due to ScreenToClient() is not available.");
            return false;
        }
        const auto resizeBorderThicknessY = static_cast<LONG>(GetWindowMetrics2(WindowMetrics::ResizeBorderThicknessY));
        const UINT captionHeight = GetWindowMetrics2(WindowMetrics::CaptionHeight);
        const LONG titleBarHeight = ((m_visibility == WindowState::Hidden) ? 0 : ((m_visibility == WindowState::Maximized) ? captionHeight : (captionHeight + resizeBorderThicknessY)));
        const bool isTitleBar = (((m_visibility == WindowState::Windowed) || (m_visibility == WindowState::Maximized)) ? (localPos.y <= titleBarHeight) : false);
        const bool isTop = ((m_visibility == WindowState::Windowed) ? (localPos.y <= resizeBorderThicknessY) : false);
        USER32_API(DefWindowProcW);
        if (DefWindowProcW_API) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcW_API(m_window, WM_NCHITTEST, 0, lParam);
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
            Utils::DisplayErrorDialog(L"Error occurred when processing WM_NCHITTEST due to DefWindowProcW() is not available.");
            return false;
        }
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
    default:
        break;
    }
    return false;
}

bool WindowPrivate::UpdateWindowFrameMargins2() noexcept
{
    DWM_API(DwmExtendFrameIntoClientArea);
    if (DwmExtendFrameIntoClientArea_API) {
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
        const UINT topFrameMargin = (((m_visibility == WindowState::Hidden) || (m_visibility == WindowState::Windowed)) ? (GetWindowMetrics2(WindowMetrics::ResizeBorderThicknessY) + GetWindowMetrics2(WindowMetrics::CaptionHeight)) : 0);
        const MARGINS margins = {0, 0, static_cast<int>(topFrameMargin), 0};
        const HRESULT hr = DwmExtendFrameIntoClientArea_API(m_window, &margins);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmExtendFrameIntoClientArea, hr, L"Failed to update the window frame margins for the window.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to update the window frame margins due to DwmExtendFrameIntoClientArea() is not available.");
        return false;
    }
}

bool WindowPrivate::EnsureNonClientAreaRendering2() const noexcept
{
    DWM_API(DwmSetWindowAttribute);
    if (DwmSetWindowAttribute_API) {
        if (!m_window) {
            Utils::DisplayErrorDialog(L"Can't enable window non-client area rendering due to the window has not been created yet.");
            return false;
        }
        // Don't use "DWMWA_NCRENDERING_ENABLED" because it's used for querying values only.
        const DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
        // Don't use "DWMWA_ALLOW_NCPAINT" because it will make the window look weird.
        const HRESULT hr = DwmSetWindowAttribute_API(m_window, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmSetWindowAttribute, hr, L"Failed to enable window non-client area rendering.")
            return false;
        } else {
            return true;
        }
    } else {
        Utils::DisplayErrorDialog(L"Can't enable window non-client area rendering due to DwmSetWindowAttribute() is not available.");
        return false;
    }
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

int Window::MessageLoop() noexcept
{
    return WindowPrivate::MessageLoop();
}

void Window::Title(const std::wstring &value) noexcept
{
    d_ptr->Title(value);
}

int Window::Icon() const noexcept
{
    return d_ptr->Icon();
}

void Window::Icon(const int value) noexcept
{
    d_ptr->Icon(value);
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

void Window::OnTitleChanged(const std::wstring &arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnIconChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnXChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnYChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnWidthChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnHeightChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnVisibilityChanged(const WindowState arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnFrameCornerChanged(const WindowFrameCorner arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnStartupLocationChanged(const WindowStartupLocation arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnTitleBarBackgroundColorChanged(const Color &arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnThemeChanged(const WindowTheme arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnDotsPerInchChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnColorizationColorChanged(const Color &arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

void Window::OnColorizationAreaChanged(const WindowColorizationArea arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

bool Window::MessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept
{
    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(result);
    return false;
}

UINT Window::GetWindowMetrics(const WindowMetrics metrics) const noexcept
{
    return d_ptr->GetWindowMetrics2(metrics);
}
