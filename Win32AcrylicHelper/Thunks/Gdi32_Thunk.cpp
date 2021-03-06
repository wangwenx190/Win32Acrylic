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

#ifndef _GDI32_
#define _GDI32_
#endif // _GDI32_

#include "WindowsAPIThunks.h"

#include <WinGdi.h>

#ifndef __GDI32_DLL_FILENAME
#define __GDI32_DLL_FILENAME gdi32.dll
#endif // __GDI32_DLL_FILENAME

__THUNK_API(__GDI32_DLL_FILENAME, GetStockObject, HGDIOBJ, DEFAULT_PTR, (int arg1), (arg1))
__THUNK_API(__GDI32_DLL_FILENAME, DeleteObject, BOOL, DEFAULT_BOOL, (HGDIOBJ arg1), (arg1))
__THUNK_API(__GDI32_DLL_FILENAME, CreateSolidBrush, HBRUSH, DEFAULT_PTR, (COLORREF arg1), (arg1))
__THUNK_API(__GDI32_DLL_FILENAME, GetDeviceCaps, int, DEFAULT_INT, (HDC arg1, int arg2), (arg1, arg2))
