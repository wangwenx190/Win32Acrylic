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

#include <SDKDDKVer.h>
#include <Windows.h>
#include <DwmApi.h>

#define DWM_TNP_FREEZE          0x00100000
#define DWM_TNP_ENABLE3D        0x04000000
#define DWM_TNP_DISABLE3D       0x08000000
#define DWM_TNP_FORCECVI        0x40000000
#define DWM_TNP_DISABLEFORCECVI 0x80000000

using IMMERSIVE_HC_CACHE_MODE = enum _IMMERSIVE_HC_CACHE_MODE
{
    IHCM_USE_CACHED_VALUE,
    IHCM_REFRESH
};

using PREFERRED_APP_MODE = enum _PREFERRED_APP_MODE
{
    PAM_DEFAULT,
    PAM_ALLOW_DARK,
    PAM_FORCE_DARK,
    PAM_FORCE_LIGHT,
    PAM_MAX
};

using WINDOWCOMPOSITIONATTRIB = enum _WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_LAST = 27
};

using ACCENT_STATE = enum _ACCENT_STATE
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_USE_HOST_BACKDROP = 5,
    ACCENT_INVALID_STATE = 6
};

using WINDOWCOMPOSITIONATTRIBDATA = struct _WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

using PWINDOWCOMPOSITIONATTRIBDATA = WINDOWCOMPOSITIONATTRIBDATA *;
using LPWINDOWCOMPOSITIONATTRIBDATA = WINDOWCOMPOSITIONATTRIBDATA *;

#ifdef __cplusplus
EXTERN_C_START
#endif // __cplusplus

// User32
BOOL WINAPI
SetWindowCompositionAttribute(
    _In_ HWND hWnd,
    _In_ PWINDOWCOMPOSITIONATTRIBDATA pwcad
);

// UxTheme
BOOL WINAPI
ShouldAppsUseDarkMode(
    VOID
);

BOOL WINAPI
AllowDarkModeForWindow(
    _In_ HWND hWnd,
    _In_ BOOL allow
);

BOOL WINAPI
AllowDarkModeForApp(
    _In_ BOOL allow
);

VOID WINAPI
FlushMenuThemes(
    VOID
);

VOID WINAPI
RefreshImmersiveColorPolicyState(
    VOID
);

BOOL WINAPI
IsDarkModeAllowedForWindow(
    _In_ HWND hWnd
);

BOOL WINAPI
GetIsImmersiveColorUsingHighContrast(
    _In_ IMMERSIVE_HC_CACHE_MODE mode
);

HTHEME WINAPI
OpenNcThemeData(
    _In_ HWND hWnd,
    _In_ LPCWSTR pszClassList
);

BOOL WINAPI
ShouldSystemUseDarkMode(
    VOID
);

PREFERRED_APP_MODE WINAPI
SetPreferredAppMode(
    _In_ PREFERRED_APP_MODE appMode
);

BOOL WINAPI
IsDarkModeAllowedForApp(
    VOID
);

// DwmApi
HRESULT WINAPI
DwmpCreateSharedThumbnailVisual(
    _In_ HWND hwndDestination,
    _In_ HWND hwndSource,
    _In_ DWORD dwThumbnailFlags,
    _In_ PDWM_THUMBNAIL_PROPERTIES pThumbnailProperties,
    _In_ PVOID pDCompDevice,
    _Out_ VOID **ppVisual,
    _Out_ PHTHUMBNAIL phThumbnailId
);

HRESULT WINAPI
DwmpCreateSharedMultiWindowVisual(
    _In_ HWND hwndDestination,
    _In_ PVOID pDCompDevice,
    _Out_ VOID **ppVisual,
    _Out_ PHTHUMBNAIL phThumbnailId
);

HRESULT WINAPI
DwmpUpdateSharedMultiWindowVisual(
    _In_ HTHUMBNAIL hThumbnailId,
    _In_ HWND *phwndsInclude,
    _In_ DWORD chwndsInclude,
    _In_ HWND *phwndsExclude,
    _In_ DWORD chwndsExclude,
    _Out_ PRECT prcSource,
    _Out_ PSIZE pDestinationSize,
    _In_ DWORD dwFlags
);

HRESULT WINAPI
DwmpCreateSharedVirtualDesktopVisual(
    _In_ HWND hwndDestination,
    _In_ PVOID pDCompDevice,
    _Out_ VOID **ppVisual,
    _Out_ PHTHUMBNAIL phThumbnailId
);

HRESULT WINAPI
DwmpUpdateSharedVirtualDesktopVisual(
    _In_ HTHUMBNAIL hThumbnailId,
    _In_ HWND *phwndsInclude,
    _In_ DWORD chwndsInclude,
    _In_ HWND *phwndsExclude,
    _In_ DWORD chwndsExclude,
    _Out_ PRECT prcSource,
    _Out_ PSIZE pDestinationSize
);

#ifdef __cplusplus
EXTERN_C_END
#endif // __cplusplus
