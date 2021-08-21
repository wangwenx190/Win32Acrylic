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

#include "acrylicmanager_global.h"

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#include <ShellScalingApi.h>
#include <UxTheme.h>
#include <DwmApi.h>
#include <D2D1.h>
#include <D3D11.h>

#ifdef __cplusplus
EXTERN_C_START
#endif

/////////////////////////////////
/////     User32
/////////////////////////////////

UINT WINAPI
GetDpiForWindow(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetDpiForWindow, hWnd)
}

UINT WINAPI
GetDpiForSystem()
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetDpiForSystem)
}

UINT WINAPI
GetSystemDpiForProcess(
    HANDLE hProcess
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetSystemDpiForProcess, hProcess)
}

int WINAPI
GetSystemMetricsForDpi(
    int  nIndex,
    UINT dpi
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetSystemMetricsForDpi, nIndex, dpi)
}

BOOL WINAPI
AdjustWindowRectExForDpi(
    LPRECT lpRect,
    DWORD  dwStyle,
    BOOL   bMenu,
    DWORD  dwExStyle,
    UINT   dpi
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(AdjustWindowRectExForDpi, lpRect, dwStyle, bMenu, dwExStyle, dpi)
}

BOOL WINAPI
SystemParametersInfoForDpi(
    UINT  uiAction,
    UINT  uiParam,
    PVOID pvParam,
    UINT  fWinIni,
    UINT  dpi
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(SystemParametersInfoForDpi, uiAction, uiParam, pvParam, fWinIni, dpi)
}

DPI_AWARENESS_CONTEXT WINAPI
GetWindowDpiAwarenessContext(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(GetWindowDpiAwarenessContext, hWnd)
}

DPI_AWARENESS WINAPI
GetAwarenessFromDpiAwarenessContext(
    DPI_AWARENESS_CONTEXT value
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION_RETURN(GetAwarenessFromDpiAwarenessContext, DPI_AWARENESS_INVALID, value)
}

BOOL WINAPI
EnableNonClientDpiScaling(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(EnableNonClientDpiScaling, hWnd)
}

BOOL WINAPI
SetProcessDpiAwarenessContext(
    DPI_AWARENESS_CONTEXT value
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(SetProcessDpiAwarenessContext, value)
}

/////////////////////////////////
/////     Gdi32
/////////////////////////////////

int WINAPI
GetDeviceCaps(
    HDC hdc,
    int index
)
{
    ACRYLICMANAGER_TRY_EXECUTE_GDI_FUNCTION_RETURN(GetDeviceCaps, 0, hdc, index)
}

HGDIOBJ WINAPI
GetStockObject(
    int i
)
{
    ACRYLICMANAGER_TRY_EXECUTE_GDI_FUNCTION_RETURN(GetStockObject, nullptr, i)
}

/////////////////////////////////
/////     SHCore
/////////////////////////////////

HRESULT WINAPI
GetDpiForMonitor(
    HMONITOR         hMonitor,
    MONITOR_DPI_TYPE dpiType,
    UINT             *dpiX,
    UINT             *dpiY
)
{
    ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION(GetDpiForMonitor, hMonitor, dpiType, dpiX, dpiY)
}

HRESULT WINAPI
SetProcessDpiAwareness(
    PROCESS_DPI_AWARENESS value
)
{
    ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION(SetProcessDpiAwareness, value)
}

HRESULT WINAPI
GetProcessDpiAwareness(
    HANDLE hProcess,
    PROCESS_DPI_AWARENESS *value
)
{
    ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION(GetProcessDpiAwareness, hProcess, value)
}

/////////////////////////////////
/////     UxTheme
/////////////////////////////////

HPAINTBUFFER WINAPI
BeginBufferedPaint(
    HDC             hdcTarget,
    const RECT      *prcTarget,
    BP_BUFFERFORMAT dwFormat,
    BP_PAINTPARAMS  *pPaintParams,
    HDC             *phdc
)
{
    ACRYLICMANAGER_TRY_EXECUTE_THEME_PTR_FUNCTION(BeginBufferedPaint, hdcTarget, prcTarget, dwFormat, pPaintParams, phdc)
}

HRESULT WINAPI
EndBufferedPaint(
    HPAINTBUFFER hBufferedPaint,
    BOOL         fUpdateTarget
)
{
    ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION(EndBufferedPaint, hBufferedPaint, fUpdateTarget)
}

