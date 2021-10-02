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

enum class WindowTheme : int
{
    Light = 0,
    Dark,
    HighContrast
};

enum class WindowState : int
{
    Normal = 0,
    Minimized,
    Maximized,
    Visible,
    Hidden
};

enum class DPIAwareness : int
{
    PerMonitorV2 = 0,
    PerMonitor,
    System,
    GdiScaled,
    Unaware
};

enum class WindowMetrics : int
{
    X = 0,
    Y,
    Width,
    Height,
    FrameWidth,
    FrameHeight,
    DotsPerInch,
    ResizeBorderThicknessX,
    ResizeBorderThicknessY,
    CaptionHeight,
    TitleBarHeight,
    FrameBorderThickness,
    DPI = DotsPerInch,
    ResizeBorderThickness = ResizeBorderThicknessX
};

enum class ColorizationArea : int
{
    None = 0,
    StartMenu_TaskBar_ActionCenter,
    TitleBar_WindowBorder,
    All
};
