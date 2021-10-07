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

#include "OperationResult.h"
#include "Utils.h"

class OperationResultPrivate
{
public:
    explicit OperationResultPrivate(OperationResult *q) noexcept;
    ~OperationResultPrivate() noexcept;

    [[nodiscard]] bool Succeeded() const noexcept;

    [[nodiscard]] bool Failed() const noexcept;

    [[nodiscard]] DWORD Code() const noexcept;
    void Code(const DWORD code) noexcept;

    [[nodiscard]] std::wstring Message() const noexcept;

private:
    void TryQueryErrorMessageFromSystem() noexcept;

private:
    OperationResultPrivate(const OperationResultPrivate &) = delete;
    OperationResultPrivate &operator=(const OperationResultPrivate &) = delete;
    OperationResultPrivate(OperationResultPrivate &&) = delete;
    OperationResultPrivate &operator=(OperationResultPrivate &&) = delete;

private:
    OperationResult *q_ptr = nullptr;
    DWORD m_code = 0;
    std::wstring m_message = {};
};

OperationResultPrivate::OperationResultPrivate(OperationResult *q) noexcept
{
    q_ptr = q;
}

OperationResultPrivate::~OperationResultPrivate() noexcept = default;

bool OperationResultPrivate::Succeeded() const noexcept
{
    return (m_code == ERROR_SUCCESS);
}

bool OperationResultPrivate::Failed() const noexcept
{
    return (m_code != ERROR_SUCCESS);
}

DWORD OperationResultPrivate::Code() const noexcept
{
    return m_code;
}

void OperationResultPrivate::Code(const DWORD code) noexcept
{
    m_code = code;
    TryQueryErrorMessageFromSystem();
}

std::wstring OperationResultPrivate::Message() const noexcept
{
    return m_message;
}

void OperationResultPrivate::TryQueryErrorMessageFromSystem() noexcept
{
    LPWSTR buf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, m_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&buf), 0, nullptr) == 0) {
        Utils::DisplayErrorDialog(L"Failed to retrieve error message from system.");
    } else {
        m_message = buf;
        LocalFree(buf);
    }
}

OperationResult::OperationResult(const DWORD code) noexcept
{
    d_ptr = std::make_unique<OperationResultPrivate>(this);
    d_ptr->Code(code);
}

OperationResult::OperationResult(const HRESULT hr) noexcept : OperationResult(CODE_FROM_HRESULT(hr))
{
}

OperationResult::OperationResult() noexcept : OperationResult(GetLastError())
{
}

OperationResult::~OperationResult() noexcept = default;

bool OperationResult::Succeeded() const noexcept
{
    return d_ptr->Succeeded();
}

bool OperationResult::Failed() const noexcept
{
    return d_ptr->Failed();
}

DWORD OperationResult::Code() const noexcept
{
    return d_ptr->Code();
}

std::wstring OperationResult::Message() const noexcept
{
    return d_ptr->Message();
}

OperationResult OperationResult::FromWin32(const DWORD code) noexcept
{
    return OperationResult(code);
}

OperationResult OperationResult::FromHResult(const HRESULT hr) noexcept
{
    return OperationResult(hr);
}
