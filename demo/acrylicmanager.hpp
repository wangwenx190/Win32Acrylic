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

using pam_CreateWindow = bool(WINAPI *)(const int, LPCWSTR *);
static pam_CreateWindow am_CreateWindow = nullptr;

using pam_DestroyWindow = bool(WINAPI *)(LPCWSTR);
static pam_DestroyWindow am_DestroyWindow = nullptr;

using pam_GetWindowGeometry = RECT(WINAPI *)(LPCWSTR);
static pam_GetWindowGeometry am_GetWindowGeometry = nullptr;

using pam_SetWindowGeometry = bool(WINAPI *)(LPCWSTR, const RECT);
static pam_SetWindowGeometry am_SetWindowGeometry = nullptr;

using pam_GetWindowPos = POINT(WINAPI *)(LPCWSTR);
static pam_GetWindowPos am_GetWindowPos = nullptr;

using pam_SetWindowPos = bool(WINAPI *)(LPCWSTR, const POINT);
static pam_SetWindowPos am_SetWindowPos = nullptr;

using pam_GetWindowSize = SIZE(WINAPI *)(LPCWSTR);
static pam_GetWindowSize am_GetWindowSize = nullptr;

using pam_SetWindowSize = bool(WINAPI *)(LPCWSTR);
static pam_SetWindowSize am_SetWindowSize = nullptr;

using pam_GetWindowState = int(WINAPI *)(LPCWSTR);
static pam_GetWindowState am_GetWindowState = nullptr;

using pam_SetWindowState = bool(WINAPI *)(LPCWSTR, const int);
static pam_SetWindowState am_SetWindowState = nullptr;

using pam_MoveToScreenCenter = bool(WINAPI *)(LPCWSTR);
static pam_MoveToScreenCenter am_MoveToScreenCenter = nullptr;

using pam_GetMessageLoopResult = int(WINAPI *)(LPCWSTR);
static pam_GetMessageLoopResult am_GetMessageLoopResult = nullptr;

[[nodiscard]] static inline bool InitializeAcrylicManager()
{
    return false;
}
