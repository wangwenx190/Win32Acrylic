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

#include "Undocumented.h"
#include "WindowsAPIThunks.h"

#ifndef __USER32_DLL_FILENAME
#define __USER32_DLL_FILENAME user32.dll
#endif // __USER32_DLL_FILENAME

static constexpr const wchar_t __UXTHEME_DLL_FILENAME[] = L"uxtheme.dll";
static constexpr const wchar_t __DWMAPI_DLL_FILENAME[] = L"dwmapi.dll";

// User32
__THUNK_API(__USER32_DLL_FILENAME, SetWindowCompositionAttribute, BOOL, DEFAULT_BOOL, (HWND arg1, PWINDOWCOMPOSITIONATTRIBDATA arg2), (arg1, arg2))

#ifdef __cplusplus
EXTERN_C_START
#endif // __cplusplus

// UxTheme
BOOL WINAPI
ShouldAppsUseDarkMode(
    VOID
)
{
    static const auto function = reinterpret_cast<decltype(&::ShouldAppsUseDarkMode)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(132)));
    return (function ? function() : DEFAULT_BOOL);
}

BOOL WINAPI
AllowDarkModeForWindow(
    _In_ HWND hWnd,
    _In_ BOOL allow
)
{
    static const auto function = reinterpret_cast<decltype(&::AllowDarkModeForWindow)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(133)));
    return (function ? function(hWnd, allow) : DEFAULT_BOOL);
}

BOOL WINAPI
AllowDarkModeForApp(
    _In_ BOOL allow
)
{
    static const auto function = reinterpret_cast<decltype(&::AllowDarkModeForApp)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(135)));
    return (function ? function(allow) : DEFAULT_BOOL);
}

VOID WINAPI
FlushMenuThemes(
    VOID
)
{
    static const auto function = reinterpret_cast<decltype(&::FlushMenuThemes)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(136)));
    if (function) {
        function();
    }
}

VOID WINAPI
RefreshImmersiveColorPolicyState(
    VOID
)
{
    static const auto function = reinterpret_cast<decltype(&::RefreshImmersiveColorPolicyState)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(104)));
    if (function) {
        function();
    }
}

BOOL WINAPI
IsDarkModeAllowedForWindow(
    _In_ HWND hWnd
)
{
    static const auto function = reinterpret_cast<decltype(&::IsDarkModeAllowedForWindow)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(137)));
    return (function ? function(hWnd) : DEFAULT_BOOL);
}

BOOL WINAPI
GetIsImmersiveColorUsingHighContrast(
    _In_ IMMERSIVE_HC_CACHE_MODE mode
)
{
    static const auto function = reinterpret_cast<decltype(&::GetIsImmersiveColorUsingHighContrast)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(106)));
    return (function ? function(mode) : DEFAULT_BOOL);
}

HTHEME WINAPI
OpenNcThemeData(
    _In_ HWND hWnd,
    _In_ LPCWSTR pszClassList
)
{
    static const auto function = reinterpret_cast<decltype(&::OpenNcThemeData)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(49)));
    return (function ? function(hWnd, pszClassList) : DEFAULT_PTR);
}

BOOL WINAPI
ShouldSystemUseDarkMode(
    VOID
)
{
    static const auto function = reinterpret_cast<decltype(&::ShouldSystemUseDarkMode)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(138)));
    return (function ? function() : DEFAULT_BOOL);
}

PREFERRED_APP_MODE WINAPI
SetPreferredAppMode(
    _In_ PREFERRED_APP_MODE appMode
)
{
    static const auto function = reinterpret_cast<decltype(&::SetPreferredAppMode)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(135)));
    return (function ? function(appMode) : PAM_DEFAULT);
}

BOOL WINAPI
IsDarkModeAllowedForApp(
    VOID
)
{
    static const auto function = reinterpret_cast<decltype(&::IsDarkModeAllowedForApp)>(GetWindowsAPIByName(__UXTHEME_DLL_FILENAME, MAKEINTRESOURCEW(139)));
    return (function ? function() : DEFAULT_BOOL);
}

