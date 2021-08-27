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

static HMODULE g_user32Module = nullptr;

using SetWindowCompositionAttributeSignature = BOOL(WINAPI *)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);
static SetWindowCompositionAttributeSignature g_setWindowCompositionAttributePfn = nullptr;

[[nodiscard]] static inline bool InitializeUndocumentedAPIs()
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

[[nodiscard]] static inline bool IsAcrylicBlurAvailableForWin32()
{
    static const bool result = (Utils::IsWindows11OrGreater()
                                || (Utils::CompareSystemVersion(WindowsVersion::Windows10_RS2, VersionCompare::Greater)
                                    && Utils::CompareSystemVersion(WindowsVersion::Windows10_RS4, VersionCompare::Less)));
    return result;
}

[[nodiscard]] static inline bool SetBlurBehindWindowEnabled(const HWND hWnd, const bool enable,
                                                            const winrt::Windows::UI::Color &color)
{
    if (!hWnd) {
        return false;
    }
    // Blur behind window will be disabled when DWM composition is disabled.
    if (!Utils::IsCompositionEnabled()) {
        return false;
    }
    // We prefer using DwmEnableBlurBehindWindow() on Windows 7.
    if (Utils::IsWindows8OrGreater()) {
        ACCENT_POLICY policy = {};
        policy.Flags = (enable ? 2 : 0); // Magic number, don't know the exact meaning.
        if (enable) {
            // The gradient color must be set otherwise it'll look like a classic blur.
            // Use semi-transparent gradient color to get better appearance.
            if (IsAcrylicBlurAvailableForWin32()) {
                policy.State = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                policy.GradientColor = 0x01FFFFFF;
            } else {
                policy.State = ACCENT_ENABLE_BLURBEHIND;
                policy.GradientColor = WINRTCOLOR_TO_WIN32COLOR(color);
            }
        } else {
            policy.State = ACCENT_DISABLED;
            policy.GradientColor = 0;
        }
        WINDOWCOMPOSITIONATTRIBDATA data = {};
        data.Attrib = WCA_ACCENT_POLICY;
        data.pvData = &policy;
        data.cbData = sizeof(policy);
        if (g_setWindowCompositionAttributePfn(hWnd, &data) == FALSE) {
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
        const HRESULT hr = DwmEnableBlurBehindWindow(hWnd, &dwmBB);
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

class AcrylicBrushSystemPrivate : public CustomFrameT<AcrylicBrushSystemPrivate>
{
    ACRYLICMANAGER_DISABLE_COPY_MOVE(AcrylicBrushSystemPrivate)

public:
    explicit AcrylicBrushSystemPrivate(AcrylicBrushSystem *q);
    ~AcrylicBrushSystemPrivate();

    [[nodiscard]] LRESULT MessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept;

    [[nodiscard]] bool Initialize();
    [[nodiscard]] HWND GetWindow() const;
    [[nodiscard]] int GetExecResult() const;
    void ReloadBrushParameters();

private:
    AcrylicBrushSystem *q_ptr = nullptr;
};

AcrylicBrushSystemPrivate::AcrylicBrushSystemPrivate(AcrylicBrushSystem *q)
{
    q_ptr = q;
}

AcrylicBrushSystemPrivate::~AcrylicBrushSystemPrivate()
{
}

LRESULT AcrylicBrushSystemPrivate::MessageHandler(UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    return base_type::MessageHandler(message, wParam, lParam);
}

void AcrylicBrushSystemPrivate::ReloadBrushParameters()
{

}

HWND AcrylicBrushSystemPrivate::GetWindow() const
{
    return GetWindowHandle();
}

int AcrylicBrushSystemPrivate::GetExecResult() const
{
    return MessageLoop();
}

bool AcrylicBrushSystemPrivate::Initialize()
{
    if (!Create()) {
        OutputDebugStringW(L"Failed to create the background window.");
        return false;
    }
    if (!InitializeUndocumentedAPIs()) {
        OutputDebugStringW(L"Failed to load the undocumented APIs.");
        return false;
    }
    if (!SetBlurBehindWindowEnabled(GetWindowHandle(), true, q_ptr->GetEffectiveTintColor())) {
        OutputDebugStringW(L"Failed to enable blur behind window.");
        return false;
    }
    return true;
}

AcrylicBrushSystem::AcrylicBrushSystem()
{
    d_ptr = std::make_unique<AcrylicBrushSystemPrivate>(this);
}

AcrylicBrushSystem::~AcrylicBrushSystem()
{
}

void AcrylicBrushSystem::Release()
{
    if (!Destroy()) {
        OutputDebugStringW(L"Failed to destroy.");
    }
    delete this;
}

bool AcrylicBrushSystem::IsSupportedByCurrentOS() const
{
    static const bool result = Utils::IsWindows7OrGreater();
    return result;
}

bool AcrylicBrushSystem::Create() const
{
    return d_ptr->Initialize();
}

bool AcrylicBrushSystem::Destroy() const
{
    return false;
}

HWND AcrylicBrushSystem::GetWindowHandle() const
{
    return d_ptr->GetWindow();
}

bool AcrylicBrushSystem::RefreshBrush() const
{
    d_ptr->ReloadBrushParameters();
    return true;
}

int AcrylicBrushSystem::EventLoop() const
{
    return d_ptr->GetExecResult();
}
