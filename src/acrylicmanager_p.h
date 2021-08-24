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

#ifdef __cplusplus
EXTERN_C_START
#endif



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
am_SetWindowCompositionAttribute_p(
    _In_ const HWND                    hWnd,
    _In_ LPWINDOWCOMPOSITIONATTRIBDATA pwcad
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
am_GetEffectiveTintColor_p(
    _In_ int    tintColorR,
    _In_ int    tintColorG,
    _In_ int    tintColorB,
    _In_ int    tintColorA,
    _In_ double tintOpacity,
    _In_ double *tintLuminosityOpacity,
    _Out_ int   *r,
    _Out_ int   *g,
    _Out_ int   *b,
    _Out_ int   *a
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetEffectiveLuminosityColor_p(
    _In_ int    tintColorR,
    _In_ int    tintColorG,
    _In_ int    tintColorB,
    _In_ int    tintColorA,
    _In_ double tintOpacity,
    _In_ double *tintLuminosityOpacity,
    _Out_ int   *r,
    _Out_ int   *g,
    _Out_ int   *b,
    _Out_ int   *a
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetIntFromIniFile_p(
    _In_ LPCWSTR ini,
    _In_ LPCWSTR section,
    _In_ LPCWSTR key,
    _Out_ int    *value
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetBoolFromIniFile_p(
    _In_ LPCWSTR ini,
    _In_ LPCWSTR section,
    _In_ LPCWSTR key,
    _Out_ bool   *value
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetStringFromIniFile_p(
    _In_ LPCWSTR ini,
    _In_ LPCWSTR section,
    _In_ LPCWSTR key,
    _Out_ LPWSTR *value
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_GetSymbolAddressFromExecutable_p(
    _In_ LPCWSTR    path,
    _In_ LPCWSTR    function,
    _In_ const bool system,
    _Out_ FARPROC   *result
);

[[nodiscard]] ACRYLICMANAGER_API HRESULT WINAPI
am_ReleaseAllLoadedModules_p();

#ifdef __cplusplus
EXTERN_C_END
#endif
