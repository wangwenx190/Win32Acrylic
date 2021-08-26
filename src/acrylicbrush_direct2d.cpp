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
#include <D2D1Effects_2.h>
#include <WinCodec.h>

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

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
constexpr CLSID am_CLSID_D2D1Opacity = {0x811d79a4, 0xde28, 0x4454, {0x80, 0x94, 0xc6, 0x46, 0x85, 0xf8, 0xbd, 0x4c}};
constexpr CLSID am_CLSID_D2D1CrossFade = {0x12f575e8, 0x4db1, 0x485f, {0x9a, 0x84, 0x03, 0xa0, 0x7d, 0xd3, 0x82, 0x9f}};
constexpr CLSID am_CLSID_D2D1Tint = {0x36312b17, 0xf7dd, 0x4014, {0x91, 0x5d, 0xff, 0xca, 0x76, 0x8c, 0xf2, 0x11}};

#ifdef __cplusplus
EXTERN_C_END
#endif

static constexpr wchar_t g_mainWindowTitle[] = L"AcrylicManager Direct2D Main Window";

static int g_refCount = 0;

class AcrylicBrushDirect2DPrivate
{
    ACRYLICMANAGER_DISABLE_COPY_MOVE(AcrylicBrushDirect2DPrivate)

public:
    explicit AcrylicBrushDirect2DPrivate(AcrylicBrushDirect2D *q);
    ~AcrylicBrushDirect2DPrivate();

    static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    [[nodiscard]] bool CreateMainWindow();
    [[nodiscard]] HWND GetMainWindowHandle() const;
    [[nodiscard]] int EventLoop() const;
    void ReloadBrushParameters();
    void Cleanup();

private:
    [[nodiscard]] LRESULT MainWindowMessageHandler(UINT message, WPARAM wParam, LPARAM lParam);
    [[nodiscard]] bool EnsureWallpaperBrush();
    [[nodiscard]] bool EnsureNoiseBrush();
    [[nodiscard]] bool PrepareEffects_Luminosity(ID2D1Effect **output);
    [[nodiscard]] bool PrepareEffects_Legacy(ID2D1Effect **output);
    [[nodiscard]] bool CreateEffects(ID2D1Effect **output);
    [[nodiscard]] bool Initialize();
    [[nodiscard]] bool DrawFinalVisual();
    void ReloadDesktopParameters();

private:
    AcrylicBrushDirect2D *q_ptr = nullptr;

    std::wstring m_mainWindowClassName = nullptr;
    HWND m_mainWindowHandle = nullptr;
    UINT m_currentDpi = 0;
    double m_currentDpr = 0.0;

    std::wstring m_wallpaperFilePath = nullptr;
    WallpaperAspectStyle m_wallpaperAspectStyle = WallpaperAspectStyle::Invalid;
    COLORREF m_desktopBackgroundColor = 0;

