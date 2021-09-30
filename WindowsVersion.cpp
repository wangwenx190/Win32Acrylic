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
#include "SystemLibrary.h"
#include "Utils.h"

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS (static_cast<NTSTATUS>(0x00000000L)) // ntsubauth
#endif

using NTSTATUS = LONG;

const VersionNumber &WindowsVersion::CurrentWindowsVersion() noexcept
{
    static bool tried = false;
    static VersionNumber version = VersionNumber();
    if (version.Empty()) {
        if (!tried) {
            tried = true;
            static const auto RtlGetVersionFunc = reinterpret_cast<NTSTATUS(WINAPI *)(PRTL_OSVERSIONINFOW)>(SystemLibrary::GetSymbol(L"NTDll.dll", L"RtlGetVersion"));
            if (RtlGetVersionFunc) {
                RTL_OSVERSIONINFOW osvi;
                SecureZeroMemory(&osvi, sizeof(osvi));
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                if (RtlGetVersionFunc(&osvi) == STATUS_SUCCESS) {
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

bool WindowsVersion::IsWindowsVersionOrGreater(const VersionNumber &version) noexcept
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
