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
#include "Utils.h"
#include "Resource.h"
#include <ShellApi.h>
#include <ShellScalingApi.h>
#include <DwmApi.h>
#include <cmath>

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI (96)
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER (92)
#endif

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
#define GET_X_LPARAM(lParam) (static_cast<int>(static_cast<short>(LOWORD(lParam))))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam) (static_cast<int>(static_cast<short>(HIWORD(lParam))))
#endif

#ifndef RECT_WIDTH
#define RECT_WIDTH(rect) (std::abs((rect).right - (rect).left))
#endif

#ifndef RECT_HEIGHT
#define RECT_HEIGHT(rect) (std::abs((rect).bottom - (rect).top))
#endif

#ifndef ATOM_TO_STRING
#define ATOM_TO_STRING(atom) (reinterpret_cast<LPCWSTR>(static_cast<WORD>(MAKELONG(atom, 0))))
#endif

static constexpr UINT g_defaultResizeBorderThickness = 8;
static constexpr UINT g_defaultCaptionHeight = 23;
static constexpr UINT g_defaultFrameBorderThickness = 1;
static constexpr UINT g_defaultWindowDPI = USER_DEFAULT_SCREEN_DPI;

static constexpr UINT g_autoHideTaskbarThickness = 2; // The thickness of an auto-hide taskbar in pixels.

static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
static constexpr DWORD g_DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37;

static constexpr wchar_t g_dwmRegistryKey[] = LR"(Software\Microsoft\Windows\DWM)";
static constexpr wchar_t g_personalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

