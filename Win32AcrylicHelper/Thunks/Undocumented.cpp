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

#ifndef __RESOLVE_UNDOC_API_INTERNAL
#define __RESOLVE_UNDOC_API_INTERNAL(library, symbol, ordinal) ( reinterpret_cast< decltype( & ::symbol ) >( GetWindowsAPIByName( L#library , MAKEINTRESOURCEW( ordinal ) ) ) )
#endif // __RESOLVE_UNDOC_API_INTERNAL

#ifndef __RESOLVE_UNDOC_API
#define __RESOLVE_UNDOC_API(library, symbol, ordinal) static const auto symbol ## _API = __RESOLVE_UNDOC_API_INTERNAL( library , symbol , ordinal )
#endif // __RESOLVE_UNDOC_API

#ifndef __THUNK_UNDOC_API
#define __THUNK_UNDOC_API(library, symbol, ordinal, result_type, default_result, argument_signature, argument_list) \
EXTERN_C result_type WINAPI \
symbol \
argument_signature \
{ \
    __RESOLVE_UNDOC_API( library , symbol , ordinal ); \
    return ( ( symbol ## _API ) ? ( symbol ## _API argument_list ) : ( default_result ) ); \
} \
_LCRT_DEFINE_IAT_SYMBOL( symbol , 0 );
#endif // __THUNK_UNDOC_API

#ifndef __USER32_DLL_FILENAME
#define __USER32_DLL_FILENAME user32.dll
#endif // __USER32_DLL_FILENAME

#ifndef __UXTHEME_DLL_FILENAME
#define __UXTHEME_DLL_FILENAME uxtheme.dll
#endif // __UXTHEME_DLL_FILENAME

#ifndef __DWMAPI_DLL_FILENAME
#define __DWMAPI_DLL_FILENAME dwmapi.dll
#endif // __DWMAPI_DLL_FILENAME

// User32
__THUNK_API(__USER32_DLL_FILENAME, SetWindowCompositionAttribute, BOOL, DEFAULT_BOOL, (HWND arg1, PWINDOWCOMPOSITIONATTRIBDATA arg2), (arg1, arg2))

// UxTheme
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, ShouldAppsUseDarkMode, 132, BOOL, DEFAULT_BOOL, (VOID), ())
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, AllowDarkModeForWindow, 133, BOOL, DEFAULT_BOOL, (HWND arg1, BOOL arg2), (arg1, arg2))
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, AllowDarkModeForApp, 135, BOOL, DEFAULT_BOOL, (BOOL arg1), (arg1))
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, FlushMenuThemes, 136, VOID, DEFAULT_VOID, (VOID), ())
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, RefreshImmersiveColorPolicyState, 104, VOID, DEFAULT_VOID, (VOID), ())
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, IsDarkModeAllowedForWindow, 137, BOOL, DEFAULT_BOOL, (HWND arg1), (arg1))
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, GetIsImmersiveColorUsingHighContrast, 106, BOOL, DEFAULT_BOOL, (IMMERSIVE_HC_CACHE_MODE arg1), (arg1))
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, OpenNcThemeData, 49, HTHEME, DEFAULT_PTR, (HWND arg1, LPCWSTR arg2), (arg1, arg2))
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, ShouldSystemUseDarkMode, 138, BOOL, DEFAULT_BOOL, (VOID), ())
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, SetPreferredAppMode, 135, PREFERRED_APP_MODE, PAM_DEFAULT, (PREFERRED_APP_MODE arg1), (arg1))
__THUNK_UNDOC_API(__UXTHEME_DLL_FILENAME, IsDarkModeAllowedForApp, 139, BOOL, DEFAULT_BOOL, (VOID), ())

// DwmApi
__THUNK_UNDOC_API(__DWMAPI_DLL_FILENAME, DwmpCreateSharedThumbnailVisual, 147, HRESULT, DEFAULT_HRESULT, (HWND arg1, HWND arg2, DWORD arg3, PDWM_THUMBNAIL_PROPERTIES arg4, PVOID arg5, VOID **arg6, PHTHUMBNAIL arg7), (arg1, arg2, arg3, arg4, arg5, arg6, arg7))
__THUNK_UNDOC_API(__DWMAPI_DLL_FILENAME, DwmpCreateSharedMultiWindowVisual, 163, HRESULT, DEFAULT_HRESULT, (HWND arg1, PVOID arg2, VOID **arg3, PHTHUMBNAIL arg4), (arg1, arg2, arg3, arg4))
__THUNK_UNDOC_API(__DWMAPI_DLL_FILENAME, DwmpUpdateSharedMultiWindowVisual, 164, HRESULT, DEFAULT_HRESULT, (HTHUMBNAIL arg1, HWND *arg2, DWORD arg3, HWND *arg4, DWORD arg5, PRECT arg6, PSIZE arg7, DWORD arg8), (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
__THUNK_UNDOC_API(__DWMAPI_DLL_FILENAME, DwmpCreateSharedVirtualDesktopVisual, 163, HRESULT, DEFAULT_HRESULT, (HWND arg1, PVOID arg2, VOID **arg3, PHTHUMBNAIL arg4), (arg1, arg2, arg3, arg4))
__THUNK_UNDOC_API(__DWMAPI_DLL_FILENAME, DwmpUpdateSharedVirtualDesktopVisual, 164, HRESULT, DEFAULT_HRESULT, (HTHUMBNAIL arg1, HWND *arg2, DWORD arg3, HWND *arg4, DWORD arg5, PRECT arg6, PSIZE arg7), (arg1, arg2, arg3, arg4, arg5, arg6, arg7))
