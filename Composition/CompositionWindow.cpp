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

#include "pch.h"
#include "CompositionWindow.h"
#include "Utils.h"
#include "WindowsAPIThunks.h"
#include <Windows.System.h>

using PDISPATCHERQUEUE = ABI::Windows::System::IDispatcherQueue *;
using PDISPATCHERQUEUECONTROLLER = ABI::Windows::System::IDispatcherQueueController *;

enum DISPATCHERQUEUE_THREAD_APARTMENTTYPE
{
    DQTAT_COM_NONE = 0,
    DQTAT_COM_ASTA = 1,
    DQTAT_COM_STA = 2
};

enum DISPATCHERQUEUE_THREAD_TYPE
{
    DQTYPE_THREAD_DEDICATED = 1,
    DQTYPE_THREAD_CURRENT = 2,
};

struct DispatcherQueueOptions
{
    DWORD                                dwSize;        // Size of the struct
    DISPATCHERQUEUE_THREAD_TYPE          threadType;    // Thread affinity on which DispatcherQueueController is created.
    DISPATCHERQUEUE_THREAD_APARTMENTTYPE apartmentType; // Initialize COM apartment on the new thread as ASTA or STA. Only relevant if threadType is DQTYPE_THREAD_DEDICATED
};

EXTERN_C HRESULT WINAPI
CreateDispatcherQueueController(
    DispatcherQueueOptions     options,
    PDISPATCHERQUEUECONTROLLER *dispatcherQueueController
)
{
    const auto function = reinterpret_cast<decltype(&::CreateDispatcherQueueController)>(GetWindowsAPIByName(L"coremessaging.dll", L"CreateDispatcherQueueController"));
    return (function ? function(options, dispatcherQueueController) : DEFAULT_HRESULT);
}

class CompositionWindowPrivate
{
public:
    explicit CompositionWindowPrivate(CompositionWindow *q) noexcept;
    ~CompositionWindowPrivate() noexcept;

    [[nodiscard]] bool Initialize(const HWND hWnd) noexcept;

private:
    [[nodiscard]] bool CreateDesktopWindowTarget(const HWND hWnd) noexcept;
    [[nodiscard]] bool EnsureDispatcherQueue() noexcept;
    [[nodiscard]] bool CreateCompositionContents() noexcept;

    [[nodiscard]] UINT GetTitleBarHeight() const noexcept;
    [[nodiscard]] UINT GetTopFrameMargin() const noexcept;

    void OnWidthChanged(const UINT arg) const noexcept;
    void OnHeightChanged(const UINT arg) const noexcept;
    void OnVisibilityChanged(const WindowState arg) const noexcept;
    void OnThemeChanged(const WindowTheme arg) noexcept;
    void OnDotsPerInchChanged(const UINT arg) noexcept;

private:
    CompositionWindowPrivate(const CompositionWindowPrivate &) = delete;
    CompositionWindowPrivate &operator=(const CompositionWindowPrivate &) = delete;
    CompositionWindowPrivate(CompositionWindowPrivate &&) = delete;
    CompositionWindowPrivate &operator=(CompositionWindowPrivate &&) = delete;

private:
    CompositionWindow *q_ptr = nullptr;
    winrt::Windows::UI::Composition::Compositor m_compositor = nullptr;
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget m_target = nullptr;
    winrt::Windows::System::DispatcherQueueController m_dispatcherQueueController = nullptr;
    winrt::Windows::UI::Composition::ContainerVisual m_rootContainer = nullptr;
};

CompositionWindowPrivate::CompositionWindowPrivate(CompositionWindow *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"CompositionWindowPrivate's q is null.");
        std::exit(-1);
    }
    q_ptr = q;
    if (Initialize(q_ptr->WindowHandle())) {
        q_ptr->WidthChangeHandler(std::bind(&CompositionWindowPrivate::OnWidthChanged, this, std::placeholders::_1));
        q_ptr->HeightChangeHandler(std::bind(&CompositionWindowPrivate::OnHeightChanged, this, std::placeholders::_1));
        q_ptr->VisibilityChangeHandler(std::bind(&CompositionWindowPrivate::OnVisibilityChanged, this, std::placeholders::_1));
        q_ptr->ThemeChangeHandler(std::bind(&CompositionWindowPrivate::OnThemeChanged, this, std::placeholders::_1));
        q_ptr->DotsPerInchChangeHandler(std::bind(&CompositionWindowPrivate::OnDotsPerInchChanged, this, std::placeholders::_1));
    } else {
        Utils::DisplayErrorDialog(L"Failed to initialize CompositionWindowPrivate.");
        std::exit(-1);
    }
}

