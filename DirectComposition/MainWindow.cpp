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

#include "MainWindow.h"
#include "Utils.h"
#include "OperationResult.h"
#include "Undocumented.h"
#include "WindowsVersion.h"
#include <D3D11.h>
#include <DComp.h>
#include <wrl\client.h>

static constexpr const double DefaultBlurRadius = 30.0;
static constexpr const double DefaultSaturation = 1.25;
[[maybe_unused]] static constexpr const double DefaultNoiseOpacity = 0.02;

class MainWindowPrivate
{
    enum class BackdropSource : int
    {
        Desktop,
        Host
    };

public:
    explicit MainWindowPrivate(MainWindow *q) noexcept;
    ~MainWindowPrivate() noexcept;

    [[nodiscard]] bool Initialize() noexcept;

private:
    [[nodiscard]] bool CreateCompositionDevice() noexcept;
    [[nodiscard]] bool CreateCompositionVisual() noexcept;
    [[nodiscard]] bool CreateCompositionTarget() noexcept;
    [[nodiscard]] bool CreateBackdrop(const BackdropSource source) noexcept;
    [[nodiscard]] bool CreateEffectGraph() noexcept;
    [[nodiscard]] bool SyncCoordinates() noexcept;
    [[nodiscard]] bool Flush() noexcept;
    [[nodiscard]] bool Commit() noexcept;

    [[nodiscard]] UINT GetTitleBarHeight() const noexcept;
    [[nodiscard]] UINT GetTopFrameMargin() const noexcept;

    void OnXChanged(const int arg) noexcept;
    void OnYChanged(const int arg) noexcept;
    void OnWidthChanged(const UINT arg) noexcept;
    void OnHeightChanged(const UINT arg) noexcept;
    void OnVisibilityChanged(const WindowState arg) noexcept;
    void OnActiveChanged(const bool arg) noexcept;
    void OnThemeChanged(const WindowTheme arg) noexcept;
    void OnDotsPerInchChanged(const UINT arg) noexcept;

private:
    MainWindowPrivate(const MainWindowPrivate &) = delete;
    MainWindowPrivate &operator=(const MainWindowPrivate &) = delete;
    MainWindowPrivate(MainWindowPrivate &&) = delete;
    MainWindowPrivate &operator=(MainWindowPrivate &&) = delete;

private:
    MainWindow *q_ptr = nullptr;

    static inline bool newAPIDetected = false;
    static inline bool shouldUseNewAPI = false;

    D3D_FEATURE_LEVEL d3dFeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;

    Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device = nullptr;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context = nullptr;
    Microsoft::WRL::ComPtr<IDXGIDevice2> dxgiDevice = nullptr;
    Microsoft::WRL::ComPtr<IDCompositionDesktopDevice> dcompDevice = nullptr;
    Microsoft::WRL::ComPtr<IDCompositionDevice3> dcompDevice3 = nullptr;
    Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget = nullptr;

    Microsoft::WRL::ComPtr<IDCompositionVisual2> rootVisual = nullptr;
    Microsoft::WRL::ComPtr<IDCompositionVisual2> desktopWindowVisual = nullptr;
    Microsoft::WRL::ComPtr<IDCompositionVisual2> topLevelWindowVisual = nullptr;

    Microsoft::WRL::ComPtr<IDCompositionGaussianBlurEffect> blurEffect = nullptr;
    Microsoft::WRL::ComPtr<IDCompositionSaturationEffect> saturationEffect = nullptr;
    Microsoft::WRL::ComPtr<IDCompositionTranslateTransform> translateTransform = nullptr;
    Microsoft::WRL::ComPtr<IDCompositionRectangleClip> clip = nullptr;

    HWND desktopWindow = nullptr;
    RECT desktopWindowRect = {};
    SIZE thumbnailSize = {};
    DWM_THUMBNAIL_PROPERTIES thumbnail = {};
    HTHUMBNAIL desktopThumbnail = nullptr;

    RECT sourceRect = {};
    SIZE destinationSize = {};
    HTHUMBNAIL topLevelWindowThumbnail = nullptr;
    HWND *hwndExclusionList = nullptr;

