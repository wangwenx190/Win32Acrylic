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
#include "WindowsVersion.h"
#include <wrl\client.h>
#include <DwmApi.h>
#include <DXGI1_2.h>
#include <D3D11.h>
#include <D2D1_2.h>
#include <D2D1Effects_2.h>
#include <WinCodec.h>

static constexpr const CLSID am_CLSID_D2D1Atlas = { 0x913e2be4, 0xfdcf, 0x4fe2, {0xa5, 0xf0, 0x24, 0x54, 0xf1, 0x4f, 0xf4, 0x8} };
static constexpr const CLSID am_CLSID_D2D1BitmapSource = { 0x5fb6c24d, 0xc6dd, 0x4231, {0x94, 0x4,  0x50, 0xf4, 0xd5, 0xc3, 0x25, 0x2d} };
static constexpr const CLSID am_CLSID_D2D1Blend = { 0x81c5b77b, 0x13f8, 0x4cdd, {0xad, 0x20, 0xc8, 0x90, 0x54, 0x7a, 0xc6, 0x5d} };
static constexpr const CLSID am_CLSID_D2D1Border = { 0x2A2D49C0, 0x4ACF, 0x43c7, {0x8C, 0x6A, 0x7C, 0x4A, 0x27, 0x87, 0x4D, 0x27} };
static constexpr const CLSID am_CLSID_D2D1Composite = { 0x48fc9f51, 0xf6ac, 0x48f1, {0x8b, 0x58, 0x3b, 0x28, 0xac, 0x46, 0xf7, 0x6d} };
static constexpr const CLSID am_CLSID_D2D1Flood = { 0x61c23c20, 0xae69, 0x4d8e, {0x94, 0xcf, 0x50, 0x07, 0x8d, 0xf6, 0x38, 0xf2} };
static constexpr const CLSID am_CLSID_D2D1GaussianBlur = { 0x1feb6d69, 0x2fe6, 0x4ac9, {0x8c, 0x58, 0x1d, 0x7f, 0x93, 0xe7, 0xa6, 0xa5} };
static constexpr const CLSID am_CLSID_D2D1Saturation = { 0x5cb2d9cf, 0x327d, 0x459f, {0xa0, 0xce, 0x40, 0xc0, 0xb2, 0x08, 0x6b, 0xf7} };
static constexpr const CLSID am_CLSID_D2D1Shadow = { 0xC67EA361, 0x1863, 0x4e69, {0x89, 0xDB, 0x69, 0x5D, 0x3E, 0x9A, 0x5B, 0x6B} };
static constexpr const CLSID am_CLSID_D2D1Opacity = { 0x811d79a4, 0xde28, 0x4454, {0x80, 0x94, 0xc6, 0x46, 0x85, 0xf8, 0xbd, 0x4c} };
static constexpr const CLSID am_CLSID_D2D1CrossFade = { 0x12f575e8, 0x4db1, 0x485f, {0x9a, 0x84, 0x03, 0xa0, 0x7d, 0xd3, 0x82, 0x9f} };
static constexpr const CLSID am_CLSID_D2D1Tint = { 0x36312b17, 0xf7dd, 0x4014, {0x91, 0x5d, 0xff, 0xca, 0x76, 0x8c, 0xf2, 0x11} };

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *q) noexcept;
    ~MainWindowPrivate() noexcept;

    [[nodiscard]] bool Initialize() noexcept;

private:
    [[nodiscard]] bool MainWindowMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT* result) const noexcept;
    [[nodiscard]] bool EnsureWindowsImagingComponent() noexcept;
    [[nodiscard]] bool EnsureWallpaperBrush() noexcept;
    [[nodiscard]] bool EnsureNoiseBrush() noexcept;
    [[nodiscard]] bool PrepareEffects_Luminosity(ID2D1Effect** output) noexcept;
    [[nodiscard]] bool PrepareEffects_Legacy(ID2D1Effect** output) noexcept;
    [[nodiscard]] bool CreateEffects(ID2D1Effect** output) noexcept;
    [[nodiscard]] bool InitializeGraphicsInfrastructure() noexcept;
    [[nodiscard]] bool DrawFinalVisual() const noexcept;
    void ReloadBrushParameters() noexcept;

    void OnThemeChanged(const WindowTheme arg) noexcept;

