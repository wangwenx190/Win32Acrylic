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

#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif // _CRT_NON_CONFORMING_SWPRINTFS

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // _CRT_SECURE_NO_WARNINGS

#ifndef _ENABLE_EXTENDED_ALIGNED_STORAGE
#define _ENABLE_EXTENDED_ALIGNED_STORAGE
#endif // _ENABLE_EXTENDED_ALIGNED_STORAGE

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#ifndef UNICODE
#define UNICODE
#endif // UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif // _UNICODE

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#ifndef WINRT_LEAN_AND_MEAN
#define WINRT_LEAN_AND_MEAN
#endif // WINRT_LEAN_AND_MEAN

#ifndef _WIN32_WINNT
#define _WIN32_WINNT (0x0A00) // _WIN32_WINNT_WIN10
#endif // _WIN32_WINNT

#ifndef _WIN32_IE
#define _WIN32_IE (0x0A00)
#endif // _WIN32_IE

#ifndef WINVER
#define WINVER (0x0A00)
#endif // WINVER

#ifndef NTDDI_VERSION
#define NTDDI_VERSION (0x0A00000B) // NTDDI_WIN10_CO
#endif // NTDDI_VERSION

#include <SDKDDKVer.h>
#include <Windows.h>
#include <Unknwn.h> // To enable support for non-WinRT interfaces, Unknwn.h must be included before any C++/WinRT headers.

// Avoid collision with WinRT's same name function.
// https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/faq#how-do-i-resolve-ambiguities-with-getcurrenttime-and-or-try-
#pragma push_macro("GetCurrentTime")
#pragma push_macro("TRY")
#undef GetCurrentTime
#undef TRY

#include <WinRT\Base.h>

// Workaround for Windows SDK bug.
// https://github.com/microsoft/Windows.UI.Composition-Win32-Samples/issues/47
namespace winrt::impl
{
    template<typename Async>
    auto wait_for(Async const& async, Windows::Foundation::TimeSpan const& timeout);
}

#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <WinRT\Windows.UI.Composition.Desktop.h>
#include <Windows.UI.Composition.Interop.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>
#include <DispatcherQueue.h>

// Restore the macros from Win32 headers.
#pragma pop_macro("TRY")
#pragma pop_macro("GetCurrentTime")
