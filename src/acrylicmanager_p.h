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

#pragma once

#include "acrylicmanager_global.h"

enum class WindowsVersion : int
{
    WindowsVista = 0,
    Windows7,
    Windows8,
    Windows8_1,
    Windows10_1507,
    Windows10_1511,
    Windows10_1607,
    Windows10_1703,
    Windows10_1709,
    Windows10_1803,
    Windows10_1809,
    Windows10_1903,
    Windows10_1909,
    Windows10_2004,
    Windows10_20H2,
    Windows10_21H1,
    Windows11,
    Windows10 = Windows10_1507,
    Windows10_TH1 = Windows10_1507,
    Windows10_TH2 = Windows10_1511,
    Windows10_RS1 = Windows10_1607,
    Windows10_RS2 = Windows10_1703,
    Windows10_RS3 = Windows10_1709,
    Windows10_RS4 = Windows10_1803,
    Windows10_RS5 = Windows10_1809,
    Windows10_19H1 = Windows10_1903,
    Windows10_19H2 = Windows10_1909,
    Windows10_20H1 = Windows10_2004
    //Windows10_20H2 = Windows10_20H2,
    //Windows10_21H1 = Windows10_21H1,
};

enum class VersionCompare : int
{
    Less = 0,
    Equal,
    Greater,
    LessOrEqual,
    GreaterOrEqual
};

enum class ColorizationArea : int
{
    None = 0,
    StartMenu_TaskBar_ActionCenter,
    TitleBar_WindowBorder,
    All
};

enum class WallpaperAspectStyle : int
{
    Invalid = -1,
    Central,
    Tiled,
    IgnoreRatioFit, // Stretch
    KeepRatioFit, // Fit
    KeepRatioByExpanding, // Fill
    Span
};

enum class DpiAwareness : int
{
    Invalid = -1,
    Unaware,
    System,
    PerMonitor,
    PerMonitorV2
};

enum class DwmWindowAttribute : int
{
    TRANSITIONS_FORCEDISABLED = 3,            // [get/set] BOOL, Enable or disable window transitions.
    CAPTION_BUTTON_BOUNDS = 5,                // [get] RECT, Bounds of the caption (titlebar) button area rectangle in window-relative space.
    EXTENDED_FRAME_BOUNDS = 9,                // [get] RECT, Bounds of the extended frame rectangle in screen space.
    EXCLUDED_FROM_PEEK = 12,                  // [get/set] BOOL, Exclude or include window from LivePreview.
    USE_HOSTBACKDROPBRUSH = 17,               // [get/set] BOOL, Allows the use of host backdrop brushes for the window.
    USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19, // [get/set] BOOL, Allows a window to either use the accent color, or dark, according to the user Color Mode preferences.
    USE_IMMERSIVE_DARK_MODE = 20,             // [get/set] BOOL, The same as "DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1".
    WINDOW_CORNER_PREFERENCE = 33,            // [get/set] WINDOW_CORNER_PREFERENCE, Controls the policy that rounds top-level window corners.
    BORDER_COLOR = 34,                        // [get/set] COLORREF, Color of the thin border around a top-level window.
    CAPTION_COLOR = 35,                       // [get/set] COLORREF, Color of the caption (titlebar).
    TEXT_COLOR = 36,                          // [get/set] COLORREF, Color of the caption (titlebar) text.
    VISIBLE_FRAME_BORDER_THICKNESS = 37       // [get] UINT, Width of the visible border around a thick frame window.
};

typedef enum _WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0x0,
    WCA_NCRENDERING_ENABLED = 0x1,
    WCA_NCRENDERING_POLICY = 0x2,
    WCA_TRANSITIONS_FORCEDISABLED = 0x3,
    WCA_ALLOW_NCPAINT = 0x4,
    WCA_CAPTION_BUTTON_BOUNDS = 0x5,
    WCA_NONCLIENT_RTL_LAYOUT = 0x6,
    WCA_FORCE_ICONIC_REPRESENTATION = 0x7,
    WCA_EXTENDED_FRAME_BOUNDS = 0x8,
    WCA_HAS_ICONIC_BITMAP = 0x9,
    WCA_THEME_ATTRIBUTES = 0xA,
    WCA_NCRENDERING_EXILED = 0xB,
    WCA_NCADORNMENTINFO = 0xC,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 0xD,
    WCA_VIDEO_OVERLAY_ACTIVE = 0xE,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 0xF,
    WCA_DISALLOW_PEEK = 0x10,
    WCA_CLOAK = 0x11,
    WCA_CLOAKED = 0x12,
    WCA_ACCENT_POLICY = 0x13,
    WCA_FREEZE_REPRESENTATION = 0x14,
    WCA_EVER_UNCLOAKED = 0x15,
    WCA_VISUAL_OWNER = 0x16,
    WCA_HOLOGRAPHIC = 0x17,
    WCA_EXCLUDED_FROM_DDA = 0x18,
    WCA_PASSIVEUPDATEMODE = 0x19,
    WCA_LAST = 0x1A
} WINDOWCOMPOSITIONATTRIB;

typedef struct ACRYLICMANAGER_API _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    LPVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA, *PWINDOWCOMPOSITIONATTRIBDATA, *LPWINDOWCOMPOSITIONATTRIBDATA;

