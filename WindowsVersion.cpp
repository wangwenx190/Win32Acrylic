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
#include <..\km\wdm.h>
#include "WindowsVersion.h"
#include "SystemLibraryManager.h"
#include "Utils.h"

#ifndef NTDLL_API
#define NTDLL_API(symbol) __RESOLVE_API(NTDll.dll, symbol)
#endif

static VersionNumber g_currentWindowsVersion = VersionNumber();

const VersionNumber &WindowsVersion::CurrentWindowsVersion() noexcept
{
    if (g_currentWindowsVersion.Empty()) {
        NTDLL_API(RtlGetVersion);
        if (RtlGetVersionFunc) {
            RTL_OSVERSIONINFOW osvi;
            SecureZeroMemory(&osvi, sizeof(osvi));
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            RtlGetVersionFunc(&osvi); // It always returns STATUS_SUCCESS.
            g_currentWindowsVersion.Major(osvi.dwMajorVersion);
            g_currentWindowsVersion.Minor(osvi.dwMinorVersion);
            g_currentWindowsVersion.Patch(osvi.dwBuildNumber);
        } else {
            Utils::DisplayErrorDialog(L"RtlGetVersion() is not available.");
        }
    }
    return g_currentWindowsVersion;
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