private:
    MainWindowPrivate(const MainWindowPrivate &) = delete;
    MainWindowPrivate &operator=(const MainWindowPrivate &) = delete;
    MainWindowPrivate(MainWindowPrivate &&) = delete;
    MainWindowPrivate &operator=(MainWindowPrivate &&) = delete;

private:
    MainWindow *q_ptr = nullptr;

    Microsoft::WRL::ComPtr<ID2D1Factory2> m_D2DFactory = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Device1> m_D2DDevice = nullptr;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext1> m_D2DContext = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_D2DTargetBitmap = nullptr;
    D2D1_BITMAP_PROPERTIES1 m_D2DBitmapProperties = {};

    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DWallpaperBitmapSourceEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DNoiseBitmapSourceEffect = nullptr;

    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DTintColorEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DFallbackColorEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DLuminosityColorEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DLuminosityBlendEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DLuminosityColorBlendEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DSaturationEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DGaussianBlurEffect = nullptr;

    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DExclusionColorEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DExclusionBlendEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DExclusionBlendEffectInner = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DExclusionCompositeEffect = nullptr;

    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DNoiseBorderEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DNoiseOpacityEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DNoiseBlendEffectOuter = nullptr;

    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DFadeInOutEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DFinalBrushEffect = nullptr;

    Microsoft::WRL::ComPtr<IWICImagingFactory2> m_WICFactory = nullptr;

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> m_WICWallpaperDecoder = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> m_WICWallpaperFrame = nullptr;
    Microsoft::WRL::ComPtr<IWICStream> m_WICWallpaperStream = nullptr;
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_WICWallpaperConverter = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapScaler> m_WICWallpaperScaler = nullptr;

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> m_WICNoiseDecoder = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> m_WICNoiseFrame = nullptr;
    Microsoft::WRL::ComPtr<IWICStream> m_WICNoiseStream = nullptr;
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_WICNoiseConverter = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapScaler> m_WICNoiseScaler = nullptr;

    Microsoft::WRL::ComPtr<ID3D11Device> m_D3D11Device = nullptr;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_D3D11Context = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_D3D11Texture = nullptr;

    Microsoft::WRL::ComPtr<IDXGIDevice1> m_DXGIDevice = nullptr;
    Microsoft::WRL::ComPtr<IDXGIAdapter> m_DXGIAdapter = nullptr;
    Microsoft::WRL::ComPtr<IDXGIFactory2> m_DXGIFactory = nullptr;
    Microsoft::WRL::ComPtr<IDXGISurface> m_DXGISurface = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_DXGISwapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 m_DXGISwapChainDesc = {};

    D3D_FEATURE_LEVEL m_D3DFeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;
};

MainWindowPrivate::MainWindowPrivate(MainWindow *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"MainWindowPrivate's q is null.");
        std::exit(-1);
    }
    q_ptr = q;
    if (Initialize()) {
        q_ptr->CustomMessageHandler(std::bind(&MainWindowPrivate::MainWindowMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        q_ptr->ThemeChangeHandler(std::bind(&MainWindowPrivate::OnThemeChanged, this, std::placeholders::_1));
    } else {
        Utils::DisplayErrorDialog(L"Failed to initialize MainWindowPrivate.");
        std::exit(-1);
    }
}

MainWindowPrivate::~MainWindowPrivate() noexcept = default;

bool MainWindowPrivate::Initialize() noexcept
{
}

bool MainWindowPrivate::MainWindowMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT* result) const noexcept
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(result);
    switch (message) {
    case WM_PAINT: {
        if (!DrawFinalVisual()) {
            Utils::DisplayErrorDialog(L"Failed to draw the D2D visual.");
        }
    } break;
    default:
        break;
    }
    return false;
}

void MainWindowPrivate::OnThemeChanged(const WindowTheme arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    ReloadBrushParameters();
}

