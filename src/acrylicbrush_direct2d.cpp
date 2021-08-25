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

#include "acrylicbrush_direct2d.h"
#include "resource.h"
#include "customframe.h"
#include "utils.h"
#include <DwmApi.h>
#include <wrl\client.h>
#include <DXGI1_2.h>
#include <D3D11.h>
#include <D2D1_2.h>
#include <WinCodec.h>

#ifdef __cplusplus
EXTERN_C_START
#endif

constexpr CLSID am_CLSID_D2D1Atlas = {0x913e2be4, 0xfdcf, 0x4fe2, {0xa5, 0xf0, 0x24, 0x54, 0xf1, 0x4f, 0xf4, 0x8}};
constexpr CLSID am_CLSID_D2D1BitmapSource = {0x5fb6c24d, 0xc6dd, 0x4231, {0x94, 0x4,  0x50, 0xf4, 0xd5, 0xc3, 0x25, 0x2d}};
constexpr CLSID am_CLSID_D2D1Blend = {0x81c5b77b, 0x13f8, 0x4cdd, {0xad, 0x20, 0xc8, 0x90, 0x54, 0x7a, 0xc6, 0x5d}};
constexpr CLSID am_CLSID_D2D1Border = {0x2A2D49C0, 0x4ACF, 0x43c7, {0x8C, 0x6A, 0x7C, 0x4A, 0x27, 0x87, 0x4D, 0x27}};
constexpr CLSID am_CLSID_D2D1Composite = {0x48fc9f51, 0xf6ac, 0x48f1, {0x8b, 0x58, 0x3b, 0x28, 0xac, 0x46, 0xf7, 0x6d}};
constexpr CLSID am_CLSID_D2D1Flood = {0x61c23c20, 0xae69, 0x4d8e, {0x94, 0xcf, 0x50, 0x07, 0x8d, 0xf6, 0x38, 0xf2}};
constexpr CLSID am_CLSID_D2D1GaussianBlur = {0x1feb6d69, 0x2fe6, 0x4ac9, {0x8c, 0x58, 0x1d, 0x7f, 0x93, 0xe7, 0xa6, 0xa5}};
constexpr CLSID am_CLSID_D2D1Saturation = {0x5cb2d9cf, 0x327d, 0x459f, {0xa0, 0xce, 0x40, 0xc0, 0xb2, 0x08, 0x6b, 0xf7}};
constexpr CLSID am_CLSID_D2D1Shadow = {0xC67EA361, 0x1863, 0x4e69, {0x89, 0xDB, 0x69, 0x5D, 0x3E, 0x9A, 0x5B, 0x6B}};
// todo, tint, opacity, CrossFade

#ifdef __cplusplus
EXTERN_C_END
#endif

static const std::wstring g_mainWindowClassNameSuffix = L"@Direct2DMainWindow";
static const std::wstring g_mainWindowTitle = L"AcrylicManager Direct2D Main Window";

static std::wstring g_mainWindowClassName = nullptr;
static ATOM g_mainWindowAtom = 0;
static HWND g_mainWindowHandle = nullptr;
static UINT g_currentDpi = 0;
static double g_currentDpr = 0.0;

static Microsoft::WRL::ComPtr<ID2D1Factory2> g_D2DFactory = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Device1> g_D2DDevice = nullptr;
static Microsoft::WRL::ComPtr<ID2D1DeviceContext1> g_D2DContext = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Bitmap1> g_D2DTargetBitmap = nullptr;
static D2D1_BITMAP_PROPERTIES1 g_D2DBitmapProperties = {};
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DWallpaperBitmapSourceEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DTintColorEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DFallbackColorEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DLuminosityColorEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DLuminosityBlendEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DLuminosityColorBlendEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DSaturationEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DGaussianBlurEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DExclusionColorEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DExclusionBlendEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DExclusionBlendEffectInner = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DExclusionCompositeEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DNoiseBitmapSourceEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DNoiseBorderEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DNoiseOpacityEffect = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DNoiseBlendEffectOuter = nullptr;
static Microsoft::WRL::ComPtr<ID2D1Effect> g_D2DFadeInOutEffect = nullptr;
static Microsoft::WRL::ComPtr<IWICImagingFactory> g_WICFactory = nullptr;
static Microsoft::WRL::ComPtr<IWICBitmapDecoder> g_WICDecoder = nullptr;
static Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> g_WICFrame = nullptr;
static Microsoft::WRL::ComPtr<IWICStream> g_WICStream = nullptr;
static Microsoft::WRL::ComPtr<IWICFormatConverter> g_WICConverter = nullptr;
static Microsoft::WRL::ComPtr<IWICBitmapScaler> g_WICScaler = nullptr;
static Microsoft::WRL::ComPtr<ID3D11Device> g_D3D11Device = nullptr;
static Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_D3D11Context = nullptr;
static Microsoft::WRL::ComPtr<ID3D11Texture2D> g_D3D11Texture = nullptr;
static Microsoft::WRL::ComPtr<IDXGIDevice1> g_DXGIDevice = nullptr;
static Microsoft::WRL::ComPtr<IDXGIAdapter> g_DXGIAdapter = nullptr;
static Microsoft::WRL::ComPtr<IDXGIFactory2> g_DXGIFactory = nullptr;
static Microsoft::WRL::ComPtr<IDXGISurface> g_DXGISurface = nullptr;
static Microsoft::WRL::ComPtr<IDXGISwapChain1> g_DXGISwapChain = nullptr;
static DXGI_SWAP_CHAIN_DESC1 g_DXGISwapChainDesc = {};
static D3D_FEATURE_LEVEL g_D3DFeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;

