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

#include "acrylicmanager_global.h"

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

#ifdef __cplusplus
EXTERN_C_BEGIN
#endif

ACRYLICMANAGER_API WINAPI [[nodiscard]] bool createWindow(const int x = -1, const int y = -1, const int w = -1, const int h = -1);
ACRYLICMANAGER_API WINAPI [[nodiscard]] RECT getWindowGeometry();
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool moveWindow(const int x, const int y);
ACRYLICMANAGER_API WINAPI [[nodiscard]] SIZE getWindowSize();
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool resizeWindow(const int w, const int h);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool centerWindow();
ACRYLICMANAGER_API WINAPI [[nodiscard]] WindowState getWindowState();
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool setWindowState(const WindowState state);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool destroyWindow();
ACRYLICMANAGER_API WINAPI [[nodiscard]] HWND getHandle();
ACRYLICMANAGER_API WINAPI [[nodiscard]] SystemTheme getAcrylicBrushTheme();
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool setAcrylicBrushTheme(const SystemTheme theme);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool getTintColor(int *r, int *g, int *b, int *a);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool setTintColor(const int r, const int g, const int b, const int a);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool getTintOpacity(double *opacity);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool setTintOpacity(const double opacity);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool getTintLuminosityOpacity(double *opacity);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool setTintLuminosityOpacity(const double opacity);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool getFallbackColor(int *r, int *g, int *b, int *a);
ACRYLICMANAGER_API WINAPI [[nodiscard]] bool setFallbackColor(const int r, const int g, const int b, const int a);
ACRYLICMANAGER_API WINAPI [[nodiscard]] int execEventLoop();

#ifdef __cplusplus
EXTERN_C_END
#endif