[[nodiscard]] static inline bool IsMinimized(const HWND hWnd) noexcept
{
    USER32_API(IsIconic);
    if (IsIconicFunc) {
        if (!hWnd) {
            return false;
        }
        return (IsIconicFunc(hWnd) != FALSE);
    } else {
        Utils::DisplayErrorDialog(L"IsIconic() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool IsMaximized(const HWND hWnd) noexcept
{
    USER32_API(IsZoomed);
    if (IsZoomedFunc) {
        if (!hWnd) {
            return false;
        }
        return (IsZoomedFunc(hWnd) != FALSE);
    } else {
        Utils::DisplayErrorDialog(L"IsZoomed() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool IsNormal(const HWND hWnd) noexcept
{
    USER32_API(GetWindowPlacement);
    if (GetWindowPlacementFunc) {
        if (!hWnd) {
            return false;
        }
        WINDOWPLACEMENT wp;
        SecureZeroMemory(&wp, sizeof(wp));
        wp.length = sizeof(wp);
        if (GetWindowPlacementFunc(hWnd, &wp) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowPlacement, L"Failed to retrieve the window state.")
            return false;
        }
        return (wp.showCmd == SW_NORMAL);
    } else {
        Utils::DisplayErrorDialog(L"GetWindowPlacement() is not available.");
        return false;
    }
}

class WindowPrivate
{
public:
    explicit WindowPrivate(Window *q) noexcept;
    ~WindowPrivate() noexcept;

    [[nodiscard]] std::wstring Title() const noexcept;
    void Title(const std::wstring &value) noexcept;

    [[nodiscard]] int Icon() const noexcept;
    void Icon(const int value) noexcept;

    [[nodiscard]] int X() const noexcept;
    void X(const int value) noexcept;

    [[nodiscard]] int Y() const noexcept;
    void Y(const int value) noexcept;

    [[nodiscard]] UINT Width() const noexcept;
    void Width(const UINT value) noexcept;

    [[nodiscard]] UINT Height() const noexcept;
    void Height(const UINT value) noexcept;

    [[nodiscard]] WindowState Visibility() const noexcept;
    void Visibility(const WindowState value) noexcept;

    [[nodiscard]] WindowTheme Theme() const noexcept;

    [[nodiscard]] UINT DotsPerInch() const noexcept;

    [[nodiscard]] COLORREF ColorizationColor() const noexcept;

    [[nodiscard]] WindowColorizationArea ColorizationArea() const noexcept;

    [[nodiscard]] HWND CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData) const noexcept;
    [[nodiscard]] HWND WindowHandle() const noexcept;
    [[nodiscard]] int MessageLoop() const noexcept;
    [[nodiscard]] bool Move(const int x, const int y) noexcept;
    [[nodiscard]] bool Resize(const UINT w, const UINT h) noexcept;
    [[nodiscard]] bool SetGeometry(const int x, const int y, const UINT w, const UINT h) noexcept;

    [[nodiscard]] static std::tuple<HWND, ATOM> CreateWindow2(const DWORD style, const DWORD extendedStyle, const HWND parentWindow, void *extraData, const WNDPROC wndProc) noexcept;
    [[nodiscard]] static bool CloseWindow2(const HWND hWnd, const std::wstring &className) noexcept;
    [[nodiscard]] static RECT GetWindowFrameGeometry2(const HWND hWnd) noexcept;
    [[nodiscard]] static SIZE GetWindowClientSize2(const HWND hWnd) noexcept;
    [[nodiscard]] static COLORREF GetColorizationColor2() noexcept;
    [[nodiscard]] static WindowColorizationArea GetGlobalColorizationArea2() noexcept;
    [[nodiscard]] static WindowTheme GetGlobalApplicationTheme2() noexcept;
    [[nodiscard]] static bool TriggerFrameChange2(const HWND hWnd) noexcept;
    [[nodiscard]] static bool SetWindowTheme2(const HWND hWnd, const WindowTheme theme) noexcept;
    [[nodiscard]] static bool OpenSystemMenu2(const HWND hWnd, const POINT pos) noexcept;
    [[nodiscard]] static bool SetWindowState2(const HWND hWnd, const WindowState state) noexcept;
    [[nodiscard]] static UINT GetWindowDPI2(const HWND hWnd) noexcept;
    [[nodiscard]] static UINT GetResizeBorderThickness2(const HWND hWnd, const bool x) noexcept;
    [[nodiscard]] static UINT GetCaptionHeight2(const HWND hWnd) noexcept;
    [[nodiscard]] static UINT GetFrameBorderThickness2(const HWND hWnd) noexcept;

private:
    WindowPrivate(const WindowPrivate &) = delete;
    WindowPrivate &operator=(const WindowPrivate &) = delete;
    WindowPrivate(WindowPrivate &&) = delete;
    WindowPrivate &operator=(WindowPrivate &&) = delete;

private:
    [[nodiscard]] static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    [[nodiscard]] bool DefaultMessageHandler(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result) noexcept;
    [[nodiscard]] bool UpdateFrameMargins() noexcept;

private:
    Window *q_ptr = nullptr;
    HWND m_window = nullptr;
    ATOM m_atom = INVALID_ATOM;
    int m_icon = 0;
    std::wstring m_title = {};
    int m_x = 0;
    int m_y = 0;
    UINT m_width = 0;
    UINT m_height = 0;
    WindowState m_visibility = WindowState::Hidden;
    WindowTheme m_theme = WindowTheme::Light;
    COLORREF m_colorizationColor = 0;
    WindowColorizationArea m_colorizationArea = WindowColorizationArea::None;
    UINT m_dpi = 0;
};

std::tuple<HWND, ATOM> WindowPrivate::CreateWindow2(const DWORD style, const DWORD extendedStyle, const HWND parentWindow, void *extraData, const WNDPROC wndProc) noexcept
{
    constexpr std::tuple<HWND, ATOM> INVALID_RESULT = std::make_tuple(nullptr, INVALID_ATOM);
    USER32_API(LoadCursorW);
    USER32_API(LoadIconW);
    USER32_API(RegisterClassExW);
    USER32_API(CreateWindowExW);
    if (LoadCursorWFunc && LoadIconWFunc && RegisterClassExWFunc && CreateWindowExWFunc) {
        if (!wndProc) {
            Utils::DisplayErrorDialog(L"Failed to register a window class due to the WindowProc function pointer is null.");
            return INVALID_RESULT;
        }
        const HINSTANCE instance = Utils::GetCurrentModuleInstance();
        if (!instance) {
            Utils::DisplayErrorDialog(L"Failed to retrieve the current module instance.");
            return INVALID_RESULT;
        }
        const std::wstring guid = Utils::GenerateGUID();
        if (guid.empty()) {
            Utils::DisplayErrorDialog(L"Failed to generate a new GUID.");
            return INVALID_RESULT;
        }
        WNDCLASSEXW wcex;
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = wndProc;
        wcex.hInstance = instance;
        wcex.lpszClassName = guid.c_str();
        wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
        wcex.hIcon = LoadIconWFunc(instance, MAKEINTRESOURCEW(IDI_APPICON));
        wcex.hIconSm = LoadIconWFunc(instance, MAKEINTRESOURCEW(IDI_APPICON_SMALL));
        const ATOM atom = RegisterClassExWFunc(&wcex);
        if (atom == INVALID_ATOM) {
            PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW, L"Failed to register a window class.")
            return INVALID_RESULT;
        }
        const HWND hWnd = CreateWindowExWFunc(
            extendedStyle,        // _In_     DWORD     dwExStyle
            ATOM_TO_STRING(atom), // _In_opt_ LPCWSTR   lpClassName
            nullptr,              // _In_opt_ LPCWSTR   lpWindowName
            style,                // _In_     DWORD     dwStyle
            CW_USEDEFAULT,        // _In_     int       X
            CW_USEDEFAULT,        // _In_     int       Y
            CW_USEDEFAULT,        // _In_     int       nWidth
            CW_USEDEFAULT,        // _In_     int       nHeight
            parentWindow,         // _In_opt_ HWND      hWndParent
            nullptr,              // _In_opt_ HMENU     hMenu
            instance,             // _In_opt_ HINSTANCE hInstance
            extraData             // _In_opt_ LPVOID    lpParam
            );
        if (!hWnd) {
            PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW, L"Failed to create a window.")
            return INVALID_RESULT;
        }
        return std::make_tuple(hWnd, atom);
    } else {
        Utils::DisplayErrorDialog(L"Failed to create a window due to LoadCursorW(), LoadIconW(), RegisterClassExW() and CreateWindowExW() are not available.");
        return INVALID_RESULT;
    }
}

bool WindowPrivate::CloseWindow2(const HWND hWnd, const std::wstring &className) noexcept
{
    USER32_API(DestroyWindow);
    USER32_API(UnregisterClassW);
    if (DestroyWindowFunc && UnregisterClassWFunc) {
        if (!hWnd) {
            Utils::DisplayErrorDialog(L"Failed to close the window due to the window handle is null.");
            return false;
        }
        if (className.empty()) {
            Utils::DisplayErrorDialog(L"Failed to close the window due to the class name is empty.");
            return false;
        }
        const HINSTANCE instance = Utils::GetCurrentModuleInstance();
        if (!instance) {
            Utils::DisplayErrorDialog(L"Failed to retrieve the current module instance.");
            return false;
        }
        if (DestroyWindowFunc(hWnd) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow, L"Failed to destroy the window.")
            return false;
        }
        if (UnregisterClassWFunc(className.c_str(), instance) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW, L"Failed to unregister the window class.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to close the window due to DestroyWindow() and UnregisterClassW() are not available.");
        return false;
    }
}

RECT WindowPrivate::GetWindowFrameGeometry2(const HWND hWnd) noexcept
{
    USER32_API(GetWindowRect);
    if (GetWindowRectFunc) {
        if (!hWnd) {
            return {};
        }
        RECT rect = {0, 0, 0, 0};
        if (GetWindowRectFunc(hWnd, &rect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the window frame geometry.")
            return {};
        }
        return rect;
    } else {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window frame geometry due to GetWindowRect() is not available.");
        return {};
    }
}

SIZE WindowPrivate::GetWindowClientSize2(const HWND hWnd) noexcept
{
    USER32_API(GetClientRect);
    if (GetClientRectFunc) {
        if (!hWnd) {
            return {};
        }
        RECT rect = {0, 0, 0, 0};
        if (GetClientRectFunc(hWnd, &rect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetClientRect, L"Failed to retrieve the window client area size.")
            return {};
        }
        return {RECT_WIDTH(rect), RECT_HEIGHT(rect)};
    } else {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window client area size due to GetClientRect() is not available.");
        return {};
    }
}

COLORREF WindowPrivate::GetColorizationColor2() noexcept
{
    DWMAPI_API(DwmGetColorizationColor);
    if (DwmGetColorizationColorFunc) {
        DWORD color = 0; // The color format of the value is 0xAARRGGBB.
        BOOL opaque = FALSE;
        const HRESULT hr = DwmGetColorizationColorFunc(&color, &opaque);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmGetColorizationColor, hr, L"Failed to retrieve the colorization color.")
            return 0;
        }
        return color;
    } else {
        Utils::DisplayErrorDialog(L"DwmGetColorizationColor() is not available.");
        return 0;
    }
}