    RECT hostWindowRect = {};
};

MainWindowPrivate::MainWindowPrivate(MainWindow *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"MainWindowPrivate's q is null.");
        std::exit(-1);
    }
    q_ptr = q;
    if (Initialize()) {
        q_ptr->XChangeHandler(std::bind(&MainWindowPrivate::OnXChanged, this, std::placeholders::_1));
        q_ptr->YChangeHandler(std::bind(&MainWindowPrivate::OnYChanged, this, std::placeholders::_1));
        q_ptr->WidthChangeHandler(std::bind(&MainWindowPrivate::OnWidthChanged, this, std::placeholders::_1));
        q_ptr->HeightChangeHandler(std::bind(&MainWindowPrivate::OnHeightChanged, this, std::placeholders::_1));
        q_ptr->VisibilityChangeHandler(std::bind(&MainWindowPrivate::OnVisibilityChanged, this, std::placeholders::_1));
        q_ptr->ActiveChangeHandler(std::bind(&MainWindowPrivate::OnActiveChanged, this, std::placeholders::_1));
        q_ptr->ThemeChangeHandler(std::bind(&MainWindowPrivate::OnThemeChanged, this, std::placeholders::_1));
        q_ptr->DotsPerInchChangeHandler(std::bind(&MainWindowPrivate::OnDotsPerInchChanged, this, std::placeholders::_1));
    } else {
        Utils::DisplayErrorDialog(L"Failed to initialize MainWindowPrivate.");
        std::exit(-1);
    }
}

MainWindowPrivate::~MainWindowPrivate() noexcept
{
    if (hwndExclusionList) {
        delete [] hwndExclusionList;
        hwndExclusionList = nullptr;
    }
}

bool MainWindowPrivate::Initialize() noexcept
{
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    if (screenWidth <= 0) {
        PRINT_WIN32_ERROR_MESSAGE(GetSystemMetrics, L"Failed to retrieve the screen width.")
        return false;
    }
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (screenHeight <= 0) {
        PRINT_WIN32_ERROR_MESSAGE(GetSystemMetrics, L"Failed to retrieve the screen height.")
        return false;
    }
    sourceRect = {0, 0, screenWidth, screenHeight};
    destinationSize = {screenWidth, screenHeight};
    if (!newAPIDetected) {
        newAPIDetected = true;
        shouldUseNewAPI = (WindowsVersion::CurrentVersion() >= WindowsVersion::Windows11);
    }
    if (!CreateCompositionDevice()) {
        Utils::DisplayErrorDialog(L"Failed to create the composition device.");
        return false;
    }
    if (!CreateEffectGraph()) {
        Utils::DisplayErrorDialog(L"Failed to create the effect graph.");
        return false;
    }
    const BOOL excludeFromLivePreview = TRUE;
    HRESULT hr = DwmSetWindowAttribute(q_ptr->WindowHandle(), DWMWA_EXCLUDED_FROM_PEEK, &excludeFromLivePreview, sizeof(excludeFromLivePreview));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DwmSetWindowAttribute, hr, L"Failed to exclude the window from live preview.")
        return false;
    }
    if (!CreateBackdrop(BackdropSource::Host)) {
        Utils::DisplayErrorDialog(L"Failed to create the host backdrop.");
        return false;
    }
    if (!CreateCompositionVisual()) {
        Utils::DisplayErrorDialog(L"Failed to create the composition visuals.");
        return false;
    }
    hr = rootVisual->RemoveAllVisuals();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(RemoveAllVisuals, hr, L"Failed to remove all visuals.")
        return false;
    }
    hr = rootVisual->AddVisual(desktopWindowVisual.Get(), FALSE, nullptr);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(AddVisual, hr, L"Failed to add the desktop window visual.")
        return false;
    }
    hr = rootVisual->AddVisual(topLevelWindowVisual.Get(), TRUE, desktopWindowVisual.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(AddVisual, hr, L"Failed to add the top level window visual.")
        return false;
    }
    hr = rootVisual->SetClip(clip.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetClip, hr, L"Failed to set the clip.")
        return false;
    }
    hr = rootVisual->SetTransform(translateTransform.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetTransform, hr, L"Failed to set the transform.")
        return false;
    }
    hr = saturationEffect->SetSaturation(DefaultSaturation);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetSaturation, hr, L"Failed to set the saturation.")
        return false;
    }
    hr = blurEffect->SetBorderMode(D2D1_BORDER_MODE_HARD);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetBorderMode, hr, L"Failed to set the border mode.")
        return false;
    }
    hr = blurEffect->SetInput(0, saturationEffect.Get(), 0);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetInput, hr, L"Failed to set input.")
        return false;
    }
    hr = blurEffect->SetStandardDeviation(DefaultBlurRadius);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetStandardDeviation, hr, L"Failed to set the standard deviation.")
        return false;
    }
    hr = rootVisual->SetEffect(blurEffect.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetEffect, hr, L"Failed to set the effect.")
        return false;
    }
    if (!SyncCoordinates()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the visuals.");
        return false;
    }
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

