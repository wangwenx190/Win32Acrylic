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

#include "acrylicbrush_system.h"
#include "customframe.h"
#include "utils.h"
#include <DwmApi.h>

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

using SetWindowCompositionAttributeSignature = BOOL(WINAPI *)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);

static constexpr wchar_t g_mainWindowTitle[] = L"AcrylicManager System Main Window";

static int g_refCount = 0;

static HMODULE g_user32Module = nullptr;
static SetWindowCompositionAttributeSignature g_setWindowCompositionAttributePfn = nullptr;

[[nodiscard]] static inline bool IsAcrylicBlurAvailableForWin32()
{
    static const bool result = (Utils::IsWindows11OrGreater()
                                || (Utils::CompareSystemVersion(WindowsVersion::Windows10_RS2, VersionCompare::Greater)
                                    && Utils::CompareSystemVersion(WindowsVersion::Windows10_RS4, VersionCompare::Less)));
    return result;
}

class AcrylicBrushSystemPrivate
{
    ACRYLICMANAGER_DISABLE_COPY_MOVE(AcrylicBrushSystemPrivate)

public:
    explicit AcrylicBrushSystemPrivate(AcrylicBrushSystem *q);
    ~AcrylicBrushSystemPrivate();

    static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    [[nodiscard]] bool CreateMainWindow();
    [[nodiscard]] HWND GetMainWindowHandle() const;
    [[nodiscard]] int EventLoop() const;
    void ReloadBrushParameters();
    void Cleanup();
    [[nodiscard]] bool InitializeInternalAPI() const;
    [[nodiscard]] bool SetBlurBehindWindowEnabled(const bool enable, const winrt::Windows::UI::Color &color) const;

private:
    [[nodiscard]] LRESULT MainWindowMessageHandler(UINT message, WPARAM wParam, LPARAM lParam);

private:
    AcrylicBrushSystem *q_ptr = nullptr;

    std::wstring m_mainWindowClassName = nullptr;
    HWND m_mainWindowHandle = nullptr;
    UINT m_currentDpi = 0;
    double m_currentDpr = 0.0;
};

AcrylicBrushSystemPrivate::AcrylicBrushSystemPrivate(AcrylicBrushSystem *q)
{
    q_ptr = q;
}

AcrylicBrushSystemPrivate::~AcrylicBrushSystemPrivate()
{
}

void AcrylicBrushSystemPrivate::Cleanup()
{
    if (m_mainWindowHandle) {
        DestroyWindow(m_mainWindowHandle);
        m_mainWindowHandle = nullptr;
    }
    if (!m_mainWindowClassName.empty()) {
        UnregisterClassW(m_mainWindowClassName.c_str(), GET_CURRENT_INSTANCE);
        m_mainWindowClassName.clear();
    }
    if (m_currentDpi != 0) {
        m_currentDpi = 0;
    }
    if (m_currentDpr != 0.0) {
        m_currentDpr = 0.0;
    }
}