WindowColorizationArea WindowPrivate::GetGlobalColorizationArea2() noexcept
{
    const HKEY rootKey = HKEY_CURRENT_USER;
    const std::wstring keyName = L"ColorPrevalence";
    const DWORD dwmValue = Utils::GetDWORDFromRegistry(rootKey, g_dwmRegistryKey, keyName);
    const DWORD themeValue = Utils::GetDWORDFromRegistry(rootKey, g_personalizeRegistryKey, keyName);
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

WindowTheme WindowPrivate::GetGlobalApplicationTheme2() noexcept
{
    if (Utils::IsHighContrastModeEnabled()) {
        return WindowTheme::HighContrast;
    } else if (Utils::GetDWORDFromRegistry(HKEY_CURRENT_USER, g_personalizeRegistryKey, L"AppsUseLightTheme") != 0) {
        return WindowTheme::Light;
    } else {
        return WindowTheme::Dark;
    }
}

bool WindowPrivate::TriggerFrameChange2(const HWND hWnd) noexcept
{
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        if (!hWnd) {
            return false;
        }
        constexpr UINT flags = (SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        if (SetWindowPosFunc(hWnd, nullptr, 0, 0, 0, 0, flags) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to trigger a frame change event for the window.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"Failed to trigger a frame change event for the window due to SetWindowPos() is not available.");
        return false;
    }
}

bool WindowPrivate::SetWindowTheme2(const HWND hWnd, const WindowTheme theme) noexcept
{
    DWMAPI_API(DwmSetWindowAttribute);
    UXTHEME_API(SetWindowTheme);
    if (DwmSetWindowAttributeFunc && SetWindowThemeFunc) {
        if (!hWnd) {
            return false;
        }
        BOOL enableDarkFrame = FALSE;
        std::wstring themeName = {};
        switch (theme) {
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
        const HRESULT hr1 = DwmSetWindowAttributeFunc(hWnd, g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &enableDarkFrame, sizeof(enableDarkFrame));
        const HRESULT hr2 = DwmSetWindowAttributeFunc(hWnd, g_DWMWA_USE_IMMERSIVE_DARK_MODE, &enableDarkFrame, sizeof(enableDarkFrame));
        const HRESULT hr3 = SetWindowThemeFunc(hWnd, themeName.c_str(), nullptr);
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
        Utils::DisplayErrorDialog(L"DwmSetWindowAttribute() and SetWindowTheme() are not available.");
        return false;
    }
}

bool WindowPrivate::OpenSystemMenu2(const HWND hWnd, const POINT pos) noexcept
{
    USER32_API(GetSystemMenu);
    USER32_API(SetMenuItemInfoW);
    USER32_API(SetMenuDefaultItem);
    USER32_API(TrackPopupMenu);
    USER32_API(PostMessageW);
    if (GetSystemMenuFunc && SetMenuItemInfoWFunc && SetMenuDefaultItemFunc && TrackPopupMenuFunc && PostMessageWFunc) {
        if (!hWnd) {
            return false;
        }
        const HMENU menu = GetSystemMenuFunc(hWnd, FALSE);
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
            if (SetMenuItemInfoWFunc(menu, item, FALSE, &mii) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW, L"Failed to set menu item information.")
                return false;
            }
            return true;
        };
        const bool max = IsMaximized(hWnd);
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
        if (SetMenuDefaultItemFunc(menu, UINT_MAX, FALSE) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetMenuDefaultItem, L"Failed to set default menu item.")
            return false;
        }
        // ### TODO: support RTL layout: TPM_LAYOUTRTL
        const auto ret = TrackPopupMenuFunc(menu, TPM_RETURNCMD, pos.x, pos.y, 0, hWnd, nullptr);
        if (ret != 0) {
            if (PostMessageWFunc(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(PostMessageW, L"Failed to post message.")
                return false;
            }
        }
        return true;
    } else {
        OutputDebugStringW(L"GetSystemMenu(), SetMenuItemInfoW(), SetMenuDefaultItem(), TrackPopupMenu() and PostMessageW() are not available.");
        return false;
    }
}

