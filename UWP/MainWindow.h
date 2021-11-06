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

#include "Window.h"
#include "Color.hpp"

class MainWindowPrivate;

class MainWindow final : public Window
{
public:
    explicit MainWindow() noexcept;
    ~MainWindow() noexcept;

    [[nodiscard]] const Color &TintColor() const noexcept;
    void TintColor(const Color &value) noexcept;

    [[nodiscard]] double TintOpacity() const noexcept;
    void TintOpacity(const double value) noexcept;

    [[nodiscard]] double LuminosityOpacity() const noexcept;
    void LuminosityOpacity(const double value) noexcept;

    [[nodiscard]] const Color &FallbackColor() const noexcept;
    void FallbackColor(const Color &value) noexcept;

private:
    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    MainWindow(MainWindow &&) = delete;
    MainWindow &operator=(MainWindow &&) = delete;

private:
    friend class MainWindowPrivate;
    std::unique_ptr<MainWindowPrivate> d_ptr;
};
