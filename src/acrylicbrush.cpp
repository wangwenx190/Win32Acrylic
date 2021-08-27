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

#include "acrylicbrush.h"
#include "utils.h"
#include "colorconversion.h"

namespace Constants
{

namespace Light
{

static constexpr winrt::Windows::UI::Color DefaultTintColor = {255, 252, 252, 252};
static constexpr double DefaultTintOpacity = 0.0;
static constexpr double DefaultLuminosityOpacity = 0.85;
static constexpr winrt::Windows::UI::Color DefaultFallbackColor = {255, 249, 249, 249};
static constexpr double DefaultBlurRadius = 30.0;
static constexpr double DefaultSaturation = 1.25;
static constexpr double DefaultNoiseOpacity = 0.02;
static constexpr winrt::Windows::UI::Color DefaultExclusionColor = {26, 255, 255, 255};

} // namespace Light

namespace Dark
{

static constexpr winrt::Windows::UI::Color DefaultTintColor = {255, 44, 44, 44};
static constexpr double DefaultTintOpacity = 0.15;
static constexpr double DefaultLuminosityOpacity = 0.96;
static constexpr winrt::Windows::UI::Color DefaultFallbackColor = {255, 44, 44, 44};
static constexpr double DefaultBlurRadius = 30.0;
static constexpr double DefaultSaturation = 1.25;
static constexpr double DefaultNoiseOpacity = 0.02;
static constexpr winrt::Windows::UI::Color DefaultExclusionColor = {26, 255, 255, 255};

} // namespace Dark

} // namespace Constants

static std::optional<winrt::Windows::UI::Color> g_tintColor = std::nullopt;
static std::optional<double> g_tintOpacity = std::nullopt;
static std::optional<double> g_luminosityOpacity = std::nullopt;
static std::optional<winrt::Windows::UI::Color> g_fallbackColor = std::nullopt;
static std::optional<double> g_blurRadius = std::nullopt;
static std::optional<double> g_saturation = std::nullopt;
static std::optional<double> g_noiseOpacity = std::nullopt;
static std::optional<winrt::Windows::UI::Color> g_exclusionColor = std::nullopt;

[[nodiscard]] static inline double GetTintOpacityModifier(const winrt::Windows::UI::Color &tintColor)
{
    // TintOpacityModifier affects the 19H1+ Luminosity-based recipe only
    if (!Utils::IsWindows1019H1OrGreater()) {
        return 1.0;
    }

    // This method supresses the maximum allowable tint opacity depending on the luminosity and saturation of a color by
    // compressing the range of allowable values - for example, a user-defined value of 100% will be mapped to 45% for pure
    // white (100% luminosity), 85% for pure black (0% luminosity), and 90% for pure gray (50% luminosity).  The intensity of
    // the effect increases linearly as luminosity deviates from 50%.  After this effect is calculated, we cancel it out
    // linearly as saturation increases from zero.

    constexpr double midPoint = 0.50; // Mid point of HsvV range that these calculations are based on. This is here for easy tuning.

    constexpr double whiteMaxOpacity = 0.45; // 100% luminosity
    constexpr double midPointMaxOpacity = 0.90; // 50% luminosity
    constexpr double blackMaxOpacity = 0.85; // 0% luminosity

    const Rgb rgb = RgbFromColor(tintColor);
    const Hsv hsv = RgbToHsv(rgb);

    double opacityModifier = midPointMaxOpacity;

    if (hsv.v != midPoint) {
        // Determine maximum suppression amount
        double lowestMaxOpacity = midPointMaxOpacity;
        double maxDeviation = midPoint;

        if (hsv.v > midPoint) {
            lowestMaxOpacity = whiteMaxOpacity; // At white (100% hsvV)
            maxDeviation = (1.0 - maxDeviation);
        } else if (hsv.v < midPoint) {
            lowestMaxOpacity = blackMaxOpacity; // At black (0% hsvV)
        }

        double maxOpacitySuppression = (midPointMaxOpacity - lowestMaxOpacity);

        // Determine normalized deviation from the midpoint
        const double deviation = std::abs(hsv.v - midPoint);
        const double normalizedDeviation = (deviation / maxDeviation);

        // If we have saturation, reduce opacity suppression to allow that color to come through more
        if (hsv.s > 0) {
            // Dampen opacity suppression based on how much saturation there is
            maxOpacitySuppression *= std::max((1.0 - (hsv.s * 2.0)), 0.0);
        }

        const double opacitySuppression = (maxOpacitySuppression * normalizedDeviation);

        opacityModifier = (midPointMaxOpacity - opacitySuppression);
    }

    return opacityModifier;
}

