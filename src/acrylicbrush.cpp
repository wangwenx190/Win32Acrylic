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

AcrylicBrush::AcrylicBrush() = default;

AcrylicBrush::~AcrylicBrush() = default;

winrt::Windows::UI::Color AcrylicBrush::GetTintColor() const
{
    if (g_tintColor.has_value()) {
        return g_tintColor.value();
    } else {
        if (Utils::IsWindows10_1607OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultTintColor;
        } else {
            return AcrylicBrush::Constants::Light::DefaultTintColor;
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
        if (Utils::IsWindows10_1607OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultTintOpacity;
        } else {
            return AcrylicBrush::Constants::Light::DefaultTintOpacity;
        }
    }
}

void AcrylicBrush::SetTintOpacity(const double value) const
{
    g_tintOpacity = value;
}

winrt::Windows::Foundation::IReference<double> AcrylicBrush::GetLuminosityOpacity() const
{
    if (g_luminosityOpacity.has_value()) {
        return g_luminosityOpacity.value();
    } else {
        if (Utils::IsWindows10_1607OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultLuminosityOpacity;
        } else {
            return AcrylicBrush::Constants::Light::DefaultLuminosityOpacity;
        }
    }
}

void AcrylicBrush::SetLuminosityOpacity(const winrt::Windows::Foundation::IReference<double> value) const
{
    if (value != nullptr) {
        g_luminosityOpacity = value.GetDouble();
    } else {
        g_luminosityOpacity = std::nullopt;
    }
}

winrt::Windows::UI::Color AcrylicBrush::GetFallbackColor() const
{
    if (g_fallbackColor.has_value()) {
        return g_fallbackColor.value();
    } else {
        if (Utils::IsWindows10_1607OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultFallbackColor;
        } else {
            return AcrylicBrush::Constants::Light::DefaultFallbackColor;
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
        if (Utils::IsWindows10_1607OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultBlurRadius;
        } else {
            return AcrylicBrush::Constants::Light::DefaultBlurRadius;
        }
    }
}

void AcrylicBrush::SetBlurRadius(const double value) const
{
    g_blurRadius = value;
}

double AcrylicBrush::GetSaturation() const
{
    if (g_saturation.has_value()) {
        return g_saturation.value();
    } else {
        if (Utils::IsWindows10_1607OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultSaturation;
        } else {
            return AcrylicBrush::Constants::Light::DefaultSaturation;
        }
    }
}

void AcrylicBrush::SetSaturation(const double value) const
{
    g_saturation = value;
}

double AcrylicBrush::GetNoiseOpacity() const
{
    if (g_noiseOpacity.has_value()) {
        return g_noiseOpacity.value();
    } else {
        if (Utils::IsWindows10_1607OrGreater() && Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultNoiseOpacity;
        } else {
            return AcrylicBrush::Constants::Light::DefaultNoiseOpacity;
        }
    }
}

void AcrylicBrush::SetNoiseOpacity(const double value) const
{
    g_noiseOpacity = value;
}
