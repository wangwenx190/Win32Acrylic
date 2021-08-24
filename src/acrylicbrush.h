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

constexpr winrt::Windows::UI::Color DefaultTintColor = {255, 252, 252, 252};
constexpr double DefaultTintOpacity = 0.0;
constexpr double DefaultLuminosityOpacity = 0.85;
constexpr winrt::Windows::UI::Color DefaultFallbackColor = {255, 249, 249, 249};
constexpr double DefaultBlurRadius = 30.0;
constexpr double DefaultSaturation = 1.25;
constexpr double DefaultNoiseOpacity = 0.02;
constexpr winrt::Windows::UI::Color DefaultExclusionColor = {26, 255, 255, 255};

}

namespace Dark
{

constexpr winrt::Windows::UI::Color DefaultTintColor = {255, 44, 44, 44};
constexpr double DefaultTintOpacity = 0.15;
constexpr double DefaultLuminosityOpacity = 0.96;
constexpr winrt::Windows::UI::Color DefaultFallbackColor = {255, 44, 44, 44};
constexpr double DefaultBlurRadius = 30.0;
constexpr double DefaultSaturation = 1.25;
constexpr double DefaultNoiseOpacity = 0.02;
constexpr winrt::Windows::UI::Color DefaultExclusionColor = {26, 255, 255, 255};

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

[[nodiscard]] double GetSaturation();
void SetSaturation(const double value);

[[nodiscard]] double GetNoiseOpacity();
void SetNoiseOpacity(const double value);

}

namespace System
{

[[nodiscard]] bool IsSupportedByCurrentOS();
[[nodiscard]] bool IsBlurEffectEnabled();
[[nodiscard]] bool SetBlurEffectEnabled(const bool enable);
[[nodiscard]] bool ReloadBlurParameters();
[[nodiscard]] HWND GetWindowHandle();

}

namespace XAML
{

[[nodiscard]] bool IsSupportedByCurrentOS();
[[nodiscard]] bool IsBlurEffectEnabled();
[[nodiscard]] bool SetBlurEffectEnabled(const bool enable);
[[nodiscard]] bool ReloadBlurParameters();
[[nodiscard]] HWND GetWindowHandle();

}

namespace Direct2D
{

[[nodiscard]] bool IsSupportedByCurrentOS();
[[nodiscard]] bool IsBlurEffectEnabled();
[[nodiscard]] bool SetBlurEffectEnabled(const bool enable);
[[nodiscard]] bool ReloadBlurParameters();
[[nodiscard]] HWND GetWindowHandle();

}

}