bool MainWindowPrivate::EnsureWindowsImagingComponent() noexcept
{
    if (m_WICFactory != nullptr) {
        return true;
    }
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CoInitialize, hr, L"Failed to initialize COM.")
        return false;
    }
    hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(m_WICFactory.GetAddressOf()));
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(CoCreateInstance, hr, L"Failed to create the WICImagingFactory2 instance.")
        return false;
    }
    return (m_WICFactory != nullptr);
}

bool MainWindowPrivate::EnsureWallpaperBrush() noexcept
{
    if (m_D2DWallpaperBitmapSourceEffect != nullptr) {
        return true;
    }
    if (!EnsureWindowsImagingComponent()) {
        Utils::DisplayErrorDialog(L"Failed to initialize Windows Imaging Component.");
        return false;
    }
    HRESULT hr = m_WICFactory->CreateDecoderFromFilename(m_wallpaperFilePath, nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &m_WICWallpaperDecoder);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateDecoderFromFilename, hr, L"Failed to create decoder from filename.")
        return false;
    }
    hr = m_WICWallpaperDecoder->GetFrame(0, &m_WICWallpaperFrame);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetFrame, hr, L"Failed to retrieve the first frame.")
        return false;
    }
    hr = m_WICFactory->CreateFormatConverter(&m_WICWallpaperConverter);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateFormatConverter, hr, L"Failed to create the format converter.")
        return false;
    }
    hr = m_WICWallpaperConverter->Initialize(m_WICWallpaperFrame.Get(), GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(Initialize, hr, L"Failed to initialize the format converter.")
        return false;
    }
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1BitmapSource, m_D2DWallpaperBitmapSourceEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1BitmapSource effect.")
        return false;
    }
    hr = m_D2DWallpaperBitmapSourceEffect->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_WICWallpaperConverter.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    return (m_D2DWallpaperBitmapSourceEffect != nullptr);
}

bool MainWindowPrivate::EnsureNoiseBrush() noexcept
{
    if (m_D2DNoiseBitmapSourceEffect != nullptr) {
        return true;
    }
    LPVOID noiseResourceData = nullptr;
    DWORD noiseResourceDataSize = 0;
    if (!Utils::LoadResourceData(MAKEINTRESOURCEW(IDB_NOISE_BITMAP), L"PNG", &noiseResourceData, &noiseResourceDataSize)) {
        Utils::DisplayErrorDialog(L"Failed to load the noise asset from resource.");
        return false;
    }
    if (!EnsureWindowsImagingComponent()) {
        Utils::DisplayErrorDialog(L"Failed to initialize Windows Imaging Component.");
        return false;
    }
    HRESULT hr = m_WICFactory->CreateStream(m_WICNoiseStream.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateStream, hr, L"Failed to create noise stream.")
        return false;
    }
    hr = m_WICNoiseStream->InitializeFromMemory(static_cast<WICInProcPointer>(noiseResourceData), noiseResourceDataSize);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(InitializeFromMemory, hr, L"Failed to initialize noise stream from memory.")
        return false;
    }
    hr = m_WICFactory->CreateDecoderFromStream(m_WICNoiseStream.Get(), nullptr,
        WICDecodeMetadataCacheOnLoad, &m_WICNoiseDecoder);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateDecoderFromFilename, hr, L"Failed to create decoder from stream.")
        return false;
    }
    hr = m_WICNoiseDecoder->GetFrame(0, &m_WICNoiseFrame);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetFrame, hr, L"Failed to retrieve the first stream.")
        return false;
    }
    hr = m_WICFactory->CreateFormatConverter(&m_WICNoiseConverter);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateFormatConverter, hr, L"Failed to create the format converter.")
        return false;
    }
    hr = m_WICNoiseConverter->Initialize(m_WICNoiseFrame.Get(), GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(Initialize, hr, L"Failed to initialize the format converter.")
        return false;
    }
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1BitmapSource, m_D2DNoiseBitmapSourceEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1BitmapSource effect.")
        return false;
    }
    hr = m_D2DNoiseBitmapSourceEffect->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_WICNoiseConverter.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    return (m_D2DNoiseBitmapSourceEffect != nullptr);
}