HRESULT WINAPI
BufferedPaintSetAlpha(
    HPAINTBUFFER hBufferedPaint,
    const RECT   *prc,
    BYTE         alpha
)
{
    ACRYLICMANAGER_TRY_EXECUTE_THEME_FUNCTION(BufferedPaintSetAlpha, hBufferedPaint, prc, alpha)
}

/////////////////////////////////
/////     DwmApi
/////////////////////////////////

HRESULT WINAPI
DwmExtendFrameIntoClientArea(
    HWND          hWnd,
    const MARGINS *pMarInset
)
{
    ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(DwmExtendFrameIntoClientArea, hWnd, pMarInset)
}

HRESULT WINAPI
DwmSetWindowAttribute(
    HWND    hWnd,
    DWORD   dwAttribute,
    LPCVOID pvAttribute,
    DWORD   cbAttribute
)
{
    ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(DwmSetWindowAttribute, hWnd, dwAttribute, pvAttribute, cbAttribute)
}

HRESULT WINAPI
DwmIsCompositionEnabled(
    BOOL *pfEnabled
)
{
    ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(DwmIsCompositionEnabled, pfEnabled)
}

HRESULT WINAPI
DwmGetWindowAttribute(
    HWND  hWnd,
    DWORD dwAttribute,
    PVOID pvAttribute,
    DWORD cbAttribute
)
{
    ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(DwmGetWindowAttribute, hWnd, dwAttribute, pvAttribute, cbAttribute)
}

HRESULT WINAPI
DwmGetColorizationColor(
    DWORD *pcrColorization,
    BOOL  *pfOpaqueBlend
)
{
    ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(DwmGetColorizationColor, pcrColorization, pfOpaqueBlend)
}

/////////////////////////////////
/////     ComBaseApi
/////////////////////////////////

HRESULT WINAPI
CoInitialize(
    LPVOID pvReserved
)
{
    ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION(CoInitialize, pvReserved)
}

HRESULT WINAPI
CoCreateGuid(
    GUID *pGuid
)
{
    ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION(CoCreateGuid, pGuid)
}

int WINAPI
StringFromGUID2(
    REFGUID  rGuid,
    LPOLESTR lpsz,
    int      cchMax
)
{
    ACRYLICMANAGER_TRY_EXECUTE_OLE_INT_FUNCTION(StringFromGUID2, rGuid, lpsz, cchMax)
}

void WINAPI
CoUninitialize()
{
    ACRYLICMANAGER_TRY_EXECUTE_OLE_VOID_FUNCTION(CoUninitialize)
}

/////////////////////////////////
/////     Direct2D
/////////////////////////////////

// D2D1CreateFactory() has some overloads so we have to load it manually here.
HRESULT WINAPI
D2D1CreateFactory(
    D2D1_FACTORY_TYPE          factoryType,
    REFIID                     riid,
    const D2D1_FACTORY_OPTIONS *pFactoryOptions,
    void                       **ppIFactory
)
{
    static bool tried = false;
    using sig = HRESULT(WINAPI *)(D2D1_FACTORY_TYPE, REFIID, const D2D1_FACTORY_OPTIONS *, void **);
    static sig func = nullptr;
    if (!func) {
        if (tried) {
            return E_NOTIMPL;
        } else {
            tried = true;
            const HMODULE dll = LoadLibraryExW(L"D2D1.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (!dll) {
                OutputDebugStringW(L"Failed to load D2D1.dll.");
                return E_NOTIMPL;
            }
            func = reinterpret_cast<sig>(GetProcAddress(dll, "D2D1CreateFactory"));
            if (!func) {
                OutputDebugStringW(L"Failed to resolve D2D1CreateFactory().");
                return E_NOTIMPL;
            }
        }
    }
    return func(factoryType, riid, pFactoryOptions, ppIFactory);
}

HRESULT WINAPI
D3D11CreateDevice(
    IDXGIAdapter            *pAdapter,
    D3D_DRIVER_TYPE         DriverType,
    HMODULE                 Software,
    UINT                    Flags,
    const D3D_FEATURE_LEVEL *pFeatureLevels,
    UINT                    FeatureLevels,
    UINT                    SDKVersion,
    ID3D11Device            **ppDevice,
    D3D_FEATURE_LEVEL       *pFeatureLevel,
    ID3D11DeviceContext     **ppImmediateContext
)
{
    ACRYLICMANAGER_TRY_EXECUTE_D3D_FUNCTION(D3D11CreateDevice, pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext)
}

#ifdef __cplusplus
EXTERN_C_END
#endif

#endif
