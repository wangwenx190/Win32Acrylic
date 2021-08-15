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
#include <ShellScalingApi.h>
#include <UxTheme.h>
#include <DwmApi.h>

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

#ifdef __cplusplus
EXTERN_C_END
#endif