bool MainWindowPrivate::PrepareEffects_Luminosity(ID2D1Effect** output) noexcept
{
    if (!output) {
        return false;
    }

    // Apply luminosity:

    // Luminosity Color
    HRESULT hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Flood, m_D2DLuminosityColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Flood effect.")
        return false;
    }

    // Luminosity blend
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Blend, m_D2DLuminosityBlendEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Blend effect.")
        return false;
    }
    hr = m_D2DLuminosityBlendEffect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_LUMINOSITY);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    m_D2DLuminosityBlendEffect->SetInputEffect(0, m_D2DGaussianBlurEffect.Get());
    m_D2DLuminosityBlendEffect->SetInputEffect(1, m_D2DLuminosityColorEffect.Get());

    // Apply tint:

    // Color blend
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Blend, m_D2DLuminosityColorBlendEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Blend effect.")
        return false;
    }
    hr = m_D2DLuminosityColorBlendEffect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_COLOR);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    m_D2DLuminosityColorBlendEffect->SetInputEffect(0, m_D2DLuminosityBlendEffect.Get());
    m_D2DLuminosityColorBlendEffect->SetInputEffect(1, m_D2DTintColorEffect.Get());

    *output = m_D2DLuminosityColorBlendEffect.Get();

    return true;
}

bool MainWindowPrivate::PrepareEffects_Legacy(ID2D1Effect** output) noexcept
{
    if (!output) {
        return false;
    }

    // Apply saturation
    HRESULT hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Saturation, m_D2DSaturationEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Saturation effect.")
        return false;
    }
    m_D2DSaturationEffect->SetInputEffect(0, m_D2DGaussianBlurEffect.Get());

    // Apply exclusion:
    // Exclusion Color
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Flood, m_D2DExclusionColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Flood effect.")
        return false;
    }
    // Exclusion blend
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Blend, m_D2DExclusionBlendEffectInner.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Blend effect.")
        return false;
    }
    hr = m_D2DExclusionBlendEffectInner->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_EXCLUSION);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    m_D2DExclusionBlendEffectInner->SetInputEffect(0, m_D2DSaturationEffect.Get());
    m_D2DExclusionBlendEffectInner->SetInputEffect(1, m_D2DExclusionColorEffect.Get());

    // Apply tint
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Composite, m_D2DExclusionCompositeEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Composite effect.")
        return false;
    }
    hr = m_D2DExclusionCompositeEffect->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    m_D2DExclusionCompositeEffect->SetInputEffect(0, m_D2DExclusionBlendEffectInner.Get());
    m_D2DExclusionCompositeEffect->SetInputEffect(1, m_D2DTintColorEffect.Get());

    *output = m_D2DExclusionCompositeEffect.Get();

    return true;
}