typedef enum _ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4, // RS4 1803
    ACCENT_ENABLE_HOSTBACKDROP = 5, // RS5 1809
    ACCENT_INVALID_STATE = 6
} ACCENT_STATE;

typedef struct ACRYLICMANAGER_API _ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    COLORREF GradientColor;
    DWORD AnimationId;
} ACCENT_POLICY, *PACCENT_POLICY, *LPACCENT_POLICY;

#ifdef __cplusplus
EXTERN_C_START
#endif

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetWindowDpi_p(
    _In_ const HWND hWnd,
    _Out_ UINT      *dpi
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsMinimized_p(
    _In_ const HWND hWnd,
    _Out_ bool      *min
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsMaximized_p(
    _In_ const HWND hWnd,
    _Out_ bool      *max
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsFullScreened_p(
    _In_ const HWND hWnd,
    _Out_ bool      *full
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsWindowNoState_p(
    _In_ const HWND hWnd,
    _Out_ bool      *normal
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsWindowVisible_p(
    _In_ const HWND hWnd,
    _Out_ bool      *visible
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetResizeBorderThickness_p(
    _In_ const bool x,
    _In_ const UINT dpi,
    _Out_ int       *thickness
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetCaptionHeight_p(
    _In_ const UINT dpi,
    _Out_ int       *height
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetTitleBarHeight_p(
    _In_ const HWND hWnd,
    _In_ const UINT dpi,
    _Out_ int       *height
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetScreenGeometry_p(
    _In_ const HWND hWnd,
    _Out_ RECT      *rect
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetScreenAvailableGeometry_p(
    _In_ const HWND hWnd,
    _Out_ RECT      *rect
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsCompositionEnabled_p(
    _Out_ bool *enabled
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_TriggerFrameChange_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsWindowTransitionsEnabled_p(
    _In_ const HWND hWnd,
    _Out_ bool      *enabled
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_SetWindowTransitionsEnabled_p(
    _In_ const HWND hWnd,
    _In_ const bool enable
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_OpenSystemMenu_p(
    _In_ const HWND  hWnd,
    _In_ const POINT pos
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_CompareSystemVersion_p(
    _In_ const WindowsVersion ver,
    _In_ const VersionCompare comp,
    _Out_ bool                *result
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GenerateGUID_p(
    _Out_ LPWSTR *guid
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_PrintErrorMessageFromHResult_p(
    _In_ LPCWSTR       function,
    _In_ const HRESULT hr
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_ShouldAppsUseLightTheme_p(
    _Out_ bool *result
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_ShouldSystemUsesLightTheme_p(
    _Out_ bool *result
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsHighContrastModeOn_p(
    _Out_ bool *enabled
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_SetWindowCompositionAttribute_p(
    _In_ const HWND                    hWnd,
    _In_ LPWINDOWCOMPOSITIONATTRIBDATA pwcad
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsWindowUsingDarkFrame_p(
    _In_ const HWND hWnd,
    _Out_ bool      *result
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_SetWindowDarkFrameEnabled_p(
    _In_ const HWND hWnd,
    _In_ const bool enable
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetColorizationColor_p(
    _Out_ COLORREF *color
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetColorizationArea_p(
    _Out_ ColorizationArea *area
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetWindowVisibleFrameBorderThickness_p(
    _In_ const HWND hWnd,
    _In_ const UINT dpi,
    _Out_ int       *thickness
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetWallpaperFilePath_p(
    _In_ const int screen,
    _Out_ LPWSTR   *path
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetDesktopBackgroundColor_p(
    _Out_ COLORREF *color
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetWallpaperAspectStyle_p(
    _In_ const int             screen,
    _Out_ WallpaperAspectStyle *style
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetWindowDpiAwareness_p(
    _In_ const HWND    hWnd,
    _Out_ DpiAwareness *result
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_SetWindowDpiAwareness_p(
    _In_ const HWND         hWnd,
    _In_ const DpiAwareness awareness
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_WideToMulti_p(
    _In_ LPCWSTR    in,
    _In_ const UINT codePage,
    _Out_ LPSTR     *out
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_MultiToWide_p(
    _In_ LPCSTR     in,
    _In_ const UINT codePage,
    _Out_ LPWSTR    *out
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetStringFromEnvironmentVariable_p(
    _In_ LPCWSTR name,
    _Out_ LPWSTR *value
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetIntFromEnvironmentVariable_p(
    _In_ LPCWSTR name,
    _Out_ int    *value
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetBoolFromEnvironmentVariable_p(
    _In_ LPCWSTR name,
    _Out_ bool   *value
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetIntFromRegistry_p(
    _In_ const HKEY rootKey,
    _In_ LPCWSTR    subKey,
    _In_ LPCWSTR    valueName,
    _Out_ int       *value
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetStringFromRegistry_p(
    _In_ const HKEY rootKey,
    _In_ LPCWSTR    subKey,
    _In_ LPCWSTR    valueName,
    _Out_ LPWSTR    *value
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_IsWindowBackgroundTranslucent_p(
    _In_ const HWND hWnd,
    _Out_ bool      *result
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_SetWindowTranslucentBackgroundEnabled_p(
    _In_ const HWND hWnd,
    _In_ const bool enable
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetSystemSymbolAddress_p(
    _In_ LPCWSTR  library,
    _In_ LPCWSTR  function,
    _Out_ FARPROC *address
);

#ifdef __cplusplus
EXTERN_C_END
#endif
