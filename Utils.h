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
#include <string>
#include "Definitions.h"

namespace Utils
{
    void DisplayErrorDialog(const std::wstring &text) noexcept;
    [[nodiscard]] HINSTANCE GetCurrentModuleInstance() noexcept;
    [[nodiscard]] HINSTANCE GetWindowInstance(const HWND hWnd) noexcept;
    [[nodiscard]] std::wstring GetSystemErrorMessage(const std::wstring &function, const DWORD code) noexcept;
    [[nodiscard]] std::wstring GetSystemErrorMessage(const std::wstring &function, const HRESULT hr) noexcept;
    [[nodiscard]] std::wstring GetSystemErrorMessage(const std::wstring &function) noexcept;
    [[nodiscard]] std::wstring GenerateGUID() noexcept;
    [[nodiscard]] bool OpenSystemMenu(const HWND hWnd, const POINT pos) noexcept;
    [[nodiscard]] bool UpdateFrameMargins(const HWND hWnd) noexcept;
    [[nodiscard]] WindowTheme GetSystemTheme() noexcept;
    [[nodiscard]] bool SetWindowTheme(const HWND hWnd, const WindowTheme theme) noexcept;
    [[nodiscard]] WindowState GetWindowState(const HWND hWnd) noexcept;
    [[nodiscard]] bool SetWindowState(const HWND hWnd, const WindowState state) noexcept;
    [[nodiscard]] DPIAwareness GetProcessDPIAwareness() noexcept;
    [[nodiscard]] bool SetProcessDPIAwareness(const DPIAwareness dpiAwareness) noexcept;
    [[nodiscard]] UINT GetWindowMetrics(const HWND hWnd, const WindowMetrics metrics) noexcept;
    [[nodiscard]] RECT GetScreenGeometry(const HWND hWnd) noexcept;
    [[nodiscard]] ColorizationArea GetColorizationArea() noexcept;
    [[nodiscard]] DWORD GetColorizationColor() noexcept;
    [[nodiscard]] std::wstring IntegerToString(const int num, const int radix) noexcept;
    [[nodiscard]] bool IsWindowFullScreen(const HWND hWnd) noexcept;
} // namespace Utils

#ifndef PRINT_WIN32_ERROR_MESSAGE
#define PRINT_WIN32_ERROR_MESSAGE(function, additionalMessage) \
    const DWORD __dw_error_code = GetLastError(); \
    const std::wstring __error_message_from_os = Utils::GetSystemErrorMessage(L#function, __dw_error_code); \
    Utils::DisplayErrorDialog(__error_message_from_os.empty() ? additionalMessage : __error_message_from_os);
#endif

#ifndef PRINT_HR_ERROR_MESSAGE
#define PRINT_HR_ERROR_MESSAGE(function, hresult, additionalMessage) \
    const std::wstring __error_message_from_os = Utils::GetSystemErrorMessage(L#function, hresult); \
    Utils::DisplayErrorDialog(__error_message_from_os.empty() ? additionalMessage : __error_message_from_os);
#endif