// The tintColor passed into this method should be the original, unmodified color created using user values for TintColor + TintOpacity
[[nodiscard]] static inline winrt::Windows::UI::Color GetLuminosityColor(
        const winrt::Windows::UI::Color &tintColor,
        const winrt::Windows::Foundation::IReference<double> luminosityOpacity)
{
    const Rgb rgbTintColor = RgbFromColor(tintColor);

    // If luminosity opacity is specified, just use the values as is
    if (luminosityOpacity != nullptr) {
        return ColorFromRgba(rgbTintColor, std::clamp(luminosityOpacity.GetDouble(), 0.0, 1.0));
    } else {
        // To create the Luminosity blend input color without luminosity opacity,
        // we're taking the TintColor input, converting to HSV, and clamping the V between these values
        constexpr double minHsvV = 0.125;
        constexpr double maxHsvV = 0.965;

        const Hsv hsvTintColor = RgbToHsv(rgbTintColor);

        const auto clampedHsvV = std::clamp(hsvTintColor.v, minHsvV, maxHsvV);

        const Hsv hsvLuminosityColor = Hsv(hsvTintColor.h, hsvTintColor.s, clampedHsvV);
        const Rgb rgbLuminosityColor = HsvToRgb(hsvLuminosityColor);

        // Now figure out luminosity opacity
        // Map original *tint* opacity to this range
        constexpr double minLuminosityOpacity = 0.15;
        constexpr double maxLuminosityOpacity = 1.03;

        const double luminosityOpacityRangeMax = (maxLuminosityOpacity - minLuminosityOpacity);
        const double mappedTintOpacity = (((tintColor.A / 255.0) * luminosityOpacityRangeMax) + minLuminosityOpacity);

        // Finally, combine the luminosity opacity and the HsvV-clamped tint color
        return ColorFromRgba(rgbLuminosityColor, std::min(mappedTintOpacity, 1.0));
    }
}

AcrylicBrush::AcrylicBrush() = default;

AcrylicBrush::~AcrylicBrush() = default;

winrt::Windows::UI::Color AcrylicBrush::GetTintColor() const
{
    if (g_tintColor.has_value()) {
        return g_tintColor.value();
    } else {
        if (Utils::IsWindows10RS1OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return Constants::Dark::DefaultTintColor;
        } else {
            return Constants::Light::DefaultTintColor;
        }
    }
}

void AcrylicBrush::SetTintColor(const winrt::Windows::UI::Color &value) const
{
    g_tintColor = value;
}

double AcrylicBrush::GetTintOpacity() const
{
    if (g_tintOpacity.has_value()) {
        return g_tintOpacity.value();
    } else {
        if (Utils::IsWindows10RS1OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return Constants::Dark::DefaultTintOpacity;
        } else {
            return Constants::Light::DefaultTintOpacity;
        }
    }
}

void AcrylicBrush::SetTintOpacity(const double value) const
{
    g_tintOpacity = std::clamp(value, 0.0, 1.0);
}

double AcrylicBrush::GetLuminosityOpacity() const
{
    if (g_luminosityOpacity.has_value()) {
        return g_luminosityOpacity.value();
    } else {
        if (Utils::IsWindows10RS1OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return Constants::Dark::DefaultLuminosityOpacity;
        } else {
            return Constants::Light::DefaultLuminosityOpacity;
        }
    }
}

