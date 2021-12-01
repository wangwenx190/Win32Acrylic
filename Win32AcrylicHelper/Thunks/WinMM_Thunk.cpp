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

#ifndef _WINMM_
#define _WINMM_
#endif // _WINMM_

#include "WindowsAPIThunks.h"

#include <TimeApi.h>

#ifndef __WINMM_DLL_FILENAME
#define __WINMM_DLL_FILENAME winmm.dll
#endif // __WINMM_DLL_FILENAME

__THUNK_API(__WINMM_DLL_FILENAME, timeGetDevCaps, MMRESULT, DEFAULT_UINT, (LPTIMECAPS arg1, UINT arg2), (arg1, arg2))
__THUNK_API(__WINMM_DLL_FILENAME, timeBeginPeriod, MMRESULT, DEFAULT_UINT, (UINT arg1), (arg1))
__THUNK_API(__WINMM_DLL_FILENAME, timeEndPeriod, MMRESULT, DEFAULT_UINT, (UINT arg1), (arg1))
