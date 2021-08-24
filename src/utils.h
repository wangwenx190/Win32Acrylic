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
#include <string>

namespace Utils
{

[[nodiscard]] bool IsWindows7OrGreater();
[[nodiscard]] bool IsWindows8OrGreater();
[[nodiscard]] bool IsWindows8Point1OrGreater();
[[nodiscard]] bool IsWindows10OrGreater();
[[nodiscard]] bool IsWindows10_1607OrGreater();
[[nodiscard]] bool IsWindows10_19H1OrGreater();
[[nodiscard]] bool IsWindows11OrGreater();
[[nodiscard]] std::wstring GetCurrentDirectoryPath();
[[nodiscard]] UINT GetDotsPerInchForWindow(const HWND hWnd);
[[nodiscard]] double GetDevicePixelRatioForWindow(const HWND hWnd);
[[nodiscard]] int GetResizeBorderThickness(const HWND hWnd);
[[nodiscard]] int GetCaptionHeight(const HWND hWnd);
[[nodiscard]] int GetTitleBarHeight(const HWND hWnd);
[[nodiscard]] int GetWindowVisibleFrameBorderThickness(const HWND hWnd);
[[nodiscard]] bool ShouldAppsUseDarkMode();
[[nodiscard]] bool ShouldSystemUsesDarkMode();
[[nodiscard]] COLORREF GetColorizationColor();
[[nodiscard]] ColorizationArea GetColorizationArea();
[[nodiscard]] bool IsHighContrastModeEnabled();
[[nodiscard]] bool IsWindowDarkFrameBorderEnabled(const HWND hWnd);
[[nodiscard]] bool SetWindowDarkFrameBorderEnabled(const HWND hWnd, const bool enable);
[[nodiscard]] bool IsWindowTransitionsEnabled(const HWND hWnd);
[[nodiscard]] bool SetWindowTransitionsEnabled(const HWND hWnd, const bool enable);
[[nodiscard]] std::wstring GetWallpaperFilePath(const int screen);
[[nodiscard]] COLORREF GetDesktopBackgroundColor(const int screen);
[[nodiscard]] WallpaperAspectStyle GetWallpaperAspectStyle(const int screen);
[[nodiscard]] std::wstring GetStringFromEnvironmentVariable(const std::wstring &name);
[[nodiscard]] int GetIntFromEnvironmentVariable(const std::wstring &name);
[[nodiscard]] bool GetBoolFromEnvironmentVariable(const std::wstring &name);
[[nodiscard]] std::wstring GetStringFromIniFile(const std::wstring &file, const std::wstring &section, const std::wstring &key);
[[nodiscard]] int GetIntFromIniFile(const std::wstring &file, const std::wstring &section, const std::wstring &key);
[[nodiscard]] bool GetBoolFromIniFile(const std::wstring &file, const std::wstring &section, const std::wstring &key);
[[nodiscard]] std::wstring GetStringFromRegistry(const HKEY rootKey, const std::wstring &subKey, const std::wstring &key);
[[nodiscard]] int GetIntFromRegistry(const HKEY rootKey, const std::wstring &subKey, const std::wstring &key);
[[nodiscard]] DpiAwareness GetDpiAwarenessForWindow(const HWND hWnd);
[[nodiscard]] bool SetDpiAwarenessForWindow(const HWND hWnd, const DpiAwareness awareness);
[[nodiscard]] std::wstring TranslateErrorCodeToMessage(const std::wstring &function, const HRESULT hr);
[[nodiscard]] std::wstring GenerateGUID();
[[nodiscard]] bool TriggerFrameChangeForWindow(const HWND hWnd);
[[nodiscard]] bool UpdateFrameMargins(const HWND hWnd);
[[nodiscard]] bool OpenSystemMenu(const HWND hWnd, const POINT pos);

}
