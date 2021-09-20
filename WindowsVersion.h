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

class WindowsVersion
{
public:
    inline explicit constexpr WindowsVersion(const int major, const int minor, const int build) noexcept {
        Major(major);
        Minor(minor);
        Build(build);
    }
    inline ~WindowsVersion() noexcept = default;

    inline constexpr void Major(const int major) noexcept {
        m_major = major;
    }
    [[nodiscard]] inline constexpr int Major() const noexcept {
        return m_major;
    }

    inline constexpr void Minor(const int minor) noexcept {
        m_minor = minor;
    }
    [[nodiscard]] inline constexpr int Minor() const noexcept {
        return m_minor;
    }

    inline constexpr void Build(const int build) noexcept {
        m_build = build;
    }
    [[nodiscard]] inline constexpr int Build() const noexcept {
        return m_build;
    }

    inline friend bool operator==(const WindowsVersion &lhs, const WindowsVersion &rhs) noexcept {
        return ((lhs.Major() == rhs.Major()) && (lhs.Minor() == rhs.Minor()) && (lhs.Build() == rhs.Build()));
    }
    inline friend bool operator!=(const WindowsVersion &lhs, const WindowsVersion &rhs) noexcept {
        return (!(lhs == rhs));
    }
    inline friend bool operator>(const WindowsVersion &lhs, const WindowsVersion &rhs) noexcept {
        if (lhs == rhs) {
            return false;
        }
        if (lhs.Major() > rhs.Major()) {
            return true;
        }
        if (lhs.Major() < rhs.Major()) {
            return false;
        }
        if (lhs.Minor() > rhs.Minor()) {
            return true;
        }
        if (lhs.Minor() < rhs.Minor()) {
            return false;
        }
        if (lhs.Build() > rhs.Build()) {
            return true;
        }
        if (lhs.Build() < rhs.Build()) {
            return false;
        }
        return false;
    }
    inline friend bool operator<(const WindowsVersion &lhs, const WindowsVersion &rhs) noexcept {
        return ((lhs != rhs) && !(lhs > rhs));
    }

private:
    WindowsVersion(const WindowsVersion &) = delete;
    WindowsVersion &operator=(const WindowsVersion &) = delete;
    WindowsVersion(WindowsVersion &&) = delete;
    WindowsVersion &operator=(WindowsVersion &&) = delete;

private:
    int m_major = 0;
    int m_minor = 0;
    int m_build = 0;
};

[[maybe_unused]] constexpr WindowsVersion Windows_2000 = WindowsVersion(5, 0, 2195);
[[maybe_unused]] constexpr WindowsVersion Windows_XP = WindowsVersion(5, 1, 2600);
[[maybe_unused]] constexpr WindowsVersion Windows_XP_x64 = WindowsVersion(5, 2, 3790);
[[maybe_unused]] constexpr WindowsVersion Windows_Vista = WindowsVersion(6, 0, 6000);
[[maybe_unused]] constexpr WindowsVersion Windows_Vista_ServicePack1 = WindowsVersion(6, 0, 6001);
[[maybe_unused]] constexpr WindowsVersion Windows_Vista_ServicePack2 = WindowsVersion(6, 0, 6002);
[[maybe_unused]] constexpr WindowsVersion Windows_7 = WindowsVersion(6, 1, 7600);
[[maybe_unused]] constexpr WindowsVersion Windows_7_ServicePack1 = WindowsVersion(6, 1, 7601);
[[maybe_unused]] constexpr WindowsVersion Windows_8 = WindowsVersion(6, 2, 9200);
[[maybe_unused]] constexpr WindowsVersion Windows_8_1 = WindowsVersion(6, 3, 9200);
[[maybe_unused]] constexpr WindowsVersion Windows_8_1_Update1 = WindowsVersion(6, 3, 9600);
[[maybe_unused]] constexpr WindowsVersion Windows10_ThresHold1 = WindowsVersion(10, 0, 10240);
[[maybe_unused]] constexpr WindowsVersion Windows10_ThresHold2 = WindowsVersion(10, 0, 10586);
[[maybe_unused]] constexpr WindowsVersion Windows10_RedStone1 = WindowsVersion(10, 0, 14393);
[[maybe_unused]] constexpr WindowsVersion Windows10_RedStone2 = WindowsVersion(10, 0, 15063);
[[maybe_unused]] constexpr WindowsVersion Windows10_RedStone3 = WindowsVersion(10, 0, 16299);
[[maybe_unused]] constexpr WindowsVersion Windows10_RedStone4 = WindowsVersion(10, 0, 17134);
[[maybe_unused]] constexpr WindowsVersion Windows10_RedStone5 = WindowsVersion(10, 0, 17763);
[[maybe_unused]] constexpr WindowsVersion Windows10_19Half1 = WindowsVersion(10, 0, 18362);
[[maybe_unused]] constexpr WindowsVersion Windows10_19Half2 = WindowsVersion(10, 0, 18363);
[[maybe_unused]] constexpr WindowsVersion Windows10_20Half1 = WindowsVersion(10, 0, 19041);
[[maybe_unused]] constexpr WindowsVersion Windows10_20Half2 = WindowsVersion(10, 0, 19042);
[[maybe_unused]] constexpr WindowsVersion Windows10_21Half1 = WindowsVersion(10, 0, 19043);

[[nodiscard]] inline bool IsWindowsVersionOrGreater(const WindowsVersion &version) noexcept
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = version.Major();
    osvi.dwMinorVersion = version.Minor();
    osvi.dwBuildNumber = version.Build();
    const BYTE op = VER_GREATER_EQUAL;
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

[[nodiscard]] inline bool IsWindows10RS1OrGreater() noexcept
{
    // Windows 10 Version 1607 (Anniversary Update)
    static const bool result = IsWindowsVersionOrGreater(Windows10_RedStone1);
    return result;
}

[[nodiscard]] inline bool IsWindows1019H1OrGreater() noexcept
{
    // Windows 10 Version 1903 (May 2019 Update)
    static const bool result = IsWindowsVersionOrGreater(Windows10_19Half1);
    return result;
}
