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

class CustomFrame
{
    ACRYLICMANAGER_DISABLE_COPY_MOVE(CustomFrame)

public:
    explicit CustomFrame();
    virtual ~CustomFrame();

    [[nodiscard]] HWND GetHandle() const noexcept;
    [[nodiscard]] int MessageLoop() const noexcept;

protected:
    [[nodiscard]] static std::wstring __RegisterWindowClass(const WNDPROC wndProc) noexcept;
    [[nodiscard]] static HWND __CreateWindow(const std::wstring &className, const DWORD style, const DWORD exStyle, const HWND parent, LPVOID data) noexcept;
    [[nodiscard]] std::wstring __GetWindowClassName() const noexcept;
    void __SetWindowClassName(const std::wstring &className) noexcept;
    void __SetWindowHandle(const HWND hWnd) noexcept;

    [[nodiscard]] virtual bool FilterMessage(const MSG *msg) const noexcept;

    static void OnNCCreate(const HWND hWnd, const LPARAM lParam) noexcept;
    static void OnNCDestroy(const HWND hWnd) noexcept;
    [[nodiscard]] static LRESULT OnNCCalcSize(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept;
    [[nodiscard]] static LRESULT OnNCHitTest(const HWND hWnd, const LPARAM lParam) noexcept;
    static void OnNCRButtonUp(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept;
    static void OnCreate(const HWND hWnd, const LPARAM lParam, UINT *dpi) noexcept;
    static void OnSize(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept;
    static void OnPaint(const HWND hWnd) noexcept;
    static void OnSettingChange(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept;
    static void OnDwmCompositionChanged() noexcept;
    static void OnDPIChanged(const HWND hWnd, const WPARAM wParam, const LPARAM lParam, UINT *newDpi) noexcept;
    static void OnClose(const HWND hWnd, const std::wstring &className) noexcept;
    static void OnDestroy() noexcept;

private:
    std::wstring m_windowClass = nullptr;
    HWND m_window = nullptr;
};

template <typename T>
struct CustomFrameT : public CustomFrame
{
public:
    [[nodiscard]] UINT GetCurrentDpi() const noexcept
    {
        return m_dpi;
    }

protected:
    using base_type = CustomFrameT<T>;

    [[nodiscard]] static T *GetThisFromHandle(const HWND hWnd) noexcept
    {
        return (hWnd ? reinterpret_cast<T *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)) : nullptr);
    }

    [[nodiscard]] bool CreateFramelessWindow() noexcept
    {
        const std::wstring className = __RegisterWindowClass(WindowProc);
        if (className.empty()) {
            OutputDebugStringW(L"Failed to register window class.");
            return false;
        }
        __SetWindowClassName(className);
        const HWND window = __CreateWindow(className, WS_OVERLAPPEDWINDOW, 0L, nullptr, this);
        if (!window) {
            OutputDebugStringW(L"Failed to create window.");
            // todo: unreg window class
            return false;
        }
        __SetWindowHandle(window);
        return true;
    }

    [[nodiscard]] static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (message == WM_NCCREATE) {
            OnNCCreate(hWnd, lParam);
        } else if (message == WM_NCDESTROY) {
            OnNCDestroy(hWnd);
        } else if (const auto that = GetThisFromHandle(hWnd)) {
            return that->MessageHandler(message, wParam, lParam);
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    [[nodiscard]] LRESULT MessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept
    {
        const HWND hWnd = GetHandle();
        switch (message) {
        case WM_NCCALCSIZE:
            return OnNCCalcSize(hWnd, wParam, lParam);
        case WM_NCHITTEST:
            return OnNCHitTest(hWnd, lParam);
        case WM_NCRBUTTONUP:
            OnNCRButtonUp(hWnd, wParam, lParam);
            break;
        case WM_CREATE:
            OnCreate(hWnd, lParam, &m_dpi);
            break;
        case WM_SIZE:
            OnSize(hWnd, wParam, lParam);
            break;
        case WM_PAINT:
            OnPaint(hWnd);
            break;
        case WM_SETTINGCHANGE:
            OnSettingChange(hWnd, wParam, lParam);
            break;
        case WM_DPICHANGED:
            OnDPIChanged(hWnd, wParam, lParam, &m_dpi);
            break;
        case WM_DWMCOMPOSITIONCHANGED:
            OnDwmCompositionChanged();
            break;
        case WM_CLOSE:
            OnClose(hWnd, __GetWindowClassName());
            break;
        case WM_DESTROY:
            OnDestroy();
            break;
        default:
            break;
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

private:
    UINT m_dpi = 0;
};
