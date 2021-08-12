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
#include <vector>
#include <string>

class AcrylicApplication;

enum class MessageType : int
{
    Information = 0,
    Question,
    Warning,
    Error
};

class AcrylicApplicationPrivate
{
public:
    explicit AcrylicApplicationPrivate(const std::vector<std::wstring> &args, AcrylicApplication *q_ptr);
    ~AcrylicApplicationPrivate();

    [[nodiscard]] bool createAcrylicWindow(const int x, const int y,
                                           const int w, const int h) const;

    [[nodiscard]] static int exec();

    [[nodiscard]] static UINT getWindowDpi(const HWND hWnd);
    [[nodiscard]] static bool isWindowFullScreened(const HWND hWnd);
    [[nodiscard]] static bool isWindowNoState(const HWND hWnd);
    [[nodiscard]] static double getDevicePixelRatio(const UINT dpi);
    [[nodiscard]] static int getResizeBorderThickness(const bool x, const UINT dpi);
    [[nodiscard]] static int getCaptionHeight(const UINT dpi);
    [[nodiscard]] static int getTitleBarHeight(const HWND hWnd, const UINT dpi);
    [[nodiscard]] static int getTopFrameMargin(const HWND hWnd, const UINT dpi);
    [[nodiscard]] static bool isCompositionEnabled();
    [[nodiscard]] static bool updateFrameMargins(const HWND hWnd, const UINT dpi);
    [[nodiscard]] static bool triggerFrameChange(const HWND hWnd);
    [[nodiscard]] static bool enableWindowTransitions(const HWND hWnd);
    [[nodiscard]] static bool openSystemMenu(const HWND hWnd, const POINT pos);
    static void print(const MessageType type, const std::wstring &title, const std::wstring &text);

private:
    AcrylicApplicationPrivate(const AcrylicApplicationPrivate &) = delete;
    AcrylicApplicationPrivate &operator=(const AcrylicApplicationPrivate &) = delete;
    AcrylicApplicationPrivate(AcrylicApplicationPrivate &&) = delete;
    AcrylicApplicationPrivate &operator=(AcrylicApplicationPrivate &&) = delete;

private:
    static LRESULT CALLBACK mainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK dragBarWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    [[nodiscard]] bool registerMainWindowClass() const;
    [[nodiscard]] bool registerDragBarWindowClass() const;
    [[nodiscard]] bool createMainWindow(const int x, const int y, const int w, const int h) const;
    [[nodiscard]] bool createDragBarWindow() const;
    [[nodiscard]] bool createXAMLIsland() const;

private:
    AcrylicApplication *q = nullptr;

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

    std::vector<std::wstring> arguments = {};
};
