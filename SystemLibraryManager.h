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
#include <memory>

class SystemLibraryManagerPrivate;

class SystemLibraryManager
{
public:
    explicit SystemLibraryManager() noexcept;
    ~SystemLibraryManager() noexcept;

    [[nodiscard]] static SystemLibraryManager &instance() noexcept;

    [[nodiscard]] FARPROC GetSymbol(LPCWSTR fileName, LPCWSTR symbolName) noexcept;

private:
    SystemLibraryManager(const SystemLibraryManager &) = delete;
    SystemLibraryManager &operator=(const SystemLibraryManager &) = delete;
    SystemLibraryManager(SystemLibraryManager &&) = delete;
    SystemLibraryManager &operator=(SystemLibraryManager &&) = delete;

private:
    std::unique_ptr<SystemLibraryManagerPrivate> d_ptr;
};

#ifndef __RESOLVE
#define __RESOLVE(file, symbol) (reinterpret_cast<decltype(&::symbol)>(SystemLibraryManager::instance().GetSymbol(L#file, L#symbol)))
#endif

#ifndef __RESOLVE_API
#define __RESOLVE_API(file, symbol) static const auto symbol##Func = __RESOLVE(file, symbol)
#endif

#ifndef USER32_API
#define USER32_API(symbol) __RESOLVE_API(User32.dll, symbol)
#endif

#ifndef GDI32_API
#define GDI32_API(symbol) __RESOLVE_API(Gdi32.dll, symbol)
#endif

#ifndef ADVAPI32_API
#define ADVAPI32_API(symbol) __RESOLVE_API(AdvApi32.dll, symbol)
#endif

#ifndef OLE32_API
#define OLE32_API(symbol) __RESOLVE_API(Ole32.dll, symbol)
#endif

#ifndef UXTHEME_API
#define UXTHEME_API(symbol) __RESOLVE_API(UxTheme.dll, symbol)
#endif

#ifndef DWMAPI_API
#define DWMAPI_API(symbol) __RESOLVE_API(DwmApi.dll, symbol)
#endif
