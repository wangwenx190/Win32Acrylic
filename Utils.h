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
#include "VersionNumber.hpp"
#include "Global.h"

[[maybe_unused]] constexpr VersionNumber Windows_2000               = VersionNumber( 5, 0,  2195);
[[maybe_unused]] constexpr VersionNumber Windows_XP                 = VersionNumber( 5, 1,  2600);
[[maybe_unused]] constexpr VersionNumber Windows_XP_64              = VersionNumber( 5, 2,  3790);
[[maybe_unused]] constexpr VersionNumber Windows_Vista              = VersionNumber( 6, 0,  6000);
[[maybe_unused]] constexpr VersionNumber Windows_Vista_ServicePack1 = VersionNumber( 6, 0,  6001);
[[maybe_unused]] constexpr VersionNumber Windows_Vista_ServicePack2 = VersionNumber( 6, 0,  6002);
[[maybe_unused]] constexpr VersionNumber Windows_7                  = VersionNumber( 6, 1,  7600);
[[maybe_unused]] constexpr VersionNumber Windows_7_ServicePack1     = VersionNumber( 6, 1,  7601);
[[maybe_unused]] constexpr VersionNumber Windows_8                  = VersionNumber( 6, 2,  9200);
[[maybe_unused]] constexpr VersionNumber Windows_8_1                = VersionNumber( 6, 3,  9200);
[[maybe_unused]] constexpr VersionNumber Windows_8_1_Update1        = VersionNumber( 6, 3,  9600);
[[maybe_unused]] constexpr VersionNumber Windows10_ThresHold1       = VersionNumber(10, 0, 10240);
[[maybe_unused]] constexpr VersionNumber Windows10_ThresHold2       = VersionNumber(10, 0, 10586);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone1        = VersionNumber(10, 0, 14393);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone2        = VersionNumber(10, 0, 15063);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone3        = VersionNumber(10, 0, 16299);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone4        = VersionNumber(10, 0, 17134);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone5        = VersionNumber(10, 0, 17763);
[[maybe_unused]] constexpr VersionNumber Windows10_19Half1          = VersionNumber(10, 0, 18362);
[[maybe_unused]] constexpr VersionNumber Windows10_19Half2          = VersionNumber(10, 0, 18363);
[[maybe_unused]] constexpr VersionNumber Windows10_20Half1          = VersionNumber(10, 0, 19041);
[[maybe_unused]] constexpr VersionNumber Windows10_20Half2          = VersionNumber(10, 0, 19042);
[[maybe_unused]] constexpr VersionNumber Windows10_21Half1          = VersionNumber(10, 0, 19043);

namespace Utils
{
    void DisplayErrorDialog(const std::wstring &text) noexcept;
    [[nodiscard]] HINSTANCE GetCurrentModuleInstance() noexcept;
    [[nodiscard]] HINSTANCE GetWindowInstance(const HWND hWnd) noexcept;
    [[nodiscard]] std::wstring GetSystemErrorMessage(const std::wstring &function, const DWORD code) noexcept;
    [[nodiscard]] std::wstring GetSystemErrorMessage(const std::wstring &function, const HRESULT hr) noexcept;
    [[nodiscard]] std::wstring GetSystemErrorMessage(const std::wstring &function) noexcept;
    [[nodiscard]] std::wstring GenerateGUID() noexcept;
    [[nodiscard]] bool CloseWindow(const HWND hWnd, const std::wstring &className) noexcept;
    [[nodiscard]] bool OpenSystemMenu(const HWND hWnd, const POINT pos) noexcept;
    [[nodiscard]] bool UpdateFrameMargins(const HWND hWnd) noexcept;
    [[nodiscard]] bool IsWindowsVersionOrGreater(const VersionNumber &version) noexcept;
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
} // namespace Utils

[[nodiscard]] inline bool IsWindows1019H1OrGreater() noexcept
{
    // Windows 10 Version 1903 (May 2019 Update)
    static const bool result = Utils::IsWindowsVersionOrGreater(Windows10_19Half1);
    return result;
}

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
