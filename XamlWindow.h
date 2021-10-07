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

class XamlWindowPrivate;

class XamlWindow final : public Window
{
public:
    explicit XamlWindow() noexcept;
    ~XamlWindow() noexcept override;

protected:
    void OnWidthChanged(const UINT arg) noexcept override;
    void OnHeightChanged(const UINT arg) noexcept override;
    void OnVisibilityChanged(const WindowState arg) noexcept override;
    void OnThemeChanged(const WindowTheme arg) noexcept override;

    [[nodiscard]] bool MessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept override;

private:
    XamlWindow(const XamlWindow &) = delete;
    XamlWindow &operator=(const XamlWindow &) = delete;
    XamlWindow(XamlWindow &&) = delete;
    XamlWindow &operator=(XamlWindow &&) = delete;

private:
    friend class XamlWindowPrivate;
    std::unique_ptr<XamlWindowPrivate> d_ptr;
};