    Microsoft::WRL::ComPtr<ID2D1Factory2> m_D2DFactory = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Device1> m_D2DDevice = nullptr;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext1> m_D2DContext = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_D2DTargetBitmap = nullptr;
    D2D1_BITMAP_PROPERTIES1 m_D2DBitmapProperties = {};
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DWallpaperBitmapSourceEffect = nullptr;
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
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DNoiseBitmapSourceEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DNoiseBorderEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DNoiseOpacityEffect = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DNoiseBlendEffectOuter = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Effect> m_D2DFadeInOutEffect = nullptr;
    Microsoft::WRL::ComPtr<IWICImagingFactory> m_WICFactory = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapDecoder> m_WICDecoder = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> m_WICFrame = nullptr;
    Microsoft::WRL::ComPtr<IWICStream> m_WICStream = nullptr;
    Microsoft::WRL::ComPtr<IWICFormatConverter> m_WICConverter = nullptr;
    Microsoft::WRL::ComPtr<IWICBitmapScaler> m_WICScaler = nullptr;
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

AcrylicBrushDirect2DPrivate::AcrylicBrushDirect2DPrivate(AcrylicBrushDirect2D *q)
{
    q_ptr = q;
}

AcrylicBrushDirect2DPrivate::~AcrylicBrushDirect2DPrivate()
{
}

void AcrylicBrushDirect2DPrivate::Cleanup()
{
    if (!m_wallpaperFilePath.empty()) {
        m_wallpaperFilePath.clear();
    }
    if (m_wallpaperAspectStyle != WallpaperAspectStyle::Invalid) {
        m_wallpaperAspectStyle = WallpaperAspectStyle::Invalid;
    }
    if (m_desktopBackgroundColor != 0) {
        m_desktopBackgroundColor = 0;
    }
    // todo: release all com ptrs
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

LRESULT CALLBACK AcrylicBrushDirect2DPrivate::MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE) {
        const auto cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
        const auto that = static_cast<AcrylicBrushDirect2DPrivate *>(cs->lpCreateParams);
        if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
        }
    } else if (message == WM_NCDESTROY) {
        if (SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowLongPtrW)
        }
    } else if (const auto that = reinterpret_cast<AcrylicBrushDirect2DPrivate *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA))) {
        return that->MainWindowMessageHandler(message, wParam, lParam);
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

LRESULT AcrylicBrushDirect2DPrivate::MainWindowMessageHandler(UINT message, WPARAM wParam, LPARAM lParam)
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
        m_currentDpi = std::round((x + y) / 2.0);
        m_currentDpr = Utils::GetDevicePixelRatioForWindow(m_mainWindowHandle);
        const auto prcNewWindow = reinterpret_cast<LPRECT>(lParam);
        if (MoveWindow(m_mainWindowHandle, prcNewWindow->left, prcNewWindow->top,
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

    if ((wallpaperChanged || themeChanged) && !Utils::IsHighContrastModeEnabled()) {
        ReloadDesktopParameters();
        ReloadBrushParameters();
    }

    return DefWindowProcW(m_mainWindowHandle, message, wParam, lParam);
}

bool AcrylicBrushDirect2DPrivate::CreateMainWindow()
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

HWND AcrylicBrushDirect2DPrivate::GetMainWindowHandle() const
{
    return m_mainWindowHandle;
}

bool AcrylicBrushDirect2DPrivate::EnsureWallpaperBrush()
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CoInitialize, hr)
        return false;
    }
    hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_WICFactory));
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(CoCreateInstance, hr)
        return false;
    }
    hr = m_WICFactory->CreateDecoderFromFilename(m_wallpaperFilePath.c_str(), nullptr, GENERIC_READ,
                                                 WICDecodeMetadataCacheOnLoad, &m_WICDecoder);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(CreateDecoderFromFilename, hr)
        return false;
    }
    hr = m_WICDecoder->GetFrame(0, &m_WICFrame);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(GetFrame, hr)
        return false;
    }
    hr = m_WICFactory->CreateFormatConverter(&m_WICConverter);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(CreateFormatConverter, hr)
        return false;
    }
    hr = m_WICConverter->Initialize(m_WICFrame.Get(), GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);
    if (FAILED(hr)) {
        CoUninitialize();
        PRINT_HR_ERROR_MESSAGE(Initialize, hr)
        return false;
    }
    CoUninitialize();
    return true;
}

bool AcrylicBrushDirect2DPrivate::EnsureNoiseBrush()
{

}

bool AcrylicBrushDirect2DPrivate::PrepareEffects_Luminosity(ID2D1Effect **output)
{
    // Apply luminosity:

    // Luminosity Color
    HRESULT hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Flood, m_D2DLuminosityColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DLuminosityColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(q_ptr->GetEffectiveLuminosityColor()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }

    // Luminosity blend
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Blend, m_D2DLuminosityBlendEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DLuminosityBlendEffect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_LUMINOSITY);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DLuminosityBlendEffect->SetInputEffect(0, m_D2DGaussianBlurEffect.Get());
    m_D2DLuminosityBlendEffect->SetInputEffect(1, m_D2DLuminosityColorEffect.Get());

    // Apply tint:

    // Color blend
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Blend, m_D2DLuminosityColorBlendEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DLuminosityColorBlendEffect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_COLOR);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DLuminosityColorBlendEffect->SetInputEffect(0, m_D2DLuminosityBlendEffect.Get());
    m_D2DLuminosityColorBlendEffect->SetInputEffect(1, m_D2DTintColorEffect.Get());

    *output = m_D2DLuminosityColorBlendEffect.Get();

    return true;
}

bool AcrylicBrushDirect2DPrivate::PrepareEffects_Legacy(ID2D1Effect **output)
{
    // Apply saturation
    HRESULT hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Saturation, m_D2DSaturationEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DSaturationEffect->SetValue(D2D1_SATURATION_PROP_SATURATION, q_ptr->GetSaturation());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DSaturationEffect->SetInputEffect(0, m_D2DGaussianBlurEffect.Get());

    // Apply exclusion:
    // Exclusion Color
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Flood, m_D2DExclusionColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DExclusionColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(q_ptr->GetExclusionColor()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    // Exclusion blend
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Blend, m_D2DExclusionBlendEffectInner.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DExclusionBlendEffectInner->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_EXCLUSION);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DExclusionBlendEffectInner->SetInputEffect(0, m_D2DSaturationEffect.Get());
    m_D2DExclusionBlendEffectInner->SetInputEffect(1, m_D2DExclusionColorEffect.Get());

    // Apply tint
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Composite, m_D2DExclusionCompositeEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DExclusionCompositeEffect->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DExclusionCompositeEffect->SetInputEffect(0, m_D2DExclusionBlendEffectInner.Get());
    m_D2DExclusionCompositeEffect->SetInputEffect(1, m_D2DTintColorEffect.Get());

    *output = m_D2DExclusionCompositeEffect.Get();

    return true;
}

