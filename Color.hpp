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
#include <cmath>

class Color
{
    static inline constexpr const double denominator = 255.0;
public:
    inline constexpr explicit Color(const double r, const double g, const double b, const double a) noexcept {
        m_r = r;
        m_g = g;
        m_b = b;
        m_a = a;
    }
    inline constexpr explicit Color(const double r, const double g, const double b) noexcept : Color(r, g, b, 1.0) {}
    inline constexpr explicit Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) noexcept {
        m_r = (static_cast<double>(r) / denominator);
        m_g = (static_cast<double>(g) / denominator);
        m_b = (static_cast<double>(b) / denominator);
        m_a = (static_cast<double>(a) / denominator);
    }
    inline constexpr explicit Color(const uint8_t r, const uint8_t g, const uint8_t b) noexcept : Color(r, g, b, 255) {}
    inline constexpr explicit Color(const COLORREF color) noexcept {
        m_r = (static_cast<double>(GetRValue(color)) / denominator);
        m_g = (static_cast<double>(GetGValue(color)) / denominator);
        m_b = (static_cast<double>(GetBValue(color)) / denominator);
        m_a = denominator;
    }
    inline constexpr explicit Color() noexcept {}
    inline ~Color() noexcept = default;

    [[nodiscard]] inline constexpr double RedF() const noexcept {
        return m_r;
    }
    inline constexpr void RedF(const double value) noexcept {
        m_r = value;
    }

    [[nodiscard]] inline constexpr double GreenF() const noexcept {
        return m_g;
    }
    inline constexpr void GreenF(const double value) noexcept {
        m_g = value;
    }

    [[nodiscard]] inline constexpr double BlueF() const noexcept {
        return m_b;
    }
    inline constexpr void BlueF(const double value) noexcept {
        m_b = value;
    }

    [[nodiscard]] inline constexpr double AlphaF() const noexcept {
        return m_a;
    }
    inline constexpr void AlphaF(const double value) noexcept {
        m_a = value;
    }

    [[nodiscard]] inline uint8_t Red() const noexcept {
        return static_cast<uint8_t>(std::round(m_r * denominator));
    }
    inline constexpr void Red(const uint8_t value) noexcept {
        m_r = (static_cast<double>(value) / denominator);
    }

    [[nodiscard]] inline uint8_t Green() const noexcept {
        return static_cast<uint8_t>(std::round(m_g * denominator));
    }
    inline constexpr void Green(const uint8_t value) noexcept {
        m_g = (static_cast<double>(value) / denominator);
    }

    [[nodiscard]] inline uint8_t Blue() const noexcept {
        return static_cast<uint8_t>(std::round(m_b * denominator));
    }
    inline constexpr void Blue(const uint8_t value) noexcept {
        m_b = (static_cast<double>(value) / denominator);
    }

    [[nodiscard]] inline uint8_t Alpha() const noexcept {
        return static_cast<uint8_t>(std::round(m_a * denominator));
    }
    inline constexpr void Alpha(const uint8_t value) noexcept {
        m_a = (static_cast<double>(value) / denominator);
    }

    [[nodiscard]] inline COLORREF ToWin32() const noexcept {
        return RGB(Red(), Green(), Blue());
    }

    [[nodiscard]] inline constexpr static Color FromRgba(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) noexcept {
        return Color(r, g, b, a);
    }
    [[nodiscard]] inline constexpr static Color FromRgbaF(const double r, const double g, const double b, const double a = 1.0) noexcept {
        return Color(r, g, b, a);
    }
    [[nodiscard]] inline constexpr static Color FromWin32(const COLORREF color) noexcept {
        return Color(color);
    }

    [[nodiscard]] inline constexpr friend bool operator==(const Color &lhs, const Color &rhs) noexcept {
        return ((lhs.m_r == rhs.m_r) && (lhs.m_g == rhs.m_g) && (lhs.m_b == rhs.m_b) && (lhs.m_a == rhs.m_a));
    }
    [[nodiscard]] inline constexpr friend bool operator!=(const Color &lhs, const Color &rhs) noexcept {
        return (!(lhs == rhs));
    }

#if 0
private:
    Color(const Color &) = delete;
    Color &operator=(const Color &) = delete;
    Color(Color &&) = delete;
    Color &operator=(Color &&) = delete;
#endif

private:
    double m_r = 0.0;
    double m_g = 0.0;
    double m_b = 0.0;
    double m_a = 0.0;
};
