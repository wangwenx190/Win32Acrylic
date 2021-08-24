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

// Helper classes used for converting between RGB, HSV, and hex.
class Rgb
{
public:
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    explicit Rgb() = default;
    explicit Rgb(const double r, const double g, const double b);
    ~Rgb() = default;
};

class Hsv
{
public:
    double h = 0.0;
    double s = 0.0;
    double v = 0.0;

    explicit Hsv() = default;
    explicit Hsv(const double h, const double s, const double v);
    ~Hsv() = default;
};

std::optional<unsigned long> TryParseInt(const std::wstring_view &s);
std::optional<unsigned long> TryParseInt(const std::wstring_view &str, const int base);

Hsv RgbToHsv(const Rgb &rgb);
Rgb HsvToRgb(const Hsv &hsv);

Rgb HexToRgb(const std::wstring_view &input);
winrt::hstring RgbToHex(const Rgb &rgb);

std::tuple<Rgb, double> HexToRgba(const std::wstring_view &input);
winrt::hstring RgbaToHex(const Rgb &rgb, const double alpha);

winrt::Windows::UI::Color ColorFromRgba(const Rgb &rgb, const double alpha = 1.0);
Rgb RgbFromColor(const winrt::Windows::UI::Color &color);

// We represent HSV and alpha using a Vector4 (float4 in C++/WinRT).
// We'll use the following helper methods to convert between the four dimensions and HSVA.
namespace hsv
{
    inline float GetHue(const winrt::Windows::Foundation::Numerics::float4 &hsva) { return hsva.x; }
    inline void SetHue(winrt::Windows::Foundation::Numerics::float4 &hsva, const float hue) { hsva.x = hue; }

    inline float GetSaturation(const winrt::Windows::Foundation::Numerics::float4 &hsva) { return hsva.y; }
    inline void SetSaturation(winrt::Windows::Foundation::Numerics::float4 &hsva, const float saturation) { hsva.y = saturation; }

    inline float GetValue(const winrt::Windows::Foundation::Numerics::float4 &hsva) { return hsva.z; }
    inline void SetValue(winrt::Windows::Foundation::Numerics::float4 &hsva, const float value) { hsva.z = value; }

    inline float GetAlpha(const winrt::Windows::Foundation::Numerics::float4 &hsva) { return hsva.w; }
    inline void SetAlpha(winrt::Windows::Foundation::Numerics::float4 &hsva, const float alpha) { hsva.w = alpha; }
}