void MainWindowPrivate::OnXChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!SyncCoordinates()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the visuals.");
    }
}

void MainWindowPrivate::OnYChanged(const int arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!SyncCoordinates()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the visuals.");
    }
}

void MainWindowPrivate::OnWidthChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!SyncCoordinates()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the visuals.");
    }
}

void MainWindowPrivate::OnHeightChanged(const UINT arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!SyncCoordinates()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the visuals.");
    }
}

void MainWindowPrivate::OnVisibilityChanged(const WindowState arg) noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void MainWindowPrivate::OnActiveChanged(const bool arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!Flush()) {
        Utils::DisplayErrorDialog(L"Failed to flush the visuals.");
    }
}

void MainWindowPrivate::OnThemeChanged(const WindowTheme arg) noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

void MainWindowPrivate::OnDotsPerInchChanged(const UINT arg) noexcept
{
    // ### TODO
    UNREFERENCED_PARAMETER(arg);
}

bool MainWindowPrivate::CreateCompositionDevice() noexcept
{
    // This array defines the set of DirectX hardware feature levels this app supports.
    // The ordering is important and you should preserve it.
    // Don't forget to declare your app's minimum required feature level in its
    // description. All apps are assumed to support 9.1 unless otherwise stated.
    static constexpr const D3D_FEATURE_LEVEL featureLevels[] =
    {
        //D3D_FEATURE_LEVEL_12_2,
        //D3D_FEATURE_LEVEL_12_1,
        //D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, d3d11Device.GetAddressOf(), &d3dFeatureLevel, d3d11Context.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(D3D11CreateDevice, hr, L"Failed to create the D3D11 device.")
        return false;
    }

    hr = d3d11Device->QueryInterface(dxgiDevice.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(QueryInterface, hr, L"Failed to retrieve the DXGI device.")
        return false;
    }

    hr = DCompositionCreateDevice3(dxgiDevice.Get(), IID_PPV_ARGS(dcompDevice.GetAddressOf()));
    if(FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DCompositionCreateDevice3, hr, L"Failed to create the Direct Composition device.")
        return false;
    }

    hr = dcompDevice->QueryInterface(IID_PPV_ARGS(dcompDevice3.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(QueryInterface, hr, L"Failed to retrieve the DCompDevice3.")
        return false;
    }

    return true;
}

bool MainWindowPrivate::CreateCompositionVisual() noexcept
{
    if (!dcompDevice3) {
        Utils::DisplayErrorDialog(L"Can't create the composition visuals due to the composition device has not been created yet.");
        return false;
    }

    HRESULT hr = dcompDevice3->CreateVisual(rootVisual.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateVisual, hr, L"Failed to create the root visual.")
        return false;
    }

    if (!CreateCompositionTarget()) {
        Utils::DisplayErrorDialog(L"Failed to create the composition target.");
        return false;
    }

    hr = dcompTarget->SetRoot(rootVisual.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetRoot, hr, L"Failed to set the root visual.")
        return false;
    }

    return true;
}

