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

#ifndef _DWMAPI_
#define _DWMAPI_
#endif // _DWMAPI_

#include "WindowsAPIThunks.h"

#include <DwmApi.h>

#ifndef __DWMAPI_DLL_FILENAME
#define __DWMAPI_DLL_FILENAME dwmapi.dll
#endif // __DWMAPI_DLL_FILENAME

__THUNK_API(__DWMAPI_DLL_FILENAME, DwmGetColorizationColor, HRESULT, DEFAULT_HRESULT, (DWORD *arg1, BOOL *arg2), (arg1, arg2))
__THUNK_API(__DWMAPI_DLL_FILENAME, DwmSetWindowAttribute, HRESULT, DEFAULT_HRESULT, (HWND arg1, DWORD arg2, LPCVOID arg3, DWORD arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__DWMAPI_DLL_FILENAME, DwmGetWindowAttribute, HRESULT, DEFAULT_HRESULT, (HWND arg1, DWORD arg2, PVOID arg3, DWORD arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__DWMAPI_DLL_FILENAME, DwmExtendFrameIntoClientArea, HRESULT, DEFAULT_HRESULT, (HWND arg1, const MARGINS *arg2), (arg1, arg2))
__THUNK_API(__DWMAPI_DLL_FILENAME, DwmFlush, HRESULT, DEFAULT_HRESULT, (VOID), ())
__THUNK_API(__DWMAPI_DLL_FILENAME, DwmIsCompositionEnabled, HRESULT, DEFAULT_HRESULT, (BOOL *arg1), (arg1))
