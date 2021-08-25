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

#include "acrylicbrush.h"
#include "utils.h"
#include <wrl\client.h>
#include <DXGI1_2.h>
#include <D3D11.h>
#include <D2D1_2.h>
#include <WinCodec.h>

static Microsoft::WRL::ComPtr<ID2D1Factory2> g_am_D2DFactory_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Device1> g_am_D2DDevice_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1DeviceContext1> g_am_D2DContext_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Bitmap1> g_am_D2DTargetBitmap_p = nullptr;
static D2D1_BITMAP_PROPERTIES1 g_am_D2DBitmapProperties_p = {};
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DWallpaperBitmapSourceEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DTintColorEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DFallbackColorEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DLuminosityColorEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DLuminosityBlendEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DLuminosityColorBlendEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DSaturationEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DGaussianBlurEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DExclusionColorEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DExclusionBlendEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DExclusionBlendEffectInner_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DExclusionCompositeEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DNoiseBitmapSourceEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DNoiseBorderEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DNoiseOpacityEffect_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DNoiseBlendEffectOuter_p = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_am_D2DFadeInOutEffect_p = nullptr;
static Microsoft::WRL::ComPtr<IWICImagingFactory> g_am_WICFactory_p = nullptr;
static Microsoft::WRL::ComPtr<IWICBitmapDecoder> g_am_WICDecoder_p = nullptr;
static Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> g_am_WICFrame_p = nullptr;
static Microsoft::WRL::ComPtr<IWICStream> g_am_WICStream_p = nullptr;
static Microsoft::WRL::ComPtr<IWICFormatConverter> g_am_WICConverter_p = nullptr;
static Microsoft::WRL::ComPtr<IWICBitmapScaler> g_am_WICScaler_p = nullptr;
static Microsoft::WRL::ComPtr<ID3D11Device> g_am_D3D11Device_p = nullptr;
static Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_am_D3D11Context_p = nullptr;
static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_am_D3D11Texture_p = nullptr;
static Microsoft::WRL::ComPtr<IDXGIDevice1> g_am_DXGIDevice_p = nullptr;
static Microsoft::WRL::ComPtr<IDXGIAdapter> g_am_DXGIAdapter_p = nullptr;
static Microsoft::WRL::ComPtr<IDXGIFactory2> g_am_DXGIFactory_p = nullptr;
static Microsoft::WRL::ComPtr<IDXGISurface> g_am_DXGISurface_p = nullptr;
static Microsoft::WRL::ComPtr<IDXGISwapChain1> g_am_DXGISwapChain_p = nullptr;
static DXGI_SWAP_CHAIN_DESC1 g_am_DXGISwapChainDesc_p = {};
static D3D_FEATURE_LEVEL g_am_D3DFeatureLevel_p = D3D_FEATURE_LEVEL_1_0_CORE;

#ifdef __cplusplus
EXTERN_C_START
#endif

constexpr GUID am_CLSID_D2D1Atlas = {0x913e2be4, 0xfdcf, 0x4fe2, {0xa5, 0xf0, 0x24, 0x54, 0xf1, 0x4f, 0xf4, 0x8}};
constexpr GUID am_CLSID_D2D1BitmapSource = {0x5fb6c24d, 0xc6dd, 0x4231, {0x94, 0x4,  0x50, 0xf4, 0xd5, 0xc3, 0x25, 0x2d}};
constexpr GUID am_CLSID_D2D1Blend = {0x81c5b77b, 0x13f8, 0x4cdd, {0xad, 0x20, 0xc8, 0x90, 0x54, 0x7a, 0xc6, 0x5d}};
constexpr GUID am_CLSID_D2D1Border = {0x2A2D49C0, 0x4ACF, 0x43c7, {0x8C, 0x6A, 0x7C, 0x4A, 0x27, 0x87, 0x4D, 0x27}};
constexpr GUID am_CLSID_D2D1Composite = {0x48fc9f51, 0xf6ac, 0x48f1, {0x8b, 0x58, 0x3b, 0x28, 0xac, 0x46, 0xf7, 0x6d}};
constexpr GUID am_CLSID_D2D1Flood = {0x61c23c20, 0xae69, 0x4d8e, {0x94, 0xcf, 0x50, 0x07, 0x8d, 0xf6, 0x38, 0xf2}};
constexpr GUID am_CLSID_D2D1GaussianBlur = {0x1feb6d69, 0x2fe6, 0x4ac9, {0x8c, 0x58, 0x1d, 0x7f, 0x93, 0xe7, 0xa6, 0xa5}};
constexpr GUID am_CLSID_D2D1Saturation = {0x5cb2d9cf, 0x327d, 0x459f, {0xa0, 0xce, 0x40, 0xc0, 0xb2, 0x08, 0x6b, 0xf7}};
constexpr GUID am_CLSID_D2D1Shadow = {0xC67EA361, 0x1863, 0x4e69, {0x89, 0xDB, 0x69, 0x5D, 0x3E, 0x9A, 0x5B, 0x6B}};

