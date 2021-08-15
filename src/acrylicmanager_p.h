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

#pragma once

#include "acrylicmanager_global.h"

enum class WindowsVersion : int
{
    WindowsVista = 0,
    Windows7,
    Windows8,
    Windows8_1,
    Windows10_1507,
    Windows10_1511,
    Windows10_1607,
    Windows10_1703,
    Windows10_1709,
    Windows10_1803,
    Windows10_1809,
    Windows10_1903,
    Windows10_1909,
    Windows10_2004,
    Windows10_20H2,
    Windows10_21H1,
    Windows11,
    Windows10 = Windows10_1507,
    Windows10_TH1 = Windows10_1507,
    Windows10_TH2 = Windows10_1511,
    Windows10_RS1 = Windows10_1607,
    Windows10_RS2 = Windows10_1703,
    Windows10_RS3 = Windows10_1709,
    Windows10_RS4 = Windows10_1803,
    Windows10_RS5 = Windows10_1809,
    Windows10_19H1 = Windows10_1903,
    Windows10_19H2 = Windows10_1909,
    Windows10_20H1 = Windows10_2004
    //Windows10_20H2 = Windows10_20H2,
    //Windows10_21H1 = Windows10_21H1,
};

enum class VersionCompare : int
{
    Less = 0,
    Equal,
    Greater,
    LessOrEqual,
    GreaterOrEqual
};

#ifdef __cplusplus
EXTERN_C_START
#endif

[[nodiscard]] ACRYLICMANAGER_API UINT WINAPI
am_GetWindowDpi_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsMinimized_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsMaximized_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsFullScreened_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsNoState_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsVisible_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API int WINAPI
am_GetResizeBorderThickness_p(
    _In_ const bool x,
    _In_ const UINT dpi
);

[[nodiscard]] ACRYLICMANAGER_API int WINAPI
am_GetCaptionHeight_p(
    _In_ const UINT dpi
);

[[nodiscard]] ACRYLICMANAGER_API int WINAPI
am_GetTitleBarHeight_p(
    _In_ const HWND hWnd,
    _In_ const UINT dpi
);

[[nodiscard]] ACRYLICMANAGER_API RECT WINAPI
am_GetScreenGeometry_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API RECT WINAPI
am_GetScreenAvailableGeometry_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_IsCompositionEnabled_p();

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_TriggerFrameChange_p(
    _In_ const HWND hWnd
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_SetWindowTransitionsEnabled_p(
    _In_ const HWND hWnd,
    _In_ const bool enable
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_OpenSystemMenu_p(
    _In_ const HWND  hWnd,
    _In_ const POINT pos
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_CompareSystemVersion_p(
    _In_ const WindowsVersion ver,
    _In_ const VersionCompare comp
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_GenerateGUID_p(
    _Out_ LPWSTR *guid
);

[[nodiscard]] ACRYLICMANAGER_API bool WINAPI
am_ShowErrorMessageFromLastErrorCode_p(
    _In_ LPCWSTR functionName
);

#ifdef __cplusplus
EXTERN_C_END
#endif
