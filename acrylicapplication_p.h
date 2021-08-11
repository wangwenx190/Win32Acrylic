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

enum class MessageBoxType : int
{
    Information = 0,
    Question,
    Warning,
    Error
};

class AcrylicApplicationPrivate
{
public:
    explicit AcrylicApplicationPrivate(const std::vector<std::wstring> &argv, AcrylicApplication *q_ptr);
    ~AcrylicApplicationPrivate();

    static void displayMessage(const MessageBoxType type, const std::wstring &title, const std::wstring &text);

    static int exec();

private:
    AcrylicApplicationPrivate(const AcrylicApplicationPrivate &) = delete;
    AcrylicApplicationPrivate &operator=(const AcrylicApplicationPrivate &) = delete;
    AcrylicApplicationPrivate(AcrylicApplicationPrivate &&) = delete;
    AcrylicApplicationPrivate &operator=(AcrylicApplicationPrivate &&) = delete;

private:
    [[nodiscard]] static bool isWindowFullScreened(const HWND hWnd);
    [[nodiscard]] static bool isWindowNoState(const HWND hWnd);
    [[nodiscard]] static double getDevicePixelRatio(const UINT dpi);
    [[nodiscard]] static int getResizeBorderThickness(const bool x, const UINT dpi);
    [[nodiscard]] static int getCaptionHeight(const UINT dpi);
    static void updateFrameMargins(const HWND hWnd);
    static void triggerFrameChange(const HWND hWnd);
    static LRESULT CALLBACK mainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    AcrylicApplication *q = nullptr;
    static const std::wstring mainWindowClassName;
    static const std::wstring mainWindowTitle;
    static UINT mainWindowDpi;
    static HWND mainWindowHandle;
    static HWND xamlIslandHandle;
    static bool mainWindowZoomed;
};
