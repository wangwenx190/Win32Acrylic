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
#include <string>
#include <memory>

class SystemLibraryPrivate;

class SystemLibrary
{
public:
    explicit SystemLibrary() noexcept;
    explicit SystemLibrary(const std::wstring &fileName) noexcept;
    ~SystemLibrary() noexcept;

    void FileName(const std::wstring &fileName) noexcept;
    [[nodiscard]] std::wstring FileName() const noexcept;

    [[nodiscard]] bool Loaded() const noexcept;
    [[nodiscard]] bool Load(const bool load) noexcept;

    [[nodiscard]] FARPROC GetSymbol(const std::wstring &function) noexcept;

    [[nodiscard]] static FARPROC GetSymbolNoCache(const std::wstring &fileName, const std::wstring &function) noexcept;

private:
    explicit SystemLibrary(const SystemLibrary &) noexcept = delete;
    explicit SystemLibrary(SystemLibrary &&) noexcept = delete;

    SystemLibrary &operator=(const SystemLibrary &) const noexcept = delete;
    SystemLibrary &operator=(SystemLibrary &&) const noexcept = delete;

private:
    std::unique_ptr<SystemLibraryPrivate> d_ptr;
};

[[nodiscard]] bool operator==(const SystemLibrary &lhs, const SystemLibrary &rhs) noexcept;
[[nodiscard]] bool operator!=(const SystemLibrary &lhs, const SystemLibrary &rhs) noexcept;
