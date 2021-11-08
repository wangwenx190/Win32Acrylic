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

#ifndef _ADVAPI32_
#define _ADVAPI32_
#endif // _ADVAPI32_

#include "WindowsAPIThunks.h"

#include <WinReg.h>

#ifndef __ADVAPI32_DLL_FILENAME
#define __ADVAPI32_DLL_FILENAME advapi32.dll
#endif // __ADVAPI32_DLL_FILENAME

__THUNK_API(__ADVAPI32_DLL_FILENAME, RegOpenKeyExW, LSTATUS, DEFAULT_INT, (HKEY arg1, LPCWSTR arg2, DWORD arg3, REGSAM arg4, PHKEY arg5), (arg1, arg2, arg3, arg4, arg5))
__THUNK_API(__ADVAPI32_DLL_FILENAME, RegQueryValueExW, LSTATUS, DEFAULT_INT, (HKEY arg1, LPCWSTR arg2, LPDWORD arg3, LPDWORD arg4, LPBYTE arg5, LPDWORD arg6), (arg1, arg2, arg3, arg4, arg5, arg6))
__THUNK_API(__ADVAPI32_DLL_FILENAME, RegCloseKey, LSTATUS, DEFAULT_INT, (HKEY arg1), (arg1))