int AcrylicBrush_Direct2D::m_refCount = 0;

static inline void Cleanup()
{
    //
}

EXTERN_C LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    SecureZeroMemory(&msg, sizeof(msg));
    msg.hwnd = hWnd;
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

    bool wallpaperChanged = false;
    bool themeChanged = false;

    switch (message) {
    case WM_PAINT: {
        // generate brushes
        // create effects
        // draw final visual
        return 0;
    } break;
    case WM_DPICHANGED: {
        wallpaperChanged = true;
        const auto x = static_cast<double>(LOWORD(wParam));
        const auto y = static_cast<double>(HIWORD(wParam));
        g_currentDpi = std::round((x + y) / 2.0);
        g_currentDpr = Utils::GetDevicePixelRatioForWindow(hWnd);
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (MoveWindow(hWnd, prcNewWindow->left, prcNewWindow->top,
                       GET_RECT_WIDTH(*prcNewWindow), GET_RECT_HEIGHT(*prcNewWindow), TRUE) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(MoveWindow)
            break;
        }
        return 0;
    } break;
    case WM_SETTINGCHANGE: {
        if (wParam == SPI_SETDESKWALLPAPER) {
            wallpaperChanged = true;
        }
        if ((wParam == 0) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            themeChanged = true;
        }
    } break;
    case WM_THEMECHANGED:
    case WM_DWMCOLORIZATIONCOLORCHANGED:
        themeChanged = true;
        break;
    case WM_DWMCOMPOSITIONCHANGED: {
        if (!Utils::IsCompositionEnabled()) {
            OutputDebugStringW(L"AcrylicManager won't be functional when DWM composition is disabled.");
            std::exit(-1);
        }
    } break;
    case WM_CLOSE: {
        Cleanup();
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

AcrylicBrush_Direct2D::AcrylicBrush_Direct2D()
{
    ++m_refCount;
}

AcrylicBrush_Direct2D::~AcrylicBrush_Direct2D()
{
    --m_refCount;
}

bool AcrylicBrush_Direct2D::IsSupportedByCurrentOS() const
{
    static const bool result = Utils::IsWindows8OrGreater();
    return result;
}

HWND AcrylicBrush_Direct2D::GetWindowHandle() const
{
    return nullptr;
}

int AcrylicBrush_Direct2D::EventLoopExec() const
{
    return 0;
}

void AcrylicBrush_Direct2D::Release()
{
    --m_refCount;
    if (m_refCount <= 0) {
        m_refCount = 0;
        delete this;
    }
}

bool AcrylicBrush_Direct2D::RegisterMainWindowClass() const
{
    g_mainWindowClassName = m_windowClassNamePrefix + Utils::GenerateGUID() + g_mainWindowClassNameSuffix;

    static const HINSTANCE instance = GET_CURRENT_INSTANCE;

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindowProc;
    wcex.hInstance = instance;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_DEFAULTICON));
    wcex.hIconSm = LoadIconW(instance, MAKEINTRESOURCEW(IDI_DEFAULTICONSM));
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = g_mainWindowClassName.c_str();

    g_mainWindowAtom = RegisterClassExW(&wcex);

    if (g_mainWindowAtom == 0) {
        PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW)
        return false;
    }

    return true;
}

