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

#include "acrylicapplication.h"
#include <vector>
#include <string>

enum class MessageType : int
{
    Information = 0,
    Question,
    Warning,
    Error
};

enum class WindowsVersion : int
{
    WindowsVista = 0,
    Windows7,
    Windows8,
    Windows8_1,
    Windows10_1507,
    Windows10_1511,
    Windows10_1607,
    Windows10_1703,
    Windows10_1709,
    Windows10_1803,
    Windows10_1809,
    Windows10_1903,
    Windows10_1909,
    Windows10_2004,
    Windows10_20H2,
    Windows10_21H1,
    Windows11,
    Windows10 = Windows10_1507,
    Windows10_TH1 = Windows10_1507,
    Windows10_TH2 = Windows10_1511,
    Windows10_RS1 = Windows10_1607,
    Windows10_RS2 = Windows10_1703,
    Windows10_RS3 = Windows10_1709,
    Windows10_RS4 = Windows10_1803,
    Windows10_RS5 = Windows10_1809,
    Windows10_19H1 = Windows10_1903,
    Windows10_19H2 = Windows10_1909,
    Windows10_20H1 = Windows10_2004
    //Windows10_20H2 = Windows10_20H2,
    //Windows10_21H1 = Windows10_21H1,
};

enum class VersionCompare : int
{
    Less = 0,
    Equal,
    Greater,
    LessOrEqual,
    GreaterOrEqual
};

class AcrylicApplicationPrivate
{
public:
    explicit AcrylicApplicationPrivate(const std::vector<std::wstring> &args);
    ~AcrylicApplicationPrivate();

    [[nodiscard]] bool createWindow(const int x, const int y,
                                    const int w, const int h) const;
    [[nodiscard]] RECT getWindowGeometry() const;
    [[nodiscard]] bool moveWindow(const int x, const int y) const;
    [[nodiscard]] SIZE getWindowSize() const;
    [[nodiscard]] bool resizeWindow(const int w, const int h) const;
    [[nodiscard]] bool centerWindow() const;
    [[nodiscard]] WindowState getWindowState() const;
    [[nodiscard]] bool setWindowState(const WindowState state) const;
    [[nodiscard]] bool destroyWindow() const;
    [[nodiscard]] HWND getHandle() const;
    [[nodiscard]] SystemTheme getTheme() const;
    [[nodiscard]] bool setTheme(const SystemTheme theme) const;

    [[nodiscard]] static UINT getWindowDpi(const HWND hWnd);
    [[nodiscard]] static bool isWindowMinimized(const HWND hWnd);
    [[nodiscard]] static bool isWindowMaximized(const HWND hWnd);
    [[nodiscard]] static bool isWindowFullScreened(const HWND hWnd);
    [[nodiscard]] static bool isWindowNoState(const HWND hWnd);
    [[nodiscard]] static bool isWindowVisible(const HWND hWnd);
    [[nodiscard]] static bool isWindowHidden(const HWND hWnd);
    [[nodiscard]] static double getDevicePixelRatio(const UINT dpi);
    [[nodiscard]] static int getResizeBorderThickness(const bool x, const UINT dpi);
    [[nodiscard]] static int getCaptionHeight(const UINT dpi);
    [[nodiscard]] static int getTitleBarHeight(const HWND hWnd, const UINT dpi);
    [[nodiscard]] static RECT getScreenGeometry(const HWND hWnd);
    [[nodiscard]] static RECT getScreenAvailableGeometry(const HWND hWnd);
    [[nodiscard]] static bool isCompositionEnabled();
    [[nodiscard]] static bool triggerFrameChange(const HWND hWnd);
    [[nodiscard]] static bool setWindowTransitionsEnabled(const HWND hWnd, const bool enable);
    [[nodiscard]] static bool openSystemMenu(const HWND hWnd, const POINT pos);
    [[nodiscard]] static SystemTheme getSystemTheme();
    [[nodiscard]] static bool compareSystemVersion(const WindowsVersion ver,
                                                   const VersionCompare comp);

    [[nodiscard]] static int exec();
    static void print(const MessageType type, const std::wstring &text);

private:
    AcrylicApplicationPrivate(const AcrylicApplicationPrivate &) = delete;
    AcrylicApplicationPrivate &operator=(const AcrylicApplicationPrivate &) = delete;
    AcrylicApplicationPrivate(AcrylicApplicationPrivate &&) = delete;
    AcrylicApplicationPrivate &operator=(AcrylicApplicationPrivate &&) = delete;

private:
    [[nodiscard]] static int getTopFrameMargin(const HWND hWnd, const UINT dpi);
    [[nodiscard]] static bool updateFrameMargins(const HWND hWnd, const UINT dpi);
    [[nodiscard]] static MONITORINFO getMonitorInfo(const HWND hWnd);
    [[nodiscard]] static bool showWindowFullScreen(const HWND hWnd);

    static LRESULT CALLBACK mainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK dragBarWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    [[nodiscard]] bool registerMainWindowClass() const;
    [[nodiscard]] bool registerDragBarWindowClass() const;
    [[nodiscard]] bool createMainWindow(const int x, const int y,
                                        const int w, const int h) const;
    [[nodiscard]] bool createDragBarWindow() const;
    [[nodiscard]] bool createXAMLIsland() const;

private:
    static const std::wstring mainWindowClassName;
    static const std::wstring dragBarWindowClassName;

    static const std::wstring mainWindowTitle;
    static const std::wstring dragBarWindowTitle;

    static UINT mainWindowDpi;

    static HWND mainWindowHandle;
    static HWND dragBarWindowHandle;
    static HWND xamlIslandHandle;

    static ATOM mainWindowAtom;
    static ATOM dragBarWindowAtom;

    static std::vector<std::wstring> arguments;

    static SystemTheme acrylicTheme;
};