bool WindowPrivate::SetWindowState2(const HWND hWnd, const WindowState state) noexcept
{
    USER32_API(ShowWindow);
    if (ShowWindowFunc) {
        if (!hWnd) {
            return false;
        }
        int nCmdShow = 0;
        switch (state) {
        case WindowState::Minimized: {
            nCmdShow = SW_SHOWMINIMIZED;
        } break;
        case WindowState::Normal: {
            nCmdShow = SW_SHOWNORMAL;
        } break;
        case WindowState::Maximized: {
            nCmdShow = SW_SHOWMAXIMIZED;
        } break;
        case WindowState::Visible: {
            nCmdShow = SW_SHOW;
        } break;
        case WindowState::Hidden: {
            nCmdShow = SW_HIDE;
        } break;
        }
        // Don't check ShowWindow()'s result because it returns the previous window state rather than
        // the operation result of itself.
        ShowWindowFunc(hWnd, nCmdShow);
        return true;
    } else {
        Utils::DisplayErrorDialog(L"ShowWindow() is not available.");
        return false;
    }
}

UINT WindowPrivate::GetWindowDPI2(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return g_defaultWindowDPI;
    }
    USER32_API(GetDpiForWindow);
    if (GetDpiForWindowFunc) {
        return GetDpiForWindowFunc(hWnd);
    } else {
        OutputDebugStringW(L"GetDpiForWindow() is not available.");
        USER32_API(GetSystemDpiForProcess);
        if (GetSystemDpiForProcessFunc) {
            return GetSystemDpiForProcessFunc(GetCurrentProcess());
        } else {
            OutputDebugStringW(L"GetSystemDpiForProcess() is not available.");
            USER32_API(GetDpiForSystem);
            if (GetDpiForSystemFunc) {
                return GetDpiForSystemFunc();
            } else {
                OutputDebugStringW(L"GetDpiForSystem() is not available.");
                SHCORE_API(GetDpiForMonitor);
                if (GetDpiForMonitorFunc) {
                    const HMONITOR mon = GetWindowScreen(hWnd, true);
                    if (mon) {
                        UINT dpiX = 0, dpiY = 0;
                        const HRESULT hr = GetDpiForMonitorFunc(mon, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
                        if (SUCCEEDED(hr)) {
                            return static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
                        } else {
                            PRINT_HR_ERROR_MESSAGE(GetDpiForMonitor, hr, L"Failed to retrieve the screen's DPI.")
                            return g_defaultWindowDPI;
                        }
                    } else {
                        OutputDebugStringW(L"Failed to retrieve the corresponding screen.");
                        return g_defaultWindowDPI;
                    }
                } else {
                    OutputDebugStringW(L"GetDpiForMonitor() is not available.");
                    USER32_API(GetDC);
                    USER32_API(ReleaseDC);
                    GDI32_API(GetDeviceCaps);
                    if (GetDCFunc && GetDeviceCapsFunc && ReleaseDCFunc) {
                        const HDC hdc = GetDCFunc(nullptr);
                        if (hdc) {
                            const int dpiX = GetDeviceCapsFunc(hdc, LOGPIXELSX);
                            const int dpiY = GetDeviceCapsFunc(hdc, LOGPIXELSY);
                            if (ReleaseDCFunc(nullptr, hdc) == 0) {
                                PRINT_WIN32_ERROR_MESSAGE(ReleaseDC, L"Failed to release the desktop window's DC.")
                                return g_defaultWindowDPI;
                            } else {
                                return static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
                            }
                        } else {
                            PRINT_WIN32_ERROR_MESSAGE(GetDC, L"Failed to retrieve the desktop window's DC.")
                            return g_defaultWindowDPI;
                        }
                    } else {
                        OutputDebugStringW(L"GetDC(), ReleaseDC() and GetDeviceCaps() are not available.");
                        return g_defaultWindowDPI;
                    }
                }
            }
        }
    }
}

