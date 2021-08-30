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

namespace Utils
{

[[nodiscard]] bool CompareSystemVersion(const WindowsVersion ver, const VersionCompare comp);
[[nodiscard]] bool IsWindows7OrGreater();
[[nodiscard]] bool IsWindows8OrGreater();
[[nodiscard]] bool IsWindows8Point1OrGreater();
[[nodiscard]] bool IsWindows10OrGreater();
[[nodiscard]] bool IsWindows10RS1OrGreater();
[[nodiscard]] bool IsWindows10RS2OrGreater();
[[nodiscard]] bool IsWindows10RS5OrGreater();
[[nodiscard]] bool IsWindows1019H1OrGreater();
[[nodiscard]] bool IsWindows11OrGreater();
[[nodiscard]] LPCWSTR GetCurrentDirectoryPath();
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
[[nodiscard]] LPCWSTR GetWallpaperFilePath(const int screen);
[[nodiscard]] COLORREF GetDesktopBackgroundColor(const int screen);
[[nodiscard]] WallpaperAspectStyle GetWallpaperAspectStyle(const int screen);
[[nodiscard]] LPCWSTR GetStringFromEnvironmentVariable(LPCWSTR name);
[[nodiscard]] int GetIntFromEnvironmentVariable(LPCWSTR name);
[[nodiscard]] bool GetBoolFromEnvironmentVariable(LPCWSTR name);
[[nodiscard]] LPCWSTR GetStringFromIniFile(LPCWSTR file, LPCWSTR section, LPCWSTR key);
[[nodiscard]] int GetIntFromIniFile(LPCWSTR file, LPCWSTR section, LPCWSTR key);
[[nodiscard]] bool GetBoolFromIniFile(LPCWSTR file, LPCWSTR section, LPCWSTR key);
[[nodiscard]] LPCWSTR GetStringFromRegistry(const HKEY rootKey, LPCWSTR subKey, LPCWSTR key);
[[nodiscard]] int GetIntFromRegistry(const HKEY rootKey, LPCWSTR subKey, LPCWSTR key);
[[nodiscard]] DpiAwareness GetDpiAwarenessForWindow(const HWND hWnd);
[[nodiscard]] bool SetDpiAwarenessForWindow(const HWND hWnd, const DpiAwareness awareness);
[[nodiscard]] LPCWSTR TranslateErrorCodeToMessage(LPCWSTR function, const HRESULT hr);
[[nodiscard]] LPCWSTR GenerateGUID();
[[nodiscard]] bool UpdateFrameMargins(const HWND hWnd);
[[nodiscard]] bool OpenSystemMenu(const HWND hWnd, const POINT pos);
[[nodiscard]] bool IsCompositionEnabled();
[[nodiscard]] bool SetWindowExcludedFromLivePreview(const HWND hWnd, const bool enable);
[[nodiscard]] bool RemoveWindowFromTaskListAndTaskBar(const HWND hWnd);
[[nodiscard]] bool LoadResourceData(LPCWSTR name, LPCWSTR type, void **data, DWORD *dataSize);

}
