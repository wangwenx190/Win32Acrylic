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
#include <cmath>
#include <string>

std::wstring __RegisterMyWindowClass(const WNDPROC wndProc);
HWND __CreateMyWindow(const std::wstring &className, LPVOID data);

class CustomFrame
{
    ACRYLICMANAGER_DISABLE_COPY_MOVE(CustomFrame)

public:
    explicit CustomFrame();
    ~CustomFrame();

protected:
    [[nodiscard]] int MessageLoop() const;

    [[nodiscard]] HWND GetWindowHandle() const;
    void SetWindowHandle(const HWND hWnd);

    static void OnNCCreate(const HWND hWnd, const LPARAM lParam) noexcept;
    static void OnNCDestroy(const HWND hWnd) noexcept;
    [[nodiscard]] static LRESULT OnNCCalcSize(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept;
    [[nodiscard]] static LRESULT OnNCHitTest(const HWND hWnd, const LPARAM lParam) noexcept;
    static void OnNCRButtonUp(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept;
    static void OnCreate(const HWND hWnd, const LPARAM lParam) noexcept;
    static void OnSize(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept;
    static void OnPaint(const HWND hWnd) noexcept;
    static void OnSettingChange(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept;
    static void OnDwmCompositionChanged() noexcept;
    static void OnClose(const HWND hWnd, const std::wstring &className) noexcept;
    static void OnDestroy() noexcept;

private:
    HWND m_window = nullptr;
};

template <typename T>
struct CustomFrameT : public CustomFrame
{
protected:
    using base_type = CustomFrameT<T>;

    [[nodiscard]] bool Create() noexcept
    {
        m_className = __RegisterMyWindowClass(WindowProc);
        if (m_className.empty()) {
            OutputDebugStringW(L"Failed to register window class.");
            return false;
        }
        const HWND hWnd = __CreateMyWindow(m_className, this);
        if (!hWnd) {
            OutputDebugStringW(L"Failed to create window.");
            return false;
        }
        SetWindowHandle(hWnd);
        return true;
    }

    [[nodiscard]] static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (message == WM_NCCREATE) {
            OnNCCreate(hWnd, lParam);
        } else if (message == WM_NCDESTROY) {
            OnNCDestroy(hWnd);
        } else if (const auto that = reinterpret_cast<T *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
            return that->MessageHandler(message, wParam, lParam);
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    [[nodiscard]] LRESULT MessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept
    {
        const HWND hWnd = GetWindowHandle();
        switch (message) {
        case WM_NCCALCSIZE:
            return OnNCCalcSize(hWnd, wParam, lParam);
        case WM_NCHITTEST:
            return OnNCHitTest(hWnd, lParam);
        case WM_NCRBUTTONUP:
            OnNCRButtonUp(hWnd, wParam, lParam);
            break;
        case WM_CREATE:
            OnCreate(hWnd, lParam);
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
            OnDPIChanged(hWnd, wParam, lParam);
            break;
        case WM_DWMCOMPOSITIONCHANGED:
            OnDwmCompositionChanged();
            break;
        case WM_CLOSE:
            OnClose(hWnd, m_className);
            break;
        case WM_DESTROY:
            OnDestroy();
            break;
        default:
            break;
        }
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    void OnDPIChanged(const HWND hWnd, const WPARAM wParam, const LPARAM lParam) noexcept
    {
        const auto x = static_cast<double>(LOWORD(wParam));
        const auto y = static_cast<double>(HIWORD(wParam));
        m_dpi = std::round((x + y) / 2.0);
        m_dpr = (static_cast<double>(m_dpi) / static_cast<double>(USER_DEFAULT_SCREEN_DPI));
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (MoveWindow(hWnd, prcNewWindow->left, prcNewWindow->top,
                       RECTWIDTH(*prcNewWindow), GET_RECT_HEIGHT(*prcNewWindow), TRUE) == FALSE) {
            //PRINT_WIN32_ERROR_MESSAGE(MoveWindow)
        }
    }

    [[nodiscard]] UINT GetCurrentDPI() const
    {
        return m_dpi;
    }

    [[nodiscard]] double GetCurrentDPR() const
    {
        return m_dpr;
    }

private:
    std::wstring m_className = nullptr;
    UINT m_dpi = 0;
    double m_dpr = 0.0;
};