UINT WindowPrivate::GetResizeBorderThickness2(const HWND hWnd, const bool x) noexcept
{
    USER32_API(GetSystemMetricsForDpi);
    if (GetSystemMetricsForDpiFunc) {
        if (!hWnd) {
            return g_defaultResizeBorderThickness;
        }
        const UINT dpi = GetWindowDPI2(hWnd);
        // There is no "SM_CYPADDEDBORDER".
        const int paddedBorderThickness = GetSystemMetricsForDpiFunc(SM_CXPADDEDBORDER, dpi);
        const int sizeFrameThickness = GetSystemMetricsForDpiFunc((x ? SM_CXSIZEFRAME : SM_CYSIZEFRAME), dpi);
        return static_cast<UINT>(paddedBorderThickness + sizeFrameThickness);
    } else {
        OutputDebugStringW(L"GetSystemMetricsForDpi() is not available.");
        return g_defaultResizeBorderThickness;
    }
}

UINT WindowPrivate::GetCaptionHeight2(const HWND hWnd) noexcept
{
    USER32_API(GetSystemMetricsForDpi);
    if (GetSystemMetricsForDpiFunc) {
        if (!hWnd) {
            return g_defaultCaptionHeight;
        }
        const UINT dpi = GetWindowDPI2(hWnd);
        return static_cast<UINT>(GetSystemMetricsForDpiFunc(SM_CYCAPTION, dpi));
    } else {
        OutputDebugStringW(L"GetSystemMetricsForDpi() is not available.");
        return g_defaultCaptionHeight;
    }
}

UINT WindowPrivate::GetFrameBorderThickness2(const HWND hWnd) noexcept
{
    DWMAPI_API(DwmGetWindowAttribute);
    if (DwmGetWindowAttributeFunc) {
        if (!hWnd) {
            return 0;
        }
        const auto dpr = (static_cast<double>(GetWindowDPI2(hWnd)) / static_cast<double>(g_defaultWindowDPI));
        UINT value = 0;
        const HRESULT hr = DwmGetWindowAttributeFunc(hWnd, g_DWMWA_VISIBLE_FRAME_BORDER_THICKNESS, &value, sizeof(value));
        if (SUCCEEDED(hr)) {
            return static_cast<UINT>(std::round(static_cast<double>(value) * dpr));
        } else {
            // We just eat this error because this enum value was introduced in a very
            // late Windows 10 version, so querying it's value will always result in
            // a "parameter error" (code: 87) on systems before that value was introduced.
            return static_cast<UINT>(std::round(static_cast<double>(g_defaultFrameBorderThickness) * dpr));
        }
    } else {
        OutputDebugStringW(L"DwmGetWindowAttribute() is not available.");
        return g_defaultFrameBorderThickness;
    }
}

WindowPrivate::WindowPrivate(Window *q) noexcept
{
    q_ptr = q;
    const auto result = CreateWindow2((WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS), WS_EX_NOREDIRECTIONBITMAP, nullptr, this, WindowProc);
    m_window = std::get<0>(result);
    m_atom = std::get<1>(result);
    if (!m_window || (m_atom == INVALID_ATOM)) {
        Utils::DisplayErrorDialog(L"Failed to create this window.");
    }
}

WindowPrivate::~WindowPrivate() noexcept
{
    if (m_window && (m_atom != INVALID_ATOM)) {
        if (CloseWindow2(m_window, ATOM_TO_STRING(m_atom))) {
            m_window = nullptr;
            m_atom = INVALID_ATOM;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close this window.");
        }
    }
}

std::wstring WindowPrivate::Title() const noexcept
{
    return m_title;
}

void WindowPrivate::Title(const std::wstring &value) noexcept
{
    USER32_API(SetWindowTextW);
    if (SetWindowTextWFunc) {
        if (SetWindowTextWFunc(m_window, value.c_str()) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowTextW, L"Failed to change the window title.")
        }
    } else {
        Utils::DisplayErrorDialog(L"Failed to change the window title due to SetWindowTextW() is not available.");
    }
}

int WindowPrivate::Icon() const noexcept
{
    return m_icon;
}

void WindowPrivate::Icon(const int value) noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(value);
}

int WindowPrivate::X() const noexcept
{
    return m_x;
}

void WindowPrivate::X(const int value) noexcept
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

void WindowPrivate::Y(const int value) noexcept
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

void WindowPrivate::Width(const UINT value) noexcept
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

void WindowPrivate::Height(const UINT value) noexcept
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
        if (!SetWindowState2(m_window, value)) {
            Utils::DisplayErrorDialog(L"Failed to update the Visibility property of this window.");
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

COLORREF WindowPrivate::ColorizationColor() const noexcept
{
    return m_colorizationColor;
}

WindowColorizationArea WindowPrivate::ColorizationArea() const noexcept
{
    return m_colorizationArea;
}

HWND WindowPrivate::CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData) const noexcept
{
    const auto result = CreateWindow2(style, (extendedStyle | WS_CHILD), m_window, extraData, wndProc);
    const HWND hWnd = std::get<0>(result);
    if (!hWnd) {
        Utils::DisplayErrorDialog(L"Failed to create child window.");
        return nullptr;
    }
    return hWnd;
}

