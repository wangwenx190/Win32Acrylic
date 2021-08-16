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
    WindowFrame,
    TitleBar,
    StartMenuAndTaskBar
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
    DWORD cbData;
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

[[nodiscard]] ACRYLICMANAGER_API UINT WINAPI
am_GetWindowDpi_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsMinimized_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsMaximized_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsFullScreened_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsWindowNoState_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsWindowVisible_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API int WINAPI
am_GetResizeBorderThickness_p(
    _In_ const bool x,
    _In_ const UINT dpi
);

[[nodiscard]] ACRYLICMANAGER_API int WINAPI
am_GetCaptionHeight_p(
    _In_ const UINT dpi
);

[[nodiscard]] ACRYLICMANAGER_API int WINAPI
am_GetTitleBarHeight_p(
    _In_ const HWND hWnd,
    _In_ const UINT dpi
);

[[nodiscard]] ACRYLICMANAGER_API RECT WINAPI
am_GetScreenGeometry_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API RECT WINAPI
am_GetScreenAvailableGeometry_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsCompositionEnabled_p();

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_TriggerFrameChange_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsWindowTransitionsEnabled_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetWindowTransitionsEnabled_p(
    _In_ const HWND hWnd,
    _In_ const bool enable
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_OpenSystemMenu_p(
    _In_ const HWND  hWnd,
    _In_ const POINT pos
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_CompareSystemVersion_p(
    _In_ const WindowsVersion ver,
    _In_ const VersionCompare comp
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_GenerateGUID_p(
    _Out_ LPWSTR *guid
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_ShowErrorMessageFromLastErrorCode_p(
    _In_ LPCWSTR functionName
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_ShouldAppsUseLightTheme_p(
    _Out_ bool *result
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_ShouldSystemUsesLightTheme_p(
    _Out_ bool *result
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsHighContrastModeOn_p();

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetWindowCompositionAttribute_p(
    _In_ const HWND hWnd,
    _In_ LPWINDOWCOMPOSITIONATTRIBDATA pwcad
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_ShouldWindowUseDarkFrame_p(
    _In_ const HWND hWnd,
    _Out_ bool      *result
);

[[nodiscard]] ACRYLICMANAGER_API COLORREF WINAPI
am_GetColorizationColor_p();

[[nodiscard]] ACRYLICMANAGER_API ColorizationArea WINAPI
am_GetColorizationArea_p();

#ifdef __cplusplus
EXTERN_C_END
#endif
