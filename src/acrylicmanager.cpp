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

#include "acrylicmanager.h"
#include "acrylicmanager_p.h"
#include "resource.h"

#include <wininet.h>
#include <ShlObj_Core.h>
#include <ShellApi.h>
#include <ShellScalingApi.h>
#include <UxTheme.h>
#include <DwmApi.h>

#include "colorconversion.h"



#ifndef ABM_GETAUTOHIDEBAREX
#define ABM_GETAUTOHIDEBAREX (0x0000000b)
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

/////////////////////////////////
/////     Global variables
/////////////////////////////////

// The thickness of an auto-hide taskbar in pixels.
static constexpr int g_am_AutoHideTaskbarThicknessPx_p = 2;
static constexpr int g_am_AutoHideTaskbarThicknessPy_p = g_am_AutoHideTaskbarThicknessPx_p;

static constexpr wchar_t g_am_ForceOfficialBlurEnvVar_p[] = L"ACRYLICMANAGER_FORCE_OFFICIAL_BLUR";
static constexpr wchar_t g_am_ForceXAMLIslandEnvVar_p[] = L"ACRYLICMANAGER_FORCE_XAML_ISLAND";
static constexpr wchar_t g_am_ForceDirect2DEnvVar_p[] = L"ACRYLICMANAGER_FORCE_DIRECT2D";
static constexpr wchar_t g_am_WindowClassNamePrefix_p[] = LR"(wangwenx190\AcrylicManager\WindowClass\)";
static constexpr wchar_t g_am_MainWindowClassNameSuffix_p[] = L"@MainWindow";
static constexpr wchar_t g_am_DragBarWindowClassNameSuffix_p[] = L"@DragBarWindow";

static LPWSTR g_am_MainWindowClassName_p = nullptr;
static LPWSTR g_am_DragBarWindowClassName_p = nullptr;
static constexpr wchar_t g_am_MainWindowTitle_p[] = L"AcrylicManager Main Window";
static constexpr wchar_t *g_am_DragBarWindowTitle_p = nullptr;
static bool g_am_AcrylicManagerInitialized_p = false;
static ATOM g_am_MainWindowAtom_p = 0;
static ATOM g_am_DragBarWindowAtom_p = 0;
static HWND g_am_MainWindowHandle_p = nullptr;
static HWND g_am_XAMLIslandWindowHandle_p = nullptr;
static HWND g_am_DragBarWindowHandle_p = nullptr;
static UINT g_am_CurrentDpi_p = 0;
static double g_am_CurrentDpr_p = 0.0;
static HWND g_am_HostWindowHandle_p = nullptr;
static WNDPROC g_am_HostWindowProc_p = nullptr;
static SystemTheme g_am_BrushTheme_p = SystemTheme::Invalid;
static LPWSTR g_am_WallpaperFilePath_p = nullptr;
static D2D1_COLOR_F g_am_DesktopBackgroundColor_p = D2D1::ColorF(D2D1::ColorF::Black);
static WallpaperAspectStyle g_am_WallpaperAspectStyle_p = WallpaperAspectStyle::Invalid;
static winrt::Windows::UI::Color g_am_TintColor_p = {};
static double g_am_TintOpacity_p = 0.0;
static winrt::Windows::Foundation::IReference<double> g_am_TintLuminosityOpacity_p = nullptr;
static winrt::Windows::UI::Color g_am_FallbackColor_p = {};
static std::unordered_map<LPCWSTR, HMODULE> g_am_LoadedModuleList_p = {};

static bool g_am_IsUsingOfficialBlur_p = false;
static bool g_am_IsUsingXAMLIsland_p = false;
static bool g_am_IsUsingDirect2D_p = false;

/////////////////////////////////
/////     Helper functions
/////////////////////////////////

[[nodiscard]] static inline HRESULT am_UpdateFrameMarginsHelper_p(const HWND hWnd, const UINT dpi)
{
    if (!hWnd || (dpi == 0)) {
        return E_INVALIDARG;
    }
    bool normal = false;
    if (FAILED(am_IsWindowNoState_p(hWnd, &normal))) {
        return E_FAIL;
    }
    int thickness = 0;
    if (FAILED(am_GetWindowVisibleFrameBorderThickness_p(hWnd, dpi, &thickness))) {
        return E_FAIL;
    }
    const LONG topFrameMargin = (normal ? thickness : 0);
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
    const MARGINS margins = {0, 0, topFrameMargin, 0};
    return DwmExtendFrameIntoClientArea(hWnd, &margins);
}