HWND WindowPrivate::WindowHandle() const noexcept
{
    return m_window;
}

int WindowPrivate::MessageLoop() const noexcept
{
    USER32_API(GetMessageW);
    USER32_API(TranslateMessage);
    USER32_API(DispatchMessageW);
    if (GetMessageWFunc && TranslateMessageFunc && DispatchMessageWFunc) {
        MSG msg = {};
        while (GetMessageWFunc(&msg, nullptr, 0, 0) != FALSE) {
            if (!q_ptr->FilterMessage(&msg)) {
                TranslateMessageFunc(&msg);
                DispatchMessageWFunc(&msg);
            }
        }
        return static_cast<int>(msg.wParam);
    } else {
        Utils::DisplayErrorDialog(L"Failed to execute the message loop due to GetMessageW(), TranslateMessage() and DispatchMessageW() are not available.");
        return -1;
    }
}

bool WindowPrivate::Move(const int x, const int y) noexcept
{
    return SetGeometry(x, y, m_width, m_height);
}

bool WindowPrivate::Resize(const UINT w, const UINT h) noexcept
{
    return SetGeometry(m_x, m_y, w, h);
}

bool WindowPrivate::SetGeometry(const int x, const int y, const UINT w, const UINT h) noexcept
{
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        constexpr UINT flags = (SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER);
        if (SetWindowPosFunc(m_window, nullptr, x, y, w, h, flags) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos, L"Failed to change the window geometry.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"SetWindowPos() is not available.");
        return false;
    }
}

LRESULT CALLBACK WindowPrivate::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    USER32_API(SetWindowLongPtrW);
    USER32_API(GetWindowLongPtrW);
    USER32_API(DefWindowProcW);
    if (SetWindowLongPtrWFunc && GetWindowLongPtrWFunc && DefWindowProcWFunc) {
        if (message == WM_NCCREATE) {
            const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            const auto that = static_cast<WindowPrivate *>(cs->lpCreateParams);
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to set the window extra data.")
            }
        } else if (message == WM_NCDESTROY) {
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, 0) == 0) {
                PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW, L"Failed to clear the window extra data.")
            }
        }
        const auto that = reinterpret_cast<WindowPrivate *>(GetWindowLongPtrWFunc(hWnd, GWLP_USERDATA));
        if (that) {
            LRESULT result = 0;
            if (that->DefaultMessageHandler(message, wParam, lParam, &result)) {
                return result;
            } else {
                return that->q_ptr->MessageHandler(message, wParam, lParam);
            }
        } else {
            return DefWindowProcWFunc(hWnd, message, wParam, lParam);
        }
    } else {
        Utils::DisplayErrorDialog(L"SetWindowLongPtrW(), GetWindowLongPtrW() and DefWindowProcW() are not available.");
        return 0;
    }
}

