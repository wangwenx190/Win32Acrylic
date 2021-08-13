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

#include <Windows.h>
#include <memory>

class AcrylicApplicationPrivate;

enum class SystemTheme : int
{
    Invalid = -1,
    Auto,
    Light,
    Dark,
    Default = Dark,
    HighContrast = Invalid
};

enum class WindowState : int
{
    Invalid = -1,
    Normal,
    Maximized,
    Minimized,
    FullScreened,
    Hidden,
    Shown
};

class AcrylicApplication
{
public:
    explicit AcrylicApplication(const int argc, wchar_t *argv[]);
    ~AcrylicApplication();

    [[nodiscard]] bool createWindow(const int x = -1, const int y = -1,
                                    const int w = -1, const int h = -1) const;
    [[nodiscard]] RECT getWindowGeometry() const;
    [[nodiscard]] bool moveWindow(const int x, const int y) const;
    [[nodiscard]] SIZE getWindowSize() const;
    [[nodiscard]] bool resizeWindow(const int w, const int h) const;
    [[nodiscard]] bool centerWindow() const;
    [[nodiscard]] WindowState getWindowState() const;
    [[nodiscard]] bool setWindowState(const WindowState state) const;
    [[nodiscard]] bool destroyWindow() const;
    [[nodiscard]] HWND getHandle() const;
    [[nodiscard]] SystemTheme getTheme() const;
    [[nodiscard]] bool setTheme(const SystemTheme theme) const;

    [[nodiscard]] static int exec();

private:
    AcrylicApplication(const AcrylicApplication &) = delete;
    AcrylicApplication &operator=(const AcrylicApplication &) = delete;
    AcrylicApplication(AcrylicApplication &&) = delete;
    AcrylicApplication &operator=(AcrylicApplication &&) = delete;

private:
    static AcrylicApplication *instance;

    std::unique_ptr<const AcrylicApplicationPrivate> d;
};
