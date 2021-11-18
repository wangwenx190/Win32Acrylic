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

#include "VersionNumber.hpp"

namespace WindowsVersion
{
    [[maybe_unused]] constexpr const VersionNumber Windows_2000        = VersionNumber( 5, 0,  2195);
    [[maybe_unused]] constexpr const VersionNumber Windows_XP          = VersionNumber( 5, 1,  2600);
    [[maybe_unused]] constexpr const VersionNumber Windows_XP_64       = VersionNumber( 5, 2,  3790);
    [[maybe_unused]] constexpr const VersionNumber Windows_Vista       = VersionNumber( 6, 0,  6000);
    [[maybe_unused]] constexpr const VersionNumber Windows_Vista_SP1   = VersionNumber( 6, 0,  6001);
    [[maybe_unused]] constexpr const VersionNumber Windows_Vista_SP2   = VersionNumber( 6, 0,  6002);
    [[maybe_unused]] constexpr const VersionNumber Windows_7           = VersionNumber( 6, 1,  7600);
    [[maybe_unused]] constexpr const VersionNumber Windows_7_SP1       = VersionNumber( 6, 1,  7601);
    [[maybe_unused]] constexpr const VersionNumber Windows_8           = VersionNumber( 6, 2,  9200);
    [[maybe_unused]] constexpr const VersionNumber Windows_8_1         = VersionNumber( 6, 3,  9200);
    [[maybe_unused]] constexpr const VersionNumber Windows_8_1_Update1 = VersionNumber( 6, 3,  9600);
    [[maybe_unused]] constexpr const VersionNumber Windows10_1507      = VersionNumber(10, 0, 10240); // TH1
    [[maybe_unused]] constexpr const VersionNumber Windows10_1511      = VersionNumber(10, 0, 10586); // TH2
    [[maybe_unused]] constexpr const VersionNumber Windows10_1607      = VersionNumber(10, 0, 14393); // RS1
    [[maybe_unused]] constexpr const VersionNumber Windows10_1703      = VersionNumber(10, 0, 15063); // RS2
    [[maybe_unused]] constexpr const VersionNumber Windows10_1709      = VersionNumber(10, 0, 16299); // RS3
    [[maybe_unused]] constexpr const VersionNumber Windows10_1803      = VersionNumber(10, 0, 17134); // RS4
    [[maybe_unused]] constexpr const VersionNumber Windows10_1809      = VersionNumber(10, 0, 17763); // RS5
    [[maybe_unused]] constexpr const VersionNumber Windows10_1903      = VersionNumber(10, 0, 18362); // 19H1
    [[maybe_unused]] constexpr const VersionNumber Windows10_1909      = VersionNumber(10, 0, 18363); // 19H2
    [[maybe_unused]] constexpr const VersionNumber Windows10_2004      = VersionNumber(10, 0, 19041); // 20H1
    [[maybe_unused]] constexpr const VersionNumber Windows10_20H2      = VersionNumber(10, 0, 19042);
    [[maybe_unused]] constexpr const VersionNumber Windows10_21H1      = VersionNumber(10, 0, 19043);
    [[maybe_unused]] constexpr const VersionNumber Windows10_21H2      = VersionNumber(10, 0, 19044);
    [[maybe_unused]] constexpr const VersionNumber Windows11           = VersionNumber(10, 0, 22000);

    [[nodiscard]] const VersionNumber &CurrentVersion() noexcept;

    [[nodiscard]] bool IsGreaterOrEqual(const VersionNumber &version) noexcept;

    [[nodiscard]] std::wstring ToHumanReadableString(const VersionNumber &version) noexcept;
} // namespace WindowsVersion
