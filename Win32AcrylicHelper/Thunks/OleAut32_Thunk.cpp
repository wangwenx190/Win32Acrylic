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

#ifndef _OLEAUT32_
#define _OLEAUT32_
#endif // _OLEAUT32_

#include "WindowsAPIThunks.h"

#include <OleAuto.h>

#ifndef __OLEAUT32_DLL_FILENAME
#define __OLEAUT32_DLL_FILENAME oleaut32.dll
#endif // __OLEAUT32_DLL_FILENAME

__THUNK_API(__OLEAUT32_DLL_FILENAME, SysFreeString, void, DEFAULT_VOID, (BSTR arg1), (arg1))
__THUNK_API(__OLEAUT32_DLL_FILENAME, SetErrorInfo, HRESULT, DEFAULT_HRESULT, (ULONG arg1, IErrorInfo *arg2), (arg1, arg2))
__THUNK_API(__OLEAUT32_DLL_FILENAME, GetErrorInfo, HRESULT, DEFAULT_HRESULT, (ULONG arg1, IErrorInfo **arg2), (arg1, arg2))
__THUNK_API(__OLEAUT32_DLL_FILENAME, SysAllocString, BSTR, DEFAULT_PTR, (const OLECHAR *arg1), (arg1))
__THUNK_API(__OLEAUT32_DLL_FILENAME, SysStringLen, UINT, DEFAULT_UINT, (BSTR arg1), (arg1))
