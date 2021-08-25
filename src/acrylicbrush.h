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

class AcrylicBrush
{
    // disable copy move
public:
    explicit AcrylicBrush();
    ~AcrylicBrush();

    [[nodiscard]] winrt::Windows::UI::Color GetTintColor() const;
    void SetTintColor(const winrt::Windows::UI::Color &value) const;

    [[nodiscard]] double GetTintOpacity() const;
    void SetTintOpacity(const double value) const;

    [[nodiscard]] winrt::Windows::Foundation::IReference<double> GetLuminosityOpacity() const;
    void SetLuminosityOpacity(const winrt::Windows::Foundation::IReference<double> value) const;

    [[nodiscard]] winrt::Windows::UI::Color GetFallbackColor() const;
    void SetFallbackColor(const winrt::Windows::UI::Color &value) const;

    [[nodiscard]] double GetBlurRadius() const;
    void SetBlurRadius(const double value) const;

    [[nodiscard]] double GetSaturation() const;
    void SetSaturation(const double value) const;

    [[nodiscard]] double GetNoiseOpacity() const;
    void SetNoiseOpacity(const double value) const;

    [[nodiscard]] winrt::Windows::UI::Color GetExclusionColor() const;
    void SetExclusionColor(const winrt::Windows::UI::Color &value) const;

    [[nodiscard]] virtual bool IsSupportedByCurrentOS() const = 0;
    [[nodiscard]] virtual bool Create() const = 0;
    virtual void ReloadBlurParameters() const = 0;
    [[nodiscard]] virtual HWND GetWindowHandle() const = 0;
    [[nodiscard]] virtual int EventLoopExec() const = 0;
    virtual void Release() = 0;

protected:
    winrt::Windows::UI::Color GetEffectiveTintColor() const;
    winrt::Windows::UI::Color GetEffectiveLuminosityColor() const;

protected:
    static const std::wstring m_windowClassNamePrefix;
};