// DwmApi
HRESULT WINAPI
DwmpCreateSharedThumbnailVisual(
    IN HWND hwndDestination,
    IN HWND hwndSource,
    IN DWORD dwThumbnailFlags,
    IN PDWM_THUMBNAIL_PROPERTIES pThumbnailProperties,
    IN PVOID pDCompDevice,
    OUT VOID** ppVisual,
    OUT PHTHUMBNAIL phThumbnailId
)
{
    static const auto function = reinterpret_cast<decltype(&::DwmpCreateSharedThumbnailVisual)>(GetWindowsAPIByName(__DWMAPI_DLL_FILENAME, MAKEINTRESOURCEW(147)));
    return (function ? function(hwndDestination, hwndSource, dwThumbnailFlags, pThumbnailProperties, pDCompDevice, ppVisual, phThumbnailId) : DEFAULT_HRESULT);
}

HRESULT WINAPI
DwmpCreateSharedMultiWindowVisual(
    IN HWND hwndDestination,
    IN PVOID pDCompDevice,
    OUT VOID** ppVisual,
    OUT PHTHUMBNAIL phThumbnailId
)
{
    static const auto function = reinterpret_cast<decltype(&::DwmpCreateSharedMultiWindowVisual)>(GetWindowsAPIByName(__DWMAPI_DLL_FILENAME, MAKEINTRESOURCEW(163)));
    return (function ? function(hwndDestination, pDCompDevice, ppVisual, phThumbnailId) : DEFAULT_HRESULT);
}

HRESULT WINAPI
DwmpUpdateSharedMultiWindowVisual(
    IN HTHUMBNAIL hThumbnailId,
    IN HWND* phwndsInclude,
    IN DWORD chwndsInclude,
    IN HWND* phwndsExclude,
    IN DWORD chwndsExclude,
    OUT PRECT prcSource,
    OUT PSIZE pDestinationSize,
    IN DWORD dwFlags
)
{
    static const auto function = reinterpret_cast<decltype(&::DwmpUpdateSharedMultiWindowVisual)>(GetWindowsAPIByName(__DWMAPI_DLL_FILENAME, MAKEINTRESOURCEW(164)));
    return (function ? function(hThumbnailId, phwndsInclude, chwndsInclude, phwndsExclude, chwndsExclude, prcSource, pDestinationSize, dwFlags) : DEFAULT_HRESULT);
}

HRESULT WINAPI
DwmpCreateSharedVirtualDesktopVisual(
    IN HWND hwndDestination,
    IN PVOID pDCompDevice,
    OUT VOID** ppVisual,
    OUT PHTHUMBNAIL phThumbnailId
)
{
    static const auto function = reinterpret_cast<decltype(&::DwmpCreateSharedVirtualDesktopVisual)>(GetWindowsAPIByName(__DWMAPI_DLL_FILENAME, MAKEINTRESOURCEW(163)));
    return (function ? function(hwndDestination, pDCompDevice, ppVisual, phThumbnailId) : DEFAULT_HRESULT);
}

HRESULT WINAPI
DwmpUpdateSharedVirtualDesktopVisual(
    IN HTHUMBNAIL hThumbnailId,
    IN HWND* phwndsInclude,
    IN DWORD chwndsInclude,
    IN HWND* phwndsExclude,
    IN DWORD chwndsExclude,
    OUT PRECT prcSource,
    OUT PSIZE pDestinationSize
)
{
    static const auto function = reinterpret_cast<decltype(&::DwmpUpdateSharedVirtualDesktopVisual)>(GetWindowsAPIByName(__DWMAPI_DLL_FILENAME, MAKEINTRESOURCEW(164)));
    return (function ? function(hThumbnailId, phwndsInclude, chwndsInclude, phwndsExclude, chwndsExclude, prcSource, pDestinationSize) : DEFAULT_HRESULT);
}

#ifdef __cplusplus
EXTERN_C_END
#endif // __cplusplus
