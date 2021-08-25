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

#include "acrylicbrush_winui3.h"

int AcrylicBrush_WinUI3::m_refCount = 0;

AcrylicBrush_WinUI3::AcrylicBrush_WinUI3()
{
    ++m_refCount;
}

AcrylicBrush_WinUI3::~AcrylicBrush_WinUI3()
{
    --m_refCount;
}

bool AcrylicBrush_WinUI3::IsSupportedByCurrentOS() const
{
    static const bool result = false;
    return result;
}

bool AcrylicBrush_WinUI3::Create() const
{
    return false;
}

void AcrylicBrush_WinUI3::ReloadBlurParameters() const
{

}

HWND AcrylicBrush_WinUI3::GetWindowHandle() const
{
    return nullptr;
}

int AcrylicBrush_WinUI3::EventLoopExec() const
{
    return -1;
}

void AcrylicBrush_WinUI3::Release()
{
    --m_refCount;
    if (m_refCount <= 0) {
        m_refCount = 0;
        delete this;
    }
}