#ifdef __cplusplus
EXTERN_C_END
#endif

bool AcrylicBrush::Direct2D::IsSupportedByCurrentOS()
{
    static const bool result = Utils::IsWindows8OrGreater();
    return result;
}

bool AcrylicBrush::Direct2D::IsBlurEffectEnabled()
{
    return false;
}

bool AcrylicBrush::Direct2D::SetBlurEffectEnabled(const bool enable)
{
    return false;
}

HWND AcrylicBrush::Direct2D::GetWindowHandle()
{
    return nullptr;
}

[[nodiscard]] static inline HRESULT am_D2DGenerateWallpaperBitmapSource_p()
{
    if (!g_am_MainWindowHandle_p || !g_am_D2DContext_p || !g_am_WallpaperFilePath_p) {
        return E_POINTER;
    }
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(CoInitialize, hr)
    }
    hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&g_am_WICFactory_p));
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(CoCreateInstance, hr)
    }
    hr = g_am_WICFactory_p->CreateDecoderFromFilename(g_am_WallpaperFilePath_p, nullptr,GENERIC_READ,
                                             WICDecodeMetadataCacheOnLoad, &g_am_WICDecoder_p);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(CreateDecoderFromFilename, hr)
    }
    hr = g_am_WICDecoder_p->GetFrame(0, &g_am_WICFrame_p);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(GetFrame, hr)
    }
    hr = g_am_WICFactory_p->CreateFormatConverter(&g_am_WICConverter_p);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(CreateFormatConverter, hr)
    }
    hr = g_am_WICConverter_p->Initialize(g_am_WICFrame_p.Get(), GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(Initialize, hr)
    }
    CoUninitialize();
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_D2DGenerateNoiseBitmapSource_p();

