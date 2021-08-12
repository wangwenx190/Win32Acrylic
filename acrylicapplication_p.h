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

    static void print(const MessageType type, const std::wstring &title, const std::wstring &text);

    static int exec();

private:
    AcrylicApplicationPrivate(const AcrylicApplicationPrivate &) = delete;
    AcrylicApplicationPrivate &operator=(const AcrylicApplicationPrivate &) = delete;
    AcrylicApplicationPrivate(AcrylicApplicationPrivate &&) = delete;
    AcrylicApplicationPrivate &operator=(AcrylicApplicationPrivate &&) = delete;

private:
    [[nodiscard]] static UINT getWindowDpi();
    [[nodiscard]] static bool isWindowFullScreened();
    [[nodiscard]] static bool isWindowNoState();
    [[nodiscard]] static double getDevicePixelRatio();
    [[nodiscard]] static int getResizeBorderThickness(const bool x);
    [[nodiscard]] static int getCaptionHeight();
    [[nodiscard]] static int getTopFrameMargin();
    [[nodiscard]] static bool isCompositionEnabled();
    static void updateFrameMargins();
    static void triggerFrameChange();
    static void enableWindowTransitions();
    static LRESULT CALLBACK mainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    [[nodiscard]] bool registerMainWindowClass();
    [[nodiscard]] bool createMainWindow() const;
    [[nodiscard]] bool createXAMLIsland() const;
    void initialize();

private:
    AcrylicApplication *q = nullptr;
    static AcrylicApplicationPrivate *instance;
    static const std::wstring mainWindowClassName;
    static const std::wstring mainWindowTitle;
    static UINT mainWindowDpi;
    static HWND mainWindowHandle;
    static HWND xamlIslandHandle;
    std::vector<std::wstring> arguments = {};
    ATOM mainWindowAtom = 0;
};