LRESULT CALLBACK AcrylicBrushSystemPrivate::MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE) {
        const auto cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        const auto that = static_cast<AcrylicBrushSystemPrivate *>(cs->lpCreateParams);
        if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
        }
    } else if (message == WM_NCDESTROY) {
        if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
        }
    } else if (const auto that = reinterpret_cast<AcrylicBrushSystemPrivate *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
        return that->MainWindowMessageHandler(message, wParam, lParam);
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

LRESULT AcrylicBrushSystemPrivate::MainWindowMessageHandler(UINT message, WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    SecureZeroMemory(&msg, sizeof(msg));
    msg.hwnd = m_mainWindowHandle;
    msg.message = message;
    msg.wParam = wParam;
    msg.lParam = lParam;
    const DWORD pos = GetMessagePos();
    msg.pt = {GET_X_LPARAM(pos), GET_Y_LPARAM(pos)};
    msg.time = GetMessageTime();
    LRESULT result = 0;
    if (CustomFrame::HandleWindowProc(&msg, &result)) {
        return result;
    }

    bool themeChanged = false;

    switch (message) {
    default:
        break;
    }

    return DefWindowProcW(m_mainWindowHandle, message, wParam, lParam);
}

bool AcrylicBrushSystemPrivate::CreateMainWindow()
{
    m_mainWindowClassName = Utils::RegisterWindowClass(MainWindowProc);
    if (m_mainWindowClassName.empty()) {
        OutputDebugStringW(L"Failed to register main window class.");
        return false;
    }

    m_mainWindowHandle = CreateWindowExW(0L,
                                         m_mainWindowClassName.c_str(),
                                         g_mainWindowTitle,
                                         WS_OVERLAPPEDWINDOW,
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                         nullptr,
                                         nullptr,
                                         GET_CURRENT_INSTANCE,
                                         this);

    if (!m_mainWindowHandle) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW)
        return false;
    }

    m_currentDpi = Utils::GetDotsPerInchForWindow(m_mainWindowHandle);
    if (m_currentDpi <= 0) {
        m_currentDpi = USER_DEFAULT_SCREEN_DPI;
    }
    m_currentDpr = Utils::GetDevicePixelRatioForWindow(m_mainWindowHandle);
    if (m_currentDpr <= 0.0) {
        m_currentDpr = 1.0;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (!Utils::UpdateFrameMargins(m_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to update main window's frame margins.");
        return false;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (!Utils::TriggerFrameChangeForWindow(m_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to trigger frame change event for main window.");
        return false;
    }
    // Ensure our window still has window transitions.
    if (!Utils::SetWindowTransitionsEnabled(m_mainWindowHandle, true)) {
        OutputDebugStringW(L"Failed to enable window transitions for main window.");
        return false;
    }

    return true;
}

bool AcrylicBrushSystemPrivate::InitializeInternalAPI() const
{
    static bool tried = false;
    if (tried) {
        return false;
    }
    tried = true;
    g_user32Module = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!g_user32Module) {
        PRINT_WIN32_ERROR_MESSAGE(LoadLibraryExW)
        return false;
    }
    g_setWindowCompositionAttributePfn = reinterpret_cast<SetWindowCompositionAttributeSignature>(GetProcAddress(g_user32Module, "SetWindowCompositionAttribute"));
    if (!g_setWindowCompositionAttributePfn) {
        PRINT_WIN32_ERROR_MESSAGE(GetProcAddress)
        return false;
    }
    return true;
}

bool AcrylicBrushSystemPrivate::SetBlurBehindWindowEnabled(const bool enable, const winrt::Windows::UI::Color &color) const
{
    // We prefer using DwmEnableBlurBehindWindow() on Windows 7.
    if (Utils::IsWindows8OrGreater()) {
        ACCENT_POLICY policy = {};
        policy.Flags = 2;
        if (enable) {
            // The gradient color must be set otherwise it'll look like a classic blur.
            // Use semi-transparent gradient color to get better appearance.
            policy.GradientColor = qRgba(gradientColor.blue(), gradientColor.green(), gradientColor.red(), gradientColor.alpha());
            if (IsAcrylicBlurAvailableForWin32()) {
                policy.State = ACCENT_ENABLE_ACRYLICBLURBEHIND;
            } else {
                policy.State = ACCENT_ENABLE_BLURBEHIND;
            }
        } else {
            policy.State = ACCENT_DISABLED;
            policy.GradientColor = 0x01FFFFFF;
        }
        WINDOWCOMPOSITIONATTRIBDATA data = {};
        data.Attrib = WCA_ACCENT_POLICY;
        data.pvData = &policy;
        data.cbData = sizeof(policy);
        if (g_setWindowCompositionAttributePfn(m_mainWindowHandle, &data) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowCompositionAttribute)
            return false;
        }
        return true;
    } else {
        DWM_BLURBEHIND dwmBB;
        SecureZeroMemory(&dwmBB, sizeof(dwmBB));
        dwmBB.dwFlags = DWM_BB_ENABLE;
        dwmBB.fEnable = (enable ? TRUE : FALSE);
        if (enable) {
            dwmBB.dwFlags |= DWM_BB_BLURREGION;
            const HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
            if (hRgn) {
                dwmBB.hRgnBlur = hRgn;
            } else {
                PRINT_WIN32_ERROR_MESSAGE(CreateRectRgn)
            }
        }
        const HRESULT hr = DwmEnableBlurBehindWindow(m_mainWindowHandle, &dwmBB);
        if (dwmBB.hRgnBlur) {
            if (DeleteObject(dwmBB.hRgnBlur) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(DeleteObject)
                return false;
            }
        }
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmEnableBlurBehindWindow, hr)
            return false;
        }
        return true;
    }
}

int AcrylicBrushSystemPrivate::EventLoop() const
{
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

AcrylicBrushSystem::AcrylicBrushSystem()
{
    d_ptr = std::make_unique<AcrylicBrushSystemPrivate>(this);
}

AcrylicBrushSystem::~AcrylicBrushSystem()
{
}

int AcrylicBrushSystem::AddRef() const
{
    ++g_refCount;
}

void AcrylicBrushSystem::Release()
{
    --g_refCount;
    if (g_refCount <= 0) {
        g_refCount = 0;
        delete this;
    }
}

bool AcrylicBrushSystem::IsSupportedByCurrentOS() const
{
    static const bool result = Utils::IsWindows7OrGreater();
    return result;
}

bool AcrylicBrushSystem::Create() const
{
    return false;
}

bool AcrylicBrushSystem::Destroy() const
{
    d_ptr->Cleanup();
    return true;
}

HWND AcrylicBrushSystem::GetWindowHandle() const
{
    return d_ptr->GetMainWindowHandle();
}

bool AcrylicBrushSystem::RefreshBrush() const
{
    d_ptr->ReloadBrushParameters();
    return true;
}

int AcrylicBrushSystem::EventLoop() const
{
    return d_ptr->EventLoop();
}
