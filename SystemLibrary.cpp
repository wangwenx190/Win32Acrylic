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
#include "OperationResult.h"
#include "Utils.h"
#include <unordered_map>

[[nodiscard]] static inline std::string WideToMulti(const std::wstring &str) noexcept
{
    if (str.empty()) {
        return {};
    }
    const int required = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    auto buf = new char[required];
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, buf, required, nullptr, nullptr);
    const std::string result = buf;
    delete [] buf;
    buf = nullptr;
    return result;
}

class SystemLibraryPrivate
{
public:
    explicit SystemLibraryPrivate(SystemLibrary *q) noexcept;
    ~SystemLibraryPrivate() noexcept;

    void FileName(const std::wstring &fileName) noexcept;
    [[nodiscard]] std::wstring FileName() const noexcept;

    [[nodiscard]] bool IsLoaded() const noexcept;
    [[nodiscard]] bool Load() noexcept;
    void Unload() noexcept;

    [[nodiscard]] FARPROC GetSymbol(const std::wstring &function) noexcept;

    [[nodiscard]] static FARPROC GetSymbol(const std::wstring &fileName, const std::wstring &function) noexcept;

private:
    SystemLibraryPrivate(const SystemLibraryPrivate &) = delete;
    SystemLibraryPrivate &operator=(const SystemLibraryPrivate &) = delete;
    SystemLibraryPrivate(SystemLibraryPrivate &&) = delete;
    SystemLibraryPrivate &operator=(SystemLibraryPrivate &&) = delete;

private:
    void Initialize() noexcept;

private:
    SystemLibrary *q_ptr = nullptr;
    bool m_failedToLoad = false;
    std::wstring m_fileName = {};
    HMODULE m_module = nullptr;
    std::unordered_map<std::wstring, FARPROC> m_resolvedSymbols = {};
    static inline bool m_tried = false;
    using LoadLibraryExWSig = decltype(&::LoadLibraryExW);
    static inline LoadLibraryExWSig m_LoadLibraryExW_API = nullptr;
};