bool MainWindowPrivate::CreateCompositionTarget() noexcept
{
    if (!dcompDevice) {
        Utils::DisplayErrorDialog(L"Can't create the composition target due to the composition device has not been created yet.");
        return false;
    }

    const HRESULT hr = dcompDevice->CreateTargetForHwnd(q_ptr->WindowHandle(), FALSE, dcompTarget.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateTargetForHwnd, hr, L"Failed to create the composition target for the window.")
        return false;
    }

    return true;
}

bool MainWindowPrivate::CreateBackdrop(const BackdropSource source) noexcept
{
    if (!dcompDevice) {
        Utils::DisplayErrorDialog(L"Can't create the backdrop due to the composition device has not been created yet.");
        return false;
    }

    const HWND hWnd = q_ptr->WindowHandle();
    if (source == BackdropSource::Desktop) {
        desktopWindow = FindWindowW(L"Progman", L"Program Manager");
        if (!desktopWindow) {
            PRINT_WIN32_ERROR_MESSAGE(FindWindowW, L"Failed to find the handle of Progman.")
            return false;
        }

        if (GetWindowRect(desktopWindow, &desktopWindowRect) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the window geometry of the window.")
            return false;
        }

        thumbnailSize.cx = RECT_WIDTH(desktopWindowRect);
        thumbnailSize.cy = RECT_HEIGHT(desktopWindowRect);
        thumbnail.dwFlags = DWM_TNP_SOURCECLIENTAREAONLY | DWM_TNP_VISIBLE | DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE | DWM_TNP_OPACITY | DWM_TNP_ENABLE3D;
        thumbnail.opacity = 255;
        thumbnail.fVisible = TRUE;
        thumbnail.fSourceClientAreaOnly = FALSE;
        thumbnail.rcDestination = {0, 0, thumbnailSize.cx, thumbnailSize.cy};
        thumbnail.rcSource = {0, 0, thumbnailSize.cx, thumbnailSize.cy};

        const HRESULT hr = DwmpCreateSharedThumbnailVisual(hWnd, desktopWindow, 2, &thumbnail, dcompDevice.Get(), reinterpret_cast<VOID **>(desktopWindowVisual.GetAddressOf()), &desktopThumbnail);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmpCreateSharedThumbnailVisual, hr, L"Failed to create the thumbnail visual.")
            return false;
        }
    } else {
        HRESULT hr = E_FAIL;
        if (shouldUseNewAPI) {
            hr = DwmpCreateSharedMultiWindowVisual(hWnd, dcompDevice.Get(), reinterpret_cast<VOID **>(topLevelWindowVisual.GetAddressOf()), &topLevelWindowThumbnail);
            if (FAILED(hr)) {
                PRINT_HR_ERROR_MESSAGE(DwmpCreateSharedMultiWindowVisual, hr, L"Failed to create the window visual.")
                return false;
            }
        } else {
            hr = DwmpCreateSharedVirtualDesktopVisual(hWnd, dcompDevice.Get(), reinterpret_cast<VOID **>(topLevelWindowVisual.GetAddressOf()), &topLevelWindowThumbnail);
            if (FAILED(hr)) {
                PRINT_HR_ERROR_MESSAGE(DwmpCreateSharedVirtualDesktopVisual, hr, L"Failed to create the desktop visual.")
                return false;
            }
        }

        if (!CreateBackdrop(BackdropSource::Desktop)) {
            Utils::DisplayErrorDialog(L"Failed to create the desktop backdrop.");
            return false;
        }

        hwndExclusionList = new HWND[1];
        hwndExclusionList[0] = static_cast<HWND>(0x0);

        if (!Flush()) {
            Utils::DisplayErrorDialog(L"Failed to flush the visuals.");
            return false;
        }
    }

    return true;
}