[[nodiscard]] static inline HRESULT am_ShowFullScreenHelper_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    auto style = static_cast<DWORD>(GetWindowLongPtrW(hWnd, GWL_STYLE));
    style &= ~(WS_OVERLAPPEDWINDOW); // fixme: check
    style |= WS_POPUP;
    SetWindowLongPtrW(hWnd, GWL_STYLE, style);
    if (enable) {
        // fixme: rethink
        RECT rect = {};
        if (FAILED(am_GetScreenGeometry_p(hWnd, &rect))) {
            return E_FAIL;
        }
        if (MoveWindow(hWnd, rect.left, rect.top,
                        GET_RECT_WIDTH(rect), GET_RECT_HEIGHT(rect), TRUE) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(MoveWindow)
        }
    } else {
        // todo
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetSystemThemeHelper_p(SystemTheme *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    bool highContrast = false;
    if (FAILED(am_IsHighContrastModeEnabled_p(&highContrast))) {
        return E_FAIL;
    }
    // Dark mode was first introduced in Windows 10 1607.
    if (g_am_IsDarkModeAvailable_p) {
        bool lightModeOn = false;
        if (FAILED(am_ShouldAppsUseLightTheme_p(&lightModeOn))) {
            if (FAILED(am_ShouldSystemUsesLightTheme_p(&lightModeOn))) {
                return E_FAIL;
            }
        }
        *result = (lightModeOn ? SystemTheme::Light : SystemTheme::Dark);
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_GetWindowGeometryHelper_p(const HWND hWnd, RECT *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    RECT rect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &rect) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetWindowRect)
    }
    *result = rect;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetWindowSizeHelper_p(const HWND hWnd, SIZE *size)
{
    if (!hWnd || !size) {
        return E_INVALIDARG;
    }
    RECT rect = {};
    if (FAILED(am_GetWindowGeometryHelper_p(hWnd, &rect))) {
        return E_FAIL;
    }
    *size = GET_RECT_SIZE(rect);
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_MoveWindowHelper_p(const HWND hWnd, const int x, const int y)
{
    if (!hWnd || (x <= 0) || (y <= 0)) {
        return E_INVALIDARG;
    }
    SIZE size = {};
    if (FAILED(am_GetWindowSizeHelper_p(hWnd, &size))) {
        return E_FAIL;
    }
    if (MoveWindow(hWnd, x, y, size.cx, size.cy, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(MoveWindow)
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_ResizeWindowHelper_p(const HWND hWnd, const int w, const int h)
{
    if (!hWnd || (w <= 0) || (h <= 0)) {
        return E_INVALIDARG;
    }
    RECT rect = {};
    if (FAILED(am_GetWindowGeometryHelper_p(hWnd, &rect))) {
        return E_FAIL;
    }
    if (MoveWindow(hWnd, rect.left, rect.top, w, h, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(MoveWindow)
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_CenterWindowHelper_p(const HWND hWnd)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &windowRect) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetWindowRect)
    }
    const int windowWidth = GET_RECT_WIDTH(windowRect);
    const int windowHeight = GET_RECT_HEIGHT(windowRect);
    RECT screenRect = {};
    if (FAILED(am_GetScreenGeometry_p(hWnd, &screenRect))) {
        return E_FAIL;
    }
    const int screenWidth = GET_RECT_WIDTH(screenRect);
    const int screenHeight = GET_RECT_HEIGHT(screenRect);
    const int newX = (screenWidth - windowWidth) / 2;
    const int newY = (screenHeight - windowHeight) / 2;
    if (MoveWindow(hWnd, newX, newY, windowWidth, windowHeight, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(MoveWindow)
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetWindowStateHelper_p(const HWND hWnd, WindowState *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    bool state = false;
    if (SUCCEEDED(am_IsFullScreened_p(hWnd, &state)) && state) {
        *result = WindowState::FullScreened;
        return S_OK;
    } else if (SUCCEEDED(am_IsMaximized_p(hWnd, &state)) && state) {
        *result = WindowState::Maximized;
        return S_OK;
    } else if (SUCCEEDED(am_IsMinimized_p(hWnd, &state)) && state) {
        *result = WindowState::Minimized;
        return S_OK;
    } else if (SUCCEEDED(am_IsWindowNoState_p(hWnd, &state)) && state) {
        *result = WindowState::Normal;
        return S_OK;
    } else if (SUCCEEDED(am_IsWindowVisible_p(hWnd, &state)) && state) {
        *result = WindowState::Shown;
        return S_OK;
    } else if (SUCCEEDED(am_IsWindowVisible_p(hWnd, &state)) && !state) {
        *result = WindowState::Hidden;
        return S_OK;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_SetWindowStateHelper_p(const HWND hWnd, const WindowState state)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    switch (state) {
    case WindowState::Normal:
        ShowWindow(hWnd, SW_NORMAL);
        return S_OK;
    case WindowState::Maximized:
        ShowWindow(hWnd, SW_MAXIMIZE);
        return S_OK;
    case WindowState::Minimized:
        ShowWindow(hWnd, SW_MINIMIZE);
        return S_OK;
    case WindowState::FullScreened:
        return am_ShowFullScreenHelper_p(hWnd, true);
    case WindowState::Hidden:
        ShowWindow(hWnd, SW_HIDE);
        return S_OK;
    case WindowState::Shown:
        ShowWindow(hWnd, SW_SHOW);
        return S_OK;
    default:
        break;
    }
    return E_FAIL;
}

[[nodiscard]] static inline HRESULT am_CleanupHelper_p()
{
    g_am_IsUsingOfficialBlur_p = false;

    // Host window
    if (g_am_HostWindowHandle_p && g_am_HostWindowProc_p) {
        SetWindowLongPtrW(g_am_HostWindowHandle_p, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_am_HostWindowProc_p));
        g_am_HostWindowProc_p = nullptr;
        g_am_HostWindowHandle_p = nullptr;
    }

    // Direct2D
    g_am_D2DBitmapProperties_p = {};
    g_am_DXGISwapChainDesc_p = {};
    SAFE_FREE_CHARARRAY(g_am_WallpaperFilePath_p)
    g_am_DesktopBackgroundColor_p = D2D1::ColorF(D2D1::ColorF::Black);
    g_am_WallpaperAspectStyle_p = WallpaperAspectStyle::Invalid;
    g_am_D3DFeatureLevel_p = D3D_FEATURE_LEVEL_1_0_CORE;
    g_am_IsUsingDirect2D_p = false;

    // XAML Island
    if (g_am_XAMLSource_p) {
        g_am_XAMLSource_p.Close();
        g_am_XAMLSource_p = nullptr;
    }
    g_am_RootGrid_p = nullptr;
    g_am_BackgroundBrush_p = nullptr;
    if (g_am_XAMLManager_p) {
        g_am_XAMLManager_p.Close();
        g_am_XAMLManager_p = nullptr;
    }
    g_am_BrushTheme_p = SystemTheme::Invalid;
    g_am_TintColor_p = {};
    g_am_TintOpacity_p = 0.0;
    g_am_TintLuminosityOpacity_p = nullptr;
    g_am_FallbackColor_p = {};
    g_am_IsUsingXAMLIsland_p = false;

    // Drag bar window
    if (g_am_DragBarWindowHandle_p) {
        if (DestroyWindow(g_am_DragBarWindowHandle_p) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow)
        }
        g_am_DragBarWindowHandle_p = nullptr;
    }
    if (g_am_DragBarWindowAtom_p != 0) {
        if (g_am_DragBarWindowClassName_p) {
            if (UnregisterClassW(g_am_DragBarWindowClassName_p, HINST_THISCOMPONENT) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW)
            }
            SAFE_FREE_CHARARRAY(g_am_DragBarWindowClassName_p)
        }
        g_am_DragBarWindowAtom_p = 0;
    }

    // Main window
    if (g_am_MainWindowHandle_p) {
        if (DestroyWindow(g_am_MainWindowHandle_p) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow)
        }
        g_am_MainWindowHandle_p = nullptr;
    }
    if (g_am_MainWindowAtom_p != 0) {
        if (g_am_MainWindowClassName_p) {
            if (UnregisterClassW(g_am_MainWindowClassName_p, HINST_THISCOMPONENT) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW)
            }
            SAFE_FREE_CHARARRAY(g_am_MainWindowClassName_p)
        }
        g_am_MainWindowAtom_p = 0;
    }
    g_am_CurrentDpi_p = 0;
    g_am_CurrentDpr_p = 0.0;

    am_ReleaseAllLoadedModules_p();

    g_am_AcrylicManagerInitialized_p = false;

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetTintColorHelper_p(int *r, int *g, int *b, int *a)
{
    if (!g_am_BackgroundBrush_p || !r || !g || !b || !a) {
        return E_INVALIDARG;
    }
    GET_WINRTCOLOR_COMPONENTS(g_am_TintColor_p, *r, *g, *b, *a)
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetTintColorHelper_p(const int r, const int g, const int b, const int a)
{
    if (!g_am_BackgroundBrush_p) {
        return E_INVALIDARG;
    }
    const winrt::Windows::UI::Color color = MAKE_WINRTCOLOR_FROM_COMPONENTS(r, g, b, a);
    g_am_BackgroundBrush_p.TintColor(color);
    g_am_TintColor_p = color;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetTintOpacityHelper_p(double *result)
{
    if (!g_am_BackgroundBrush_p || !result) {
        return E_INVALIDARG;
    }
    *result = g_am_TintOpacity_p;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetTintOpacityHelper_p(const double opacity)
{
    if (!g_am_BackgroundBrush_p) {
        return E_INVALIDARG;
    }
    const double value = std::clamp(opacity, 0.0, 1.0);
    g_am_BackgroundBrush_p.TintOpacity(value);
    g_am_TintOpacity_p = value;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetTintLuminosityOpacityHelper_p(double *result)
{
    if (!g_am_BackgroundBrush_p || !result) {
        return E_INVALIDARG;
    }
    if (g_am_TintLuminosityOpacity_p != nullptr) {
        *result = g_am_TintLuminosityOpacity_p.GetDouble();
    } else {
        *result = -1.0;
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetTintLuminosityOpacityHelper_p(const double *opacity)
{
    if (!g_am_BackgroundBrush_p) {
        return E_INVALIDARG;
    }
    if (opacity && (*opacity >= 0.0)) {
        const double value = std::clamp(*opacity, 0.0, 1.0);
        g_am_BackgroundBrush_p.TintLuminosityOpacity(value);
        g_am_TintLuminosityOpacity_p = value;
    } else {
        g_am_BackgroundBrush_p.TintLuminosityOpacity(nullptr);
        g_am_TintLuminosityOpacity_p = nullptr;
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetFallbackColorHelper_p(int *r, int *g, int *b, int *a)
{
    if (!g_am_BackgroundBrush_p || !r || !g || !b || !a) {
        return E_INVALIDARG;
    }
    GET_WINRTCOLOR_COMPONENTS(g_am_FallbackColor_p, *r, *g, *b, *a)
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetFallbackColorHelper_p(const int r, const int g, const int b, const int a)
{
    if (!g_am_BackgroundBrush_p) {
        return E_INVALIDARG;
    }
    const winrt::Windows::UI::Color color = MAKE_WINRTCOLOR_FROM_COMPONENTS(r, g, b, a);
    g_am_BackgroundBrush_p.FallbackColor(color);
    g_am_FallbackColor_p = color;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SwitchAcrylicBrushThemeHelper_p(const SystemTheme theme)
{
    if ((theme == SystemTheme::Invalid) || (theme == SystemTheme::HighContrast)) {
        return E_INVALIDARG;
    }
    winrt::Windows::UI::Color tc = {};
    double to = 0.0;
    double tlo = 0.0;
    winrt::Windows::UI::Color fbc = {};
    if (theme == SystemTheme::Light) {
        tc = MAKE_WINRTCOLOR_FROM_COMPONENTS(252, 252, 252, 255); // #FCFCFC
        to = 0.0;
        tlo = 0.85;
        fbc = MAKE_WINRTCOLOR_FROM_COMPONENTS(249, 249, 249, 255); // #F9F9F9
    } else {
        tc = MAKE_WINRTCOLOR_FROM_COMPONENTS(44, 44, 44, 255); // #2C2C2C
        to = 0.15;
        tlo = 0.96;
        fbc = MAKE_WINRTCOLOR_FROM_COMPONENTS(44, 44, 44, 255); // #2C2C2C
    }
    int r = 0, g = 0, b = 0, a = 0;
    GET_WINRTCOLOR_COMPONENTS(tc, r, g, b, a)
    if (FAILED(am_SetTintColorHelper_p(r, g, b, a))) {
        return E_FAIL;
    }
    if (FAILED(am_SetTintOpacityHelper_p(to))) {
        return E_FAIL;
    }
    if (FAILED(am_SetTintLuminosityOpacityHelper_p(&tlo))) {
        return E_FAIL;
    }
    GET_WINRTCOLOR_COMPONENTS(fbc, r, g, b, a)
    if (FAILED(am_SetFallbackColorHelper_p(r, g, b, a))) {
        return E_FAIL;
    }
    g_am_BrushTheme_p = theme;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_SetWindowGeometryHelper_p(const HWND hWnd, const int x, const int y, const int w, const int h)
{
    if (!hWnd || (x <= 0) || (y <= 0) || (w <= 0) || (h <= 0)) {
        return E_INVALIDARG;
    }
    if (MoveWindow(hWnd, x, y, w, h, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(MoveWindow)
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_IsWindowActiveHelper_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    *result = (GetActiveWindow() == hWnd);
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetWindowHandleHelper_p(HWND *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    *result = g_am_MainWindowHandle_p;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetBrushThemeHelper_p(SystemTheme *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    *result = g_am_BrushTheme_p;
    return S_OK;
}

// XAML

[[nodiscard]] static inline double am_GetTintOpacityModifier_p(const winrt::Windows::UI::Color &tintColor)
{
    // TintOpacityModifier affects the 19H1+ Luminosity-based recipe only
    if (!g_am_IsWindows10_19H1OrGreater_p) {
        return 1.0;
    }

    // This method supresses the maximum allowable tint opacity depending on the luminosity and saturation of a color by
    // compressing the range of allowable values - for example, a user-defined value of 100% will be mapped to 45% for pure
    // white (100% luminosity), 85% for pure black (0% luminosity), and 90% for pure gray (50% luminosity).  The intensity of
    // the effect increases linearly as luminosity deviates from 50%.  After this effect is calculated, we cancel it out
    // linearly as saturation increases from zero.

    const double midPoint = 0.50; // Mid point of HsvV range that these calculations are based on. This is here for easy tuning.

    const double whiteMaxOpacity = 0.45; // 100% luminosity
    const double midPointMaxOpacity = 0.90; // 50% luminosity
    const double blackMaxOpacity = 0.85; // 0% luminosity

    const Rgb rgb = RgbFromColor(tintColor);
    const Hsv hsv = RgbToHsv(rgb);

    double opacityModifier = midPointMaxOpacity;

    if (hsv.v != midPoint) {
        // Determine maximum suppression amount
        double lowestMaxOpacity = midPointMaxOpacity;
        double maxDeviation = midPoint;

        if (hsv.v > midPoint) {
            lowestMaxOpacity = whiteMaxOpacity; // At white (100% hsvV)
            maxDeviation = (1.0 - maxDeviation);
        } else if (hsv.v < midPoint) {
            lowestMaxOpacity = blackMaxOpacity; // At black (0% hsvV)
        }

        double maxOpacitySuppression = (midPointMaxOpacity - lowestMaxOpacity);

        // Determine normalized deviation from the midpoint
        const double deviation = std::abs(hsv.v - midPoint);
        const double normalizedDeviation = (deviation / maxDeviation);

        // If we have saturation, reduce opacity suppression to allow that color to come through more
        if (hsv.s > 0.0) {
            // Dampen opacity suppression based on how much saturation there is
            maxOpacitySuppression *= std::max((1.0 - (hsv.s * 2.0)), 0.0);
        }

        const double opacitySuppression = (maxOpacitySuppression * normalizedDeviation);

        opacityModifier = (midPointMaxOpacity - opacitySuppression);
    }

    return opacityModifier;
}

// The tintColor passed into this method should be the original, unmodified color created using user values for TintColor + TintOpacity
[[nodiscard]] static inline winrt::Windows::UI::Color am_GetLuminosityColor_p(
        const winrt::Windows::UI::Color &tintColor,
        const winrt::Windows::Foundation::IReference<double> luminosityOpacity)
{
    const Rgb rgbTintColor = RgbFromColor(tintColor);

    // If luminosity opacity is specified, just use the values as is
    if (luminosityOpacity != nullptr) {
        return ColorFromRgba(rgbTintColor, std::clamp(luminosityOpacity.GetDouble(), 0.0, 1.0));
    } else {
        // To create the Luminosity blend input color without luminosity opacity,
        // we're taking the TintColor input, converting to HSV, and clamping the V between these values
        const double minHsvV = 0.125;
        const double maxHsvV = 0.965;

        const Hsv hsvTintColor = RgbToHsv(rgbTintColor);

        const auto clampedHsvV = std::clamp(hsvTintColor.v, minHsvV, maxHsvV);

        const Hsv hsvLuminosityColor = Hsv(hsvTintColor.h, hsvTintColor.s, clampedHsvV);
        const Rgb rgbLuminosityColor = HsvToRgb(hsvLuminosityColor);

        // Now figure out luminosity opacity
        // Map original *tint* opacity to this range
        const double minLuminosityOpacity = 0.15;
        const double maxLuminosityOpacity = 1.03;

        const double luminosityOpacityRangeMax = (maxLuminosityOpacity - minLuminosityOpacity);
        const double mappedTintOpacity = (((tintColor.A / 255.0) * luminosityOpacityRangeMax) + minLuminosityOpacity);

        // Finally, combine the luminosity opacity and the HsvV-clamped tint color
        return ColorFromRgba(rgbLuminosityColor, std::min(mappedTintOpacity, 1.0));
    }
}

[[nodiscard]] static inline HRESULT am_MainWindowEventLoopHelper_p(int *result)
{
    if (!result || !g_am_MainWindowHandle_p) {
        return E_INVALIDARG;
    }

    MSG msg = {};

    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        BOOL filtered = FALSE;
        if (g_am_XAMLSource_p) {
            const auto interop = g_am_XAMLSource_p.as<IDesktopWindowXamlSourceNative2>();
            if (interop) {
                winrt::check_hresult(interop->PreTranslateMessage(&msg, &filtered));
            }
        }
        if (filtered == FALSE) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    *result = static_cast<int>(msg.wParam);

    return S_OK;
}

[[nodiscard]] static inline LRESULT CALLBACK am_MainWindowProcHelper_p(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    bool wallpaperChanged = false;
    bool systemThemeChanged = false;

    switch (uMsg)
    {
    case WM_NCCALCSIZE: {
        if (!wParam) {
            return 0;
        }
        const auto clientRect = &(reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam)->rgrc[0]);
        if (g_am_IsWindows10OrGreater_p) {
            // Store the original top before the default window proc applies the default frame.
            const LONG originalTop = clientRect->top;
            // Apply the default frame
            const LRESULT ret = DefWindowProcW(hWnd, uMsg, wParam, lParam);
            if (ret != 0) {
                return ret;
            }
            // Re-apply the original top from before the size of the default frame was applied.
            clientRect->top = originalTop;
        }
        // We don't need this correction when we're fullscreen. We will
        // have the WS_POPUP size, so we don't have to worry about
        // borders, and the default frame will be fine.
        bool nonClientAreaExists = false;
        bool max = false, full = false;
        if (FAILED(am_IsMaximized_p(hWnd, &max)) || FAILED(am_IsFullScreened_p(hWnd, &full))) {
            PRINT(L"Failed to retrieve the window state.")
            break;
        }
        if (max && !full) {
            // When a window is maximized, its size is actually a little bit more
            // than the monitor's work area. The window is positioned and sized in
            // such a way that the resize handles are outside of the monitor and
            // then the window is clipped to the monitor so that the resize handle
            // do not appear because you don't need them (because you can't resize
            // a window when it's maximized unless you restore it).
            int rbtY = 0;
            if (SUCCEEDED(am_GetResizeBorderThickness_p(false, g_am_CurrentDpi_p, &rbtY))) {
                clientRect->top += rbtY;
                if (!g_am_IsWindows10OrGreater_p) {
                    clientRect->bottom -= rbtY;
                    int rbtX = 0;
                    if (SUCCEEDED(am_GetResizeBorderThickness_p(true, g_am_CurrentDpi_p, &rbtX))) {
                        clientRect->left += rbtX;
                        clientRect->right -= rbtX;
                    } else {
                        PRINT(L"Failed to retrieve the resize border thickness (x).")
                        break;
                    }
                }
            } else {
                PRINT(L"Failed to retrieve the resize border thickness (y).")
                break;
            }
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
                bool top = false, bottom = false, left = false, right = false;
                // Due to "ABM_GETAUTOHIDEBAREX" only has effect since Windows 8.1,
                // we have to use another way to judge the edge of the auto-hide taskbar
                // when the application is running on Windows 7 or Windows 8.
                if (g_am_IsWindows8Point1OrGreater_p) {
                    RECT screenRect = {0, 0, 0, 0};
                    if (SUCCEEDED(am_GetScreenGeometry_p(hWnd, &screenRect))) {
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
                        top = hasAutohideTaskbar(ABE_TOP);
                        bottom = hasAutohideTaskbar(ABE_BOTTOM);
                        left = hasAutohideTaskbar(ABE_LEFT);
                        right = hasAutohideTaskbar(ABE_RIGHT);
                    } else {
                        PRINT(L"Failed to retrieve the current screen geometry.")
                        break;
                    }
                } else {
                    // The following code is copied from Mozilla Firefox with some modifications.
                    int edge = -1;
                    APPBARDATA _abd;
                    SecureZeroMemory(&_abd, sizeof(_abd));
                    _abd.cbSize = sizeof(_abd);
                    _abd.hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
                    if (_abd.hWnd) {
                        const HMONITOR windowMonitor = GET_CURRENT_SCREEN(hWnd);
                        if (!windowMonitor) {
                            PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow)
                            break;
                        }
                        const HMONITOR taskbarMonitor = GET_PRIMARY_SCREEN(_abd.hWnd);
                        if (!taskbarMonitor) {
                            PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow)
                            break;
                        }
                        if (taskbarMonitor == windowMonitor) {
                            SHAppBarMessage(ABM_GETTASKBARPOS, &_abd);
                            edge = _abd.uEdge;
                        }
                    } else {
                        PRINT_WIN32_ERROR_MESSAGE(FindWindowW)
                        break;
                    }
                    top = (edge == ABE_TOP);
                    bottom = (edge == ABE_BOTTOM);
                    left = (edge == ABE_LEFT);
                    right = (edge == ABE_RIGHT);
                }
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
                if (top) {
                    // Peculiarly, when we're fullscreen,
                    clientRect->top += g_am_AutoHideTaskbarThicknessPy_p;
                    nonClientAreaExists = true;
                } else if (bottom) {
                    clientRect->bottom -= g_am_AutoHideTaskbarThicknessPy_p;
                    nonClientAreaExists = true;
                } else if (left) {
                    clientRect->left += g_am_AutoHideTaskbarThicknessPx_p;
                    nonClientAreaExists = true;
                } else if (right) {
                    clientRect->right -= g_am_AutoHideTaskbarThicknessPx_p;
                    nonClientAreaExists = true;
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
        return nonClientAreaExists ? 0 : WVR_REDRAW;
    } break;
    case WM_NCHITTEST: {
        const POINT globalPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        POINT localPos = globalPos;
        if (ScreenToClient(hWnd, &localPos) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(ScreenToClient)
            break;
        }
        bool max = false, full = false, normal = false;
        if (FAILED(am_IsMaximized_p(hWnd, &max)) || FAILED(am_IsFullScreened_p(hWnd, &full))
                || FAILED(am_IsWindowNoState_p(hWnd, &normal))) {
            PRINT(L"Failed to retrieve the window state.")
            break;
        }
        const bool maxOrFull = (max || full);
        SIZE ws = {0, 0};
        if (FAILED(am_GetWindowSizeHelper_p(hWnd, &ws))) {
            PRINT(L"Failed to retrieve the window size.")
            break;
        }
        const LONG ww = ws.cx;
        int rbtX = 0, rbtY = 0, cth = 0;
        if (FAILED(am_GetResizeBorderThickness_p(true, g_am_CurrentDpi_p, &rbtX))
                || FAILED(am_GetResizeBorderThickness_p(false, g_am_CurrentDpi_p, &rbtY))
                || FAILED(am_GetCaptionHeight_p(g_am_CurrentDpi_p, &cth))) {
            PRINT(L"Failed retrieve the resize border thickness or caption height.")
            break;
        }
        const bool hitTestVisible = /*am_IsHitTestVisibleInChrome_p(hWnd)*/false; // todo
        bool isTitleBar = false;
        if (maxOrFull) {
            isTitleBar = ((localPos.y >= 0) && (localPos.y <= cth)
                          && (localPos.x >= 0) && (localPos.x <= ww)
                          && !hitTestVisible);
        } else if (normal) {
            isTitleBar = ((localPos.y > rbtY) && (localPos.y <= (rbtY + cth))
                          && (localPos.x > rbtX) && (localPos.x < (ww - rbtX))
                          && !hitTestVisible);
        }
        const bool isTop = (normal ? (localPos.y <= rbtY) : false);
        if (g_am_IsWindows10OrGreater_p) {
            // This will handle the left, right and bottom parts of the frame
            // because we didn't change them.
            const LRESULT originalRet = DefWindowProcW(hWnd, uMsg, wParam, lParam);
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
            if (maxOrFull) {
                if (isTitleBar) {
                    return HTCAPTION;
                }
                return HTCLIENT;
            }
            const LONG wh = ws.cy;
            const bool isBottom = (normal ? (localPos.y >= (wh - rbtY)) : false);
            // Make the border a little wider to let the user easy to resize on corners.
            const double factor = (normal ? ((isTop || isBottom) ? 2.0 : 1.0) : 0.0);
            const bool isLeft = (normal ? (localPos.x <= std::round(static_cast<double>(rbtX) * factor)) : false);
            const bool isRight = (normal ? (localPos.x >= (ww - std::round(static_cast<double>(rbtX) * factor))) : false);
            if (isTop) {
                if (isLeft) {
                    return HTTOPLEFT;
                }
                if (isRight) {
                    return HTTOPRIGHT;
                }
                return HTTOP;
            }
            if (isBottom) {
                if (isLeft) {
                    return HTBOTTOMLEFT;
                }
                if (isRight) {
                    return HTBOTTOMRIGHT;
                }
                return HTBOTTOM;
            }
            if (isLeft) {
                return HTLEFT;
            }
            if (isRight) {
                return HTRIGHT;
            }
            if (isTitleBar) {
                return HTCAPTION;
            }
            return HTCLIENT;
        }
        return HTNOWHERE;
    } break;
    case WM_PAINT: {
        if (g_am_IsWindows10OrGreater_p) {
            PAINTSTRUCT ps = {};
            const HDC hdc = BeginPaint(hWnd, &ps);
            if (!hdc) {
                PRINT_WIN32_ERROR_MESSAGE(BeginPaint)
                break;
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
            int borderThickness = 0;
            if (FAILED(am_GetWindowVisibleFrameBorderThickness_p(hWnd, g_am_CurrentDpi_p, &borderThickness))) {
                PRINT(L"Failed to retrieve the window visible frame border thickness.")
                break;
            }
            if (ps.rcPaint.top < borderThickness) {
                RECT rcPaint = ps.rcPaint;
                rcPaint.bottom = borderThickness;
                // To show the original top border, we have to paint on top
                // of it with the alpha component set to 0. This page
                // recommends to paint the area in black using the stock
                // BLACK_BRUSH to do this:
                // https://docs.microsoft.com/en-us/windows/win32/dwm/customframe#extending-the-client-frame
                if (FillRect(hdc, &rcPaint, GET_BLACK_BRUSH) == 0) {
                    PRINT_WIN32_ERROR_MESSAGE(FillRect)
                    break;
                }
            }
            if (ps.rcPaint.bottom > borderThickness) {
                RECT rcPaint = ps.rcPaint;
                rcPaint.top = borderThickness;
                // To hide the original title bar, we have to paint on top
                // of it with the alpha component set to 255. This is a hack
                // to do it with GDI. See updateFrameMargins() for more information.
                HDC opaqueDc = nullptr;
                BP_PAINTPARAMS params;
                SecureZeroMemory(&params, sizeof(params));
                params.cbSize = sizeof(params);
                params.dwFlags = BPPF_NOCLIP | BPPF_ERASE;
                const HPAINTBUFFER buf = BeginBufferedPaint(hdc, &rcPaint, BPBF_TOPDOWNDIB, &params, &opaqueDc);
                if (!buf) {
                    PRINT_WIN32_ERROR_MESSAGE(BeginBufferedPaint)
                    break;
                }
                if (FillRect(opaqueDc, &rcPaint,
                             reinterpret_cast<HBRUSH>(GetClassLongPtrW(hWnd, GCLP_HBRBACKGROUND))) == 0) {
                    PRINT_WIN32_ERROR_MESSAGE(FillRect)
                    break;
                }
                HRESULT hr = BufferedPaintSetAlpha(buf, nullptr, 255);
                if (FAILED(hr)) {
                    PRINT_HR_ERROR_MESSAGE(BufferedPaintSetAlpha, hr)
                    break;
                }
                hr = EndBufferedPaint(buf, TRUE);
                if (FAILED(hr)) {
                    PRINT_HR_ERROR_MESSAGE(EndBufferedPaint, hr)
                    break;
                }
            }
            if (EndPaint(hWnd, &ps) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(EndPaint)
                break;
            }
        }
        if (g_am_IsUsingDirect2D_p) {
            if (!g_am_WICConverter_p) {
                if (FAILED(am_D2DGenerateWICBitmapSource_p())) {
                    PRINT(L"WM_PAINT: Failed to generate the WICBitmapSource.")
                    break;
                }
            }
            if (!g_am_D2DGaussianBlurEffect_p) {
                if (FAILED(am_D2DCreateEffects_p())) {
                    PRINT(L"WM_PAINT: Failed to create Direct2D effects.")
                    break;
                }
            }
            if (FAILED(am_D2DDrawFinalVisual_p())) {
                PRINT(L"WM_PAINT: Failed to draw the Direct2D visuals.")
                break;
            }
        }
        return 0;
    } break;
    case WM_DPICHANGED: {
        wallpaperChanged = true;
        const auto x = static_cast<double>(LOWORD(wParam));
        const auto y = static_cast<double>(HIWORD(wParam));
        g_am_CurrentDpi_p = std::round((x + y) / 2.0);
        if (FAILED(am_GetWindowDevicePixelRatio_p(hWnd, &g_am_CurrentDpr_p))) {
            PRINT(L"WM_DPICHANGED: Failed to retrieve the device scale factor of the current monitor.")
            break;
        }
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (MoveWindow(hWnd, prcNewWindow->left, prcNewWindow->top,
                   GET_RECT_WIDTH(*prcNewWindow), GET_RECT_HEIGHT(*prcNewWindow), TRUE) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(MoveWindow)
            break;
        }
        return 0;
    } break;
    case WM_SIZE: {
        bool full = false;
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED)
                || (SUCCEEDED(am_IsFullScreened_p(hWnd, &full)) && full)) {
            if (FAILED(am_UpdateFrameMarginsHelper_p(hWnd, g_am_CurrentDpi_p))) {
                PRINT(L"WM_SIZE: Failed to update frame margins.")
                break;
            }
        }
        const auto width = LOWORD(lParam);
        const UINT flags = SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER;
        if (g_am_XAMLIslandWindowHandle_p) {
            // Give enough space to our thin homemade top border.
            int borderThickness = 0;
            if (SUCCEEDED(am_GetWindowVisibleFrameBorderThickness_p(hWnd, g_am_CurrentDpi_p, &borderThickness))) {
                const int height = (HIWORD(lParam) - borderThickness);
                if (SetWindowPos(g_am_XAMLIslandWindowHandle_p, HWND_BOTTOM, 0, borderThickness,
                             width, height, flags) == FALSE) {
                    PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
                    break;
                }
            } else {
                PRINT(L"Failed to retrieve the window visible frame border thickness.")
                break;
            }
        }
        if (g_am_DragBarWindowHandle_p) {
            int tbh = 0;
            if (SUCCEEDED(am_GetTitleBarHeight_p(hWnd, g_am_CurrentDpi_p, &tbh))) {
                if (SetWindowPos(g_am_DragBarWindowHandle_p, HWND_TOP, 0, 0, width, tbh, flags) == FALSE) {
                    PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
                    break;
                }
            } else {
                PRINT(L"Failed to retrieve the title bar height.")
                break;
            }
        }
    } break;
    case WM_SETFOCUS: {
        if (g_am_XAMLIslandWindowHandle_p) {
            // Send focus to the XAML Island child window.
            if (SetFocus(g_am_XAMLIslandWindowHandle_p) == nullptr) {
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
    case WM_NCRBUTTONUP: {
        // The `DefWindowProc` function doesn't open the system menu for some
        // reason so we have to do it ourselves.
        if (wParam == HTCAPTION) {
            if (FAILED(am_OpenSystemMenu_p(hWnd, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)}))) {
                PRINT(L"WM_NCRBUTTONUP: Failed to open the system menu.")
                break;
            }
        }
    } break;
    case WM_SETTINGCHANGE: {
        if (wParam == SPI_SETDESKWALLPAPER) {
            wallpaperChanged = true;
        }
        if ((wParam == 0) && (wcscmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            systemThemeChanged = true;
        }
    } break;
    case WM_THEMECHANGED:
    case WM_DWMCOLORIZATIONCOLORCHANGED:
        systemThemeChanged = true;
        break;
    case WM_DWMCOMPOSITIONCHANGED: {
        bool enabled = false;
        if (FAILED(am_IsCompositionEnabled_p(&enabled)) || !enabled) {
            PRINT(L"AcrylicManager won't be functional when DWM composition is disabled.")
            std::exit(-1);
        }
    } break;
    case WM_CLOSE: {
        SAFE_RELEASE_RESOURCES
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }

    if (g_am_BackgroundBrush_p && (g_am_BrushTheme_p == SystemTheme::Auto) && systemThemeChanged) {
        SystemTheme systemTheme = SystemTheme::Invalid;
        if (SUCCEEDED(am_GetSystemThemeHelper_p(&systemTheme))) {
            if ((systemTheme != SystemTheme::Invalid) && (systemTheme != SystemTheme::HighContrast)) {
                if (SUCCEEDED(am_SwitchAcrylicBrushThemeHelper_p(systemTheme))) {
                    g_am_BrushTheme_p = SystemTheme::Auto;
                } else {
                    PRINT(L"Failed to switch acrylic brush theme.")
                }
            } else {
                PRINT(L"The system theme is not valid or high contrast mode is on.")
            }
        } else {
            PRINT(L"Failed to retrieve the system theme.")
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

[[nodiscard]] static inline LRESULT CALLBACK am_DragBarWindowProcHelper_p(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    std::optional<UINT> nonClientMessage = std::nullopt;

    switch (uMsg)
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

    if (nonClientMessage.has_value() && g_am_MainWindowHandle_p)
    {
        POINT pos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (ClientToScreen(hWnd, &pos) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(ClientToScreen)
            return 0;
        }
        const LPARAM newLParam = MAKELPARAM(pos.x, pos.y);
        // Hit test the parent window at the screen coordinates the user clicked in the drag input sink window,
        // then pass that click through as an NC click in that location.
        const LRESULT hitTestResult = SendMessageW(g_am_MainWindowHandle_p, WM_NCHITTEST, 0, newLParam);
        SendMessageW(g_am_MainWindowHandle_p, nonClientMessage.value(), hitTestResult, newLParam);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

[[nodiscard]] static inline LRESULT CALLBACK am_HookWindowProcHelper_p(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const auto getOriginalResult = [hWnd, uMsg, wParam, lParam]() {
        if (g_am_HostWindowProc_p) {
            return CallWindowProcW(g_am_HostWindowProc_p, hWnd, uMsg, wParam, lParam);
        } else {
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }
    };

    if (!g_am_MainWindowHandle_p) {
        return getOriginalResult();
    }

    switch (uMsg) {
    case WM_WINDOWPOSCHANGED: {
        const auto newPos = reinterpret_cast<LPWINDOWPOS>(lParam);
        // We'll also get the "WM_WINDOWPOSCHANGED" message if the Z order of our window changes.
        // So we have to make sure the background window's Z order is updated in time as well.
        if (SetWindowPos(g_am_MainWindowHandle_p, hWnd,
                         newPos->x, newPos->y, newPos->cx, newPos->cy,
                         SWP_NOACTIVATE | SWP_NOOWNERZORDER) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
        }
    } break;
    case WM_ACTIVATE: {
        // todo
    } break;
    case WM_CLOSE: {
        SendMessageW(g_am_MainWindowHandle_p, WM_CLOSE, 0, 0);
    } break;
    default:
        break;
    }

    return getOriginalResult();
}

[[nodiscard]] static inline HRESULT am_RegisterMainWindowClassHelper_p()
{
    if (g_am_MainWindowAtom_p != 0) {
        PRINT_AND_SAFE_RETURN(L"The main window class has been registered already.")
    }

    LPWSTR guid = nullptr;
    if (FAILED(am_GenerateGUID_p(&guid))) {
        PRINT_AND_SAFE_RETURN(L"Failed to generate a GUID for main window class.")
    }
    g_am_MainWindowClassName_p = new wchar_t[MAX_PATH];
    SecureZeroMemory(g_am_MainWindowClassName_p, sizeof(g_am_MainWindowClassName_p));
    swprintf(g_am_MainWindowClassName_p, L"%s%s%s", g_am_WindowClassNamePrefix_p, guid, g_am_MainWindowClassNameSuffix_p);
    SAFE_FREE_CHARARRAY(guid)

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = am_MainWindowProcHelper_p;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIconW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_DEFAULTICON));
    wcex.hIconSm = LoadIconW(HINST_THISCOMPONENT, MAKEINTRESOURCEW(IDI_DEFAULTICONSM));
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = g_am_MainWindowClassName_p;

    g_am_MainWindowAtom_p = RegisterClassExW(&wcex);

    if (g_am_MainWindowAtom_p == 0) {
        PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN(RegisterClassExW)
    }

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_RegisterDragBarWindowClassHelper_p()
{
    if (!g_am_IsWindows8OrGreater_p) {
        PRINT_AND_SAFE_RETURN(L"Drag bar window is only available on Windows 8 and onwards.")
    }

    if ((g_am_MainWindowAtom_p == 0) || (g_am_DragBarWindowAtom_p != 0)) {
        PRINT_AND_SAFE_RETURN(L"The main window class has not been registered or "
                               "the drag bar window class has been registered already.")
    }

    LPWSTR guid = nullptr;
    if (FAILED(am_GenerateGUID_p(&guid))) {
        PRINT_AND_SAFE_RETURN(L"Failed to generate a GUID for drag bar window class.")
    }
    g_am_DragBarWindowClassName_p = new wchar_t[MAX_PATH];
    SecureZeroMemory(g_am_DragBarWindowClassName_p, sizeof(g_am_DragBarWindowClassName_p));
    swprintf(g_am_DragBarWindowClassName_p, L"%s%s%s", g_am_WindowClassNamePrefix_p, guid, g_am_DragBarWindowClassNameSuffix_p);
    SAFE_FREE_CHARARRAY(guid)

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = am_DragBarWindowProcHelper_p;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = GET_BLACK_BRUSH;
    wcex.lpszClassName = g_am_DragBarWindowClassName_p;

    g_am_DragBarWindowAtom_p = RegisterClassExW(&wcex);

    if (g_am_DragBarWindowAtom_p == 0) {
        PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN(RegisterClassExW)
    }

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_CreateMainWindowHelper_p(const int x, const int y, const int w, const int h)
{
    if ((g_am_MainWindowAtom_p == 0) || g_am_MainWindowHandle_p) {
        PRINT_AND_SAFE_RETURN(L"The main window class has not been registered or "
                               "the main window has been created already.")
    }

    g_am_MainWindowHandle_p = CreateWindowExW(0L,
                                       g_am_MainWindowClassName_p, g_am_MainWindowTitle_p,
                                       WS_OVERLAPPEDWINDOW,
                                       ((x > 0) ? x : CW_USEDEFAULT),
                                       ((y > 0) ? y : CW_USEDEFAULT),
                                       ((w > 0) ? w : CW_USEDEFAULT),
                                       ((h > 0) ? h : CW_USEDEFAULT),
                                       nullptr, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!g_am_MainWindowHandle_p) {
        PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN(CreateWindowExW)
    }

    if (FAILED(am_GetWindowDotsPerInch_p(g_am_MainWindowHandle_p, &g_am_CurrentDpi_p))
            || (g_am_CurrentDpi_p == 0)) {
        g_am_CurrentDpi_p = USER_DEFAULT_SCREEN_DPI;
    }
    if (FAILED(am_GetWindowDevicePixelRatio_p(g_am_MainWindowHandle_p, &g_am_CurrentDpr_p))
            || (g_am_CurrentDpr_p <= 0.0)) {
        g_am_CurrentDpr_p = 1.0;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (FAILED(am_UpdateFrameMarginsHelper_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p))) {
        PRINT_AND_SAFE_RETURN(L"Failed to update main window's frame margins.")
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (FAILED(am_TriggerFrameChange_p(g_am_MainWindowHandle_p))) {
        PRINT_AND_SAFE_RETURN(L"Failed to trigger frame change event for main window.")
    }
    // Ensure our window still has window transitions.
    if (FAILED(am_SetWindowTransitionsEnabled_p(g_am_MainWindowHandle_p, true))) {
        PRINT_AND_SAFE_RETURN(L"Failed to enable window transitions for main window.")
    }

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_CreateDragBarWindowHelper_p()
{
    if (!g_am_MainWindowHandle_p) {
        PRINT_AND_SAFE_RETURN(L"The main window has not been created.")
    }

    // Please refer to the "IMPORTANT NOTE" section below.
    if (!g_am_IsWindows8OrGreater_p) {
        PRINT_AND_SAFE_RETURN(L"Drag bar window is only available on Windows 8 and onwards.")
    }
    if ((g_am_DragBarWindowAtom_p == 0) || g_am_DragBarWindowHandle_p) {
        PRINT_AND_SAFE_RETURN(L"The drag bar window class has not been registered or "
                               "the drag bar window has been created already.")
    }

    // The drag bar window is a child window of the top level window that is put
    // right on top of the drag bar. The XAML island window "steals" our mouse
    // messages which makes it hard to implement a custom drag area. By putting
    // a window on top of it, we prevent it from "stealing" the mouse messages.
    //
    // IMPORTANT NOTE: The WS_EX_LAYERED style is supported for both top-level
    // windows and child windows since Windows 8. Previous Windows versions support
    // WS_EX_LAYERED only for top-level windows.
    g_am_DragBarWindowHandle_p = CreateWindowExW(WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP,
                                          g_am_DragBarWindowClassName_p, g_am_DragBarWindowTitle_p,
                                          WS_CHILD,
                                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                          g_am_MainWindowHandle_p, nullptr, HINST_THISCOMPONENT, nullptr);

    if (!g_am_DragBarWindowHandle_p) {
        PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN(CreateWindowExW)
    }

    // Layered window won't be visible until we call SetLayeredWindowAttributes()
    // or UpdateLayeredWindow().
    if (SetLayeredWindowAttributes(g_am_DragBarWindowHandle_p, RGB(0, 0, 0), 255, LWA_ALPHA) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN(SetLayeredWindowAttributes)
    }

    SIZE size = {0, 0};
    if (FAILED(am_GetWindowClientSize_p(g_am_MainWindowHandle_p, &size))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the client area size of main window.")
    }
    int tbh = 0;
    if (FAILED(am_GetTitleBarHeight_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p, &tbh))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the title bar height.")
    }
    if (SetWindowPos(g_am_DragBarWindowHandle_p, HWND_TOP, 0, 0, size.cx, tbh,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN(SetWindowPos)
    }

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_InitializeAcrylicManagerHelper_p(const int x, const int y, const int w, const int h)
{
    if (g_am_AcrylicManagerInitialized_p) {
        return E_FAIL;
    }
    if (FAILED(am_RegisterMainWindowClassHelper_p())) {
        PRINT_AND_RETURN(L"Failed to register main window class.")
    }
    if (FAILED(am_CreateMainWindowHelper_p(x, y, w, h))) {
        PRINT_AND_RETURN(L"Failed to create main window.")
    }
    const auto official = []() -> HRESULT {
        // todo
        g_am_IsUsingOfficialBlur_p = false;
        return E_FAIL;
    };
    const auto xamlIsland = []() -> HRESULT {
        if (SUCCEEDED(am_CreateXAMLIslandHelper_p())) {
            if (SUCCEEDED(am_RegisterDragBarWindowClassHelper_p())) {
                if (SUCCEEDED(am_CreateDragBarWindowHelper_p())) {
                    g_am_IsUsingXAMLIsland_p = true;
                    g_am_AcrylicManagerInitialized_p = true;
                    return S_OK;
                } else {
                    PRINT_AND_RETURN(L"Failed to create drag bar window.")
                }
            } else {
                PRINT_AND_RETURN(L"Failed to register drag bar window class.")
            }
        } else {
            PRINT_AND_RETURN(L"Failed to create XAML Island.")
        }
    };
    const auto direct2d = []() -> HRESULT {
        if (SUCCEEDED(am_InitializeDirect2DInfrastructureHelper_p())) {
            g_am_IsUsingDirect2D_p = true;
            g_am_AcrylicManagerInitialized_p = true;
            return S_OK;
        } else {
            PRINT_AND_RETURN(L"Failed to initialize the Direct2D infrastructure.")
        }
    };
    if (g_am_IsForceOfficialBlur_p) {
        return official();
    }
    if (g_am_IsForceXAMLIsland_p) {
        return xamlIsland();
    }
    if (g_am_IsForceDirect2D_p) {
        return direct2d();
    }
    if (g_am_IsOfficialBlurAvailable_p) {
        return official();
    }
    if (g_am_IsXAMLIslandAvailable_p) {
        return xamlIsland();
    }
    if (g_am_IsDirect2DAvailable_p) {
        return direct2d();
    }
    // Just don't crash.
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_InstallHostWindowHookHelper_p(const HWND hWnd, const bool enable)
{
    if (!g_am_MainWindowHandle_p || !hWnd || g_am_HostWindowHandle_p || g_am_HostWindowProc_p) {
        return E_INVALIDARG;
    }
    const auto exStyle = static_cast<DWORD>(GetWindowLongPtrW(g_am_MainWindowHandle_p, GWL_EXSTYLE));
    if (enable) {
        const auto hostWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hWnd, GWLP_WNDPROC));
        if (!hostWndProc) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetWindowLongPtrW)
        }
        if (SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(am_HookWindowProcHelper_p)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
        }
        if (SetWindowLongPtrW(g_am_MainWindowHandle_p, GWL_EXSTYLE, static_cast<LONG_PTR>(exStyle | WS_EX_NOACTIVATE)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
        }
        g_am_HostWindowHandle_p = hWnd;
        g_am_HostWindowProc_p = hostWndProc;
    } else {
        if (SetWindowLongPtrW(g_am_MainWindowHandle_p, GWL_EXSTYLE, static_cast<LONG_PTR>(exStyle & ~WS_EX_NOACTIVATE)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
        }
        if (SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_am_HostWindowProc_p)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
        }
        g_am_HostWindowProc_p = nullptr;
        g_am_HostWindowHandle_p = nullptr;
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetLibraryVersion_p(LPWSTR *ver)
{
    if (!ver) {
        return E_INVALIDARG;
    }
    const auto str = new wchar_t[20]; // 20 should be enough for a version string...
    SecureZeroMemory(str, sizeof(str));
    wcscpy(str, ACRYLICMANAGER_VERSION_STR);
    *ver = str;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_CreateAcrylicWindow_p(const int x, const int y, const int w, const int h)
{
    if (!g_am_IsWindows7OrGreater_p) {
        PRINT_AND_RETURN(L"AcrylicManager won't be functional on such old systems.")
    }

    bool dwmComp = false;
    if (FAILED(am_IsCompositionEnabled_p(&dwmComp)) || !dwmComp) {
        PRINT_AND_RETURN(L"AcrylicManager won't be functional when DWM composition is disabled.")
    }

    return am_InitializeAcrylicManagerHelper_p(x, y, w, h);
}

[[nodiscard]] static inline HRESULT am_LocalFreeA_p(LPSTR str)
{
    if (!str) {
        return E_INVALIDARG;
    }
    SAFE_FREE_CHARARRAY(str)
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_LocalFreeW_p(LPWSTR str)
{
    if (!str) {
        return E_INVALIDARG;
    }
    SAFE_FREE_CHARARRAY(str)
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetHostWindow_p(HWND *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    *result = g_am_HostWindowHandle_p;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_RemoveHostWindow_p()
{
    if (!g_am_HostWindowHandle_p || !g_am_HostWindowProc_p || !g_am_MainWindowHandle_p) {
        return E_INVALIDARG;
    }
    return am_InstallHostWindowHookHelper_p(g_am_HostWindowHandle_p, false);
}

/////////////////////////////////
/////     Private interface
/////////////////////////////////

HRESULT am_GetScreenGeometry_p(const HWND hWnd, RECT *rect)
{
    if (!hWnd || !rect) {
        return E_INVALIDARG;
    }
    MONITORINFO mi = {};
    if (FAILED(am_GetMonitorInfoHelper_p(hWnd, &mi))) {
        return E_FAIL;
    }
    *rect = mi.rcMonitor;
    return S_OK;
}

HRESULT am_GetScreenAvailableGeometry_p(const HWND hWnd, RECT *rect)
{
    if (!hWnd || !rect) {
        return E_INVALIDARG;
    }
    MONITORINFO mi = {};
    if (FAILED(am_GetMonitorInfoHelper_p(hWnd, &mi))) {
        return E_FAIL;
    }
    *rect = mi.rcWork;
    return S_OK;
}

HRESULT am_IsCompositionEnabled_p(bool *result)
{
    if (!result) {
        return E_INVALIDARG;
    }
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (g_am_IsWindows8OrGreater_p) {
        *result = true;
        return S_OK;
    }
    BOOL enabled = FALSE;
    const HRESULT hr = DwmIsCompositionEnabled(&enabled);
    if (SUCCEEDED(hr)) {
        *result = (enabled != FALSE);
        return S_OK;
    } else {
        PRINT_HR_ERROR_MESSAGE(DwmIsCompositionEnabled, hr)
    }
    DWORD dwmComp = 0;
    if (SUCCEEDED(am_GetDWORDFromRegistryHelper_p(HKEY_CURRENT_USER, g_am_DWMRegistryKey_p, L"Composition", &dwmComp))) {
        *result = (dwmComp != 0);
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_IsWindowVisible_p(const HWND hWnd, bool *visible)
{
    if (!hWnd || !visible) {
        return E_INVALIDARG;
    }
    *visible = (IsWindowVisible(hWnd) != FALSE);
    return S_OK;
}

HRESULT am_TriggerFrameChange_p(const HWND hWnd)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    if (SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
              SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowPos)
    }
    return S_OK;
}

HRESULT am_OpenSystemMenu_p(const HWND hWnd, const POINT pos)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    const HMENU menu = GetSystemMenu(hWnd, FALSE);
    if (!menu) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetSystemMenu)
    }
    // Update the options based on window state.
    MENUITEMINFOW mii;
    SecureZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fType = MFT_STRING;
    const auto setState = [&mii, menu](const UINT item, const bool enabled) -> bool {
        mii.fState = enabled ? MF_ENABLED : MF_DISABLED;
        return (SetMenuItemInfoW(menu, item, FALSE, &mii) != FALSE);
    };
    bool isMaximized = false;
    if (FAILED(am_IsMaximized_p(hWnd, &isMaximized))) {
        return E_FAIL;
    }
    if (!setState(SC_RESTORE, isMaximized)) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetMenuItemInfoW)
    }
    if (!setState(SC_MOVE, !isMaximized)) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetMenuItemInfoW)
    }
    if (!setState(SC_SIZE, !isMaximized)) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetMenuItemInfoW)
    }
    if (!setState(SC_MINIMIZE, true)) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetMenuItemInfoW)
    }
    if (!setState(SC_MAXIMIZE, !isMaximized)) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetMenuItemInfoW)
    }
    if (!setState(SC_CLOSE, true)) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetMenuItemInfoW)
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetMenuDefaultItem)
    }
    // ### TODO: support LTR layout.
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, pos.x, pos.y, 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(PostMessageW)
        }
    }
    return S_OK;
}

HRESULT am_SetWindowCompositionAttribute_p(const HWND hWnd, LPWINDOWCOMPOSITIONATTRIBDATA pwcad)
{
    if (!hWnd || !pwcad) {
        return E_INVALIDARG;
    }
    static bool tried = false;
    using sig = BOOL(WINAPI *)(HWND, LPWINDOWCOMPOSITIONATTRIBDATA);
    static sig func = nullptr;
    if (!func) {
        if (tried) {
            return E_FAIL;
        } else {
            tried = true;
            FARPROC address = nullptr;
            if (FAILED(am_GetSymbolAddressFromExecutable_p(L"User32.dll", L"SetWindowCompositionAttribute", true, &address))) {
                return E_FAIL;
            }
            func = reinterpret_cast<sig>(address);
            if (!func) {
                return E_FAIL;
            }
        }
    }
    return ((func(hWnd, pwcad) != FALSE) ? S_OK : E_FAIL);
}

HRESULT am_WideToMulti_p(LPCWSTR in, const UINT codePage, LPSTR *out)
{
    if (!in || !out) {
        return E_INVALIDARG;
    }
    const int required = WideCharToMultiByte(codePage, 0, in, -1, nullptr, 0, nullptr, nullptr);
    const auto result = new char[required];
    WideCharToMultiByte(codePage, 0, in, -1, result, required, nullptr, nullptr);
    *out = result;
    return S_OK;
}

HRESULT am_MultiToWide_p(LPCSTR in, const UINT codePage, LPWSTR *out)
{
    if (!in || !out) {
        return E_INVALIDARG;
    }
    const int required = MultiByteToWideChar(codePage, 0, in, -1, nullptr, 0);
    const auto result = new wchar_t[required];
    MultiByteToWideChar(codePage, 0, in, -1, result, required);
    *out = result;
    return S_OK;
}

HRESULT am_GetStringFromEnvironmentVariable_p(LPCWSTR name, LPWSTR *value)
{
    if (!name || !value) {
        return E_INVALIDARG;
    }
    return am_GetStringFromEnvironmentVariableHelper_p(name, value);
}

HRESULT am_GetIntFromEnvironmentVariable_p(LPCWSTR name, int *value)
{
    if (!name || !value) {
        return E_INVALIDARG;
    }
    LPWSTR str = nullptr;
    if (SUCCEEDED(am_GetStringFromEnvironmentVariableHelper_p(name, &str))) {
        *value = _wtoi(str);
        SAFE_FREE_CHARARRAY(str)
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_GetBoolFromEnvironmentVariable_p(LPCWSTR name, bool *value)
{
    if (!name || !value) {
        return E_INVALIDARG;
    }
    LPWSTR str = nullptr;
    if (SUCCEEDED(am_GetStringFromEnvironmentVariableHelper_p(name, &str))) {
        *value = ((_wcsicmp(str, L"True") == 0) || (_wcsicmp(str, L"Yes") == 0)
                  || (_wcsicmp(str, L"Enable") == 0) || (_wcsicmp(str, L"Enabled") == 0)
                  || (_wcsicmp(str, L"On") == 0) || (_wcsicmp(str, L"0") != 0));
        SAFE_FREE_CHARARRAY(str)
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_GetIntFromRegistry_p(const HKEY rootKey, LPCWSTR subKey, LPCWSTR valueName, int *value)
{
    if (!rootKey || !subKey || !valueName || !value) {
        return E_INVALIDARG;
    }
    DWORD dwValue = 0;
    if (SUCCEEDED(am_GetDWORDFromRegistryHelper_p(rootKey, subKey, valueName, &dwValue))) {
        const auto result = static_cast<int>(dwValue);
        *value = result;
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_GetStringFromRegistry_p(const HKEY rootKey, LPCWSTR subKey, LPCWSTR valueName, LPWSTR *value)
{
    if (!rootKey || !subKey || !valueName || !value) {
        return E_INVALIDARG;
    }
    LPWSTR str = nullptr;
    if (SUCCEEDED(am_GetStringFromRegistryHelper_p(rootKey, subKey, valueName, &str))) {
        *value = str;
        return S_OK;
    }
    return E_FAIL;
}

HRESULT am_IsWindowBackgroundTranslucent_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    BOOL enabled = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::USE_HOSTBACKDROPBRUSH), &enabled, sizeof(enabled)))) {
        *result = (enabled != FALSE);
        return S_OK;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    // todo: GetWindowCompositionAttribute()
    const DWORD exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    *result = (exStyle & WS_EX_LAYERED);
    return S_OK;
}

HRESULT am_SetWindowTranslucentBackgroundEnabled_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    const BOOL enabled = enable ? TRUE : FALSE;
    if (SUCCEEDED(DwmSetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::USE_HOSTBACKDROPBRUSH), &enabled, sizeof(enabled)))) {
        return S_OK;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so changing it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    ACCENT_POLICY policy;
    SecureZeroMemory(&policy, sizeof(policy));
    policy.AccentState = enable ? ACCENT_ENABLE_HOSTBACKDROP : ACCENT_DISABLED;
    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &policy;
    data.cbData = sizeof(policy);
    if (SUCCEEDED(am_SetWindowCompositionAttribute_p(hWnd, &data))) {
        return S_OK;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so changing it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
        policy.AccentState = enable ? ACCENT_INVALID_STATE : ACCENT_DISABLED;
        if (SUCCEEDED(am_SetWindowCompositionAttribute_p(hWnd, &data))) {
            return S_OK;
        } else {
            // We just eat this error because this enum value was introduced in a very
            // late Windows 10 version, so changing it's value will always result in
            // a "parameter error" (code: 87) on systems before that value was introduced.
        }
    }
    const DWORD exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    const COLORREF colorKey = RGB(0, 0, 0);
    const BYTE alpha = enable ? 0 : 255;
    if (!enable) {
        if (!(exStyle & WS_EX_LAYERED)) {
            PRINT_AND_RETURN(L"Can't change alpha channel for non-layered window.")
        }
        if (SetLayeredWindowAttributes(hWnd, colorKey, alpha, LWA_COLORKEY) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetLayeredWindowAttributes)
        }
    }
    if (SetWindowLongPtrW(hWnd, GWL_EXSTYLE, (enable ? (exStyle | WS_EX_LAYERED) : (exStyle & ~WS_EX_LAYERED))) == 0) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
    }
    if (enable) {
        if (SetLayeredWindowAttributes(hWnd, colorKey, alpha, LWA_COLORKEY) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetLayeredWindowAttributes)
        }
    }
    return S_OK;
}

HRESULT am_SetOfficialBlurEnabled_p(const HWND hWnd, const int r, const int g, const int b, const int a)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    // todo
    return E_FAIL;
}

HRESULT am_IsOfficialBlurEnabled_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    // todo
    return E_FAIL;
}

HRESULT am_GetWindowClientSize_p(const HWND hWnd, SIZE *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    RECT rect = {0, 0, 0, 0};
    if (GetClientRect(hWnd, &rect) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetClientRect)
    }
    *result = GET_RECT_SIZE(rect);
    return S_OK;
}

HRESULT am_GetEffectiveTintColor_p(
        int tintColorR, int tintColorG, int tintColorB, int tintColorA,
        double tintOpacity, double *tintLuminosityOpacity,
        int *r, int *g, int *b, int *a)
{
    if (!r || !g || !b || !a) {
        return E_INVALIDARG;
    }

    winrt::Windows::UI::Color tintColor = MAKE_WINRTCOLOR_FROM_COMPONENTS(tintColorR, tintColorG, tintColorB, tintColorA);

    // Update tintColor's alpha with the combined opacity value
    // If LuminosityOpacity was specified, we don't intervene into users parameters
    if (tintLuminosityOpacity) {
        tintColor.A = static_cast<uint8_t>(std::round(static_cast<double>(tintColor.A) * tintOpacity));
    } else {
        const double tintOpacityModifier = am_GetTintOpacityModifier_p(tintColor);
        tintColor.A = static_cast<uint8_t>(std::round(static_cast<double>(tintColor.A) * tintOpacity * tintOpacityModifier));
    }

    GET_WINRTCOLOR_COMPONENTS(tintColor, *r, *g, *b, *a)

    return S_OK;
}

HRESULT am_GetEffectiveLuminosityColor_p(
        int tintColorR, int tintColorG, int tintColorB, int tintColorA,
        double tintOpacity, double *tintLuminosityOpacity,
        int *r, int *g, int *b, int *a)
{
    if (!r || !g || !b || !a) {
        return E_INVALIDARG;
    }

    winrt::Windows::UI::Color tintColor = MAKE_WINRTCOLOR_FROM_COMPONENTS(tintColorR, tintColorG, tintColorB, tintColorA);

    // Purposely leaving out tint opacity modifier here because GetLuminosityColor needs the *original* tint opacity set by the user.
    tintColor.A = static_cast<uint8_t>(std::round(static_cast<double>(tintColor.A) * std::clamp(tintOpacity, 0.0, 1.0)));

    winrt::Windows::Foundation::IReference<double> luminosityOpacity = nullptr;
    if (tintLuminosityOpacity) {
        luminosityOpacity = std::clamp(*tintLuminosityOpacity, 0.0, 1.0);
    }

    const winrt::Windows::UI::Color finalColor = am_GetLuminosityColor_p(tintColor, luminosityOpacity);

    GET_WINRTCOLOR_COMPONENTS(finalColor, *r, *g, *b, *a)

    return S_OK;
}

HRESULT am_GetBoolFromIniFile_p(LPCWSTR ini, LPCWSTR section, LPCWSTR key, bool *value)
{
    if (!ini || !section || !key || !value) {
        return E_INVALIDARG;
    }
    auto buf = new wchar_t[MAX_PATH];
    SecureZeroMemory(buf, sizeof(buf));
    if (GetPrivateProfileStringW(section, key, nullptr, buf, MAX_PATH, ini) == 0) {
        SAFE_FREE_CHARARRAY(buf)
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetPrivateProfileStringW)
    }
    *value = ((_wcsicmp(buf, L"True") == 0) || (_wcsicmp(buf, L"Yes") == 0)
              || (_wcsicmp(buf, L"Enable") == 0) || (_wcsicmp(buf, L"Enabled") == 0)
              || (_wcsicmp(buf, L"On") == 0) || (_wcsicmp(buf, L"0") != 0));
    SAFE_FREE_CHARARRAY(buf)
    return S_OK;
}

HRESULT am_GetSymbolAddressFromExecutable_p(LPCWSTR path, LPCWSTR function, const bool system, FARPROC *result)
{
    if (!path || !function || !result) {
        return E_INVALIDARG;
    }
    HMODULE module = nullptr;
    const auto search = g_am_LoadedModuleList_p.find(path);
    if (search == g_am_LoadedModuleList_p.end()) {
        const DWORD flag = (system ? LOAD_LIBRARY_SEARCH_SYSTEM32 : LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        module = LoadLibraryExW(path, nullptr, flag);
#else
        static bool tried = false;
        using sig = decltype(&::LoadLibraryExW);
        static sig func = nullptr;
        if (!func) {
            if (tried) {
                return E_FAIL;
            } else {
                tried = true;
                MEMORY_BASIC_INFORMATION mbi;
                SecureZeroMemory(&mbi, sizeof(mbi));
                if (VirtualQuery(reinterpret_cast<LPCVOID>(&VirtualQuery), &mbi, sizeof(mbi)) == 0) {
                    PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(VirtualQuery)
                }
                const auto kernel32 = static_cast<HMODULE>(mbi.AllocationBase);
                if (!kernel32) {
                    PRINT_AND_RETURN(L"Failed to retrieve the Kernel32.dll's base address.")
                }
                func = reinterpret_cast<sig>(GetProcAddress(kernel32, "LoadLibraryExW"));
                if (!func) {
                    PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetProcAddress)
                }
            }
        }
        module = func(path, nullptr, flag);
#endif
        if (module) {
            g_am_LoadedModuleList_p.insert({path, module});
        } else {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(LoadLibraryExW)
        }
    } else {
        module = search->second;
        if (!module) {
            PRINT_AND_RETURN(L"The cached module handle is null.")
        }
    }
    LPSTR functionNameAnsi = nullptr;
    if (FAILED(am_WideToMulti_p(function, CP_UTF8, &functionNameAnsi))) {
        PRINT_AND_RETURN(L"Failed to convert a wide string to multibyte string.")
    }
    const FARPROC address = GetProcAddress(module, functionNameAnsi);
    SAFE_FREE_CHARARRAY(functionNameAnsi)
    if (!address) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetProcAddress)
    }
    *result = address;
    return S_OK;
}

HRESULT am_ReleaseAllLoadedModules_p()
{
    if (g_am_LoadedModuleList_p.empty()) {
        return S_OK;
    }
    for (auto &&module : std::as_const(g_am_LoadedModuleList_p)) {
        const HMODULE handle = module.second;
        if (!handle) {
            PRINT(L"The cached module handle is null.")
            continue;
        }
        if (FreeLibrary(handle) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(FreeLibrary)
        }
    }
    g_am_LoadedModuleList_p.clear();
    return S_OK;
}

/////////////////////////////////
/////     Public interface
/////////////////////////////////

HRESULT am_GetVersion(LPWSTR *ver)
{
    return am_GetLibraryVersion_p(ver);
}

HRESULT am_CreateWindow(const int x, const int y, const int w, const int h)
{
    return am_CreateAcrylicWindow_p(x, y, w, h);
}

HRESULT am_GetWindowGeometry(RECT *result)
{
    return am_GetWindowGeometryHelper_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_SetWindowGeometry(const int x, const int y, const int w, const int h)
{
    return am_SetWindowGeometryHelper_p(g_am_MainWindowHandle_p, x, y, w, h);
}

HRESULT am_MoveWindow(const int x, const int y)
{
    return am_MoveWindowHelper_p(g_am_MainWindowHandle_p, x, y);
}

HRESULT am_GetWindowSize(SIZE *result)
{
    return am_GetWindowSizeHelper_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_ResizeWindow(const int w, const int h)
{
    return am_ResizeWindowHelper_p(g_am_MainWindowHandle_p, w, h);
}

HRESULT am_CenterWindow()
{
    return am_CenterWindowHelper_p(g_am_MainWindowHandle_p);
}

HRESULT am_GetWindowState(WindowState *result)
{
    return am_GetWindowStateHelper_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_SetWindowState(const WindowState state)
{
    return am_SetWindowStateHelper_p(g_am_MainWindowHandle_p, state);
}

HRESULT am_CloseWindow()
{
    return am_Release();
}

HRESULT am_GetWindowHandle(HWND *result)
{
    return am_GetWindowHandleHelper_p(result);
}

HRESULT am_GetBrushTheme(SystemTheme *result)
{
    return am_GetBrushThemeHelper_p(result);
}

HRESULT am_SetBrushTheme(const SystemTheme theme)
{
    return am_SwitchAcrylicBrushThemeHelper_p(theme);
}

HRESULT am_GetTintColor(int *r, int *g, int *b, int *a)
{
    return am_GetTintColorHelper_p(r, g, b, a);
}

HRESULT am_SetTintColor(const int r, const int g, const int b, const int a)
{
    return am_SetTintColorHelper_p(r, g, b, a);
}

HRESULT am_GetTintOpacity(double *opacity)
{
    return am_GetTintOpacityHelper_p(opacity);
}

HRESULT am_SetTintOpacity(const double opacity)
{
    return am_SetTintOpacityHelper_p(opacity);
}

HRESULT am_GetTintLuminosityOpacity(double *opacity)
{
    return am_GetTintLuminosityOpacityHelper_p(opacity);
}

HRESULT am_SetTintLuminosityOpacity(const double *opacity)
{
    return am_SetTintLuminosityOpacityHelper_p(opacity);
}

HRESULT am_GetFallbackColor(int *r, int *g, int *b, int *a)
{
    return am_GetFallbackColorHelper_p(r, g, b, a);
}

HRESULT am_SetFallbackColor(const int r, const int g, const int b, const int a)
{
    return am_SetFallbackColorHelper_p(r, g, b, a);
}

HRESULT am_EventLoopExec(int *result)
{
    return am_MainWindowEventLoopHelper_p(result);
}

HRESULT am_IsWindowActive(bool *result)
{
    return am_IsWindowActiveHelper_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_FreeStringA(LPSTR str)
{
    return am_LocalFreeA_p(str);
}

HRESULT am_FreeStringW(LPWSTR str)
{
    return am_LocalFreeW_p(str);
}

HRESULT am_Release()
{
    return am_CleanupHelper_p();
}

HRESULT am_SetHostWindow(const HWND hWnd)
{
    return am_InstallHostWindowHookHelper_p(hWnd, true);
}

HRESULT am_GetHostWindow(HWND *result)
{
    return am_GetHostWindow_p(result);
}

HRESULT am_RemoveHostWindow()
{
    return am_RemoveHostWindow_p();
}

/////////////////////////////////
/////     DLL entry point
/////////////////////////////////

#ifndef ACRYLICMANAGER_STATIC

[[nodiscard]] ACRYLICMANAGER_API BOOL APIENTRY
DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        am_CleanupHelper_p();
        break;
    }
    return TRUE;
}

#endif