bool MainWindowPrivate::CreateEffects(ID2D1Effect** output) noexcept
{
    if (!output) {
        return false;
    }

    if (!EnsureWallpaperBrush()) {
        Utils::DisplayErrorDialog(L"Failed to create the wallpaper brush.");
        return false;
    }
    if (!EnsureNoiseBrush()) {
        Utils::DisplayErrorDialog(L"Failed to create the noise brush.");
        return false;
    }

    HRESULT hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Flood, m_D2DTintColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Flood effect.")
        return false;
    }
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1GaussianBlur, m_D2DGaussianBlurEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1GaussianBlur effect.")
        return false;
    }
    hr = m_D2DGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    m_D2DGaussianBlurEffect->SetInputEffect(0, m_D2DWallpaperBitmapSourceEffect.Get());

    Microsoft::WRL::ComPtr<ID2D1Effect> tintOutput = nullptr;
    if (WindowsVersion::CurrentVersion() >= WindowsVersion::Windows10_19Half1) {
        if (!PrepareEffects_Luminosity(tintOutput.GetAddressOf())) {
            Utils::DisplayErrorDialog(L"Failed to create the luminosity based D2D effects.");
            return false;
        }
    } else {
        if (!PrepareEffects_Legacy(tintOutput.GetAddressOf())) {
            Utils::DisplayErrorDialog(L"Failed to create the legacy D2D effects.");
            return false;
        }
    }

    // Create noise with alpha and wrap:
    // Noise image BorderEffect (infinitely tiles noise image)
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Border, m_D2DNoiseBorderEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Border effect.")
        return false;
    }
    hr = m_D2DNoiseBorderEffect->SetValue(D2D1_BORDER_PROP_EDGE_MODE_X, D2D1_BORDER_EDGE_MODE_WRAP);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    hr = m_D2DNoiseBorderEffect->SetValue(D2D1_BORDER_PROP_EDGE_MODE_Y, D2D1_BORDER_EDGE_MODE_WRAP);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    m_D2DNoiseBorderEffect->SetInputEffect(0, m_D2DNoiseBitmapSourceEffect.Get());
    // OpacityEffect applied to wrapped noise
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Opacity, m_D2DNoiseOpacityEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Opacity effect.")
        return false;
    }
    m_D2DNoiseOpacityEffect->SetInputEffect(0, m_D2DNoiseBorderEffect.Get());

    // Blend noise on top of tint
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Composite, m_D2DNoiseBlendEffectOuter.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Composite effect.")
        return false;
    }
    hr = m_D2DNoiseBlendEffectOuter->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    m_D2DNoiseBlendEffectOuter->SetInputEffect(0, tintOutput.Get());
    m_D2DNoiseBlendEffectOuter->SetInputEffect(1, m_D2DNoiseOpacityEffect.Get());

    // Fallback color
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Flood, m_D2DFallbackColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1Flood effect.")
        return false;
    }
    // CrossFade with the fallback color. Weight = 0 means full fallback, 1 means full acrylic.
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1CrossFade, m_D2DFadeInOutEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr, L"Failed to create the D2D1CrossFade effect.")
        return false;
    }
    hr = m_D2DFadeInOutEffect->SetValue(D2D1_CROSSFADE_PROP_WEIGHT, 1.0f);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        return false;
    }
    m_D2DFadeInOutEffect->SetInputEffect(0, m_D2DFallbackColorEffect.Get());
    m_D2DFadeInOutEffect->SetInputEffect(1, m_D2DNoiseBlendEffectOuter.Get());

    ReloadBrushParameters(); // <-- All visual effect parameters are setted here.

    *output = m_D2DFadeInOutEffect.Get();

    return true;
}

bool MainWindowPrivate::DrawFinalVisual() const noexcept
{
    if (!m_D2DFinalBrushEffect) {
        Utils::DisplayErrorDialog(L"The final D2D visual image is not created.");
        return false;
    }
    m_D2DContext->BeginDraw();
    //m_D2DContext->Clear(D2D1::ColorF(0.0, 0.0, 0.0, 0.0)); // todo: check: fully transparent color.
    m_D2DContext->DrawImage(m_D2DFinalBrushEffect.Get());
    HRESULT hr = m_D2DContext->EndDraw();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(EndDraw, hr, L"Failed to draw the D2D image.")
        return false;
    }
    DXGI_PRESENT_PARAMETERS parameters;
    SecureZeroMemory(&parameters, sizeof(parameters));
    // Without this step, nothing will be visible to the user.
    hr = m_DXGISwapChain->Present1(1, 0, &parameters);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(Present, hr, L"Failed to present.")
        return false;
    }
    // Try to reduce flicker as much as possible.
    hr = DwmFlush();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DwmFlush, hr, L"Failed to flush.")
        return false;
    }
    return true;
}

