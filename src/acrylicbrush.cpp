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

static std::optional<winrt::Windows::UI::Color> g_tintColor = std::nullopt;
static std::optional<double> g_tintOpacity = std::nullopt;
static std::optional<double> g_luminosityOpacity = std::nullopt;
static std::optional<winrt::Windows::UI::Color> g_fallbackColor = std::nullopt;
static std::optional<double> g_blurRadius = std::nullopt;
static std::optional<double> g_saturation = std::nullopt;
static std::optional<double> g_noiseOpacity = std::nullopt;
static std::optional<winrt::Windows::UI::Color> g_exclusionColor = std::nullopt;

winrt::Windows::UI::Color AcrylicBrush::Base::GetTintColor()
{
    if (g_tintColor.has_value()) {
        return g_tintColor.value();
    } else {
        if (Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultTintColor;
        } else {
            return AcrylicBrush::Constants::Light::DefaultTintColor;
        }
    }
}

void AcrylicBrush::Base::SetTintColor(const winrt::Windows::UI::Color &value)
{
    g_tintColor = value;
}

double AcrylicBrush::Base::GetTintOpacity()
{
    if (g_tintOpacity.has_value()) {
        return g_tintOpacity.value();
    } else {
        if (Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultTintOpacity;
        } else {
            return AcrylicBrush::Constants::Light::DefaultTintOpacity;
        }
    }
}

void AcrylicBrush::Base::SetTintOpacity(const double value)
{
    g_tintOpacity = value;
}

winrt::Windows::Foundation::IReference<double> AcrylicBrush::Base::GetLuminosityOpacity()
{
    if (g_luminosityOpacity.has_value()) {
        return g_luminosityOpacity.value();
    } else {
        if (Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultLuminosityOpacity;
        } else {
            return AcrylicBrush::Constants::Light::DefaultLuminosityOpacity;
        }
    }
}

void AcrylicBrush::Base::SetLuminosityOpacity(const winrt::Windows::Foundation::IReference<double> value)
{
    if (value != nullptr) {
        g_luminosityOpacity = value.GetDouble();
    } else {
        g_luminosityOpacity = std::nullopt;
    }
}

winrt::Windows::UI::Color AcrylicBrush::Base::GetFallbackColor()
{
    if (g_fallbackColor.has_value()) {
        return g_fallbackColor.value();
    } else {
        if (Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultFallbackColor;
        } else {
            return AcrylicBrush::Constants::Light::DefaultFallbackColor;
        }
    }
}

void AcrylicBrush::Base::SetFallbackColor(const winrt::Windows::UI::Color &value)
{
    g_fallbackColor = value;
}

double AcrylicBrush::Base::GetBlurRadius()
{
    if (g_blurRadius.has_value()) {
        return g_blurRadius.value();
    } else {
        if (Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultBlurRadius;
        } else {
            return AcrylicBrush::Constants::Light::DefaultBlurRadius;
        }
    }
}

void AcrylicBrush::Base::SetBlurRadius(const double value)
{
    g_blurRadius = value;
}

double AcrylicBrush::Base::GetSaturation()
{
    if (g_saturation.has_value()) {
        return g_saturation.value();
    } else {
        if (Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultSaturation;
        } else {
            return AcrylicBrush::Constants::Light::DefaultSaturation;
        }
    }
}

void AcrylicBrush::Base::SetSaturation(const double value)
{
    g_saturation = value;
}

double AcrylicBrush::Base::GetNoiseOpacity()
{
    if (g_noiseOpacity.has_value()) {
        return g_noiseOpacity.value();
    } else {
        if (Utils::ShouldAppsUseDarkMode()) {
            return AcrylicBrush::Constants::Dark::DefaultNoiseOpacity;
        } else {
            return AcrylicBrush::Constants::Light::DefaultNoiseOpacity;
        }
    }
}

void AcrylicBrush::Base::SetNoiseOpacity(const double value)
{
    g_noiseOpacity = value;
}
