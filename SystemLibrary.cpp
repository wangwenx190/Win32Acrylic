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
#include "Utils.h"
#include <unordered_map>

[[nodiscard]] static inline LPCSTR WideToMulti(LPCWSTR str) noexcept
{
    if (!str || (wcscmp(str, L"") == 0)) {
        return nullptr;
    }
    const int required = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    const auto result = new char[required];
    WideCharToMultiByte(CP_UTF8, 0, str, -1, result, required, nullptr, nullptr);
    return result;
}

class SystemLibraryPrivate
{
public:
    explicit SystemLibraryPrivate(SystemLibrary *q) noexcept;
    ~SystemLibraryPrivate() noexcept;

    void FileName(LPCWSTR fileName) noexcept;
    [[nodiscard]] LPCWSTR FileName() const noexcept;

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
    using LoadLibraryExWSig = decltype(&::LoadLibraryExW);
    static inline LoadLibraryExWSig m_LoadLibraryExWFunc = nullptr;
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
        PRINT_WIN32_ERROR_MESSAGE(VirtualQuery, L"Failed to retrieve the memory basic information.")
    } else {
        const auto kernel32 = static_cast<HMODULE>(mbi.AllocationBase);
        if (kernel32) {
            m_LoadLibraryExWFunc = reinterpret_cast<LoadLibraryExWSig>(GetProcAddress(kernel32, "LoadLibraryExW"));
            if (!m_LoadLibraryExWFunc) {
                PRINT_WIN32_ERROR_MESSAGE(GetProcAddress, L"Failed to resolve symbol LoadLibraryExW().")
            }
        } else {
            OutputDebugStringW(L"Failed to retrieve the base address of Kernel32.dll.");
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

void SystemLibraryPrivate::FileName(LPCWSTR fileName) noexcept
{
    if (IsLoaded()) {
        OutputDebugStringW(L"The library has been loaded already, can't change the file name now.");
        return;
    }
    if (fileName && (wcscmp(fileName, L"") != 0)) {
        m_fileName = fileName;
    }
}

LPCWSTR SystemLibraryPrivate::FileName() const noexcept
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
    if (!m_fileName || (wcscmp(m_fileName, L"") == 0)) {
        OutputDebugStringW(L"The file name has not been set, can't load library now.");
        return false;
    }
    m_module = m_LoadLibraryExWFunc(m_fileName, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!m_module) {
        auto buf = new wchar_t[MAX_PATH];
        swprintf(buf, L"Failed to load dynamic link library \"%s\".", m_fileName);
        PRINT_WIN32_ERROR_MESSAGE(LoadLibraryExW, buf)
        delete [] buf;
        buf = nullptr;
        return false;
    }
    return true;
}

void SystemLibraryPrivate::Unload() noexcept
{
    if (!IsLoaded()) {
        return;
    }
    if (FreeLibrary(m_module) == FALSE) {
        auto buf = new wchar_t[MAX_PATH];
        swprintf(buf, L"Failed to unload dynamic link library \"%s\".", m_fileName);
        PRINT_WIN32_ERROR_MESSAGE(FreeLibrary, buf)
        delete [] buf;
        buf = nullptr;
    }
    m_module = nullptr;
    m_fileName = nullptr;
    m_resolvedSymbols.clear();
}

FARPROC SystemLibraryPrivate::GetSymbol(LPCWSTR function) noexcept
{
    if (!IsLoaded()) {
        if (!Load()) {
            OutputDebugStringW(L"Can't resolve the symbol due to the library can't be loaded.");
            return nullptr;
        }
    }
    if (!function || (wcscmp(function, L"") == 0)) {
        OutputDebugStringW(L"Can't resolve the symbol due to its name is empty.");
        return nullptr;
    }
    const auto search = m_resolvedSymbols.find(function);
    if (search == m_resolvedSymbols.cend()) {
        auto name = WideToMulti(function);
        if (!name/* || (strcmp(name, "") == 0)*/) {
            OutputDebugStringW(L"Can't convert a wide char array to multi-byte char array.");
            return nullptr;
        }
        const auto addr = GetProcAddress(m_module, name);
        delete [] name;
        name = nullptr;
        if (!addr) {
            auto buf = new wchar_t[MAX_PATH];
            swprintf(buf, L"Failed to resolve symbol \"%s\".", function);
            PRINT_WIN32_ERROR_MESSAGE(GetProcAddress, buf)
            delete [] buf;
            buf = nullptr;
        }
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
    FileName(fileName);
}

SystemLibrary::~SystemLibrary() noexcept = default;

void SystemLibrary::FileName(LPCWSTR fileName) noexcept
{
    d_ptr->FileName(fileName);
}

LPCWSTR SystemLibrary::FileName() const noexcept
{
    return d_ptr->FileName();
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