CompositionWindowPrivate::~CompositionWindowPrivate() noexcept = default;

bool CompositionWindowPrivate::Initialize(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return false;
    }
    if (!EnsureDispatcherQueue()) {
        return false;
    }
    m_compositor = winrt::Windows::UI::Composition::Compositor();
    if (m_compositor == nullptr) {
        return false;
    }
    if (!CreateDesktopWindowTarget(hWnd)) {
        return false;
    }
    if (!CreateCompositionContents()) {
        return false;
    }
    return true;
}

bool CompositionWindowPrivate::CreateDesktopWindowTarget(const HWND hWnd) noexcept
{
    if (!hWnd) {
        return false;
    }
    if (m_compositor == nullptr) {
        return false;
    }
    const auto interop = m_compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();
    if (!interop) {
        return false;
    }
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target = nullptr;
    const HRESULT hr = interop->CreateDesktopWindowTarget(hWnd, false, reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget **>(winrt::put_abi(target)));
    if (FAILED(hr)) {
        return false;
    }
    m_target = target;
    return (m_target != nullptr);
}

bool CompositionWindowPrivate::EnsureDispatcherQueue() noexcept
{
    if (m_dispatcherQueueController != nullptr) {
        return true;
    }
    DispatcherQueueOptions options;
    SecureZeroMemory(&options, sizeof(options));
    options.dwSize = sizeof(options);
    options.threadType = DQTYPE_THREAD_CURRENT;
    options.apartmentType = DQTAT_COM_NONE;
    winrt::Windows::System::DispatcherQueueController controller = nullptr;
    const HRESULT hr = CreateDispatcherQueueController(options, reinterpret_cast<ABI::Windows::System::IDispatcherQueueController **>(winrt::put_abi(controller)));
    if (FAILED(hr)) {
        return false;
    }
    m_dispatcherQueueController = controller;
    return (m_dispatcherQueueController != nullptr);
}

bool CompositionWindowPrivate::CreateCompositionContents() noexcept
{
    if (m_compositor == nullptr) {
        return false;
    }
    if (m_target == nullptr) {
        return false;
    }
    m_rootContainer = m_compositor.CreateContainerVisual();
    if (m_rootContainer == nullptr) {
        return false;
    }
    m_rootContainer.RelativeSizeAdjustment({ 1.0f, 1.0f });
    auto visual = m_compositor.CreateSpriteVisual();
    if (visual == nullptr) {
        return false;
    }
    visual.Brush(m_compositor.CreateHostBackdropBrush());
    m_rootContainer.Children().InsertAtTop(visual);
    m_target.Root(m_rootContainer);
    return true;
}

UINT CompositionWindowPrivate::GetTitleBarHeight() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't retrieve the title bar height due to the q_ptr is null.");
        return 0;
    }
    const UINT resizeBorderThicknessY = q_ptr->GetWindowMetrics(WindowMetrics::ResizeBorderThicknessY);
    const UINT captionHeight = q_ptr->GetWindowMetrics(WindowMetrics::CaptionHeight);
    const UINT titleBarHeight = ((q_ptr->Visibility() == WindowState::Maximized) ? captionHeight : (captionHeight + resizeBorderThicknessY));
    return titleBarHeight;
}

UINT CompositionWindowPrivate::GetTopFrameMargin() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't retrieve the top frame margin due to q_ptr is null.");
        return 0;
    }
    if (q_ptr->Visibility() == WindowState::Maximized) {
        return 0;
    } else {
        return q_ptr->GetWindowMetrics(WindowMetrics::WindowVisibleFrameBorderThickness);
    }
}

void CompositionWindowPrivate::OnWidthChanged(const UINT arg) const noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void CompositionWindowPrivate::OnHeightChanged(const UINT arg) const noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void CompositionWindowPrivate::OnVisibilityChanged(const WindowState arg) const noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void CompositionWindowPrivate::OnThemeChanged(const WindowTheme arg) noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void CompositionWindowPrivate::OnDotsPerInchChanged(const UINT arg) noexcept
{
    if (arg == 0) {
        return;
    }
    if (m_rootContainer == nullptr) {
        return;
    }
    const auto scaleFactor = (static_cast<float>(arg) / 100.0f);
    m_rootContainer.Scale({ scaleFactor, scaleFactor, 1.0f });
}

CompositionWindow::CompositionWindow() noexcept
{
    d_ptr = std::make_unique<CompositionWindowPrivate>(this);
}

CompositionWindow::~CompositionWindow() noexcept = default;
