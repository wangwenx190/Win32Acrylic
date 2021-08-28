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

#ifdef __cplusplus
EXTERN_C_START
#endif

[[nodiscard]] bool WINAPI
am_CreateWindow(
    _In_ const BrushType type,
    _Out_ LPWSTR *id
);

[[nodiscard]] bool WINAPI
am_DestroyWindow(
    _In_ LPCWSTR id
);

[[nodiscard]] RECT WINAPI
am_GetWindowGeometry(
    _In_ LPCWSTR id
);

[[nodiscard]] bool WINAPI
am_SetWindowGeometry(
    _In_ LPCWSTR id,
    _In_ const RECT geometry
);

[[nodiscard]] POINT WINAPI
am_GetWindowPos(
    _In_ LPCWSTR id
);

[[nodiscard]] bool WINAPI
am_SetWindowPos(
    _In_ LPCWSTR id,
    _In_ const POINT pos
);

[[nodiscard]] SIZE WINAPI
am_GetWindowSize(
    _In_ LPCWSTR id
);

[[nodiscard]] bool WINAPI
am_SetWindowSize(
    _In_ LPCWSTR id,
    _In_ const SIZE size
);

[[nodiscard]] WindowState WINAPI
am_GetWindowState(
    _In_ LPCWSTR id
);

[[nodiscard]] bool WINAPI
am_SetWindowState(
    _In_ LPCWSTR id,
    _In_ const WindowState state
);

[[nodiscard]] bool WINAPI
am_MoveToScreenCenter(
    _In_ LPCWSTR id
);

#ifdef __cplusplus
EXTERN_C_END
#endif