[[nodiscard]] static inline HRESULT am_D2DPrepareEffects_Luminosity_p(ID2D1Effect **output)
{
    // Apply luminosity:

    // Luminosity Color
    HRESULT hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Flood, g_am_D2DLuminosityColorEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DLuminosityColorEffect_p->SetValue(D2D1_FLOOD_PROP_COLOR, effectiveLuminosityColor);
    if (FAILED(hr)) {
        //
    }

    // Luminosity blend
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Blend, g_am_D2DLuminosityBlendEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DLuminosityBlendEffect_p->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_LUMINOSITY);
    if (FAILED(hr)) {
        //
    }
    g_am_D2DLuminosityBlendEffect_p->SetInputEffect(0, g_am_D2DGaussianBlurEffect_p.Get());
    g_am_D2DLuminosityBlendEffect_p->SetInputEffect(1, g_am_D2DLuminosityColorEffect_p.Get());

    // Apply tint:

    // Color blend
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Blend, g_am_D2DLuminosityColorBlendEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DLuminosityColorBlendEffect_p->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_COLOR);
    if (FAILED(hr)) {
        //
    }
    g_am_D2DLuminosityColorBlendEffect_p->SetInputEffect(0, g_am_D2DLuminosityBlendEffect_p.Get());
    g_am_D2DLuminosityColorBlendEffect_p->SetInputEffect(1, g_am_D2DTintColorEffect_p.Get());

    *output = g_am_D2DLuminosityColorBlendEffect_p.Get();

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_D2DPrepareEffects_Legacy_p(ID2D1Effect **output)
{
    // Apply saturation
    HRESULT hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Saturation, g_am_D2DSaturationEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DSaturationEffect_p->SetValue(D2D1_SATURATION_PROP_SATURATION, sc_saturation);
    if (FAILED(hr)) {
        //
    }
    g_am_D2DSaturationEffect_p->SetInputEffect(0, g_am_D2DGaussianBlurEffect_p.Get());

    // Apply exclusion:
    // Exclusion Color
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Flood, g_am_D2DExclusionColorEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DExclusionColorEffect_p->SetValue(D2D1_FLOOD_PROP_COLOR, sc_exclusionColor);
    if (FAILED(hr)) {
        //
    }
    // Exclusion blend
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Blend, g_am_D2DExclusionBlendEffectInner_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DExclusionBlendEffectInner_p->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_EXCLUSION);
    if (FAILED(hr)) {
        //
    }
    g_am_D2DExclusionBlendEffectInner_p->SetInputEffect(0, g_am_D2DSaturationEffect_p.Get());
    g_am_D2DExclusionBlendEffectInner_p->SetInputEffect(1, g_am_D2DExclusionColorEffect_p.Get());

    // Apply tint
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Composite, g_am_D2DExclusionCompositeEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DExclusionCompositeEffect_p->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    if (FAILED(hr)) {
        //
    }
    g_am_D2DExclusionCompositeEffect_p->SetInputEffect(0, g_am_D2DExclusionBlendEffectInner_p.Get());
    g_am_D2DExclusionCompositeEffect_p->SetInputEffect(1, g_am_D2DTintColorEffect_p.Get());

    *output = g_am_D2DExclusionCompositeEffect_p.Get();

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_D2DCreateEffects_p(ID2D1Effect **output)
{
    if (!g_am_MainWindowHandle_p || !g_am_D2DContext_p || !g_am_WICConverter_p) {
        return E_POINTER;
    }

    // todo: move theme check to common area
    SystemTheme systemTheme = SystemTheme::Invalid;
    if (FAILED(am_GetSystemThemeHelper_p(&systemTheme)) || (systemTheme == SystemTheme::Invalid)) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the system theme.")
    }
    if (systemTheme == SystemTheme::HighContrast) {
        PRINT_AND_SAFE_RETURN(L"AcrylicManager won't be functional when high contrast mode is on.")
    }

    HRESULT hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1BitmapSource, g_am_D2DWallpaperBitmapSourceEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateEffect, hr)
    }
    hr = g_am_D2DWallpaperBitmapSourceEffect_p->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, g_am_WICConverter_p.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(SetValue, hr)
    }
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Flood, g_am_D2DTintColorEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DTintColorEffect_p->SetValue(D2D1_FLOOD_PROP_COLOR, tintColor);
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1GaussianBlur, g_am_D2DGaussianBlurEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateEffect, hr)
    }
    hr = g_am_D2DGaussianBlurEffect_p->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DGaussianBlurEffect_p->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_DIRECTIONALBLUR_OPTIMIZATION_QUALITY);
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DGaussianBlurEffect_p->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, sc_blurRadius);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(SetValue, hr)
    }
    g_am_D2DGaussianBlurEffect_p->SetInputEffect(0, g_am_D2DWallpaperBitmapSourceEffect_p.Get());

    ID2D1Effect *tintOutput = nullptr;
    if (g_am_IsWindows10_19H1OrGreater_p) {
        if (FAILED(am_D2DPrepareEffects_Luminosity_p(&tintOutput))) {
            //
        }
    } else {
        if (FAILED(am_D2DPrepareEffects_Legacy_p(&tintOutput))) {
            //
        }
    }

    // Create noise with alpha and wrap:
    // Noise image BorderEffect (infinitely tiles noise image)
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Border, g_am_D2DNoiseBorderEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DNoiseBorderEffect_p->SetValue(D2D1_BORDER_PROP_EDGE_MODE_X, D2D1_BORDER_EDGE_MODE_WRAP);
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DNoiseBorderEffect_p->SetValue(D2D1_BORDER_PROP_EDGE_MODE_Y, D2D1_BORDER_EDGE_MODE_WRAP);
    if (FAILED(hr)) {
        //
    }
    g_am_D2DNoiseBorderEffect_p->SetInputEffect(0, g_am_D2DNoiseBitmapSourceEffect_p.Get());
    // OpacityEffect applied to wrapped noise
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Opacity, g_am_D2DNoiseOpacityEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DNoiseOpacityEffect_p->SetValue(D2D1_OPACITY_PROP_OPACITY, sc_noiseOpacity);
    if (FAILED(hr)) {
        //
    }
    g_am_D2DNoiseOpacityEffect_p->SetInputEffect(0, g_am_D2DNoiseBorderEffect_p.Get());

    // Blend noise on top of tint
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Composite, g_am_D2DNoiseBlendEffectOuter_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DNoiseBlendEffectOuter_p->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    if (FAILED(hr)) {
        //
    }
    g_am_D2DNoiseBlendEffectOuter_p->SetInputEffect(0, tintOutput);
    g_am_D2DNoiseBlendEffectOuter_p->SetInputEffect(1, g_am_D2DNoiseOpacityEffect_p.Get());

    // Fallback color
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1Flood, g_am_D2DFallbackColorEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DFallbackColorEffect_p->SetValue(D2D1_FLOOD_PROP_COLOR, fallbackColor);
    if (FAILED(hr)) {
        //
    }
    // CrossFade with the fallback color. Weight = 0 means full fallback, 1 means full acrylic.
    hr = g_am_D2DContext_p->CreateEffect(am_CLSID_D2D1CrossFade, g_am_D2DFadeInOutEffect_p.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_am_D2DFadeInOutEffect_p->SetValue(D2D1_CROSSFADE_PROP_WEIGHT, 1.0f);
    if (FAILED(hr)) {
        //
    }
    // fixme: check which one is destination (index 0), which one is source (index 1)
    g_am_D2DFadeInOutEffect_p->SetInputEffect(0, g_am_D2DNoiseBlendEffectOuter_p.Get());
    g_am_D2DFadeInOutEffect_p->SetInputEffect(1, g_am_D2DFallbackColorEffect_p.Get());

    *output = g_am_D2DFadeInOutEffect_p.Get();

    return S_OK;
}

