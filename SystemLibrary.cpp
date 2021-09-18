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

#include "SystemLibrary.h"
#include <unordered_map>

[[nodiscard]] static inline LPCSTR WideToMulti(LPCWSTR str) noexcept
{
    if (!str) {
        return nullptr;
    }
    const int required = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    const auto result = new char[required];
    SecureZeroMemory(result, sizeof(result));
    WideCharToMultiByte(CP_UTF8, 0, str, -1, result, required, nullptr, nullptr);
    return result;
}

class SystemLibraryPrivate
{
public:
    explicit SystemLibraryPrivate(SystemLibrary *q) noexcept;
    ~SystemLibraryPrivate() noexcept;

    void SetFileName(LPCWSTR fileName) noexcept;
    [[nodiscard]] LPCWSTR GetFileName() const noexcept;

    [[nodiscard]] bool IsLoaded() const noexcept;
    [[nodiscard]] bool Load() noexcept;
    void Unload() noexcept;

    [[nodiscard]] FARPROC GetSymbol(LPCWSTR function) noexcept;

private:
    SystemLibraryPrivate(const SystemLibraryPrivate &) = delete;
    SystemLibraryPrivate &operator=(const SystemLibraryPrivate &) = delete;
    SystemLibraryPrivate(SystemLibraryPrivate &&) = delete;
    SystemLibraryPrivate &operator=(SystemLibraryPrivate &&) = delete;

private:
    void Initialize() noexcept;

private:
    SystemLibrary *q_ptr = nullptr;
    LPCWSTR m_fileName = nullptr;
    HMODULE m_module = nullptr;
    std::unordered_map<LPCWSTR, FARPROC> m_resolvedSymbols = {};
    static inline bool m_tried = false;
    static inline decltype(&::LoadLibraryExW) m_LoadLibraryExWFunc = nullptr;
};

void SystemLibraryPrivate::Initialize() noexcept
{
    if (m_tried) {
        return;
    }
    m_tried = true;
    if (m_LoadLibraryExWFunc) {
        return;
    }
    MEMORY_BASIC_INFORMATION mbi;
    SecureZeroMemory(&mbi, sizeof(mbi));
    if (VirtualQuery(reinterpret_cast<LPCVOID>(&VirtualQuery), &mbi, sizeof(mbi)) == 0) {
        //
    } else {
        const auto kernel32 = static_cast<HMODULE>(mbi.AllocationBase);
        if (kernel32) {
            m_LoadLibraryExWFunc = reinterpret_cast<decltype(&::LoadLibraryExW)>(GetProcAddress(kernel32, "LoadLibraryExW"));
            if (!m_LoadLibraryExWFunc) {
                //
            }
        } else {
            //
        }
    }
}

SystemLibraryPrivate::SystemLibraryPrivate(SystemLibrary *q) noexcept
{
    q_ptr = q;
    Initialize();
}

SystemLibraryPrivate::~SystemLibraryPrivate() noexcept
{
    if (IsLoaded()) {
        Unload();
    }
}

void SystemLibraryPrivate::SetFileName(LPCWSTR fileName) noexcept
{
    if (IsLoaded()) {
        OutputDebugStringW(L"The library has been loaded already, can't change the file name now.");
        return;
    }
    if (fileName) {
        m_fileName = fileName;
    }
}

LPCWSTR SystemLibraryPrivate::GetFileName() const noexcept
{
    return m_fileName;
}

bool SystemLibraryPrivate::IsLoaded() const noexcept
{
    return (m_module != nullptr);
}

bool SystemLibraryPrivate::Load() noexcept
{
    if (!m_LoadLibraryExWFunc) {
        OutputDebugStringW(L"LoadLibraryExW() is not available.");
        return false;
    }
    if (!m_fileName) {
        OutputDebugStringW(L"The file name has not been set, can't load library now.");
        return false;
    }
    m_module = m_LoadLibraryExWFunc(m_fileName, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    return IsLoaded();
}

void SystemLibraryPrivate::Unload() noexcept
{
    if (!IsLoaded()) {
        return;
    }
    m_fileName = nullptr;
    m_resolvedSymbols.clear();
    if (FreeLibrary(m_module) == FALSE) {
        OutputDebugStringW(L"Failed to free library.");
    }
    m_module = nullptr;
}

FARPROC SystemLibraryPrivate::GetSymbol(LPCWSTR function) noexcept
{
    if (!IsLoaded()) {
        if (!Load()) {
            OutputDebugStringW(L"Can't resolve the symbol due to the library can't be loaded.");
            return nullptr;
        }
    }
    if (!function) {
        OutputDebugStringW(L"Can't resolve the symbol due to its name is empty.");
        return nullptr;
    }
    const auto search = m_resolvedSymbols.find(function);
    if (search == m_resolvedSymbols.cend()) {
        const auto name = WideToMulti(function);
        if (!name) {
            OutputDebugStringW(L"Can't convert a wide char array to multi-byte char array.");
            return nullptr;
        }
        const auto addr = GetProcAddress(m_module, name);
        if (!addr) {
            OutputDebugStringW(L"Failed to resolve symbol.");
        }
        delete [] name;
        m_resolvedSymbols.insert({function, addr});
        return addr;
    } else {
        return search->second;
    }
}

SystemLibrary::SystemLibrary() noexcept
{
    d_ptr = std::make_unique<SystemLibraryPrivate>(this);
}

SystemLibrary::SystemLibrary(LPCWSTR fileName) noexcept : SystemLibrary()
{
    SetFileName(fileName);
}

SystemLibrary::~SystemLibrary() noexcept = default;

void SystemLibrary::SetFileName(LPCWSTR fileName) noexcept
{
    d_ptr->SetFileName(fileName);
}

LPCWSTR SystemLibrary::GetFileName() noexcept
{
    return d_ptr->GetFileName();
}

bool SystemLibrary::IsLoaded() const noexcept
{
    return d_ptr->IsLoaded();
}

bool SystemLibrary::Load() noexcept
{
    return d_ptr->Load();
}

void SystemLibrary::Unload() noexcept
{
    d_ptr->Unload();
}

FARPROC SystemLibrary::GetSymbol(LPCWSTR function) noexcept
{
    return d_ptr->GetSymbol(function);
}