void SystemLibraryPrivate::Initialize() noexcept
{
    if (m_tried) {
        return;
    }
    m_tried = true;
    if (m_LoadLibraryExW_API) {
        return;
    }
    MEMORY_BASIC_INFORMATION mbi;
    SecureZeroMemory(&mbi, sizeof(mbi));
    if (VirtualQuery(reinterpret_cast<LPCVOID>(&VirtualQuery), &mbi, sizeof(mbi)) == 0) {
        PRINT_WIN32_ERROR_MESSAGE(VirtualQuery, L"Failed to retrieve the memory basic information.")
    } else {
        const auto kernel32 = static_cast<HMODULE>(mbi.AllocationBase);
        if (kernel32) {
            m_LoadLibraryExW_API = reinterpret_cast<LoadLibraryExWSig>(GetProcAddress(kernel32, "LoadLibraryExW"));
            if (!m_LoadLibraryExW_API) {
                PRINT_WIN32_ERROR_MESSAGE(GetProcAddress, L"Failed to resolve symbol \"LoadLibraryExW()\".")
            }
        } else {
            OutputDebugStringW(L"Failed to retrieve the base address of \"Kernel32.dll\".");
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

void SystemLibraryPrivate::FileName(const std::wstring &fileName) noexcept
{
    if (m_fileName != fileName) {
        if (IsLoaded()) {
            OutputDebugStringW(L"The library has been loaded already, can't change the file name now.");
            return;
        }
        if (!fileName.empty()) {
            m_fileName = fileName;
            const auto suffix = m_fileName.find(L".dll");
            if (suffix == std::string::npos) {
                m_fileName += L".dll";
            }
        }
    }
}

std::wstring SystemLibraryPrivate::FileName() const noexcept
{
    return m_fileName;
}

bool SystemLibraryPrivate::IsLoaded() const noexcept
{
    return (m_module != nullptr);
}

bool SystemLibraryPrivate::Load() noexcept
{
    if (m_failedToLoad) {
        return false;
    }
    if (!m_LoadLibraryExW_API) {
        OutputDebugStringW(L"LoadLibraryExW() is not available.");
        return false;
    }
    if (m_fileName.empty()) {
        OutputDebugStringW(L"The file name has not been set, can't load library now.");
        return false;
    }
    m_module = m_LoadLibraryExW_API(m_fileName.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!m_module) {
        m_failedToLoad = true;
        const std::wstring msg = L"Failed to load dynamic link library \"" + m_fileName + L"\".";
        PRINT_WIN32_ERROR_MESSAGE(LoadLibraryExW, msg)
        return false;
    }
    return true;
}

void SystemLibraryPrivate::Unload() noexcept
{
    if (m_failedToLoad) {
        return;
    }
    if (!IsLoaded()) {
        return;
    }
    {
        const std::wstring msg = L"Unloading dynamic link library \"" + m_fileName + L"\" ...";
        OutputDebugStringW(msg.c_str());
    }
    if (FreeLibrary(m_module) == FALSE) {
        const std::wstring msg = L"Failed to unload dynamic link library \"" + m_fileName + L"\".";
        PRINT_WIN32_ERROR_MESSAGE(FreeLibrary, msg)
    }
    m_module = nullptr;
    m_fileName = {};
    if (m_resolvedSymbols.empty()) {
        OutputDebugStringW(L"No symbols cached.");
    } else {
        bool hasSymbol = false;
        std::wstring msg = L"Cached symbols: [";
        for (auto &&symbol : std::as_const(m_resolvedSymbols)) {
            const auto name = symbol.first;
            if (!name.empty()) {
                msg += L"\"" + name + L"()\", ";
                if (!hasSymbol) {
                    hasSymbol = true;
                }
            }
        }
        if (hasSymbol) {
            msg.erase(msg.cend() - 2, msg.cend());
        }
        msg += L"].";
        OutputDebugStringW(msg.c_str());
        m_resolvedSymbols.clear();
    }
}

FARPROC SystemLibraryPrivate::GetSymbol(const std::wstring &function) noexcept
{
    if (m_failedToLoad) {
        return nullptr;
    }
    if (!IsLoaded()) {
        if (!Load()) {
            OutputDebugStringW(L"Can't resolve the symbol due to the library can't be loaded.");
            return nullptr;
        }
    }
    if (function.empty()) {
        OutputDebugStringW(L"Can't resolve the symbol due to its name is empty.");
        return nullptr;
    }
    const auto search = m_resolvedSymbols.find(function);
    if (search == m_resolvedSymbols.cend()) {
        const std::string name = WideToMulti(function);
        if (name.empty()) {
            OutputDebugStringW(L"Can't convert a wide char array to multi-byte char array.");
            return nullptr;
        }
        const auto addr = GetProcAddress(m_module, name.c_str());
        if (!addr) {
            const std::wstring msg = L"Failed to resolve symbol \"" + function + L"()\".";
            PRINT_WIN32_ERROR_MESSAGE(GetProcAddress, msg)
        }
        m_resolvedSymbols.insert({function, addr});
        return addr;
    } else {
        return search->second;
    }
}

FARPROC SystemLibraryPrivate::GetSymbol(const std::wstring &fileName, const std::wstring &function) noexcept
{
    if (fileName.empty() || function.empty()) {
        Utils::DisplayErrorDialog(L"Failed to resolve the given symbol due to the file name or symbol name is empty.");
        return nullptr;
    }
    const HMODULE module = GetModuleHandleW(fileName.c_str());
    if (module) {
        const std::string functionA = WideToMulti(function);
        if (functionA.empty()) {
            Utils::DisplayErrorDialog(L"Failed to convert a wide char array to multi-byte char array.");
            return nullptr;
        } else {
            const FARPROC addr = GetProcAddress(module, functionA.c_str());
            if (addr) {
                return addr;
            } else {
                PRINT_WIN32_ERROR_MESSAGE(GetProcAddress, L"Failed to resolve the given symbol.")
                return nullptr;
            }
        }
    } else {
        PRINT_WIN32_ERROR_MESSAGE(GetModuleHandleW, L"Failed to load the given module.")
        return nullptr;
    }
}

SystemLibrary::SystemLibrary() noexcept
{
    d_ptr = std::make_unique<SystemLibraryPrivate>(this);
}

SystemLibrary::SystemLibrary(const std::wstring &fileName) noexcept : SystemLibrary()
{
    FileName(fileName);
}

SystemLibrary::~SystemLibrary() noexcept = default;

void SystemLibrary::FileName(const std::wstring &fileName) noexcept
{
    d_ptr->FileName(fileName);
}

std::wstring SystemLibrary::FileName() const noexcept
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

FARPROC SystemLibrary::GetSymbol(const std::wstring &function) noexcept
{
    return d_ptr->GetSymbol(function);
}

FARPROC SystemLibrary::GetSymbol(const std::wstring &fileName, const std::wstring &function) noexcept
{
    return SystemLibraryPrivate::GetSymbol(fileName, function);
}
