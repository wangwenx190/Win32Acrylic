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

#include <ShellApi.h>
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

BOOL WINAPI
GetMessageW(
    LPMSG lpMsg,
    HWND  hWnd,
    UINT  wMsgFilterMin,
    UINT  wMsgFilterMax
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(GetMessageW, lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax)
}

BOOL WINAPI
TranslateMessage(
    const MSG *lpMsg
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(TranslateMessage, lpMsg)
}

LRESULT WINAPI
DispatchMessageW(
    const MSG *lpMsg
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(DispatchMessageW, lpMsg)
}

DWORD WINAPI
GetMessagePos()
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetMessagePos)
}

LONG WINAPI
GetMessageTime()
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetMessageTime)
}

LRESULT WINAPI
SendMessageW(
    HWND   hWnd,
    UINT   Msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(SendMessageW, hWnd, Msg, wParam, lParam)
}

BOOL WINAPI
PostMessageW(
    HWND   hWnd,
    UINT   Msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(PostMessageW, hWnd, Msg, wParam, lParam)
}

LRESULT CALLBACK
DefWindowProcW(
    HWND   hWnd,
    UINT   Msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(DefWindowProcW, hWnd, Msg, wParam, lParam)
}

void WINAPI
PostQuitMessage(
    int nExitCode
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_VOID_FUNCTION(PostQuitMessage, nExitCode)
}

LRESULT WINAPI
CallWindowProcW(
    WNDPROC lpPrevWndFunc,
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(CallWindowProcW, lpPrevWndFunc, hWnd, Msg, wParam, lParam)
}

BOOL WINAPI
UnregisterClassW(
    LPCWSTR   lpClassName,
    HINSTANCE hInstance
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(UnregisterClassW, lpClassName, hInstance)
}

ATOM WINAPI
RegisterClassExW(
    const WNDCLASSEXW *unnamedParam
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(RegisterClassExW, unnamedParam)
}

HWND WINAPI
CreateWindowExW(
    DWORD     dwExStyle,
    LPCWSTR   lpClassName,
    LPCWSTR   lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(CreateWindowExW, dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
}

BOOL WINAPI
DestroyWindow(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(DestroyWindow, hWnd)
}

BOOL WINAPI
ShowWindow(
    HWND hWnd,
    int  nCmdShow
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(ShowWindow, hWnd, nCmdShow)
}

BOOL WINAPI
SetLayeredWindowAttributes(
    HWND     hWnd,
    COLORREF crKey,
    BYTE     bAlpha,
    DWORD    dwFlags
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(SetLayeredWindowAttributes, hWnd, crKey, bAlpha, dwFlags)
}

BOOL WINAPI
MoveWindow(
    HWND hWnd,
    int  X,
    int  Y,
    int  nWidth,
    int  nHeight,
    BOOL bRepaint
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(MoveWindow, hWnd, X, Y, nWidth, nHeight, bRepaint)
}

BOOL WINAPI
SetWindowPos(
    HWND hWnd,
    HWND hWndInsertAfter,
    int  X,
    int  Y,
    int  cx,
    int  cy,
    UINT uFlags
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(SetWindowPos, hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags)
}

BOOL WINAPI
GetWindowPlacement(
    HWND            hWnd,
    WINDOWPLACEMENT *lpwndpl
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(GetWindowPlacement, hWnd, lpwndpl)
}

BOOL WINAPI
IsWindowVisible(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(IsWindowVisible, hWnd)
}

BOOL WINAPI
IsIconic(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(IsIconic, hWnd)
}

BOOL WINAPI
IsZoomed(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(IsZoomed, hWnd)
}

HWND WINAPI
SetFocus(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(SetFocus, hWnd)
}

HWND WINAPI
GetActiveWindow()
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(GetActiveWindow)
}

HMENU WINAPI
GetSystemMenu(
    HWND hWnd,
    BOOL bRevert
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(GetSystemMenu, hWnd, bRevert)
}

BOOL WINAPI
TrackPopupMenu(
    HMENU      hMenu,
    UINT       uFlags,
    int        x,
    int        y,
    int        nReserved,
    HWND       hWnd,
    const RECT *prcRect
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(TrackPopupMenu, hMenu, uFlags, x, y, nReserved, hWnd, prcRect)
}

BOOL WINAPI
SetMenuItemInfoW(
    HMENU            hmenu,
    UINT             item,
    BOOL             fByPosition,
    LPCMENUITEMINFOW lpmii
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(SetMenuItemInfoW, hmenu, item, fByPosition, lpmii)
}

BOOL WINAPI
SetMenuDefaultItem(
    HMENU hMenu,
    UINT  uItem,
    UINT  fByPos
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(SetMenuDefaultItem, hMenu, uItem, fByPos)
}

HDC WINAPI
GetDC(
    HWND hWnd
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(GetDC, hWnd)
}

