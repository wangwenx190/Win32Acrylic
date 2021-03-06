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

#ifndef _OLE32_
#define _OLE32_
#endif // _OLE32_

#ifndef _COMBASEAPI_
#define _COMBASEAPI_
#endif // _COMBASEAPI_

#include "WindowsAPIThunks.h"

#include <ComBaseApi.h>

#ifndef __OLE32_DLL_FILENAME
#define __OLE32_DLL_FILENAME ole32.dll
#endif // __OLE32_DLL_FILENAME

__THUNK_API(__OLE32_DLL_FILENAME, CoCreateGuid, HRESULT, DEFAULT_HRESULT, (GUID FAR *arg1), (arg1))
__THUNK_API(__OLE32_DLL_FILENAME, StringFromGUID2, int, DEFAULT_INT, (REFGUID arg1, LPOLESTR arg2, int arg3), (arg1, arg2, arg3))
__THUNK_API(__OLE32_DLL_FILENAME, IIDFromString, HRESULT, DEFAULT_HRESULT, (LPCOLESTR arg1, LPIID arg2), (arg1, arg2))
__THUNK_API(__OLE32_DLL_FILENAME, CoIncrementMTAUsage, HRESULT, DEFAULT_HRESULT, (CO_MTA_USAGE_COOKIE *arg1), (arg1))
__THUNK_API(__OLE32_DLL_FILENAME, CoInitializeEx, HRESULT, DEFAULT_HRESULT, (LPVOID arg1, DWORD arg2), (arg1, arg2))
