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

#pragma once

#include "acrylicbrush.h"

class ID2D1Effect;

class AcrylicBrush_Direct2D final : public AcrylicBrush
{
    // dis copy move
public:
    explicit AcrylicBrush_Direct2D();
    ~AcrylicBrush_Direct2D();

    [[nodiscard]] bool IsSupportedByCurrentOS() const override;
    [[nodiscard]] bool Create() const override;
    void ReloadBlurParameters() const override;
    [[nodiscard]] HWND GetWindowHandle() const override;
    [[nodiscard]] int EventLoopExec() const override;
    void Release() override;

private:
    [[nodiscard]] bool RegisterMainWindowClass() const;
    [[nodiscard]] bool CreateMainWindow() const;
    void ReloadDesktopParameters() const;
    [[nodiscard]] bool InitializeDirect2D() const;
    [[nodiscard]] bool EnsureWallpaperBrush() const;
    [[nodiscard]] bool EnsureNoiseBrush() const;
    [[nodiscard]] bool PrepareEffects_Luminosity(ID2D1Effect **output) const;
    [[nodiscard]] bool PrepareEffects_Legacy(ID2D1Effect **output) const;
    [[nodiscard]] bool CreateEffects(ID2D1Effect **output) const;
    [[nodiscard]] bool DrawFinalVisual() const;

private:
    static int m_refCount;
};