bool AcrylicBrushDirect2DPrivate::CreateEffects(ID2D1Effect **output)
{
    HRESULT hr = m_D2DContext->CreateEffect(am_CLSID_D2D1BitmapSource, m_D2DWallpaperBitmapSourceEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DWallpaperBitmapSourceEffect->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, m_WICConverter.Get());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Flood, m_D2DTintColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DTintColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(q_ptr->GetTintColor()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1GaussianBlur, m_D2DGaussianBlurEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    hr = m_D2DGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_DIRECTIONALBLUR_OPTIMIZATION_QUALITY);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    hr = m_D2DGaussianBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, q_ptr->GetBlurRadius());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DGaussianBlurEffect->SetInputEffect(0, m_D2DWallpaperBitmapSourceEffect.Get());

    ID2D1Effect *tintOutput = nullptr;
    if (Utils::IsWindows1019H1OrGreater()) {
        if (!PrepareEffects_Luminosity(&tintOutput)) {
            OutputDebugStringW(L"Failed to create the luminosity based D2D effects.");
            return false;
        }
    } else {
        if (!PrepareEffects_Legacy(&tintOutput)) {
            OutputDebugStringW(L"Failed to create the legacy D2D effects.");
            return false;
        }
    }

    // Create noise with alpha and wrap:
    // Noise image BorderEffect (infinitely tiles noise image)
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Border, m_D2DNoiseBorderEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DNoiseBorderEffect->SetValue(D2D1_BORDER_PROP_EDGE_MODE_X, D2D1_BORDER_EDGE_MODE_WRAP);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    hr = m_D2DNoiseBorderEffect->SetValue(D2D1_BORDER_PROP_EDGE_MODE_Y, D2D1_BORDER_EDGE_MODE_WRAP);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DNoiseBorderEffect->SetInputEffect(0, m_D2DNoiseBitmapSourceEffect.Get());
    // OpacityEffect applied to wrapped noise
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Opacity, m_D2DNoiseOpacityEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DNoiseOpacityEffect->SetValue(D2D1_OPACITY_PROP_OPACITY, q_ptr->GetNoiseOpacity());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DNoiseOpacityEffect->SetInputEffect(0, m_D2DNoiseBorderEffect.Get());

    // Blend noise on top of tint
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Composite, m_D2DNoiseBlendEffectOuter.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DNoiseBlendEffectOuter->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OVER);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    m_D2DNoiseBlendEffectOuter->SetInputEffect(0, tintOutput);
    m_D2DNoiseBlendEffectOuter->SetInputEffect(1, m_D2DNoiseOpacityEffect.Get());

    // Fallback color
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1Flood, m_D2DFallbackColorEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DFallbackColorEffect->SetValue(D2D1_FLOOD_PROP_COLOR, WINRTCOLOR_TO_D2DCOLOR4F(q_ptr->GetFallbackColor()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    // CrossFade with the fallback color. Weight = 0 means full fallback, 1 means full acrylic.
    hr = m_D2DContext->CreateEffect(am_CLSID_D2D1CrossFade, m_D2DFadeInOutEffect.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateEffect, hr)
        return false;
    }
    hr = m_D2DFadeInOutEffect->SetValue(D2D1_CROSSFADE_PROP_WEIGHT, 1.0f);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetValue, hr)
        return false;
    }
    // fixme: check which one is destination (index 0), which one is source (index 1)
    m_D2DFadeInOutEffect->SetInputEffect(0, m_D2DNoiseBlendEffectOuter.Get());
    m_D2DFadeInOutEffect->SetInputEffect(1, m_D2DFallbackColorEffect.Get());

    *output = m_D2DFadeInOutEffect.Get();

    return true;
}

