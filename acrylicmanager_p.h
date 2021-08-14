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
EXTERN_C_BEGIN
#endif

ACRYLICMANAGER_API [[nodiscard]] UINT getWindowDpi_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] bool isWindowMinimized_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] bool isWindowMaximized_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] bool isWindowFullScreened_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] bool isWindowNoState_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] bool isWindowVisible_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] int getResizeBorderThickness_p(const bool x, const UINT dpi);
ACRYLICMANAGER_API [[nodiscard]] int getCaptionHeight_p(const UINT dpi);
ACRYLICMANAGER_API [[nodiscard]] int getTitleBarHeight_p(const HWND hWnd, const UINT dpi);
ACRYLICMANAGER_API [[nodiscard]] RECT getScreenGeometry_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] RECT getScreenAvailableGeometry_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] bool isCompositionEnabled_p();
ACRYLICMANAGER_API [[nodiscard]] bool triggerFrameChange_p(const HWND hWnd);
ACRYLICMANAGER_API [[nodiscard]] bool setWindowTransitionsEnabled_p(const HWND hWnd, const bool enable);
ACRYLICMANAGER_API [[nodiscard]] bool openSystemMenu_p(const HWND hWnd, const POINT pos);
ACRYLICMANAGER_API [[nodiscard]] bool compareSystemVersion_p(const WindowsVersion ver, const VersionCompare comp);

#ifdef __cplusplus
EXTERN_C_END
#endif
