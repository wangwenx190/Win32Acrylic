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

class AcrylicBrushWinUI3Private;

class AcrylicBrushWinUI3 final : public AcrylicBrush
{
    ACRYLICMANAGER_DISABLE_COPY_MOVE(AcrylicBrushWinUI3)

public:
    explicit AcrylicBrushWinUI3();
    ~AcrylicBrushWinUI3() override;

    [[nodiscard]] bool IsSupportedByCurrentOS() const override;
    [[nodiscard]] bool Initialize() const override;
    [[nodiscard]] bool RefreshBrush() const override;
    [[nodiscard]] HWND GetWindowHandle() const override;
    [[nodiscard]] int MessageLoop() const override;

private:
    std::unique_ptr<AcrylicBrushWinUI3Private> d_ptr;
};
