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

#include <SDKDDKVer.h>
#include <Windows.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS (static_cast<NTSTATUS>(0x00000000L))
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT (reinterpret_cast<HINSTANCE>(&__ImageBase))
#endif

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI (96)
#endif

#ifndef SM_CXPADDEDBORDER
#define SM_CXPADDEDBORDER (92)
#endif

#ifndef SM_CYPADDEDBORDER
#define SM_CYPADDEDBORDER (SM_CXPADDEDBORDER)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED (0x02E0)
#endif

#ifndef CODE_FROM_HRESULT
#define CODE_FROM_HRESULT(hr) (static_cast<DWORD>(HRESULT_CODE(hr)))
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam) (static_cast<int>(static_cast<short>(LOWORD(lParam))))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam) (static_cast<int>(static_cast<short>(HIWORD(lParam))))
#endif

#ifndef RECT_WIDTH
#define RECT_WIDTH(rect) (std::abs((rect).right - (rect).left))
#endif

#ifndef RECT_HEIGHT
#define RECT_HEIGHT(rect) (std::abs((rect).bottom - (rect).top))
#endif

using NTSTATUS = LONG;

[[maybe_unused]] constexpr const UINT DefaultWindowVisibleFrameBorderThickness = 1;

[[maybe_unused]] constexpr const UINT DefaultAutoHideTaskBarThicknessX = 2;
[[maybe_unused]] constexpr const UINT DefaultAutoHideTaskBarThicknessY = 2;

[[maybe_unused]] constexpr const DWORD _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
[[maybe_unused]] constexpr const DWORD _DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
[[maybe_unused]] constexpr const DWORD _DWMWA_WINDOW_CORNER_PREFERENCE = 33;
[[maybe_unused]] constexpr const DWORD _DWMWA_VISIBLE_FRAME_BORDER_THICKNESS = 37;

[[maybe_unused]] constexpr const int DPI_AWARENESS_PER_MONITOR_AWARE_V2 = 3;
[[maybe_unused]] constexpr const int PROCESS_PER_MONITOR_DPI_AWARE_V2 = 3;
[[maybe_unused]] constexpr const int DPI_AWARENESS_UNAWARE_GDISCALED = 4;
[[maybe_unused]] constexpr const int PROCESS_DPI_UNAWARE_GDISCALED = 4;

[[maybe_unused]] constexpr const wchar_t DwmRegistryKeyPath[] = LR"(Software\Microsoft\Windows\DWM)";
[[maybe_unused]] constexpr const wchar_t PersonalizeRegistryKeyPath[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

enum class DwmWindowCornerPreference : int
{
    DoNotRound = 1,
    Round = 2,
    RoundSmall = 3
};

enum class WindowTheme : int
{
    Light = 0,
    Dark,
    HighContrast
};

enum class WindowState : int
{
    Windowed = 0,
    Minimized,
    Maximized,
    //FullScreen,
    Hidden
};

enum class ProcessDPIAwareness : int
{
    PerMonitorV2 = 0,
    PerMonitor,
    System,
    GdiScaled,
    Unaware
};

enum class WindowColorizationArea : int
{
    None = 0,
    StartMenu_TaskBar_ActionCenter,
    TitleBar_WindowBorder,
    All
};

enum class WindowMetrics : int
{
    ResizeBorderThicknessX = 0,
    ResizeBorderThicknessY,
    WindowVisibleFrameBorderThickness,
    CaptionHeight,
    WindowIconWidth,
    WindowIconHeight,
    WindowSmallIconWidth,
    WindowSmallIconHeight
};

enum class WindowFrameCorner : int
{
    Square = 0,
    Round
};

enum class WindowStartupLocation : int
{
    Default = 0,
    OwnerCenter,
    DesktopCenter, // take the task bar into account
    ScreenCenter // regardless of the task bar
};