bool AcrylicBrush_Direct2D::CreateMainWindow() const
{
    g_mainWindowHandle = CreateWindowExW(0L,
                                         g_mainWindowClassName.c_str(),
                                         g_mainWindowTitle.c_str(),
                                         WS_OVERLAPPEDWINDOW,
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                         nullptr, nullptr, GET_CURRENT_INSTANCE, nullptr);

    if (!g_mainWindowHandle) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW)
        return false;
    }

    g_currentDpi = Utils::GetDotsPerInchForWindow(g_mainWindowHandle);
    if (g_currentDpi <= 0) {
        g_currentDpi = USER_DEFAULT_SCREEN_DPI;
    }
    g_currentDpr = Utils::GetDevicePixelRatioForWindow(g_mainWindowHandle);
    if (g_currentDpr <= 0.0) {
        g_currentDpr = 1.0;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (!Utils::UpdateFrameMargins(g_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to update main window's frame margins.");
        return false;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (!Utils::TriggerFrameChangeForWindow(g_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to trigger frame change event for main window.");
        return false;
    }
    // Ensure our window still has window transitions.
    if (!Utils::SetWindowTransitionsEnabled(g_mainWindowHandle, true)) {
        OutputDebugStringW(L"Failed to enable window transitions for main window.");
        return false;
    }

    return true;
}

bool AcrylicBrush_Direct2D::EnsureWallpaperBrush() const
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CoInitialize, hr)
        return false;
    }
    hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_WICFactory));
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(CoCreateInstance, hr)
        return false;
    }
    hr = g_WICFactory->CreateDecoderFromFilename(g_wallpaperFilePath, nullptr, GENERIC_READ,
                                             WICDecodeMetadataCacheOnLoad, &g_WICDecoder);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(CreateDecoderFromFilename, hr)
        return false;
    }
    hr = g_WICDecoder->GetFrame(0, &g_WICFrame);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(GetFrame, hr)
        return false;
    }
    hr = g_WICFactory->CreateFormatConverter(&g_WICConverter);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(CreateFormatConverter, hr)
        return false;
    }
    hr = g_WICConverter->Initialize(g_WICFrame.Get(), GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(Initialize, hr)
        return false;
    }
    CoUninitialize();
    return true;
}

bool AcrylicBrush_Direct2D::EnsureNoiseBrush() const
{

}

bool AcrylicBrush_Direct2D::PrepareEffects_Luminosity(ID2D1Effect **output) const
{
    // Apply luminosity:

    // Luminosity Color
    HRESULT hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Flood, g_D2DLuminosityColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DLuminosityColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(GetEffectiveLuminosityColor()));
    if (FAILED(hr)) {
        //
    }

    // Luminosity blend
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Blend, g_D2DLuminosityBlendEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DLuminosityBlendEffect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_LUMINOSITY);
    if (FAILED(hr)) {
        //
    }
    g_D2DLuminosityBlendEffect->SetInputEffect(0, g_D2DGaussianBlurEffect.Get());
    g_D2DLuminosityBlendEffect->SetInputEffect(1, g_D2DLuminosityColorEffect.Get());

    // Apply tint:

    // Color blend
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Blend, g_D2DLuminosityColorBlendEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DLuminosityColorBlendEffect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_COLOR);
    if (FAILED(hr)) {
        //
    }
    g_D2DLuminosityColorBlendEffect->SetInputEffect(0, g_D2DLuminosityBlendEffect.Get());
    g_D2DLuminosityColorBlendEffect->SetInputEffect(1, g_D2DTintColorEffect.Get());

    *output = g_D2DLuminosityColorBlendEffect.Get();

    return true;
}

bool AcrylicBrush_Direct2D::PrepareEffects_Legacy(ID2D1Effect **output) const
{
    // Apply saturation
    HRESULT hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Saturation, g_D2DSaturationEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DSaturationEffect->SetValue(D2D1_SATURATION_PROP_SATURATION, GetSaturation());
    if (FAILED(hr)) {
        //
    }
    g_D2DSaturationEffect->SetInputEffect(0, g_D2DGaussianBlurEffect.Get());

    // Apply exclusion:
    // Exclusion Color
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Flood, g_D2DExclusionColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DExclusionColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(GetExclusionColor()));
    if (FAILED(hr)) {
        //
    }
    // Exclusion blend
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Blend, g_D2DExclusionBlendEffectInner.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DExclusionBlendEffectInner->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_EXCLUSION);
    if (FAILED(hr)) {
        //
    }
    g_D2DExclusionBlendEffectInner->SetInputEffect(0, g_D2DSaturationEffect.Get());
    g_D2DExclusionBlendEffectInner->SetInputEffect(1, g_D2DExclusionColorEffect.Get());

    // Apply tint
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Composite, g_D2DExclusionCompositeEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DExclusionCompositeEffect->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    if (FAILED(hr)) {
        //
    }
    g_D2DExclusionCompositeEffect->SetInputEffect(0, g_D2DExclusionBlendEffectInner.Get());
    g_D2DExclusionCompositeEffect->SetInputEffect(1, g_D2DTintColorEffect.Get());

    *output = g_D2DExclusionCompositeEffect.Get();

    return true;
}

