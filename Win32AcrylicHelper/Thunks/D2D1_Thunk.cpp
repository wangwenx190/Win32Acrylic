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

#include <D2D1.h>

static constexpr const wchar_t __D2D1_DLL_FILENAME[] = L"D2D1.dll";

EXTERN_C HRESULT WINAPI
D2D1CreateFactory(
    D2D1_FACTORY_TYPE          factoryType,
    REFIID                     riid,
    const D2D1_FACTORY_OPTIONS *pFactoryOptions,
    void                       **ppIFactory
)
{
    static const auto function = reinterpret_cast<HRESULT(WINAPI *)(D2D1_FACTORY_TYPE, REFIID, const D2D1_FACTORY_OPTIONS *, void **)>(GetWindowsAPIByName(__D2D1_DLL_FILENAME, L"D2D1CreateFactory"));
    return (function ? function(factoryType, riid, pFactoryOptions, ppIFactory) : DEFAULT_HRESULT);
}