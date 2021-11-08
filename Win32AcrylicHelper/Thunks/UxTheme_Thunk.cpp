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

#ifndef _UXTHEME_
#define _UXTHEME_
#endif // _UXTHEME_

#include "WindowsAPIThunks.h"

#include <UxTheme.h>

#ifndef __UXTHEME_DLL_FILENAME
#define __UXTHEME_DLL_FILENAME uxtheme.dll
#endif // __UXTHEME_DLL_FILENAME

__THUNK_API(__UXTHEME_DLL_FILENAME, SetWindowTheme, HRESULT, DEFAULT_HRESULT, (HWND arg1, LPCWSTR arg2, LPCWSTR arg3), (arg1, arg2, arg3))
__THUNK_API(__UXTHEME_DLL_FILENAME, BeginBufferedPaint, HPAINTBUFFER, DEFAULT_PTR, (HDC arg1, const RECT *arg2, BP_BUFFERFORMAT arg3, BP_PAINTPARAMS *arg4, HDC *arg5), (arg1, arg2, arg3, arg4, arg5))
__THUNK_API(__UXTHEME_DLL_FILENAME, BufferedPaintSetAlpha, HRESULT, DEFAULT_HRESULT, (HPAINTBUFFER arg1, const RECT *arg2, BYTE arg3), (arg1, arg2, arg3))
__THUNK_API(__UXTHEME_DLL_FILENAME, EndBufferedPaint, HRESULT, DEFAULT_HRESULT, (HPAINTBUFFER arg1, BOOL arg2), (arg1, arg2))