bool AcrylicBrush_Direct2D::CreateEffects(ID2D1Effect **output) const
{
    HRESULT hr = g_D2DContext->CreateEffect(am_CLSID_D2D1BitmapSource, g_D2DWallpaperBitmapSourceEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = g_D2DWallpaperBitmapSourceEffect->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, g_WICConverter.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Flood, g_D2DTintColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DTintColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(GetTintColor()));
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1GaussianBlur, g_D2DGaussianBlurEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = g_D2DGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_DIRECTIONALBLUR_OPTIMIZATION_QUALITY);
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, GetBlurRadius());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    g_D2DGaussianBlurEffect->SetInputEffect(0, g_D2DWallpaperBitmapSourceEffect.Get());

    ID2D1Effect *tintOutput = nullptr;
    if (Utils::IsWindows10_19H1OrGreater()) {
        if (!PrepareEffects_Luminosity(&tintOutput)) {
            //
        }
    } else {
        if (!PrepareEffects_Legacy(&tintOutput)) {
            //
        }
    }

    // Create noise with alpha and wrap:
    // Noise image BorderEffect (infinitely tiles noise image)
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Border, g_D2DNoiseBorderEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DNoiseBorderEffect->SetValue(D2D1_BORDER_PROP_EDGE_MODE_X, D2D1_BORDER_EDGE_MODE_WRAP);
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DNoiseBorderEffect->SetValue(D2D1_BORDER_PROP_EDGE_MODE_Y, D2D1_BORDER_EDGE_MODE_WRAP);
    if (FAILED(hr)) {
        //
    }
    g_D2DNoiseBorderEffect->SetInputEffect(0, g_D2DNoiseBitmapSourceEffect.Get());
    // OpacityEffect applied to wrapped noise
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Opacity, g_D2DNoiseOpacityEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DNoiseOpacityEffect->SetValue(D2D1_OPACITY_PROP_OPACITY, GetNoiseOpacity());
    if (FAILED(hr)) {
        //
    }
    g_D2DNoiseOpacityEffect->SetInputEffect(0, g_D2DNoiseBorderEffect.Get());

    // Blend noise on top of tint
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Composite, g_D2DNoiseBlendEffectOuter.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DNoiseBlendEffectOuter->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    if (FAILED(hr)) {
        //
    }
    g_D2DNoiseBlendEffectOuter->SetInputEffect(0, tintOutput);
    g_D2DNoiseBlendEffectOuter->SetInputEffect(1, g_D2DNoiseOpacityEffect.Get());

    // Fallback color
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1Flood, g_D2DFallbackColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DFallbackColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(GetFallbackColor()));
    if (FAILED(hr)) {
        //
    }
    // CrossFade with the fallback color. Weight = 0 means full fallback, 1 means full acrylic.
    hr = g_D2DContext->CreateEffect(am_CLSID_D2D1CrossFade, g_D2DFadeInOutEffect.GetAddressOf());
    if (FAILED(hr)) {
        //
    }
    hr = g_D2DFadeInOutEffect->SetValue(D2D1_CROSSFADE_PROP_WEIGHT, 1.0f);
    if (FAILED(hr)) {
        //
    }
    // fixme: check which one is destination (index 0), which one is source (index 1)
    g_D2DFadeInOutEffect->SetInputEffect(0, g_D2DNoiseBlendEffectOuter.Get());
    g_D2DFadeInOutEffect->SetInputEffect(1, g_D2DFallbackColorEffect.Get());

    *output = g_D2DFadeInOutEffect.Get();

    return true;
}

