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
EXTERN_C_START
#endif

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI isAcrylicAvailable();
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI isAcrylicInitialized();
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI createWindow(const int x = -1, const int y = -1, const int w = -1, const int h = -1);
[[nodiscard]] ACRYLICMANAGER_API RECT WINAPI getWindowGeometry();
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI moveWindow(const int x, const int y);
[[nodiscard]] ACRYLICMANAGER_API SIZE WINAPI getWindowSize();
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI resizeWindow(const int w, const int h);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI centerWindow();
[[nodiscard]] ACRYLICMANAGER_API WindowState WINAPI getWindowState();
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI setWindowState(const WindowState state);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI destroyWindow();
[[nodiscard]] ACRYLICMANAGER_API HWND WINAPI getWindowHandle();
[[nodiscard]] ACRYLICMANAGER_API SystemTheme WINAPI getAcrylicBrushTheme();
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI setAcrylicBrushTheme(const SystemTheme theme);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI getTintColor(int *r, int *g, int *b, int *a);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI setTintColor(const int r, const int g, const int b, const int a);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI getTintOpacity(double *opacity);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI setTintOpacity(const double opacity);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI getTintLuminosityOpacity(double *opacity);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI setTintLuminosityOpacity(const double opacity);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI getFallbackColor(int *r, int *g, int *b, int *a);
[[nodiscard]] ACRYLICMANAGER_API bool WINAPI setFallbackColor(const int r, const int g, const int b, const int a);
[[nodiscard]] ACRYLICMANAGER_API int WINAPI acrylicEventLoop();

#ifdef __cplusplus
EXTERN_C_END
#endif
