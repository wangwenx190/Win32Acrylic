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
#include "MainWindow.h"
#include "Utils.h"
#include "OperationResult.h"
#include "DispatcherQueue_Thunk.h"

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *q) noexcept;
    ~MainWindowPrivate() noexcept;

    [[nodiscard]] bool Initialize() noexcept;

private:
    [[nodiscard]] bool CreateDesktopWindowTarget() noexcept;
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
    MainWindowPrivate(const MainWindowPrivate &) = delete;
    MainWindowPrivate &operator=(const MainWindowPrivate &) = delete;
    MainWindowPrivate(MainWindowPrivate &&) = delete;
    MainWindowPrivate &operator=(MainWindowPrivate &&) = delete;

private:
    MainWindow *q_ptr = nullptr;
    winrt::Windows::UI::Composition::Compositor m_compositor = nullptr;
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget m_target = nullptr;
    winrt::Windows::System::DispatcherQueueController m_dispatcherQueueController = nullptr;
    winrt::Windows::UI::Composition::SpriteVisual m_rootVisual = nullptr;
};

MainWindowPrivate::MainWindowPrivate(MainWindow *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"MainWindowPrivate's q is null.");
        std::exit(-1);
    }
    q_ptr = q;
    if (Initialize()) {
        q_ptr->WidthChangeHandler(std::bind(&MainWindowPrivate::OnWidthChanged, this, std::placeholders::_1));
        q_ptr->HeightChangeHandler(std::bind(&MainWindowPrivate::OnHeightChanged, this, std::placeholders::_1));
        q_ptr->VisibilityChangeHandler(std::bind(&MainWindowPrivate::OnVisibilityChanged, this, std::placeholders::_1));
        q_ptr->ThemeChangeHandler(std::bind(&MainWindowPrivate::OnThemeChanged, this, std::placeholders::_1));
        q_ptr->DotsPerInchChangeHandler(std::bind(&MainWindowPrivate::OnDotsPerInchChanged, this, std::placeholders::_1));
    } else {
        Utils::DisplayErrorDialog(L"Failed to initialize MainWindowPrivate.");
        std::exit(-1);
    }
}

MainWindowPrivate::~MainWindowPrivate() noexcept = default;

bool MainWindowPrivate::Initialize() noexcept
{
    if (!EnsureDispatcherQueue()) {
        Utils::DisplayErrorDialog(L"Failed to create the dispatcher queue.");
        return false;
    }
    m_compositor = winrt::Windows::UI::Composition::Compositor();
    if (m_compositor == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the compositor.");
        return false;
    }
    if (!CreateDesktopWindowTarget()) {
        Utils::DisplayErrorDialog(L"Failed to create the desktop window target.");
        return false;
    }
    if (!CreateCompositionContents()) {
        Utils::DisplayErrorDialog(L"Failed to create the Windows.UI.Composition contents.");
        return false;
    }
    m_target.Root(m_rootVisual);
    return true;
}

bool MainWindowPrivate::CreateDesktopWindowTarget() noexcept
{
    if (m_compositor == nullptr) {
        Utils::DisplayErrorDialog(L"Can't create the desktop window target due to the compositor has not been created yet.");
        return false;
    }
    const auto interop = m_compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();
    if (!interop) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the native window of the compositor.");
        return false;
    }
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target = nullptr;
    const HRESULT hr = interop->CreateDesktopWindowTarget(q_ptr->WindowHandle(), false, reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget **>(winrt::put_abi(target)));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateDesktopWindowTarget, hr, L"Failed to create the desktop window target.")
        return false;
    }
    m_target = target;
    return (m_target != nullptr);
}

bool MainWindowPrivate::EnsureDispatcherQueue() noexcept
{
    if (m_dispatcherQueueController != nullptr) {
        return true;
    }
    DispatcherQueueOptions options;
    SecureZeroMemory(&options, sizeof(options));
    options.dwSize = sizeof(options);
    options.threadType = DQTYPE_THREAD_CURRENT;
    options.apartmentType = DQTAT_COM_NONE; // This member is not needed when the thread type is "DQTYPE_THREAD_CURRENT".
    winrt::Windows::System::DispatcherQueueController controller = nullptr;
    const HRESULT hr = CreateDispatcherQueueController(options, reinterpret_cast<ABI::Windows::System::IDispatcherQueueController **>(winrt::put_abi(controller)));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateDispatcherQueueController, hr, L"Failed to create the dispatcher queue controller.")
        return false;
    }
    m_dispatcherQueueController = controller;
    return (m_dispatcherQueueController != nullptr);
}

bool MainWindowPrivate::CreateCompositionContents() noexcept
{
    if (m_compositor == nullptr) {
        Utils::DisplayErrorDialog(L"Can't create any contents due to the compositor has not been create yet.");
        return false;
    }
    if (m_target == nullptr) {
        Utils::DisplayErrorDialog(L"Can't create any contents due to the desktop window target has not been create yet.");
        return false;
    }
    auto visual = m_compositor.CreateSpriteVisual();
    if (visual == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the sprite visual.");
        return false;
    }
    visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
    visual.Brush(m_compositor.CreateHostBackdropBrush());
    m_rootVisual = visual;
    return true;
}

UINT MainWindowPrivate::GetTitleBarHeight() const noexcept
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

UINT MainWindowPrivate::GetTopFrameMargin() const noexcept
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

void MainWindowPrivate::OnWidthChanged(const UINT arg) const noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void MainWindowPrivate::OnHeightChanged(const UINT arg) const noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void MainWindowPrivate::OnVisibilityChanged(const WindowState arg) const noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void MainWindowPrivate::OnThemeChanged(const WindowTheme arg) noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void MainWindowPrivate::OnDotsPerInchChanged(const UINT arg) noexcept
{
    if (arg == 0) {
        return;
    }
    if (m_rootVisual == nullptr) {
        return;
    }
    const auto scaleFactor = (static_cast<float>(arg) / 100.0f);
    m_rootVisual.Scale({ scaleFactor, scaleFactor, 1.0f });
}

MainWindow::MainWindow() noexcept : Window(true)
{
    d_ptr = std::make_unique<MainWindowPrivate>(this);
}

MainWindow::~MainWindow() noexcept = default;
