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

#include "WindowsAPIThunks.h"

#include <D3D11.h>

#ifndef __D3D11_DLL_FILENAME
#define __D3D11_DLL_FILENAME d3d11.dll
#endif // __D3D11_DLL_FILENAME

__THUNK_API(__D3D11_DLL_FILENAME, D3D11CreateDevice, HRESULT, DEFAULT_HRESULT, (IDXGIAdapter *arg1, D3D_DRIVER_TYPE arg2, HMODULE arg3, UINT arg4, CONST D3D_FEATURE_LEVEL *arg5, UINT arg6, UINT arg7, ID3D11Device **arg8, D3D_FEATURE_LEVEL *arg9, ID3D11DeviceContext **arg10), (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10))