bool MainWindowPrivate::CreateEffectGraph() noexcept
{
    if (!dcompDevice3) {
        Utils::DisplayErrorDialog(L"Can't create the effect graph due to the composition device has not been created yet.");
        return false;
    }
    HRESULT hr = dcompDevice3->CreateGaussianBlurEffect(blurEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateGaussianBlurEffect, hr, L"Failed to create the gaussian blur effect.")
        return false;
    }
    hr = dcompDevice3->CreateSaturationEffect(saturationEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateSaturationEffect, hr, L"Failed to create the saturation effect.")
        return false;
    }
    hr = dcompDevice3->CreateTranslateTransform(translateTransform.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateTranslateTransform, hr, L"Failed to create the translate transform.")
        return false;
    }
    hr = dcompDevice3->CreateRectangleClip(clip.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateRectangleClip, hr, L"Failed to create the rectangle clip.")
        return false;
    }
    return true;
}

bool MainWindowPrivate::SyncCoordinates() noexcept
{
    if (GetWindowRect(q_ptr->WindowHandle(), &hostWindowRect) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(GetWindowRect, L"Failed to retrieve the window geometry.")
        return false;
    }
    HRESULT hr = clip->SetLeft(static_cast<float>(hostWindowRect.left));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetLeft, hr, L"Failed to set left.")
        return false;
    }
    hr = clip->SetRight(static_cast<float>(hostWindowRect.right));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetRight, hr, L"Failed to set right.")
        return false;
    }
    hr = clip->SetTop(static_cast<float>(hostWindowRect.top));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetTop, hr, L"Failed to set top.")
        return false;
    }
    hr = clip->SetBottom(static_cast<float>(hostWindowRect.bottom));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetBottom, hr, L"Failed to set bottom.")
        return false;
    }
    hr = rootVisual->SetClip(clip.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetClip, hr, L"Failed to set clip.")
        return false;
    }
    hr = translateTransform->SetOffsetX(-1.0f * static_cast<float>(hostWindowRect.left) - static_cast<float>(q_ptr->GetWindowMetrics(WindowMetrics::ResizeBorderThicknessX)));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetOffsetX, hr, L"Failed to set offsetX.")
        return false;
    }
    hr = translateTransform->SetOffsetY(-1.0f * static_cast<float>(hostWindowRect.top) - static_cast<float>(q_ptr->GetWindowMetrics(WindowMetrics::ResizeBorderThicknessY)));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetOffsetY, hr, L"Failed to set offsetY.")
        return false;
    }
    hr = rootVisual->SetTransform(translateTransform.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetTransform, hr, L"Failed to set transform.")
        return false;
    }
    if (!Commit()) {
        Utils::DisplayErrorDialog(L"Failed to commit the visuals.");
        return false;
    }
    if (!Flush()) {
        Utils::DisplayErrorDialog(L"Failed to flush the visuals.");
        return false;
    }
    return true;
}

bool MainWindowPrivate::Flush() noexcept
{
    if (topLevelWindowThumbnail) {
        HRESULT hr = E_FAIL;
        if (shouldUseNewAPI) {
            hr = DwmpUpdateSharedMultiWindowVisual(topLevelWindowThumbnail, nullptr, 0, hwndExclusionList, 1, &sourceRect, &destinationSize, 1);
            if (FAILED(hr)) {
                PRINT_HR_ERROR_MESSAGE(DwmpUpdateSharedMultiWindowVisual, hr, L"Failed to update window visual.")
                return false;
            }
        } else {
            hr = DwmpUpdateSharedVirtualDesktopVisual(topLevelWindowThumbnail, nullptr, 0, hwndExclusionList, 1, &sourceRect, &destinationSize);
            if (FAILED(hr)) {
                PRINT_HR_ERROR_MESSAGE(DwmpUpdateSharedVirtualDesktopVisual, hr, L"Failed to update desktop visual.")
                return false;
            }
        }
        hr = DwmFlush();
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmFlush, hr, L"Failed to flush.")
            return false;
        }
    }
    return true;
}

bool MainWindowPrivate::Commit() noexcept
{
    if (!dcompDevice) {
        Utils::DisplayErrorDialog(L"Can't commit due to the composition device has not been created yet.");
        return false;
    }
    const HRESULT hr = dcompDevice->Commit();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(Commit, hr, L"Failed to commit.")
        return false;
    }
    return true;
}

MainWindow::MainWindow() noexcept
{
    d_ptr = std::make_unique<MainWindowPrivate>(this);
}

MainWindow::~MainWindow() noexcept = default;