int WINAPI
ReleaseDC(
    HWND hWnd,
    HDC  hDC
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(ReleaseDC, hWnd, hDC)
}

HDC WINAPI
BeginPaint(
    HWND          hWnd,
    LPPAINTSTRUCT lpPaint
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(BeginPaint, hWnd, lpPaint)
}

BOOL WINAPI
EndPaint(
    HWND              hWnd,
    const PAINTSTRUCT *lpPaint
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(EndPaint, hWnd, lpPaint)
}

BOOL WINAPI
GetClientRect(
    HWND   hWnd,
    LPRECT lpRect
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(GetClientRect, hWnd, lpRect)
}

BOOL WINAPI
GetWindowRect(
    HWND   hWnd,
    LPRECT lpRect
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(GetWindowRect, hWnd, lpRect)
}

int WINAPI
MessageBoxW(
    HWND    hWnd,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT    uType
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(MessageBoxW, hWnd, lpText, lpCaption, uType)
}

HCURSOR WINAPI
SetCursor(
    HCURSOR hCursor
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(SetCursor, hCursor)
}

BOOL WINAPI
ClientToScreen(
    HWND    hWnd,
    LPPOINT lpPoint
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(ClientToScreen, hWnd, lpPoint)
}

BOOL WINAPI
ScreenToClient(
    HWND    hWnd,
    LPPOINT lpPoint
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(ScreenToClient, hWnd, lpPoint)
}

int WINAPI
FillRect(
    HDC        hDC,
    const RECT *lprc,
    HBRUSH     hbr
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(FillRect, hDC, lprc, hbr)
}

LONG_PTR WINAPI
GetWindowLongPtrW(
    HWND hWnd,
    int  nIndex
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetWindowLongPtrW, hWnd, nIndex)
}

LONG_PTR WINAPI
SetWindowLongPtrW(
    HWND     hWnd,
    int      nIndex,
    LONG_PTR dwNewLong
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(SetWindowLongPtrW, hWnd, nIndex, dwNewLong)
}

ULONG_PTR WINAPI
GetClassLongPtrW(
    HWND hWnd,
    int  nIndex
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetClassLongPtrW, hWnd, nIndex)
}

ULONG_PTR WINAPI
SetClassLongPtrW(
    HWND     hWnd,
    int      nIndex,
    LONG_PTR dwNewLong
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(SetClassLongPtrW, hWnd, nIndex, dwNewLong)
}

HWND WINAPI
FindWindowW(
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(FindWindowW, lpClassName, lpWindowName)
}

HCURSOR WINAPI
LoadCursorW(
    HINSTANCE hInstance,
    LPCWSTR   lpCursorName
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(LoadCursorW, hInstance, lpCursorName)
}

HICON WINAPI
LoadIconW(
    HINSTANCE hInstance,
    LPCWSTR   lpIconName
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(LoadIconW, hInstance, lpIconName)
}

BOOL WINAPI
SystemParametersInfoW(
    UINT  uiAction,
    UINT  uiParam,
    PVOID pvParam,
    UINT  fWinIni
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(SystemParametersInfoW, uiAction, uiParam, pvParam, fWinIni)
}

HMONITOR WINAPI
MonitorFromWindow(
    HWND  hWnd,
    DWORD dwFlags
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_PTR_FUNCTION(MonitorFromWindow, hWnd, dwFlags)
}

BOOL WINAPI
GetMonitorInfoW(
    HMONITOR      hMonitor,
    LPMONITORINFO lpmi
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(GetMonitorInfoW, hMonitor, lpmi)
}

BOOL WINAPI
SetProcessDPIAware()
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(SetProcessDPIAware)
}

BOOL WINAPI
IsProcessDPIAware()
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_FUNCTION(IsProcessDPIAware)
}

int WINAPI
GetSystemMetrics(
    int nIndex
)
{
    ACRYLICMANAGER_TRY_EXECUTE_USER_INT_FUNCTION(GetSystemMetrics, nIndex)
}

/////////////////////////////////
/////     Shell32
/////////////////////////////////