bool AcrylicBrushDirect2DPrivate::DrawFinalVisual()
{
    const SIZE windowSize = GET_WINDOW_CLIENT_SIZE(m_mainWindowHandle);
    const int borderThickness = Utils::GetWindowVisibleFrameBorderThickness(m_mainWindowHandle);
    m_D2DContext->BeginDraw();
    m_D2DContext->Clear(D2D1::ColorF(0.0, 0.0, 0.0, 0.0));
    m_D2DContext->DrawImage(m_D2DGaussianBlurEffect.Get(),
                            D2D1::Point2F(0.0, static_cast<float>(borderThickness)),
                            D2D1::RectF(0.0, 0.0,
                                        static_cast<float>(windowSize.cx), static_cast<float>(windowSize.cy)));
    HRESULT hr = m_D2DContext->EndDraw();
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(EndDraw, hr)
        return false;
    }
    hr = m_DXGISwapChain->Present(1, 0);
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

void AcrylicBrushDirect2DPrivate::ReloadDesktopParameters()
{
    constexpr int screen = 0; // fixme: use the correct screen id.
    m_wallpaperFilePath = Utils::GetWallpaperFilePath(screen);
    m_wallpaperAspectStyle = Utils::GetWallpaperAspectStyle(screen);
    m_desktopBackgroundColor = Utils::GetDesktopBackgroundColor(screen);
}

bool AcrylicBrushDirect2DPrivate::Initialize()
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(m_D2DFactory.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(D2D1CreateFactory, hr)
        return false;
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
                           m_D3D11Device.GetAddressOf(), &m_D3DFeatureLevel,
                           m_D3D11Context.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(D3D11CreateDevice, hr)
        return false;
    }
    hr = m_D3D11Device.As(&m_DXGIDevice);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(As, hr)
        return false;
    }
    hr = m_D2DFactory->CreateDevice(m_DXGIDevice.Get(), m_D2DDevice.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateDevice, hr)
        return false;
    }
    hr = m_D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_D2DContext.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateDeviceContext, hr)
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
        PRINT_HR_ERROR_MESSAGE(GetAdapter, hr)
        return false;
    }
    hr = m_DXGIAdapter->GetParent(IID_PPV_ARGS(m_DXGIFactory.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetParent, hr)
        return false;
    }
    hr = m_DXGIFactory->CreateSwapChainForHwnd(m_D3D11Device.Get(), m_mainWindowHandle,
                                               &m_DXGISwapChainDesc, nullptr, nullptr,
                                               m_DXGISwapChain.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateSwapChainForHwnd, hr)
        return false;
    }
    hr = m_DXGIDevice->SetMaximumFrameLatency(1);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(SetMaximumFrameLatency, hr)
        return false;
    }
    hr = m_DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(m_D3D11Texture.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetBuffer, hr)
        return false;
    }
    SecureZeroMemory(&m_D2DBitmapProperties, sizeof(m_D2DBitmapProperties));
    m_D2DBitmapProperties = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
                static_cast<float>(m_currentDpi), static_cast<float>(m_currentDpi));
    hr = m_DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(m_DXGISurface.GetAddressOf()));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetBuffer, hr)
        return false;
    }
    hr = m_D2DContext->CreateBitmapFromDxgiSurface(m_DXGISurface.Get(), &m_D2DBitmapProperties,
                                                   m_D2DTargetBitmap.GetAddressOf());
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateBitmapFromDxgiSurface, hr)
        return false;
    }
    m_D2DContext->SetTarget(m_D2DTargetBitmap.Get());
    return true;
}

int AcrylicBrushDirect2DPrivate::EventLoop() const
{
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void AcrylicBrushDirect2DPrivate::ReloadBrushParameters()
{

}

AcrylicBrushDirect2D::AcrylicBrushDirect2D()
{
    d_ptr = std::make_unique<AcrylicBrushDirect2DPrivate>(this);
}

AcrylicBrushDirect2D::~AcrylicBrushDirect2D()
{
}

int AcrylicBrushDirect2D::AddRef() const
{
    ++g_refCount;
}

void AcrylicBrushDirect2D::Release()
{
    --g_refCount;
    if (g_refCount <= 0) {
        g_refCount = 0;
        delete this;
    }
}

bool AcrylicBrushDirect2D::IsSupportedByCurrentOS() const
{
    static const bool result = Utils::IsWindows8OrGreater();
    return result;
}

bool AcrylicBrushDirect2D::Create() const
{
    return false;
}

bool AcrylicBrushDirect2D::Destroy() const
{
    d_ptr->Cleanup();
    return true;
}

bool AcrylicBrushDirect2D::RefreshBrush() const
{
    d_ptr->ReloadBrushParameters();
    return true;
}

HWND AcrylicBrushDirect2D::GetWindowHandle() const
{
    return d_ptr->GetMainWindowHandle();
}

int AcrylicBrushDirect2D::EventLoop() const
{
    return d_ptr->EventLoop();
}
