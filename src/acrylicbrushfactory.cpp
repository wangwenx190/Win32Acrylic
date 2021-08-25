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

#include "acrylicbrushfactory.h"
#include "acrylicbrush_system.h"
#include "acrylicbrush_composition.h"
#include "acrylicbrush_winui3.h"
#include "acrylicbrush_winui2.h"
#include "acrylicbrush_direct2d.h"

#ifdef __cplusplus
EXTERN_C_START
#endif

constexpr IID IID_ACRYLICBRUSH_SYSTEM = {0x2f2c9d3d, 0xc9d1, 0x4218, {0x89, 0x15, 0xb5, 0x52, 0xfd, 0xa8, 0x69, 0x39}};
constexpr IID IID_ACRYLICBRUSH_COMPOSITION = {0xcfe22b95, 0x3e46, 0x4add, {0x81, 0x55, 0x89, 0x7b, 0xdb, 0x7b, 0xb3, 0x0}};
constexpr IID IID_ACRYLICBRUSH_WINUI3 = {0xe6ebc6a6, 0xfde7, 0x4f1e, {0x89, 0x96, 0xd7, 0xe7, 0x88, 0x25, 0x4a, 0x4d}};
constexpr IID IID_ACRYLICBRUSH_WINUI2 = {0xaf69d050, 0x500a, 0x417b, {0xa1, 0x8c, 0x32, 0x93, 0x89, 0x12, 0xac, 0x36}};
constexpr IID IID_ACRYLICBRUSH_DIRECT2D = {0x55269e0f, 0xdd0, 0x4ba1, {0x89, 0xf, 0x74, 0x68, 0x64, 0x55, 0xdc, 0x4e}};
constexpr IID IID_ACRYLICBRUSH_NULL = {0xb1c629f0, 0x3262, 0x4d75, {0xac, 0x75, 0xd2, 0xc7, 0x85, 0x3c, 0x38, 0x8f}};

#ifdef __cplusplus
EXTERN_C_END
#endif

HRESULT AcrylicBrushFactory::QueryInterface(REFIID riid, void **ppInterface)
{
    if (!ppInterface) {
        return E_INVALIDARG;
    }
    AcrylicBrush *brush = nullptr;
    if (riid == IID_ACRYLICBRUSH_SYSTEM) {
        brush = new AcrylicBrush_System;
    } else if (riid == IID_ACRYLICBRUSH_COMPOSITION) {
        brush = new AcrylicBrush_Composition;
    } else if (riid == IID_ACRYLICBRUSH_WINUI3) {
        brush = new AcrylicBrush_WinUI3;
    } else if (riid == IID_ACRYLICBRUSH_WINUI2) {
        brush = new AcrylicBrush_WinUI2;
    } else if (riid == IID_ACRYLICBRUSH_DIRECT2D) {
        brush = new AcrylicBrush_Direct2D;
    } else if (riid == IID_ACRYLICBRUSH_NULL) {
        return S_OK;
    } else {
        return E_FAIL; // fixme: use wrong iid error
    }
    if (brush->IsSupportedByCurrentOS()) {
        *ppInterface = brush;
        return S_OK;
    } else {
        brush->Release();
        brush = nullptr;
        return E_FAIL;
    }
}
