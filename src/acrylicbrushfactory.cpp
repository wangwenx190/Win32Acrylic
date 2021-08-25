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
//#include "acrylicbrush_winui3.h"
#include "acrylicbrush_winui2.h"
#include "acrylicbrush_direct2d.h"

HRESULT AcrylicBrushFactory::QueryInterface(REFID iid, void **interface)
{
    if (!interface) {
        return E_INVALIDARG;
    }
    AcrylicBrush *brush = nullptr;
    if (iid == IID_ACRYLICBRUSH_SYSTEM) {
        brush = new AcrylicBrush_System;
    } else if (iid == IID_ACRYLICBRUSH_WINUI3) {
        //brush = new AcrylicBrush_WinUI3;
    } else if (iid == IID_ACRYLICBRUSH_WINUI2) {
        brush = new AcrylicBrush_WinUI2;
    } else if (iid == IID_ACRYLICBRUSH_DIRECT2D) {
        brush = new AcrylicBrush_Direct2D;
    } else if (iid == IID_ACRYLICBRUSH_NULL) {
        return S_OK;
    } else {
        return E_FAIL; // fixme: use wrong iid error
    }
    if (brush->IsSupportedByCurrentOS()) {
        *interface = brush;
        return S_OK;
    } else {
        brush->Release();
        brush = nullptr;
        return E_FAIL;
    }
}
