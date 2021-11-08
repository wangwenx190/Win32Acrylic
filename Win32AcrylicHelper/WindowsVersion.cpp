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

#include <SDKDDKVer.h>
#include <Windows.h>
#include "WindowsVersion.h"
#include "OperationResult.h"
#include "Utils.h"
#include "SystemLibrary.h"

const VersionNumber &WindowsVersion::CurrentVersion() noexcept
{
    static bool tried = false;
    static VersionNumber version = VersionNumber();
    if (version.Empty()) {
        if (!tried) {
            tried = true;
            static const auto RtlGetVersion_API = reinterpret_cast<NTSTATUS(WINAPI *)(PRTL_OSVERSIONINFOW)>(SystemLibrary::GetSymbolNoCache(L"ntdll.dll", L"RtlGetVersion"));
            if (RtlGetVersion_API) {
                RTL_OSVERSIONINFOEXW osvi;
                SecureZeroMemory(&osvi, sizeof(osvi));
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                if (RtlGetVersion_API(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi)) == STATUS_SUCCESS) {
                    version.Major(osvi.dwMajorVersion);
                    version.Minor(osvi.dwMinorVersion);
                    version.Patch(osvi.dwBuildNumber);
                } else {
                    PRINT_WIN32_ERROR_MESSAGE(RtlGetVersion, L"Failed to retrieve the current system version.")
                }
            } else {
                Utils::DisplayErrorDialog(L"Failed to resolve symbol \"RtlGetVersion()\" from dynamic link library \"NTDll.dll\".");
            }
        }
    }
    return version;
}

bool WindowsVersion::IsGreaterOrEqual(const VersionNumber &version) noexcept
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

std::wstring WindowsVersion::ToHumanReadableString(const VersionNumber &version) noexcept
{
    std::wstring humanReadableString = {};
    if (version >= Windows11) {
        humanReadableString = L"Windows 11";
    } else if (version >= Windows10_21Half2) {
        humanReadableString = L"Windows 10 Version 21H2 (November 2021 Update)";
    } else if (version >= Windows10_21Half1) {
        humanReadableString = L"Windows 10 Version 21H1 (May 2021 Update)";
    } else if (version >= Windows10_20Half2) {
        humanReadableString = L"Windows 10 Version 20H2 (October 2020 Update)";
    } else if (version >= Windows10_20Half1) {
        humanReadableString = L"Windows 10 Version 2004 (May 2020 Update)";
    } else if (version >= Windows10_19Half2) {
        humanReadableString = L"Windows 10 Version 1909 (November 2019 Update)";
    } else if (version >= Windows10_19Half1) {
        humanReadableString = L"Windows 10 Version 1903 (May 2019 Update)";
    } else if (version >= Windows10_RedStone5) {
        humanReadableString = L"Windows 10 Version 1809 (October 2018 Update)";
    } else if (version >= Windows10_RedStone4) {
        humanReadableString = L"Windows 10 Version 1803 (April 2018 Update)";
    } else if (version >= Windows10_RedStone3) {
        humanReadableString = L"Windows 10 Version 1709 (Fall Creators Update)";
    } else if (version >= Windows10_RedStone2) {
        humanReadableString = L"Windows 10 Version 1703 (Creators Update)";
    } else if (version >= Windows10_RedStone1) {
        humanReadableString = L"Windows 10 Version 1607 (Anniversary Update)";
    } else if (version >= Windows10_ThresHold2) {
        humanReadableString = L"Windows 10 Version 1511 (November Update)";
    } else if (version >= Windows10_ThresHold1) {
        humanReadableString = L"Windows 10 Version 1507";
    } else if (version >= Windows_8_1_Update1) {
        humanReadableString = L"Windows 8.1 with Update 1";
    } else if (version >= Windows_8_1) {
        humanReadableString = L"Windows 8.1";
    } else if (version >= Windows_8) {
        humanReadableString = L"Windows 8";
    } else if (version >= Windows_7_ServicePack1) {
        humanReadableString = L"Windows 7 with Service Pack 1";
    } else if (version >= Windows_7) {
        humanReadableString = L"Windows 7";
    } else if (version >= Windows_Vista_ServicePack2) {
        humanReadableString = L"Windows Vista with Service Pack 2";
    } else if (version >= Windows_Vista_ServicePack1) {
        humanReadableString = L"Windows Vista with Service Pack 1";
    } else if (version >= Windows_Vista) {
        humanReadableString = L"Windows Vista";
    } else if (version >= Windows_XP_64) {
        humanReadableString = L"Windows XP x64 Edition";
    } else if (version >= Windows_XP) {
        humanReadableString = L"Windows XP";
    } else if (version >= Windows_2000) {
        humanReadableString = L"Windows 2000";
    }
    const std::wstring versionString = version.ToString();
    if (humanReadableString.empty()) {
        return versionString;
    } else {
        return (humanReadableString + L" (" + versionString + L")");
    }
}
