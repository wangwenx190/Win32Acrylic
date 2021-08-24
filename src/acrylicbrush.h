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

#include <Unknwn.h>
#include <WinRT\Windows.UI.h>
#include <WinRT\Windows.Foundation.Numerics.h>

namespace AcrylicBrush
{

namespace Constants
{

namespace Light
{

constexpr winrt::Windows::UI::Color DefaultTintColor = {};
constexpr double DefaultTintOpacity = {};
constexpr double DefaultLuminosityOpacity = {};
constexpr winrt::Windows::UI::Color DefaultFallbackColor = {};
constexpr double DefaultBlurRadius = {};
constexpr double DefaultSaturationAmount = {};
constexpr double DefaultNoiseOpacity = {};
constexpr winrt::Windows::UI::Color DefaultExclusionColor = {};

}

namespace Dark
{

constexpr winrt::Windows::UI::Color DefaultTintColor = {};
constexpr double DefaultTintOpacity = {};
constexpr double DefaultLuminosityOpacity = {};
constexpr winrt::Windows::UI::Color DefaultFallbackColor = {};
constexpr double DefaultBlurRadius = {};
constexpr double DefaultSaturationAmount = {};
constexpr double DefaultNoiseOpacity = {};
constexpr winrt::Windows::UI::Color DefaultExclusionColor = {};

}

}

namespace Base
{

[[nodiscard]] winrt::Windows::UI::Color GetTintColor();
void SetTintColor(const winrt::Windows::UI::Color &value);

[[nodiscard]] double GetTintOpacity();
void SetTintOpacity(const double value);

[[nodiscard]] winrt::Windows::Foundation::IReference<double> GetLuminosityOpacity();
void SetLuminosityOpacity(const winrt::Windows::Foundation::IReference<double> value);

[[nodiscard]] winrt::Windows::UI::Color GetFallbackColor();
void SetFallbackColor(const winrt::Windows::UI::Color &value);

[[nodiscard]] double GetBlurRadius();
void SetBlurRadius(const double value);

[[nodiscard]] double GetSaturationAmount();
void SetSaturationAmount(const double value);

[[nodiscard]] double GetNoiseOpacity();
void SetNoiseOpacity(const double value);

}

namespace System
{

[[nodiscard]] bool IsSupportedByCurrentOS();
[[nodiscard]] bool IsBlurEffectEnabled();
[[nodiscard]] bool SetBlurEffectEnabled(const bool enable);
[[nodiscard]] HWND GetWindowHandle();

}

namespace XAML
{

[[nodiscard]] bool IsSupportedByCurrentOS();
[[nodiscard]] bool IsBlurEffectEnabled();
[[nodiscard]] bool SetBlurEffectEnabled(const bool enable);
[[nodiscard]] HWND GetWindowHandle();

}

namespace Direct2D
{

[[nodiscard]] bool IsSupportedByCurrentOS();
[[nodiscard]] bool IsBlurEffectEnabled();
[[nodiscard]] bool SetBlurEffectEnabled(const bool enable);
[[nodiscard]] HWND GetWindowHandle();

}

}
