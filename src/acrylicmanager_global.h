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

#ifndef _ADVAPI32_
#define _ADVAPI32_
#endif

#ifndef _SHELL32_
#define _SHELL32_
#endif

#ifndef _GDI32_
#define _GDI32_
#endif

#ifndef _OLEAUT32_
#define _OLEAUT32_
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

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART1
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART1(funcName) \
static bool __tried = false; \
using __sig = decltype(&::funcName); \
static __sig __func = nullptr; \
if (!__func) { \
    if (__tried) {
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART2
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART2(libName) \
    } else { \
        __tried = true; \
        const HMODULE __module = LoadLibraryExW(L#libName ".dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); \
        if (!__module) { \
            OutputDebugStringW(L"Failed to load dynamic link library " #libName ".dll.");
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART3
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_PART3(funcName) \
        } \
        __func = reinterpret_cast<__sig>(GetProcAddress(__module, #funcName)); \
        if (!__func) { \
            OutputDebugStringW(L"Failed to resolve symbol " #funcName "().");
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
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC(...) __func(__VA_ARGS__);
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN(...) return __func(__VA_ARGS__);
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION(libName, funcName, ...) \
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
#define ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(libName, funcName, defVal, ...) \
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
#define ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(User32, funcName, defVal, ##__VA_ARGS__)
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

#ifndef ACRYLICMANAGER_TRY_EXECUTE_USER_VOID_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_USER_VOID_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION(User32, funcName, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_SHELL_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_SHELL_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(Shell32, funcName, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_GDI_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_GDI_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(GDI32, funcName, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_ADVAPI_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_ADVAPI_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(AdvApi32, funcName, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(SHCore, funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(UxTheme, funcName, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION_RETURN(funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_THEME_PTR_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_THEME_PTR_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION_RETURN(funcName, nullptr, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(DwmApi, funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(ComBase, funcName, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION(ComBase, funcName, ##__VA_ARGS__)
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

#ifndef ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION_RETURN
#define ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION_RETURN(funcName, defVal, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(OLE32, funcName, defVal, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION_RETURN(funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_COM_INT_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_COM_INT_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION_RETURN(funcName, 0, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_COM_VOID_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_COM_VOID_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION(OLE32, funcName, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_D2D_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_D2D_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(D2D1, funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_D3D_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_D3D_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_RETURN_FUNCTION(D3D11, funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef ACRYLICMANAGER_TRY_EXECUTE_COMAUTO_VOID_FUNCTION
#define ACRYLICMANAGER_TRY_EXECUTE_COMAUTO_VOID_FUNCTION(funcName, ...) ACRYLICMANAGER_TRY_EXECUTE_VOID_FUNCTION(OLEAut32, funcName, ##__VA_ARGS__)
#endif

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

#ifndef GET_CURRENT_INSTANCE
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define GET_CURRENT_INSTANCE (reinterpret_cast<HINSTANCE>(&__ImageBase))
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI (96)
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER (92)
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED (0x02E0)
#endif

#ifndef IsMaximized
#define IsMaximized(window) (window ? (IsZoomed(window) != FALSE) : false)
#endif

#ifndef IsMinimized
#define IsMinimized(window) (window ? (IsIconic(window) != FALSE) : false)
#endif

#ifndef IsFullScreened
#define IsFullScreened(window) \
[=]{ \
    if (!window) { \
        return false; \
    } \
    const HMONITOR __ps = GET_PRIMARY_SCREEN(window); \
    if (!__ps) { \
        return false; \
    } \
    const MONITORINFO __mi = GET_MONITOR_INFO(__ps); \
    const RECT __sr = __mi.rcMonitor; \
    const RECT __wr = GET_WINDOW_RECT(window); \
    return ((__wr.left == __sr.left) \
             && (__wr.right == __sr.right) \
             && (__wr.top == __sr.top) \
             && (__wr.bottom == __sr.bottom)); \
}()
#endif

#ifndef IsWindowNoState
#define IsWindowNoState(window) \
[=]{ \
    if (!window) { \
        return false; \
    } \
    WINDOWPLACEMENT __wp; \
    SecureZeroMemory(&__wp, sizeof(__wp)); \
    __wp.length = sizeof(__wp); \
    if (GetWindowPlacement(window, &__wp) == FALSE) { \
        PRINT_WIN32_ERROR_MESSAGE(GetWindowPlacement) \
        return false; \
    } \
    return (__wp.showCmd == SW_NORMAL); \
}()
#endif

#ifndef IsWindowShown
#define IsWindowShown(window) (window ? (IsWindowVisible(window) != FALSE) : false)
#endif

#ifndef IsWindowHidden
#define IsWindowHidden(window) (window ? (IsWindowVisible(window) == FALSE) : false)
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
#define GET_CURRENT_SCREEN(window) (window ? MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST) : nullptr)
#endif

#ifndef GET_PRIMARY_SCREEN
#define GET_PRIMARY_SCREEN(window) (window ? MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY) : nullptr)
#endif

#ifndef GET_MONITOR_INFO
#define GET_MONITOR_INFO(monitor) \
[=]{ \
    if (!monitor) { \
        return MONITORINFO{}; \
    } \
    MONITORINFO __mi; \
    SecureZeroMemory(&__mi, sizeof(__mi)); \
    __mi.cbSize = sizeof(__mi); \
    if (GetMonitorInfoW(monitor, &__mi) == FALSE) { \
        PRINT_WIN32_ERROR_MESSAGE(GetMonitorInfoW) \
        return MONITORINFO{}; \
    } \
    return __mi; \
}()
#endif

#ifndef GET_WINDOW_RECT
#define GET_WINDOW_RECT(window) \
[=]{ \
    if (!window) { \
        return RECT{}; \
    } \
    RECT __rect = {0, 0, 0, 0}; \
    if (GetWindowRect(window, &__rect) == FALSE) { \
        PRINT_WIN32_ERROR_MESSAGE(GetWindowRect) \
        return RECT{}; \
    } \
    return __rect; \
}()
#endif

#ifndef GET_WINDOW_SIZE
#define GET_WINDOW_SIZE(window) \
[=]{ \
    if (!window) { \
        return SIZE{}; \
    } \
    const RECT __rect = GET_WINDOW_RECT(window); \
    return GET_RECT_SIZE(__rect); \
}()
#endif

#ifndef GET_WINDOW_CLIENT_SIZE
#define GET_WINDOW_CLIENT_SIZE(window) \
[=]{ \
    if (!window) { \
        return SIZE{}; \
    } \
    RECT __rect = {0, 0, 0, 0}; \
    if (GetClientRect(window, &__rect) == FALSE) { \
        PRINT_WIN32_ERROR_MESSAGE(GetClientRect) \
        return SIZE{}; \
    } \
    return GET_RECT_SIZE(__rect); \
}()
#endif

#ifndef GET_SCREEN_GEOMETRY
#define GET_SCREEN_GEOMETRY(monitor) \
[=]{ \
    if (!monitor) { \
        return RECT{}; \
    } \
    return GET_MONITOR_INFO(monitor).rcMonitor; \
}()
#endif

#ifndef GET_SCREEN_AVAILABLE_GEOMETRY
#define GET_SCREEN_AVAILABLE_GEOMETRY(monitor) \
[=]{ \
    if (!monitor) { \
        return RECT{}; \
    } \
    return GET_MONITOR_INFO(monitor).rcWork; \
}()
#endif

#ifndef DECLARE_UNUSED
#define DECLARE_UNUSED(var) (static_cast<void>(var))
#endif

#ifndef PRINT_HR_ERROR_MESSAGE
#define PRINT_HR_ERROR_MESSAGE(function, hresult) \
{ \
    if (FAILED(hresult)) { \
        const std::wstring __message = Utils::TranslateErrorCodeToMessage(L#function, hresult); \
        if (!__message.empty()) { \
            OutputDebugStringW(__message.c_str()); \
            MessageBoxW(nullptr, __message.c_str(), L"AcrylicManager", MB_ICONERROR | MB_OK); \
        } \
    } \
}
#endif

#ifndef PRINT_WIN32_ERROR_MESSAGE
#define PRINT_WIN32_ERROR_MESSAGE(function) PRINT_HR_ERROR_MESSAGE(function, HRESULT_FROM_WIN32(GetLastError()))
#endif

#ifndef GET_WINRTCOLOR_COMPONENTS
#define GET_WINRTCOLOR_COMPONENTS(color, r, g, b, a) \
{ \
    constexpr int __min = 0; \
    constexpr int __max = 255; \
    r = std::clamp(static_cast<int>((color).R), __min, __max); \
    g = std::clamp(static_cast<int>((color).G), __min, __max); \
    b = std::clamp(static_cast<int>((color).B), __min, __max); \
    a = std::clamp(static_cast<int>((color).A), __min, __max); \
}
#endif

#ifndef MAKE_WINRTCOLOR_FROM_COMPONENTS
#define MAKE_WINRTCOLOR_FROM_COMPONENTS(r, g, b, a) \
[=]{ \
    constexpr int __min = 0; \
    constexpr int __max = 255; \
    const auto __r = static_cast<uint8_t>(std::clamp(r, __min, __max)); \
    const auto __g = static_cast<uint8_t>(std::clamp(g, __min, __max)); \
    const auto __b = static_cast<uint8_t>(std::clamp(b, __min, __max)); \
    const auto __a = static_cast<uint8_t>(std::clamp(a, __min, __max)); \
    return winrt::Windows::UI::ColorHelper::FromArgb(__a, __r, __g, __b); \
}()
#endif

#ifndef WINRTCOLOR_TO_D2DCOLOR4F
#define WINRTCOLOR_TO_D2DCOLOR4F(color) \
[=]{ \
    constexpr double __max = 255.0; \
    const float __r = (static_cast<float>((color).R) / __max); \
    const float __g = (static_cast<float>((color).G) / __max); \
    const float __b = (static_cast<float>((color).B) / __max); \
    const float __a = (static_cast<float>((color).A) / __max); \
    return D2D1::Vector4F(__r, __g, __b, __a); \
}()
#endif

#ifndef D2DCOLOR4F_TO_WINRTCOLOR
#define D2DCOLOR4F_TO_WINRTCOLOR(color) \
[=]{ \
    constexpr double __max = 255.0; \
    const auto __r = static_cast<uint8_t>(std::round((color).x * __max)); \
    const auto __g = static_cast<uint8_t>(std::round((color).y * __max)); \
    const auto __b = static_cast<uint8_t>(std::round((color).z * __max)); \
    const auto __a = static_cast<uint8_t>(std::round((color).w * __max)); \
    return winrt::Windows::UI::ColorHelper::FromArgb(__a, __r, __g, __b); \
}()
#endif

#ifndef WINRTCOLOR_TO_WIN32COLOR
#define WINRTCOLOR_TO_WIN32COLOR(color) \
[=]{ \
    constexpr double __max = 255.0; \
    const auto __a = static_cast<double>(color.A); \
    const auto __r = static_cast<int>(std::round((static_cast<double>(color.R) * (__a / __max)) + __max - __a)); \
    const auto __g = static_cast<int>(std::round((static_cast<double>(color.G) * (__a / __max)) + __max - __a)); \
    const auto __b = static_cast<int>(std::round((static_cast<double>(color.B) * (__a / __max)) + __max - __a)); \
    return RGB(__r, __g, __b); \
}()
#endif

#ifndef WIN32COLOR_TO_WINRTCOLOR
#define WIN32COLOR_TO_WINRTCOLOR(color) \
[=]{ \
    const auto __r = static_cast<uint8_t>(GetRValue(color)); \
    const auto __g = static_cast<uint8_t>(GetGValue(color)); \
    const auto __b = static_cast<uint8_t>(GetBValue(color)); \
    constexpr uint8_t __a = 255; \
    return winrt::Windows::UI::ColorHelper::FromArgb(__a, __r, __g, __b); \
}()
#endif

#ifndef COM_SAFE_RELEASE
#define COM_SAFE_RELEASE(pInterface) \
{ \
    if (pInterface) { \
        pInterface->Release(); \
        pInterface = nullptr; \
    } \
}
#endif

#ifndef SAFE_FREE_CHARARRAY
#define SAFE_FREE_CHARARRAY(array) \
{ \
    if (array) { \
        delete [] array; \
        array = nullptr; \
    } \
}
#endif

#ifndef ACRYLICMANAGER_DISABLE_COPY_MOVE
#define ACRYLICMANAGER_DISABLE_COPY_MOVE(class) \
class(const class &) = delete; \
class &operator=(const class &) = delete; \
class(class &&) = delete; \
class &operator=(class &&) = delete;
#endif

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

enum class WindowState : int
{
    Invalid = -1,
    Normal,
    Maximized,
    Minimized,
    FullScreened,
    Hidden,
    Shown
};

enum class BrushType : int
{
    Auto = 0,
    Null,
    Direct2D,
    WinUI2,
    WinUI3,
    Composition,
    System
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

enum class MonitorDpiType : int
{
    EFFECTIVE_DPI = 0,
    DEFAULT = EFFECTIVE_DPI
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

typedef struct _WINDOWCOMPOSITIONATTRIBDATA
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

typedef struct _ACCENT_POLICY
{
    ACCENT_STATE State;
    DWORD Flags;
    COLORREF GradientColor;
    DWORD AnimationId;
} ACCENT_POLICY, *PACCENT_POLICY, *LPACCENT_POLICY;