[[nodiscard]] static inline HRESULT am_D2DDrawFinalVisual_p()
{
    if (!g_am_MainWindowHandle_p || !g_am_D2DContext_p || !g_am_D2DGaussianBlurEffect_p) {
        return E_POINTER;
    }
    SIZE size = {0, 0};
    if (FAILED(am_GetWindowClientSize_p(g_am_MainWindowHandle_p, &size))) {
        return E_FAIL;
    }
    int borderThickness = 0;
    if (FAILED(am_GetWindowVisibleFrameBorderThickness_p(g_am_MainWindowHandle_p, g_am_CurrentDpi_p, &borderThickness))) {
        return E_FAIL;
    }
    g_am_D2DContext_p->BeginDraw();
    g_am_D2DContext_p->Clear(g_am_DesktopBackgroundColor_p);
    g_am_D2DContext_p->DrawImage(g_am_D2DGaussianBlurEffect_p.Get(),
                                 D2D1::Point2F(0.0, static_cast<float>(borderThickness)),
                                 D2D1::RectF(0.0, 0.0,
                                             static_cast<float>(size.cx), static_cast<float>(size.cy)));
    HRESULT hr = g_am_D2DContext_p->EndDraw();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(EndDraw, hr)
    }
    hr = g_am_DXGISwapChain_p->Present(1, 0);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(Present, hr)
    }
    hr = DwmFlush();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_RETURN(DwmFlush, hr)
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_InitializeDirect2DInfrastructureHelper_p()
{
    if (!g_am_MainWindowHandle_p) {
        PRINT_AND_SAFE_RETURN(L"The main window has not been created.")
    }
    if (!g_am_IsDirect2DAvailable_p) {
        PRINT_AND_SAFE_RETURN(L"Direct2D effects are only available on Windows 8 and onwards.")
    }
    const int screen = 0; // fixme: use the correct screen id.
    if (FAILED(am_GetWallpaperFilePath_p(screen, &g_am_WallpaperFilePath_p))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the wallpaper file path.")
    }
    COLORREF color = RGB(0, 0, 0);
    if (FAILED(am_GetDesktopBackgroundColor_p(&color))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the desktop background color.")
    }
    g_am_DesktopBackgroundColor_p = D2D1::ColorF(color);
    if (FAILED(am_GetWallpaperAspectStyle_p(screen, &g_am_WallpaperAspectStyle_p))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the wallpaper aspect style.")
    }
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                   IID_PPV_ARGS(g_am_D2DFactory_p.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(D2D1CreateFactory, hr)
    }
    // This array defines the set of DirectX hardware feature levels this app supports.
    // The ordering is important and you should preserve it.
    // Don't forget to declare your app's minimum required feature level in its
    // description. All apps are assumed to support 9.1 unless otherwise stated.
    const D3D_FEATURE_LEVEL featureLevels[] = {
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
                           g_am_D3D11Device_p.GetAddressOf(), &g_am_D3DFeatureLevel_p,
                           g_am_D3D11Context_p.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(D3D11CreateDevice, hr)
    }
    hr = g_am_D3D11Device_p.As(&g_am_DXGIDevice_p);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(As, hr)
    }
    hr = g_am_D2DFactory_p->CreateDevice(g_am_DXGIDevice_p.Get(), g_am_D2DDevice_p.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateDevice, hr)
    }
    hr = g_am_D2DDevice_p->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                                               g_am_D2DContext_p.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateDeviceContext, hr)
    }
    // Selecing a target
    SecureZeroMemory(&g_am_DXGISwapChainDesc_p, sizeof(g_am_DXGISwapChainDesc_p));
    g_am_DXGISwapChainDesc_p.Width = 0; // Use automatic sizing
    g_am_DXGISwapChainDesc_p.Height = 0; // Use automatic sizing
    g_am_DXGISwapChainDesc_p.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    g_am_DXGISwapChainDesc_p.Stereo = FALSE;
    g_am_DXGISwapChainDesc_p.SampleDesc.Count = 1;
    g_am_DXGISwapChainDesc_p.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    g_am_DXGISwapChainDesc_p.BufferCount = 2;
    g_am_DXGISwapChainDesc_p.Scaling = DXGI_SCALING_NONE;
    g_am_DXGISwapChainDesc_p.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    hr = g_am_DXGIDevice_p->GetAdapter(g_am_DXGIAdapter_p.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(GetAdapter, hr)
    }
    hr = g_am_DXGIAdapter_p->GetParent(IID_PPV_ARGS(g_am_DXGIFactory_p.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(GetParent, hr)
    }
    hr = g_am_DXGIFactory_p->CreateSwapChainForHwnd(g_am_D3D11Device_p.Get(), g_am_MainWindowHandle_p,
                                                    &g_am_DXGISwapChainDesc_p, nullptr, nullptr,
                                                    g_am_DXGISwapChain_p.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateSwapChainForHwnd, hr)
    }

    hr = g_am_DXGIDevice_p->SetMaximumFrameLatency(1);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(SetMaximumFrameLatency, hr)
    }

    hr = g_am_DXGISwapChain_p->GetBuffer(0, IID_PPV_ARGS(g_am_D3D11Texture_p.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(GetBuffer, hr)
    }

    SecureZeroMemory(&g_am_D2DBitmapProperties_p, sizeof(g_am_D2DBitmapProperties_p));
    g_am_D2DBitmapProperties_p = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                static_cast<float>(g_am_CurrentDpi_p), static_cast<float>(g_am_CurrentDpi_p));

    hr = g_am_DXGISwapChain_p->GetBuffer(0, IID_PPV_ARGS(g_am_DXGISurface_p.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(GetBuffer, hr)
    }
    hr = g_am_D2DContext_p->CreateBitmapFromDxgiSurface(g_am_DXGISurface_p.Get(),
                                                        &g_am_D2DBitmapProperties_p,
                                                        g_am_D2DTargetBitmap_p.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateBitmapFromDxgiSurface, hr)
    }
    g_am_D2DContext_p->SetTarget(g_am_D2DTargetBitmap_p.Get());
    return S_OK;
}