void AcrylicBrush::SetLuminosityOpacity(const double value) const
{
    if (value >= 0.0) {
        g_luminosityOpacity = std::clamp(value, 0.0, 1.0);
    } else {
        g_luminosityOpacity = std::nullopt;
    }
}

winrt::Windows::UI::Color AcrylicBrush::GetFallbackColor() const
{
    if (g_fallbackColor.has_value()) {
        return g_fallbackColor.value();
    } else {
        if (Utils::IsWindows10RS1OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return Constants::Dark::DefaultFallbackColor;
        } else {
            return Constants::Light::DefaultFallbackColor;
        }
    }
}

void AcrylicBrush::SetFallbackColor(const winrt::Windows::UI::Color &value) const
{
    g_fallbackColor = value;
}

double AcrylicBrush::GetBlurRadius() const
{
    if (g_blurRadius.has_value()) {
        return g_blurRadius.value();
    } else {
        if (Utils::IsWindows10RS1OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return Constants::Dark::DefaultBlurRadius;
        } else {
            return Constants::Light::DefaultBlurRadius;
        }
    }
}

void AcrylicBrush::SetBlurRadius(const double value) const
{
    g_blurRadius = value; // fixme: what's the valid range
}

double AcrylicBrush::GetSaturation() const
{
    if (g_saturation.has_value()) {
        return g_saturation.value();
    } else {
        if (Utils::IsWindows10RS1OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return Constants::Dark::DefaultSaturation;
        } else {
            return Constants::Light::DefaultSaturation;
        }
    }
}

void AcrylicBrush::SetSaturation(const double value) const
{
    g_saturation = value; // fixme: what's the valid range
}

double AcrylicBrush::GetNoiseOpacity() const
{
    if (g_noiseOpacity.has_value()) {
        return g_noiseOpacity.value();
    } else {
        if (Utils::IsWindows10RS1OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return Constants::Dark::DefaultNoiseOpacity;
        } else {
            return Constants::Light::DefaultNoiseOpacity;
        }
    }
}

void AcrylicBrush::SetNoiseOpacity(const double value) const
{
    g_noiseOpacity = std::clamp(value, 0.0, 1.0);
}

winrt::Windows::UI::Color AcrylicBrush::GetExclusionColor() const
{
    if (g_exclusionColor.has_value()) {
        return g_exclusionColor.value();
    } else {
        if (Utils::IsWindows10RS1OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return Constants::Dark::DefaultExclusionColor;
        } else {
            return Constants::Light::DefaultExclusionColor;
        }
    }
}

void AcrylicBrush::SetExclusionColor(const winrt::Windows::UI::Color &value) const
{
    g_exclusionColor = value;
}

winrt::Windows::UI::Color AcrylicBrush::GetEffectiveTintColor() const
{
    winrt::Windows::UI::Color tintColor = GetTintColor();
    const double tintOpacity = GetTintOpacity();

    // Update tintColor's alpha with the combined opacity value
    // If LuminosityOpacity was specified, we don't intervene into users parameters
    if (GetLuminosityOpacity() != nullptr) {
        tintColor.A = static_cast<uint8_t>(std::round(tintColor.A * tintOpacity));
    } else {
        const double tintOpacityModifier = GetTintOpacityModifier(tintColor);
        tintColor.A = static_cast<uint8_t>(std::round(tintColor.A * tintOpacity * tintOpacityModifier));
    }

    return tintColor;
}

winrt::Windows::UI::Color AcrylicBrush::GetEffectiveLuminosityColor() const
{
    winrt::Windows::UI::Color tintColor = GetTintColor();
    const double tintOpacity = GetTintOpacity();

    // Purposely leaving out tint opacity modifier here because GetLuminosityColor needs the *original* tint opacity set by the user.
    tintColor.A = static_cast<uint8_t>(std::round(tintColor.A * tintOpacity));

    winrt::Windows::Foundation::IReference<double> luminosityOpacity = GetLuminosityOpacity();

    return GetLuminosityColor(tintColor, luminosityOpacity);
}
