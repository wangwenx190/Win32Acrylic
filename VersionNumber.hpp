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
#include <iostream>

class VersionNumber
{
public:
    inline explicit constexpr VersionNumber(const int major, const int minor, const int patch, const int tweak) noexcept {
        Major(major);
        Minor(minor);
        Patch(patch);
        Tweak(tweak);
    }
    inline explicit constexpr VersionNumber(const int major, const int minor, const int patch) noexcept : VersionNumber(major, minor, patch, 0) {}
    inline explicit constexpr VersionNumber(const int major, const int minor) noexcept : VersionNumber(major, minor, 0, 0) {}
    inline explicit constexpr VersionNumber(const int major) noexcept : VersionNumber(major, 0, 0, 0) {}
    inline explicit constexpr VersionNumber() noexcept : VersionNumber(0, 0, 0, 0) {}
    inline ~VersionNumber() noexcept = default;

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

    inline constexpr void Patch(const int patch) noexcept {
        m_patch = patch;
    }
    [[nodiscard]] inline constexpr int Patch() const noexcept {
        return m_patch;
    }

    inline constexpr void Tweak(const int tweak) noexcept {
        m_tweak = tweak;
    }
    [[nodiscard]] inline constexpr int Tweak() const noexcept {
        return m_tweak;
    }

    [[nodiscard]] inline constexpr static VersionNumber FromString(LPCWSTR str) noexcept {
        if (!str || (wcscmp(str, L"") == 0)) {
            return VersionNumber();
        } else {
            int major = 0;
            int minor = 0;
            int patch = 0;
            int tweak = 0;
            swscanf(str, L"%d.%d.%d.%d", &major, &minor, &patch, &tweak);
            return VersionNumber(major, minor, patch, tweak);
        }
    }

    [[nodiscard]] inline LPCWSTR ToString() const noexcept {
        const auto buf = new wchar_t[MAX_PATH];
        SecureZeroMemory(buf, sizeof(buf));
        swprintf(buf, L"%d.%d.%d.%d", m_major, m_minor, m_patch, m_tweak);
        return buf;
    }

    inline friend bool operator==(const VersionNumber &lhs, const VersionNumber &rhs) noexcept {
        return ((lhs.Major() == rhs.Major()) && (lhs.Minor() == rhs.Minor()) && (lhs.Patch() == rhs.Patch()) && (lhs.Tweak() == rhs.Tweak()));
    }
    inline friend bool operator!=(const VersionNumber &lhs, const VersionNumber &rhs) noexcept {
        return (!(lhs == rhs));
    }
    inline friend bool operator>(const VersionNumber &lhs, const VersionNumber &rhs) noexcept {
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
        if (lhs.Patch() > rhs.Patch()) {
            return true;
        }
        if (lhs.Patch() < rhs.Patch()) {
            return false;
        }
        if (lhs.Tweak() > rhs.Tweak()) {
            return true;
        }
        if (lhs.Tweak() < rhs.Tweak()) {
            return false;
        }
        return false;
    }
    inline friend bool operator<(const VersionNumber &lhs, const VersionNumber &rhs) noexcept {
        return ((lhs != rhs) && !(lhs > rhs));
    }

private:
    VersionNumber(const VersionNumber &) = delete;
    VersionNumber &operator=(const VersionNumber &) = delete;
    VersionNumber(VersionNumber &&) = delete;
    VersionNumber &operator=(VersionNumber &&) = delete;

private:
    int m_major = 0;
    int m_minor = 0;
    int m_patch = 0;
    int m_tweak = 0;
};

[[maybe_unused]] constexpr VersionNumber Windows_2000 = VersionNumber(5, 0, 2195);
[[maybe_unused]] constexpr VersionNumber Windows_XP = VersionNumber(5, 1, 2600);
[[maybe_unused]] constexpr VersionNumber Windows_XP_x64 = VersionNumber(5, 2, 3790);
[[maybe_unused]] constexpr VersionNumber Windows_Vista = VersionNumber(6, 0, 6000);
[[maybe_unused]] constexpr VersionNumber Windows_Vista_ServicePack1 = VersionNumber(6, 0, 6001);
[[maybe_unused]] constexpr VersionNumber Windows_Vista_ServicePack2 = VersionNumber(6, 0, 6002);
[[maybe_unused]] constexpr VersionNumber Windows_7 = VersionNumber(6, 1, 7600);
[[maybe_unused]] constexpr VersionNumber Windows_7_ServicePack1 = VersionNumber(6, 1, 7601);
[[maybe_unused]] constexpr VersionNumber Windows_8 = VersionNumber(6, 2, 9200);
[[maybe_unused]] constexpr VersionNumber Windows_8_1 = VersionNumber(6, 3, 9200);
[[maybe_unused]] constexpr VersionNumber Windows_8_1_Update1 = VersionNumber(6, 3, 9600);
[[maybe_unused]] constexpr VersionNumber Windows10_ThresHold1 = VersionNumber(10, 0, 10240);
[[maybe_unused]] constexpr VersionNumber Windows10_ThresHold2 = VersionNumber(10, 0, 10586);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone1 = VersionNumber(10, 0, 14393);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone2 = VersionNumber(10, 0, 15063);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone3 = VersionNumber(10, 0, 16299);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone4 = VersionNumber(10, 0, 17134);
[[maybe_unused]] constexpr VersionNumber Windows10_RedStone5 = VersionNumber(10, 0, 17763);
[[maybe_unused]] constexpr VersionNumber Windows10_19Half1 = VersionNumber(10, 0, 18362);
[[maybe_unused]] constexpr VersionNumber Windows10_19Half2 = VersionNumber(10, 0, 18363);
[[maybe_unused]] constexpr VersionNumber Windows10_20Half1 = VersionNumber(10, 0, 19041);
[[maybe_unused]] constexpr VersionNumber Windows10_20Half2 = VersionNumber(10, 0, 19042);
[[maybe_unused]] constexpr VersionNumber Windows10_21Half1 = VersionNumber(10, 0, 19043);

[[nodiscard]] inline bool IsWindowsVersionOrGreater(const VersionNumber &version) noexcept
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = version.Major();
    osvi.dwMinorVersion = version.Minor();
    osvi.dwBuildNumber = version.Patch();
    const BYTE op = VER_GREATER_EQUAL;
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

[[nodiscard]] inline bool IsWindows1019H1OrGreater() noexcept
{
    // Windows 10 Version 1903 (May 2019 Update)
    static const bool result = IsWindowsVersionOrGreater(Windows10_19Half1);
    return result;
}
