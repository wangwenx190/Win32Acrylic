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

#ifndef _USER32_
#define _USER32_
#endif // _USER32_

#include "WindowsAPIThunks.h"

#include <WinUser.h>

#ifndef __USER32_DLL_FILENAME
#define __USER32_DLL_FILENAME user32.dll
#endif // __USER32_DLL_FILENAME

__THUNK_API(__USER32_DLL_FILENAME, GetWindowRect, BOOL, DEFAULT_BOOL, (HWND arg1, LPRECT arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, IsWindowVisible, BOOL, DEFAULT_BOOL, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, IsIconic, BOOL, DEFAULT_BOOL, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, IsZoomed, BOOL, DEFAULT_BOOL, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, GetWindowPlacement, BOOL, DEFAULT_BOOL, (HWND arg1, WINDOWPLACEMENT *arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, SystemParametersInfoW, BOOL, DEFAULT_BOOL, (UINT arg1, UINT arg2, PVOID arg3, UINT arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__USER32_DLL_FILENAME, MonitorFromWindow, HMONITOR, DEFAULT_PTR, (HWND arg1, DWORD arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, GetMonitorInfoW, BOOL, DEFAULT_BOOL, (HMONITOR arg1, LPMONITORINFO arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, LoadCursorW, HCURSOR, DEFAULT_PTR, (HINSTANCE arg1, LPCWSTR arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, LoadIconW, HICON, DEFAULT_PTR, (HINSTANCE arg1, LPCWSTR arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, RegisterClassExW, ATOM, INVALID_ATOM, (CONST WNDCLASSEXW *arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, CreateWindowExW, HWND, DEFAULT_PTR, (DWORD arg1, LPCWSTR arg2, LPCWSTR arg3, DWORD arg4, int arg5, int arg6, int arg7, int arg8, HWND arg9, HMENU arg10, HINSTANCE arg11, LPVOID arg12), (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12))
__THUNK_API(__USER32_DLL_FILENAME, GetClassNameW, int, DEFAULT_INT, (HWND arg1, LPWSTR arg2, int arg3), (arg1, arg2, arg3))
__THUNK_API(__USER32_DLL_FILENAME, DestroyWindow, BOOL, DEFAULT_BOOL, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, UnregisterClassW, BOOL, DEFAULT_BOOL, (LPCWSTR arg1, HINSTANCE arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, GetWindowLongPtrW, LONG_PTR, DEFAULT_INT, (HWND arg1, int arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, GetClientRect, BOOL, DEFAULT_BOOL, (HWND arg1, LPRECT arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, SetWindowPos, BOOL, DEFAULT_BOOL, (HWND arg1, HWND arg2, int arg3, int arg4, int arg5, int arg6, UINT arg7), (arg1, arg2, arg3, arg4, arg5, arg6, arg7))
__THUNK_API(__USER32_DLL_FILENAME, GetSystemMenu, HMENU, DEFAULT_PTR, (HWND arg1, BOOL arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, SetMenuItemInfoW, BOOL, DEFAULT_BOOL, (HMENU arg1, UINT arg2, BOOL arg3, LPCMENUITEMINFOW arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__USER32_DLL_FILENAME, SetMenuDefaultItem, BOOL, DEFAULT_BOOL, (HMENU arg1, UINT arg2, UINT arg3), (arg1, arg2, arg3))
__THUNK_API(__USER32_DLL_FILENAME, TrackPopupMenu, BOOL, DEFAULT_BOOL, (HMENU arg1, UINT arg2, int arg3, int arg4, int arg5, HWND arg6, CONST RECT *arg7), (arg1, arg2, arg3, arg4, arg5, arg6, arg7))
__THUNK_API(__USER32_DLL_FILENAME, PostMessageW, BOOL, DEFAULT_BOOL, (HWND arg1, UINT arg2, WPARAM arg3, LPARAM arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__USER32_DLL_FILENAME, ShowWindow, BOOL, DEFAULT_BOOL, (HWND arg1, int arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, UpdateWindow, BOOL, DEFAULT_BOOL, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, GetDpiForWindow, UINT, DEFAULT_UINT, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, GetMessageW, BOOL, DEFAULT_BOOL, (LPMSG arg1, HWND arg2, UINT arg3, UINT arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__USER32_DLL_FILENAME, TranslateMessage, BOOL, DEFAULT_BOOL, (CONST MSG *arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, DispatchMessageW, LRESULT, DEFAULT_INT, (CONST MSG *arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, SetWindowTextW, BOOL, DEFAULT_BOOL, (HWND arg1, LPCWSTR arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, SetWindowRgn, int, DEFAULT_INT, (HWND arg1, HRGN arg2, BOOL arg3), (arg1, arg2, arg3))
__THUNK_API(__USER32_DLL_FILENAME, GetSystemMetricsForDpi, int, DEFAULT_INT, (int arg1, UINT arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, SetWindowLongPtrW, LONG_PTR, DEFAULT_INT, (HWND arg1, int arg2, LONG_PTR arg3), (arg1, arg2, arg3))
__THUNK_API(__USER32_DLL_FILENAME, DefWindowProcW, LRESULT, DEFAULT_INT, (HWND arg1, UINT arg2, WPARAM arg3, LPARAM arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__USER32_DLL_FILENAME, BeginPaint, HDC, DEFAULT_PTR, (HWND arg1, LPPAINTSTRUCT arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, FillRect, int, DEFAULT_INT, (HDC arg1, CONST RECT *arg2, HBRUSH arg3), (arg1, arg2, arg3))
__THUNK_API(__USER32_DLL_FILENAME, EndPaint, BOOL, DEFAULT_BOOL, (HWND arg1, CONST PAINTSTRUCT *arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, PostQuitMessage, VOID, DEFAULT_VOID, (int arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, EnableNonClientDpiScaling, BOOL, DEFAULT_BOOL, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, ScreenToClient, BOOL, DEFAULT_BOOL, (HWND arg1, LPPOINT arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, GetThreadDpiAwarenessContext, DPI_AWARENESS_CONTEXT, DEFAULT_PTR, (VOID), ())
__THUNK_API(__USER32_DLL_FILENAME, GetAwarenessFromDpiAwarenessContext, DPI_AWARENESS, DPI_AWARENESS_INVALID, (DPI_AWARENESS_CONTEXT arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, SetProcessDpiAwarenessContext, BOOL, DEFAULT_BOOL, (DPI_AWARENESS_CONTEXT arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, ClientToScreen, BOOL, DEFAULT_BOOL, (HWND arg1, LPPOINT arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, SendMessageW, LRESULT, DEFAULT_INT, (HWND arg1, UINT arg2, WPARAM arg3, LPARAM arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__USER32_DLL_FILENAME, GetCursorPos, BOOL, DEFAULT_BOOL, (LPPOINT arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, SetFocus, HWND, DEFAULT_PTR, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, SetCursor, HCURSOR, DEFAULT_PTR, (HCURSOR arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, GetMessagePos, DWORD, DEFAULT_UINT, (VOID), ())
__THUNK_API(__USER32_DLL_FILENAME, SetLayeredWindowAttributes, BOOL, DEFAULT_BOOL, (HWND arg1, COLORREF arg2, BYTE arg3, DWORD arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__USER32_DLL_FILENAME, MessageBoxW, int, DEFAULT_INT, (HWND arg1, LPCWSTR arg2, LPCWSTR arg3, UINT arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__USER32_DLL_FILENAME, FindWindowW, HWND, DEFAULT_PTR, (LPCWSTR arg1, LPCWSTR arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, GetSystemMetrics, int, DEFAULT_INT, (int arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, AttachThreadInput, BOOL, DEFAULT_BOOL, (DWORD arg1, DWORD arg2, BOOL arg3), (arg1, arg2, arg3))
__THUNK_API(__USER32_DLL_FILENAME, GetForegroundWindow, HWND, DEFAULT_PTR, (VOID), ())
__THUNK_API(__USER32_DLL_FILENAME, GetWindowThreadProcessId, DWORD, DEFAULT_UINT, (HWND arg1, LPDWORD arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, SetForegroundWindow, BOOL, DEFAULT_BOOL, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, GetSystemDpiForProcess, UINT, DEFAULT_UINT, (HANDLE arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, GetDpiForSystem, UINT, DEFAULT_UINT, (VOID), ())
__THUNK_API(__USER32_DLL_FILENAME, GetDC, HDC, DEFAULT_PTR, (HWND arg1), (arg1))
__THUNK_API(__USER32_DLL_FILENAME, ReleaseDC, int, DEFAULT_INT, (HWND arg1, HDC arg2), (arg1, arg2))
__THUNK_API(__USER32_DLL_FILENAME, InvalidateRect, BOOL, DEFAULT_BOOL, (HWND arg1, CONST RECT *arg2, BOOL arg3), (arg1, arg2, arg3))
__THUNK_API(__USER32_DLL_FILENAME, SystemParametersInfoForDpi, BOOL, DEFAULT_BOOL, (UINT arg1, UINT arg2, PVOID arg3, UINT arg4, UINT arg5), (arg1, arg2, arg3, arg4, arg5))
__THUNK_API(__USER32_DLL_FILENAME, AdjustWindowRectExForDpi, BOOL, DEFAULT_BOOL, (LPRECT arg1, DWORD arg2, BOOL arg3, DWORD arg4, UINT arg5), (arg1, arg2, arg3, arg4, arg5))
__THUNK_API(__USER32_DLL_FILENAME, IsProcessDPIAware, BOOL, DEFAULT_BOOL, (VOID), ())
__THUNK_API(__USER32_DLL_FILENAME, SetProcessDPIAware, BOOL, DEFAULT_BOOL, (VOID), ())
