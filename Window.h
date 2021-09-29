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

class Window
{
    // dis co
public:
    explicit Window() noexcept;
    ~Window() noexcept;

    [[nodiscard]] bool Create(const std::wstring &title, const std::wstring &className) noexcept;
    [[nodiscard]] bool CreateChild(const WNDPROC WndProc) noexcept;
    [[nodiscard]] bool Destroy() noexcept;

    [[nodiscard]] HWND WindowHandle() const noexcept;
    [[nodiscard]] std::wstring ClassName() const noexcept;

    [[nodiscard]] int MessageLoop() const noexcept;

    [[nodiscard]] int X() const noexcept;
    void X(const int value) noexcept;

    [[nodiscard]] int Y() const noexcept;
    void Y(const int value) noexcept;

    [[nodiscard]] UINT Width() const noexcept;
    void Width(const UINT value) noexcept;

    [[nodiscard]] UINT Height() const noexcept;
    void Height(const UINT value) noexcept;

    [[nodiscard]] WindowState Visibility() const noexcept;
    void Visibility(const WindowState value) noexcept;

    [[nodiscard]] WindowTheme Theme() const noexcept;
    void Theme(const WindowTheme value) noexcept;

    [[nodiscard]] bool Move(const int x, const int y) noexcept;
    [[nodiscard]] bool Resize(const UINT w, const UINT h) noexcept;
    [[nodiscard]] bool SetGeometry(const int x, const int y, const UINT w, const UINT h) noexcept;

protected:
    [[nodiscard]] virtual LRESULT MessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept = 0;
    [[nodiscard]] virtual bool FilterMessage(const MSG *msg) const noexcept = 0;

private:
    [[nodiscard]] static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    [[nodiscard]] LRESULT DefaultMessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept;
    [[nodiscard]] bool CreateWindow2() noexcept;
    [[nodiscard]] bool CloseWindow() noexcept;

private:
    HWND m_window = nullptr;
    std::wstring m_class = {};
    int m_x = 0;
    int m_y = 0;
    UINT m_width = 0;
    UINT m_height = 0;
    WindowState m_visibility = WindowState::Normal;
    WindowTheme m_theme = WindowTheme::Light;
    DWORD m_savedWindowStyles = 0;
    DWORD m_savedExtendedWindowStyles = 0;
    RECT m_savedWindowGeometry = {};
};