UINT_PTR WINAPI
SHAppBarMessage(
    DWORD       dwMessage,
    PAPPBARDATA pData
)
{
    ACRYLICMANAGER_TRY_EXECUTE_SHELL_FUNCTION_RETURN(SHAppBarMessage, 0, dwMessage, pData)
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

BOOL WINAPI
DeleteObject(
    HGDIOBJ ho
)
{
    ACRYLICMANAGER_TRY_EXECUTE_GDI_FUNCTION_RETURN(DeleteObject, FALSE, ho)
}

HRGN WINAPI
CreateRectRgn(
    int x1,
    int y1,
    int x2,
    int y2
)
{
    ACRYLICMANAGER_TRY_EXECUTE_GDI_FUNCTION_RETURN(CreateRectRgn, nullptr, x1, y1, x2, y2)
}

/////////////////////////////////
/////     AdvApi32
/////////////////////////////////

LSTATUS WINAPI
RegOpenKeyExW(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    DWORD   ulOptions,
    REGSAM  samDesired,
    PHKEY   phkResult
)
{
    ACRYLICMANAGER_TRY_EXECUTE_ADVAPI_FUNCTION_RETURN(RegOpenKeyExW, -1, hKey, lpSubKey, ulOptions, samDesired, phkResult)
}

LSTATUS WINAPI
RegQueryValueExW(
    HKEY    hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
)
{
    ACRYLICMANAGER_TRY_EXECUTE_ADVAPI_FUNCTION_RETURN(RegQueryValueExW, -1, hKey, lpValueName, lpReserved, lpType, lpData, lpcbData)
}

LSTATUS WINAPI
RegCloseKey(
    HKEY hKey
)
{
    ACRYLICMANAGER_TRY_EXECUTE_ADVAPI_FUNCTION_RETURN(RegCloseKey, -1, hKey)
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
    HANDLE                hProcess,
    PROCESS_DPI_AWARENESS *value
)
{
    ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION(GetProcessDpiAwareness, hProcess, value)
}

HRESULT WINAPI
GetScaleFactorForMonitor(
    HMONITOR            hMon,
    DEVICE_SCALE_FACTOR *pScale
)
{
    ACRYLICMANAGER_TRY_EXECUTE_SHCORE_FUNCTION(GetScaleFactorForMonitor, hMon, pScale)
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

HRESULT WINAPI
DwmFlush()
{
    ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(DwmFlush)
}

HRESULT WINAPI
DwmEnableBlurBehindWindow(
    HWND                 hWnd,
    const DWM_BLURBEHIND *pBlurBehind
)
{
    ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION(DwmEnableBlurBehindWindow, hWnd, pBlurBehind)
}

BOOL WINAPI
DwmDefWindowProc(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam,
    LRESULT *pLResult
)
{
    ACRYLICMANAGER_TRY_EXECUTE_DWM_FUNCTION_RETURN(DwmDefWindowProc, FALSE, hWnd, message, wParam, lParam, pLResult)
}

/////////////////////////////////
/////     ComBaseApi
/////////////////////////////////

HRESULT WINAPI
CoInitialize(
    LPVOID pvReserved
)
{
    ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION(CoInitialize, pvReserved)
}

HRESULT WINAPI
CoCreateGuid(
    GUID *pGuid
)
{
    ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION(CoCreateGuid, pGuid)
}

int WINAPI
StringFromGUID2(
    REFGUID  rGuid,
    LPOLESTR lpsz,
    int      cchMax
)
{
    ACRYLICMANAGER_TRY_EXECUTE_COM_INT_FUNCTION(StringFromGUID2, rGuid, lpsz, cchMax)
}

HRESULT WINAPI
IIDFromString(
    LPCOLESTR lpsz,
    LPIID     lpiid
)
{
    ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION(IIDFromString, lpsz, lpiid)
}

void WINAPI
CoUninitialize()
{
    ACRYLICMANAGER_TRY_EXECUTE_COM_VOID_FUNCTION(CoUninitialize)
}

HRESULT WINAPI
CoCreateInstance(
    REFCLSID   rclsid,
    LPUNKNOWN  pUnkOuter,
    DWORD      dwClsContext,
    REFIID     riid,
    LPVOID FAR *ppv
)
{
    ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION(CoCreateInstance, rclsid, pUnkOuter, dwClsContext, riid, ppv)
}

void WINAPI
CoTaskMemFree(
    LPVOID pv
)
{
    ACRYLICMANAGER_TRY_EXECUTE_COM_VOID_FUNCTION(CoTaskMemFree, pv)
}

HRESULT WINAPI
CoIncrementMTAUsage(
    CO_MTA_USAGE_COOKIE *pCookie
)
{
    ACRYLICMANAGER_TRY_EXECUTE_COM_FUNCTION(CoIncrementMTAUsage, pCookie)
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
                OutputDebugStringW(L"Failed to load dynamic link library D2D1.dll.");
                return E_NOTIMPL;
            }
            func = reinterpret_cast<sig>(GetProcAddress(dll, "D2D1CreateFactory"));
            if (!func) {
                OutputDebugStringW(L"Failed to resolve symbol D2D1CreateFactory().");
                return E_NOTIMPL;
            }
        }
    }
    return func(factoryType, riid, pFactoryOptions, ppIFactory);
}

/////////////////////////////////
/////     Direct3D
/////////////////////////////////

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

/////////////////////////////////
/////     COM Automation
/////////////////////////////////

void WINAPI
SysFreeString(
    BSTR bstrString
)
{
    ACRYLICMANAGER_TRY_EXECUTE_COMAUTO_VOID_FUNCTION(SysFreeString, bstrString)
}

#ifdef __cplusplus
EXTERN_C_END
#endif

#endif
