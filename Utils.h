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

#include <SDKDDKVer.h>
#include <Windows.h>

namespace Utils
{
    [[nodiscard]] HINSTANCE GetCurrentInstance() noexcept;
    [[nodiscard]] LPCWSTR GetWindowClassName(const ATOM atom) noexcept;
    [[nodiscard]] LPCWSTR GetSystemErrorMessage(LPCWSTR function, const DWORD code) noexcept;
    [[nodiscard]] LPCWSTR GetSystemErrorMessage(LPCWSTR function, const HRESULT hr) noexcept;
    [[nodiscard]] LPCWSTR GetSystemErrorMessage(LPCWSTR function) noexcept;
    void DisplayErrorDialog(LPCWSTR text) noexcept;
    [[nodiscard]] bool IsHighContrastModeEnabled() noexcept;
    [[nodiscard]] bool ShouldAppsUseDarkMode() noexcept;
    [[nodiscard]] LPCWSTR GenerateGUID() noexcept;
    [[nodiscard]] bool RefreshWindowTheme(const HWND hWnd) noexcept;
    [[nodiscard]] bool CloseWindow(const HWND hWnd, const ATOM atom) noexcept;
    [[nodiscard]] bool IsWindowMinimized(const HWND hWnd) noexcept;
    [[nodiscard]] bool IsWindowMaximized(const HWND hWnd) noexcept;
    [[nodiscard]] bool IsWindowFullScreen(const HWND hWnd) noexcept;
    [[nodiscard]] bool IsWindowNoState(const HWND hWnd) noexcept;
    [[nodiscard]] UINT GetWindowDPI(const HWND hWnd) noexcept;
    [[nodiscard]] UINT GetResizeBorderThickness(const HWND hWnd, const bool x) noexcept;
    [[nodiscard]] UINT GetCaptionHeight(const HWND hWnd) noexcept;
    [[nodiscard]] UINT GetTitleBarHeight(const HWND hWnd) noexcept;
    [[nodiscard]] UINT GetFrameBorderThickness(const HWND hWnd) noexcept;
    [[nodiscard]] HMONITOR GetWindowScreen(const HWND hWnd, const bool current) noexcept;
} // namespace Utils

#ifndef __PRINT_ERROR_MESSAGE
#define __PRINT_ERROR_MESSAGE(additionalMessage) \
    if (__error_message_from_os) { \
        Utils::DisplayErrorDialog(__error_message_from_os); \
        delete [] __error_message_from_os; \
        __error_message_from_os = nullptr; \
    } else { \
        OutputDebugStringW(additionalMessage); \
    }
#endif

#ifndef PRINT_WIN32_ERROR_MESSAGE
#define PRINT_WIN32_ERROR_MESSAGE(function, additionalMessage) \
    auto __error_message_from_os = Utils::GetSystemErrorMessage(L#function, GetLastError()); \
    __PRINT_ERROR_MESSAGE(additionalMessage)
#endif

#ifndef PRINT_HR_ERROR_MESSAGE
#define PRINT_HR_ERROR_MESSAGE(function, hresult, additionalMessage) \
    auto __error_message_from_os = Utils::GetSystemErrorMessage(L#function, hresult); \
    __PRINT_ERROR_MESSAGE(additionalMessage)
#endif
