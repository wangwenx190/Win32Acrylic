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

class OperationResultPrivate;

class OperationResult
{
public:
    explicit OperationResult(const DWORD code) noexcept;
    explicit OperationResult(const HRESULT hr) noexcept;
    explicit OperationResult() noexcept;
    ~OperationResult() noexcept;

    [[nodiscard]] bool Succeeded() const noexcept;
    [[nodiscard]] bool Failed() const noexcept;
    [[nodiscard]] DWORD Code() const noexcept;
    [[nodiscard]] std::wstring Message() const noexcept;

    [[nodiscard]] static OperationResult FromWin32(const DWORD code) noexcept;
    [[nodiscard]] static OperationResult FromHResult(const HRESULT hr) noexcept;

    [[nodiscard]] inline friend bool operator==(const OperationResult &lhs, const OperationResult &rhs) noexcept {
        return (lhs.Code() == rhs.Code());
    }
    [[nodiscard]] inline friend bool operator!=(const OperationResult &lhs, const OperationResult &rhs) noexcept {
        return (lhs.Code() != rhs.Code());
    }
    [[nodiscard]] inline friend bool operator>(const OperationResult &lhs, const OperationResult &rhs) noexcept {
        return (lhs.Code() > rhs.Code());
    }
    [[nodiscard]] inline friend bool operator<(const OperationResult &lhs, const OperationResult &rhs) noexcept {
        return (lhs.Code() < rhs.Code());
    }
    [[nodiscard]] inline friend bool operator>=(const OperationResult &lhs, const OperationResult &rhs) noexcept {
        return ((lhs > rhs) || (lhs == rhs));
    }
    [[nodiscard]] inline friend bool operator<=(const OperationResult &lhs, const OperationResult &rhs) noexcept {
        return ((lhs < rhs) || (lhs == rhs));
    }

private:
    OperationResult(const OperationResult &) = delete;
    OperationResult &operator=(const OperationResult &) = delete;
    OperationResult(OperationResult &&) = delete;
    OperationResult &operator=(OperationResult &&) = delete;

private:
    std::unique_ptr<OperationResultPrivate> d_ptr;
};
