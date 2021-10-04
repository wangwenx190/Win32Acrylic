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
#include <memory>
#include "Definitions.h"

class WindowPrivate;

class Window
{
public:
    explicit Window() noexcept;
    virtual ~Window() noexcept;

    [[nodiscard]] std::wstring Title() const noexcept;
    void Title(const std::wstring &value) noexcept;
    virtual void OnTitleChanged(const std::wstring &arg) noexcept;

    [[nodiscard]] int Icon() const noexcept;
    void Icon(const int value) noexcept;
    virtual void OnIconChanged(const int arg) noexcept;

    [[nodiscard]] int X() const noexcept;
    void X(const int value) noexcept;
    virtual void OnXChanged(const int arg) noexcept;

    [[nodiscard]] int Y() const noexcept;
    void Y(const int value) noexcept;
    virtual void OnYChanged(const int arg) noexcept;

    [[nodiscard]] UINT Width() const noexcept;
    void Width(const UINT value) noexcept;
    virtual void OnWidthChanged(const UINT arg) noexcept;

    [[nodiscard]] UINT Height() const noexcept;
    void Height(const UINT value) noexcept;
    virtual void OnHeightChanged(const UINT arg) noexcept;

    [[nodiscard]] WindowState Visibility() const noexcept;
    void Visibility(const WindowState value) noexcept;
    virtual void OnVisibilityChanged(const WindowState arg) noexcept;

    [[nodiscard]] WindowTheme Theme() const noexcept;
    virtual void OnThemeChanged(const WindowTheme arg) noexcept;

    [[nodiscard]] UINT DotsPerInch() const noexcept;
    virtual void OnDotsPerInchChanged(const UINT arg) noexcept;

    [[nodiscard]] COLORREF ColorizationColor() const noexcept;
    virtual void OnColorizationColorChanged(const COLORREF arg) noexcept;

    [[nodiscard]] WindowColorizationArea ColorizationArea() const noexcept;
    virtual void OnColorizationAreaChanged(const WindowColorizationArea arg) noexcept;

    [[nodiscard]] HWND CreateChildWindow(const DWORD style, const DWORD extendedStyle, const WNDPROC wndProc, void *extraData) const noexcept;
    [[nodiscard]] HWND WindowHandle() const noexcept;
    [[nodiscard]] int MessageLoop() const noexcept;
    [[nodiscard]] bool Move(const int x, const int y) const noexcept;
    [[nodiscard]] bool Resize(const UINT w, const UINT h) const noexcept;
    [[nodiscard]] bool SetGeometry(const int x, const int y, const UINT w, const UINT h) const noexcept;

    [[nodiscard]] virtual bool MessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) noexcept;
    [[nodiscard]] virtual bool FilterMessage(const MSG *msg) const noexcept;

    [[nodiscard]] inline friend bool operator==(const Window &lhs, const Window &rhs) noexcept {
        return (lhs.WindowHandle() == rhs.WindowHandle());
    }
    [[nodiscard]] inline friend bool operator!=(const Window &lhs, const Window &rhs) noexcept {
        return (lhs.WindowHandle() != rhs.WindowHandle());
    }

protected:
    [[nodiscard]] UINT GetInternalMetrics(const std::wstring &name) const noexcept;

private:
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) = delete;
    Window &operator=(Window &&) = delete;

private:
    friend class WindowPrivate;
    std::unique_ptr<WindowPrivate> d_ptr;
};
