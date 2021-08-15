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
    HighContrast
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

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_CreateWindow(
    _Out_ int      *idx,
    _In_ const int x = -1,
    _In_ const int y = -1,
    _In_ const int w = -1,
    _In_ const int h = -1
);

[[nodiscard]] ACRYLICMANAGER_API RECT WINAPI
am_GetWindowGeometry(
    _In_ const int idx
);

[[nodiscard]] ACRYLICMANAGER_API RECT WINAPI
am_SetWindowGeometry(
    _In_ const int idx,
    _In_ const int x,
    _In_ const int y,
    _In_ const int w,
    _In_ const int h
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_MoveWindow(
    _In_ const int idx,
    _In_ const int x,
    _In_ const int y
);

[[nodiscard]] ACRYLICMANAGER_API SIZE WINAPI
am_GetWindowSize(
    _In_ const int idx
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_ResizeWindow(
    _In_ const int idx,
    _In_ const int w,
    _In_ const int h
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_CenterWindow(
    _In_ const int idx
);

[[nodiscard]] ACRYLICMANAGER_API WindowState WINAPI
am_GetWindowState(
    _In_ const int idx
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetWindowState(
    _In_ const int         idx,
    _In_ const WindowState state
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_CloseWindow(
    _In_ const int idx
);

[[nodiscard]] ACRYLICMANAGER_API HWND WINAPI
am_GetWindowHandle(
    _In_ const int idx
);

[[nodiscard]] ACRYLICMANAGER_API SystemTheme WINAPI
am_GetBrushTheme(
    _In_ const int idx
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetBrushTheme(
    _In_ const int         idx,
    _In_ const SystemTheme theme
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_GetTintColor(
    _In_ const int idx,
    _Out_ int      *r,
    _Out_ int      *g,
    _Out_ int      *b,
    _Out_ int      *a
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetTintColor(
    _In_ const int idx,
    _In_ const int r,
    _In_ const int g,
    _In_ const int b,
    _In_ const int a
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_GetTintOpacity(
    _In_ const int idx,
    _Out_ double   *opacity
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetTintOpacity(
    _In_ const int    idx,
    _In_ const double opacity
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_GetTintLuminosityOpacity(
    _In_ const int idx,
    _Out_ double   *opacity
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetTintLuminosityOpacity(
    _In_ const int    idx,
    _In_ const double opacity
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_GetFallbackColor(
    _In_ const int idx,
    _Out_ int      *r,
    _Out_ int      *g,
    _Out_ int      *b,
    _Out_ int      *a
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetFallbackColor(
    _In_ const int idx,
    _In_ const int r,
    _In_ const int g,
    _In_ const int b,
    _In_ const int a
);

[[nodiscard]] ACRYLICMANAGER_API int WINAPI
am_EventLoopExec();

#ifdef __cplusplus
EXTERN_C_END
#endif
