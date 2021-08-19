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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WINRT_LEAN_AND_MEAN
#define WINRT_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef _ENABLE_EXTENDED_ALIGNED_STORAGE
#define _ENABLE_EXTENDED_ALIGNED_STORAGE
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Define these macros first before including their header files to avoid linking
// to their import libraries.
#ifndef _USER32_
#define _USER32_
#endif

#ifndef _SHELL32_
#define _SHELL32_
#endif

#ifndef _GDI32_
#define _GDI32_
#endif

// Compat name for ComBaseApi, remove it once Microsoft renamed it
#ifndef _OLE32_
#define _OLE32_
#endif

#ifndef _COMBASEAPI_
#define _COMBASEAPI_
#endif

#ifndef _UXTHEME_
#define _UXTHEME_
#endif

#ifndef _DWMAPI_
#define _DWMAPI_
#endif

#include <SDKDDKVer.h>

#ifndef NTDDI_WIN10_19H1
#define NTDDI_WIN10_19H1 (0x0A000007)
#endif

#ifdef WINVER
#undef WINVER
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define WINVER NTDDI_WIN10_19H1
#define _WIN32_WINNT NTDDI_WIN10_19H1

#include <Windows.h>

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART1
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART1(funcName) \
static bool tried = false; \
using sig = decltype(&::funcName); \
static sig func = nullptr; \
if (!func) { \
    if (tried) {
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART2
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART2(libName) \
    } else { \
        tried = true; \
        const HMODULE dll = LoadLibraryExW(L#libName ".dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); \
        if (!dll) { \
            OutputDebugStringW(L"Failed to load " #libName ".dll.");
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART3
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART3(funcName) \
        } \
        func = reinterpret_cast<sig>(GetProcAddress(dll, #funcName)); \
        if (!func) { \
            OutputDebugStringW(L"Failed to resolve " #funcName "().");
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART4
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART4 \
        } \
    } \
}
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VOID
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VOID \
{ \
    SetLastError(ERROR_SUCCESS); \
    return; \
}
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VALUE
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VALUE(value) \
{ \
    SetLastError(ERROR_SUCCESS); \
    return (value); \
}
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC(...) func(__VA_ARGS__);
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN(...) return func(__VA_ARGS__);
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION(funcName, libName, ...) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART1(funcName) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VOID \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART2(libName) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VOID \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART3(funcName) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VOID \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART4 \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC(__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, libName, defVal, ...) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART1(funcName) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART2(libName) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART3(funcName) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART4 \
ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN(__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, User32, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION_RETURN(funcName, FALSE, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION_RETURN(funcName, 0, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION_RETURN(funcName, nullptr, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_GDI_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_GDI_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, GDI32, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, SHCore, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, UxTheme, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION_RETURN(funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_THEME_PTR_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_THEME_PTR_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION_RETURN(funcName, nullptr, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, DwmApi, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, ComBase, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION(funcName, ComBase, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_INT_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_INT_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, 0, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, FALSE, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_PTR_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_PTR_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, nullptr, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, OLE32, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION_RETURN(funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_OLE_INT_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_OLE_INT_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION_RETURN(funcName, 0, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_OLE_VOID_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_OLE_VOID_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION(funcName, OLE32, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_D2D_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_D2D_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(funcName, D2D1, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_API
#ifdef ACRYLICMANAGER_STATIC
#define ACRYLICMANAGER_API
#else
#ifdef ACRYLICMANAGER_BUILD_LIBRARY
#define ACRYLICMANAGER_API __declspec(dllexport)
#else
#define ACRYLICMANAGER_API __declspec(dllimport)
#endif
#endif
#endif

#ifndef ACRYLICMANAGER_VERSION_STR
#define ACRYLICMANAGER_VERSION_STR L"1.0.0.0"
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT (reinterpret_cast<HINSTANCE>(&__ImageBase))
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI (96)
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER (92)
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED (0x02E0)
#endif

#ifndef IsMaximized
#define IsMaximized(window) (IsZoomed(window))
#endif

#ifndef IsMinimized
#define IsMinimized(window) (IsIconic(window))
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#ifndef GET_RECT_WIDTH
#define GET_RECT_WIDTH(rect) (std::abs((rect).right - (rect).left))
#endif

#ifndef GET_RECT_HEIGHT
#define GET_RECT_HEIGHT(rect) (std::abs((rect).bottom - (rect).top))
#endif

#ifndef GET_RECT_SIZE
#define GET_RECT_SIZE(rect) (SIZE{GET_RECT_WIDTH(rect), GET_RECT_HEIGHT(rect)})
#endif

#ifndef GET_BLACK_BRUSH
#define GET_BLACK_BRUSH (reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)))
#endif

#ifndef GET_CURRENT_SCREEN
#define GET_CURRENT_SCREEN(window) (MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST))
#endif

#ifndef GET_DEVICE_PIXEL_RATIO
#define GET_DEVICE_PIXEL_RATIO(dpi) ((dpi > 0) ? (static_cast<double>(dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI)) : 1.0)
#endif

#ifndef DECLARE_UNUSED
#define DECLARE_UNUSED(var) (static_cast<void>(var))
#endif

#ifndef __PRINT_WIN32_ERROR_MESSAGE_HEAD
#define __PRINT_WIN32_ERROR_MESSAGE_HEAD(function) \
{ \
    const HRESULT __hr = HRESULT_FROM_WIN32(GetLastError()); \
    if (FAILED(__hr)) { \
        const HRESULT __hr_ = am_PrintErrorMessageFromHResult_p(L#function, __hr); \
        DECLARE_UNUSED(__hr_); \
    }
#endif

#ifndef __PRINT_WIN32_ERROR_MESSAGE_FOOT
#define __PRINT_WIN32_ERROR_MESSAGE_FOOT \
}
#endif

#ifndef PRINT_WIN32_ERROR_MESSAGE_AND_RETURN
#define PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(function) \
__PRINT_WIN32_ERROR_MESSAGE_HEAD(function) \
return __hr; \
__PRINT_WIN32_ERROR_MESSAGE_FOOT
#endif

#ifndef PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN
#define PRINT_WIN32_ERROR_MESSAGE_AND_SAFE_RETURN(function) \
__PRINT_WIN32_ERROR_MESSAGE_HEAD(function) \
SAFE_RELEASE_RESOURCES \
return __hr; \
__PRINT_WIN32_ERROR_MESSAGE_FOOT
#endif

#ifndef PRINT_WIN32_ERROR_MESSAGE
#define PRINT_WIN32_ERROR_MESSAGE(function) \
__PRINT_WIN32_ERROR_MESSAGE_HEAD(function) \
__PRINT_WIN32_ERROR_MESSAGE_FOOT
#endif

#ifndef __PRINT_HR_ERROR_MESSAGE_HEAD
#define __PRINT_HR_ERROR_MESSAGE_HEAD(function, hresult) \
{ \
    if (FAILED(hresult)) { \
        const HRESULT __hr = am_PrintErrorMessageFromHResult_p(L#function, hresult); \
        DECLARE_UNUSED(__hr); \
    }
#endif

#ifndef __PRINT_HR_ERROR_MESSAGE_FOOT
#define __PRINT_HR_ERROR_MESSAGE_FOOT \
}
#endif

#ifndef PRINT_HR_ERROR_MESSAGE
#define PRINT_HR_ERROR_MESSAGE(function, hresult) \
__PRINT_HR_ERROR_MESSAGE_HEAD(function, hresult) \
__PRINT_HR_ERROR_MESSAGE_FOOT
#endif

#ifndef PRINT_HR_ERROR_MESSAGE_AND_RETURN
#define PRINT_HR_ERROR_MESSAGE_AND_RETURN(function, hresult) \
__PRINT_HR_ERROR_MESSAGE_HEAD(function, hresult) \
return hresult; \
__PRINT_HR_ERROR_MESSAGE_FOOT
#endif

#ifndef PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN
#define PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(function, hresult) \
__PRINT_HR_ERROR_MESSAGE_HEAD(function, hresult) \
SAFE_RELEASE_RESOURCES \
return hresult; \
__PRINT_HR_ERROR_MESSAGE_FOOT
#endif

#ifndef SAFE_RELEASE_RESOURCES
#define SAFE_RELEASE_RESOURCES \
{ \
    const HRESULT __hr = am_CleanupHelper_p(); \
    DECLARE_UNUSED(__hr); \
}
#endif

#ifndef GET_COLOR_COMPONENTS
#define GET_COLOR_COMPONENTS(color, r, g, b, a) \
{ \
    r = std::clamp(static_cast<int>((color).R), 0, 255); \
    g = std::clamp(static_cast<int>((color).G), 0, 255); \
    b = std::clamp(static_cast<int>((color).B), 0, 255); \
    a = std::clamp(static_cast<int>((color).A), 0, 255); \
}
#endif

#ifndef MAKE_COLOR_FROM_COMPONENTS
#define MAKE_COLOR_FROM_COMPONENTS(color, r, g, b, a) \
{ \
    (color).R = static_cast<uint8_t>(std::clamp(r, 0, 255)); \
    (color).G = static_cast<uint8_t>(std::clamp(g, 0, 255)); \
    (color).B = static_cast<uint8_t>(std::clamp(b, 0, 255)); \
    (color).A = static_cast<uint8_t>(std::clamp(a, 0, 255)); \
}
#endif

#ifndef SAFE_RETURN
#define SAFE_RETURN \
{ \
    SAFE_RELEASE_RESOURCES \
    return E_FAIL; \
}
#endif

#ifndef PRINT
#define PRINT(message) \
{ \
    const HRESULT __hr = am_PrintHelper_p(message, true); \
    DECLARE_UNUSED(__hr); \
}
#endif

#ifndef PRINT_AND_RETURN
#define PRINT_AND_RETURN(message) \
{ \
    PRINT(message) \
    return E_FAIL; \
}
#endif

#ifndef PRINT_AND_SAFE_RETURN
#define PRINT_AND_SAFE_RETURN(message) \
{ \
    PRINT(message) \
    SAFE_RETURN \
}
#endif