bool AcrylicBrush_Direct2D::DrawFinalVisual() const
{
    const SIZE windowSize = GET_WINDOW_CLIENT_SIZE(g_mainWindowHandle);
    const int borderThickness = Utils::GetWindowVisibleFrameBorderThickness(g_mainWindowHandle);
    g_D2DContext->BeginDraw();
    g_D2DContext->Clear(D2D1::ColorF(0.0, 0.0, 0.0, 0.0));
    g_D2DContext->DrawImage(g_D2DGaussianBlurEffect.Get(),
                                 D2D1::Point2F(0.0, static_cast<float>(borderThickness)),
                                 D2D1::RectF(0.0, 0.0,
                                             static_cast<float>(windowSize.cx), static_cast<float>(windowSize.cy)));
    HRESULT hr = g_D2DContext->EndDraw();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(EndDraw, hr)
        return false;
    }
    hr = g_DXGISwapChain->Present(1, 0);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(Present, hr)
        return false;
    }
    hr = DwmFlush();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DwmFlush, hr)
        return false;
    }
    return true;
}

bool AcrylicBrush_Direct2D::InitializeDirect2D() const
{
    const int screen = 0; // fixme: use the correct screen id.
    if (FAILED(am_GetWallpaperFilePath_p(screen, &g_WallpaperFilePath_p))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the wallpaper file path.")
    }
    COLORREF color = RGB(0, 0, 0);
    if (FAILED(am_GetDesktopBackgroundColor_p(&color))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the desktop background color.")
    }
    g_DesktopBackgroundColor_p = D2D1::ColorF(color);
    if (FAILED(am_GetWallpaperAspectStyle_p(screen, &g_WallpaperAspectStyle_p))) {
        PRINT_AND_SAFE_RETURN(L"Failed to retrieve the wallpaper aspect style.")
    }
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                   IID_PPV_ARGS(g_D2DFactory.GetAddressOf()));
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
                           g_D3D11Device.GetAddressOf(), &g_D3DFeatureLevel,
                           g_D3D11Context.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(D3D11CreateDevice, hr)
    }
    hr = g_D3D11Device.As(&g_DXGIDevice);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(As, hr)
    }
    hr = g_D2DFactory->CreateDevice(g_DXGIDevice.Get(), g_D2DDevice.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateDevice, hr)
    }
    hr = g_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                                               g_D2DContext.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateDeviceContext, hr)
    }
    // Selecing a target
    SecureZeroMemory(&g_DXGISwapChainDesc, sizeof(g_DXGISwapChainDesc));
    g_DXGISwapChainDesc.Width = 0; // Use automatic sizing
    g_DXGISwapChainDesc.Height = 0; // Use automatic sizing
    g_DXGISwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    g_DXGISwapChainDesc.Stereo = FALSE;
    g_DXGISwapChainDesc.SampleDesc.Count = 1;
    g_DXGISwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    g_DXGISwapChainDesc.BufferCount = 2;
    g_DXGISwapChainDesc.Scaling = DXGI_SCALING_NONE;
    g_DXGISwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    hr = g_DXGIDevice->GetAdapter(g_DXGIAdapter.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(GetAdapter, hr)
    }
    hr = g_DXGIAdapter->GetParent(IID_PPV_ARGS(g_DXGIFactory.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(GetParent, hr)
    }
    hr = g_DXGIFactory->CreateSwapChainForHwnd(g_D3D11Device.Get(), g_MainWindowHandle,
                                                    &g_DXGISwapChainDesc, nullptr, nullptr,
                                                    g_DXGISwapChain.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateSwapChainForHwnd, hr)
    }

    hr = g_DXGIDevice->SetMaximumFrameLatency(1);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(SetMaximumFrameLatency, hr)
    }

    hr = g_DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(g_D3D11Texture.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(GetBuffer, hr)
    }

    SecureZeroMemory(&g_D2DBitmapProperties, sizeof(g_D2DBitmapProperties));
    g_D2DBitmapProperties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                static_cast<float>(g_CurrentDpi), static_cast<float>(g_CurrentDpi));

    hr = g_DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(g_DXGISurface.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(GetBuffer, hr)
    }
    hr = g_D2DContext->CreateBitmapFromDxgiSurface(g_DXGISurface.Get(),
                                                        &g_D2DBitmapProperties,
                                                        g_D2DTargetBitmap.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE_AND_SAFE_RETURN(CreateBitmapFromDxgiSurface, hr)
    }
    g_D2DContext->SetTarget(g_D2DTargetBitmap.Get());
    return S_OK;
}
