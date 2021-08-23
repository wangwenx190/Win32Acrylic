// Copyright (C) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

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