bool WindowPrivate::DefaultMessageHandler(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result) noexcept
{
    if (!result) {
        Utils::DisplayErrorDialog(L"DefaultMessageHandler: the pointer to the result of the WindowProc function is null.");
        return false;
    }
    switch (message) {
    case WM_CREATE: {
        m_dpi = GetWindowDPI2(m_window);
        q_ptr->OnDotsPerInchChanged(m_dpi);
        const std::wstring dpiMsg = L"Current window's dots-per-inch (DPI): " + Utils::IntegerToString(m_dpi, 10);
        OutputDebugStringW(dpiMsg.c_str());
        if (!UpdateFrameMargins()) {
            Utils::DisplayErrorDialog(L"Failed to update the window frame margins.");
            return false;
        }
        if (!TriggerFrameChange2(m_window)) {
            Utils::DisplayErrorDialog(L"Failed to trigger a frame change event for this window.");
            return false;
        }
        m_theme = GetGlobalApplicationTheme2();
        q_ptr->OnThemeChanged(m_theme);
        const std::wstring themeMsg = L"Current window's theme: " + Utils::ThemeToString(m_theme);
        OutputDebugStringW(themeMsg.c_str());
        if (!SetWindowTheme2(m_window, m_theme)) {
            Utils::DisplayErrorDialog(L"Failed to change the window theme.");
            return false;
        }
        m_colorizationColor = GetColorizationColor2();
        q_ptr->OnColorizationColorChanged(m_colorizationColor);
        m_colorizationArea = GetGlobalColorizationArea2();
        q_ptr->OnColorizationAreaChanged(m_colorizationArea);
        m_visibility = WindowState::Hidden;
        q_ptr->OnVisibilityChanged(m_visibility);
        const auto cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        if ((cs->x == CW_USEDEFAULT) || (cs->y == CW_USEDEFAULT) || (cs->cx == CW_USEDEFAULT) || (cs->cy == CW_USEDEFAULT)) {
            const RECT frameGeometry = GetWindowFrameGeometry2(m_window);
            m_x = frameGeometry.left;
            m_y = frameGeometry.top;
            const SIZE windowSize = GetWindowClientSize2(m_window);
            m_width = windowSize.cx;
            m_height = windowSize.cy;
        } else {
            m_x = cs->x;
            m_y = cs->y;
            m_width = cs->cx;
            m_height = cs->cy;
        }
        q_ptr->OnXChanged(m_x);
        q_ptr->OnYChanged(m_y);
        q_ptr->OnWidthChanged(m_width);
        q_ptr->OnHeightChanged(m_height);
        LPCWSTR title = cs->lpszName;
        if (title) {
            m_title = title;
        } else {
            m_title = {};
        }
        q_ptr->OnTitleChanged(m_title);
    } break;
    case WM_MOVE: {
        m_x = GET_X_LPARAM(lParam);
        q_ptr->OnXChanged(m_x);
        m_y = GET_Y_LPARAM(lParam);
        q_ptr->OnYChanged(m_y);
    } break;
    case WM_SIZE: {
        bool needNotify = false;
        bool needChangeFrameMargins = false;
        if (wParam == SIZE_RESTORED) {
            if ((m_visibility == WindowState::Minimized) || (m_visibility == WindowState::Maximized)) {
                m_visibility = WindowState::Normal;
                needNotify = true;
                needChangeFrameMargins = true;
            }
        } else if (wParam == SIZE_MINIMIZED) {
            m_visibility = WindowState::Minimized;
            needNotify = true;
        } else if (wParam == SIZE_MAXIMIZED) {
            m_visibility = WindowState::Maximized;
            needNotify = true;
            needChangeFrameMargins = true;
        }
        if (needNotify) {
            q_ptr->OnVisibilityChanged(m_visibility);
        }
        m_width = LOWORD(lParam);
        q_ptr->OnWidthChanged(m_width);
        m_height = HIWORD(lParam);
        q_ptr->OnHeightChanged(m_height);
        if (needChangeFrameMargins) {
            if (!UpdateFrameMargins()) {
                Utils::DisplayErrorDialog(L"Failed to update the window frame margins.");
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
            q_ptr->OnThemeChanged(m_theme);
            if (!SetWindowTheme2(m_window, m_theme)) {
                Utils::DisplayErrorDialog(L"Failed to change the window theme.");
                return false;
            }
        }
    } break;
    case WM_DPICHANGED: {
        const UINT oldDPI = m_dpi;
        const UINT dpiX = LOWORD(wParam);
        const UINT dpiY = HIWORD(wParam);
        m_dpi = static_cast<UINT>(std::round(static_cast<double>(dpiX + dpiY) / 2.0));
        q_ptr->OnDotsPerInchChanged(m_dpi);
        const std::wstring dpiMsg = L"Current window's dots-per-inch (DPI) has changed from " + Utils::IntegerToString(oldDPI, 10) + L" to " + Utils::IntegerToString(m_dpi, 10) + L".";
        OutputDebugStringW(dpiMsg.c_str());
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (SetGeometry(prcNewWindow->left, prcNewWindow->top, RECT_WIDTH(*prcNewWindow), RECT_HEIGHT(*prcNewWindow))) {
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Failed to scale the window when DPI changes.");
            return false;
        }
    } break;
    case WM_DWMCOLORIZATIONCOLORCHANGED: {
        m_colorizationColor = wParam; // The color format is 0xAARRGGBB.
    } break;
    case WM_PAINT: {
        *result = 0;
        return true;
    } break;
    case WM_ERASEBKGND: {
        *result = 1;
        return true;
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
        if (CloseWindow2(m_window, ATOM_TO_STRING(m_atom))) {
            m_window = nullptr;
            m_atom = INVALID_ATOM;
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close this window.");
            return false;
        }
    } break;
    case WM_DESTROY: {
        USER32_API(PostQuitMessage);
        if (PostQuitMessageFunc) {
            PostQuitMessageFunc(0);
            *result = 0;
            return true;
        } else {
            Utils::DisplayErrorDialog(L"Failed to destroy this window due to PostQuitMessage() is not available.");
            return false;
        }
    } break;
    case WM_NCCREATE: {
        USER32_API(EnableNonClientDpiScaling);
        if (EnableNonClientDpiScalingFunc) {
            if (EnableNonClientDpiScalingFunc(m_window) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(EnableNonClientDpiScaling, L"Failed to enable non-client area DPI auto scaling.")
                return false;
            }
        } else {
            Utils::DisplayErrorDialog(L"Failed to enable non-client area DPI auto scaling due to EnableNonClientDpiScaling() is not available.");
            return false;
        }
    } break;
    case WM_NCCALCSIZE: {
        if (static_cast<BOOL>(wParam) == FALSE) {
            *result = 0;
            return true;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        USER32_API(DefWindowProcW);
        if (DefWindowProcWFunc) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcWFunc(m_window, WM_NCCALCSIZE, TRUE, lParam);
            if (ret != 0) {
                *result = ret;
                return true;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        } else {
            Utils::DisplayErrorDialog(L"DefWindowProcW() is not available.");
            return false;
        }
        bool nonClientAreaExists = false;
        const bool max = (m_visibility == WindowState::Maximized);
        const bool full = false;
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
            const UINT resizeBorderThicknessY = GetResizeBorderThickness2(m_window, false);
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
                    const RECT screenRect = Utils::GetScreenGeometry(m_window);
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
            Utils::DisplayErrorDialog(L"SHAppBarMessage() is not available.");
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
        // ### TODO: HTSYSMENU/HTMINBUTTON/HTMAXBUTTON/HTCLOSE
        const POINT globalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT localPos = globalPos;
        USER32_API(ScreenToClient);
        if (ScreenToClientFunc) {
            if (ScreenToClientFunc(m_window, &localPos) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(ScreenToClient, L"Failed to translate from screen coordinate to window coordinate.")
                return false;
            }
        } else {
            Utils::DisplayErrorDialog(L"ScreenToClient() is not available.");
            return false;
        }
        const auto resizeBorderThicknessY = static_cast<LONG>(GetResizeBorderThickness2(m_window, false));
        const UINT captionHeight = GetCaptionHeight2(m_window);
        const LONG titleBarHeight = ((m_visibility == WindowState::Maximized) ? captionHeight : (captionHeight + resizeBorderThicknessY));
        const bool isTitleBar = ((m_visibility != WindowState::Minimized) ? (localPos.y <= titleBarHeight) : false);
        const bool isTop = ((m_visibility == WindowState::Normal) ? (localPos.y <= resizeBorderThicknessY) : false);
        USER32_API(DefWindowProcW);
        if (DefWindowProcWFunc) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcWFunc(m_window, WM_NCHITTEST, 0, lParam);
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
            Utils::DisplayErrorDialog(L"DefWindowProcW() is not available.");
            return false;
        }
    } break;
    case WM_NCRBUTTONUP: {
        if (wParam == HTCAPTION) {
            const POINT mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (OpenSystemMenu2(m_window, mousePos)) {
                *result = 0;
                return true;
            } else {
                Utils::DisplayErrorDialog(L"Failed to open the system menu for the main window.");
                return false;
            }
        }
    } break;
    default:
        break;
    }
    return false;
}

bool WindowPrivate::UpdateFrameMargins() noexcept
{
    DWMAPI_API(DwmExtendFrameIntoClientArea);
    if (DwmExtendFrameIntoClientAreaFunc) {
        if (!m_window) {
            return false;
        }
        const auto borderThickness = static_cast<int>(GetFrameBorderThickness2(m_window));
        const MARGINS margins = {0, 0, ((m_visibility == WindowState::Maximized) ? 0 : borderThickness), 0};
        const HRESULT hr = DwmExtendFrameIntoClientAreaFunc(m_window, &margins);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmExtendFrameIntoClientArea, hr, L"Failed to update the frame margins for the window.")
            return false;
        }
        return true;
    } else {
        Utils::DisplayErrorDialog(L"DwmExtendFrameIntoClientArea() is not available.");
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

void Window::Title(const std::wstring &value) noexcept
{
    d_ptr->Title(value);
}

void Window::OnTitleChanged(const std::wstring &arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

int Window::Icon() const noexcept
{
    return d_ptr->Icon();
}

void Window::Icon(const int value) noexcept
{
    d_ptr->Icon(value);
}

void Window::OnIconChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

int Window::X() const noexcept
{
    return d_ptr->X();
}

void Window::X(const int value) noexcept
{
    d_ptr->X(value);
}

void Window::OnXChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

int Window::Y() const noexcept
{
    return d_ptr->Y();
}

void Window::Y(const int value) noexcept
{
    d_ptr->Y(value);
}

void Window::OnYChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

UINT Window::Width() const noexcept
{
    return d_ptr->Width();
}

void Window::Width(const UINT value) noexcept
{
    d_ptr->Width(value);
}

void Window::OnWidthChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

UINT Window::Height() const noexcept
{
    return d_ptr->Height();
}

void Window::Height(const UINT value) noexcept
{
    d_ptr->Height(value);
}

void Window::OnHeightChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

WindowState Window::Visibility() const noexcept
{
    return d_ptr->Visibility();
}

void Window::Visibility(const WindowState value) noexcept
{
    d_ptr->Visibility(value);
}

void Window::OnVisibilityChanged(const WindowState arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

WindowTheme Window::Theme() const noexcept
{
    return d_ptr->Theme();
}

void Window::OnThemeChanged(const WindowTheme arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

UINT Window::DotsPerInch() const noexcept
{
    return d_ptr->DotsPerInch();
}

void Window::OnDotsPerInchChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

COLORREF Window::ColorizationColor() const noexcept
{
    return d_ptr->ColorizationColor();
}

void Window::OnColorizationColorChanged(const COLORREF arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

WindowColorizationArea Window::ColorizationArea() const noexcept
{
    return d_ptr->ColorizationArea();
}

void Window::OnColorizationAreaChanged(const WindowColorizationArea arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
}

HWND Window::CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData) const noexcept
{
    return d_ptr->CreateChildWindow(style, extendedStyle, wndProc, extraData);
}

HWND Window::WindowHandle() const noexcept
{
    return d_ptr->WindowHandle();
}

int Window::MessageLoop() const noexcept
{
    return d_ptr->MessageLoop();
}

bool Window::Move(const int x, const int y) noexcept
{
    return d_ptr->Move(x, y);
}

bool Window::Resize(const UINT w, const UINT h) noexcept
{
    return d_ptr->Resize(w, h);
}

bool Window::SetGeometry(const int x, const int y, const UINT w, const UINT h) noexcept
{
    return d_ptr->SetGeometry(x, y, w, h);
}
