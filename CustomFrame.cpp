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

#include "CustomFrame.h"
#include "SystemLibraryManager.h"

Window::Window() noexcept
{

}

Window::~Window() noexcept
{

}

bool Window::Create(const std::wstring &title, const std::wstring &className) noexcept
{

}

bool Window::CreateChild(const WNDPROC WndProc) noexcept
{

}

bool Window::Destroy() noexcept
{

}

HWND Window::WindowHandle() const noexcept
{
    return m_window;
}

std::wstring Window::ClassName() const noexcept
{
    return m_class;
}

int Window::MessageLoop() const noexcept
{

}

int Window::X() const noexcept
{
    return m_x;
}

void Window::X(const int value) noexcept
{

}

int Window::Y() const noexcept
{
    return m_y;
}

void Window::Y(const int value) noexcept
{

}

UINT Window::Width() const noexcept
{
    return m_width;
}

void Window::Width(const UINT value) noexcept
{

}

UINT Window::Height() const noexcept
{
    return m_height;
}

void Window::Height(const UINT value) noexcept
{

}

WindowState Window::Visibility() const noexcept
{
    return m_visibility;
}

void Window::Visibility(const WindowState value) noexcept
{

}

bool Window::Move(const int x, const int y) noexcept
{
    return SetGeometry(x, y, m_width, m_height);
}

bool Window::Resize(const UINT w, const UINT h) noexcept
{
    return SetGeometry(m_x, m_y, w, h);
}

bool Window::SetGeometry(const int x, const int y, const UINT w, const UINT h) noexcept
{
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        static constexpr UINT flags = (SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER);
        if (SetWindowPosFunc(m_window, nullptr, x, y, w, h, flags) == FALSE) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

LRESULT Window::DefaultMessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    return MessageHandler(message, wParam, lParam);
}

LRESULT Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    USER32_API(SetWindowLongPtrW);
    USER32_API(GetWindowLongPtrW);
    USER32_API(DefWindowProcW);
    if (SetWindowLongPtrWFunc && GetWindowLongPtrWFunc && DefWindowProcWFunc) {
        if (message == WM_NCCREATE) {
            const auto cs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            const auto that = static_cast<Window *>(cs->lpCreateParams);
            //that->m_window.reset(window); // take ownership of the window
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
                //
            }
        } else if (message == WM_NCDESTROY) {
            if (SetWindowLongPtrWFunc(hWnd, GWLP_USERDATA, 0) == 0) {
                //
            }
        } else if (const auto that = reinterpret_cast<Window *>(GetWindowLongPtrWFunc(hWnd, GWLP_USERDATA))) {
            return that->DefaultMessageHandler(message, wParam, lParam);
        } else {
            return DefWindowProcWFunc(hWnd, message, wParam, lParam);
        }
    } else {
        //
    }
}