bool MainWindowPrivate::InitializeGraphicsInfrastructure() noexcept
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(m_D2DFactory.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(D2D1CreateFactory, hr, L"Failed to create the D2D factory.")
        return false;
    }
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
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
        m_D3D11Device.GetAddressOf(), &m_D3DFeatureLevel,
        m_D3D11Context.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(D3D11CreateDevice, hr, L"Failed to create the D3D11 device.")
        return false;
    }
    hr = m_D3D11Device.As(&m_DXGIDevice);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(As, hr, L"Failed to get the DXGI device.")
        return false;
    }
    hr = m_D2DFactory->CreateDevice(m_DXGIDevice.Get(), m_D2DDevice.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateDevice, hr, L"Failed to create the DXGI device.")
        return false;
    }
    hr = m_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_D2DContext.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateDeviceContext, hr, L"Failed to create the D2D device context.")
        return false;
    }
    SecureZeroMemory(&m_DXGISwapChainDesc, sizeof(m_DXGISwapChainDesc));
    m_DXGISwapChainDesc.Width = 0; // Use automatic sizing.
    m_DXGISwapChainDesc.Height = 0; // Use automatic sizing.
    m_DXGISwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    m_DXGISwapChainDesc.Stereo = FALSE;
    m_DXGISwapChainDesc.SampleDesc.Count = 1;
    m_DXGISwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    m_DXGISwapChainDesc.BufferCount = 2;
    m_DXGISwapChainDesc.Scaling = DXGI_SCALING_NONE;
    m_DXGISwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    hr = m_DXGIDevice->GetAdapter(m_DXGIAdapter.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetAdapter, hr, L"Failed to get the adapter.")
        return false;
    }
    hr = m_DXGIAdapter->GetParent(IID_PPV_ARGS(m_DXGIFactory.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetParent, hr, L"Failed to get the parent.")
        return false;
    }
    hr = m_DXGIFactory->CreateSwapChainForHwnd(m_D3D11Device.Get(), q_ptr->WindowHandle(),
        &m_DXGISwapChainDesc, nullptr, nullptr,
        m_DXGISwapChain.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateSwapChainForHwnd, hr, L"Failed to create the swapchain.")
        return false;
    }
    hr = m_DXGIDevice->SetMaximumFrameLatency(1);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetMaximumFrameLatency, hr, L"Failed to set the maximum frame latency.")
        return false;
    }
    hr = m_DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(m_D3D11Texture.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetBuffer, hr, L"Failed to get the buffer.")
        return false;
    }
    SecureZeroMemory(&m_D2DBitmapProperties, sizeof(m_D2DBitmapProperties));
    const auto dpiF = static_cast<float>(q_ptr->DotsPerInch());
    m_D2DBitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        dpiF, dpiF);
    hr = m_DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(m_DXGISurface.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetBuffer, hr, L"Failed to get the buffer.")
        return false;
    }
    hr = m_D2DContext->CreateBitmapFromDxgiSurface(m_DXGISurface.Get(), &m_D2DBitmapProperties,
        m_D2DTargetBitmap.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateBitmapFromDxgiSurface, hr, L"Failed to create D2D bitmap.")
        return false;
    }
    m_D2DContext->SetTarget(m_D2DTargetBitmap.Get());
    return true;
}

void MainWindowPrivate::ReloadBrushParameters() noexcept
{
    if (m_D2DLuminosityColorEffect) {
        const HRESULT hr = m_D2DLuminosityColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(q_ptr->GetEffectiveLuminosityColor()));
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        }
    }
    if (m_D2DSaturationEffect) {
        const HRESULT hr = m_D2DSaturationEffect->SetValue(D2D1_SATURATION_PROP_SATURATION, q_ptr->GetSaturation());
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        }
    }
    if (m_D2DExclusionColorEffect) {
        const HRESULT hr = m_D2DExclusionColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(q_ptr->GetExclusionColor()));
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        }
    }
    if (m_D2DTintColorEffect) {
        const HRESULT hr = m_D2DTintColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(q_ptr->GetEffectiveTintColor()));
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        }
    }
    if (m_D2DGaussianBlurEffect) {
        const HRESULT hr = m_D2DGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, q_ptr->GetBlurRadius());
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        }
    }
    if (m_D2DNoiseOpacityEffect) {
        const HRESULT hr = m_D2DNoiseOpacityEffect->SetValue(D2D1_OPACITY_PROP_OPACITY, q_ptr->GetNoiseOpacity());
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        }
    }
    if (m_D2DFallbackColorEffect) {
        const HRESULT hr = m_D2DFallbackColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(q_ptr->GetFallbackColor()));
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(SetValue, hr, L"Failed to set value.")
        }
    }
}

MainWindow::MainWindow() noexcept : Window(true)
{
    d_ptr = std::make_unique<MainWindowPrivate>(this);
}

MainWindow::~MainWindow() noexcept = default;
