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
#include <VersionHelpers.h>
#include <unordered_map>

static constexpr const wchar_t __NEW_LINE[] = L"\r\n";

static bool g_bSystemEnvironmentDetected = false;
static bool g_bLoadFromSystem32Available = false;

[[nodiscard]] static inline std::string UTF16ToUTF8(const std::wstring &UTF16String) noexcept
{
    if (UTF16String.empty()) {
        return {};
    }
    const auto originalLength = static_cast<int>(UTF16String.size());
    const auto originalString = &UTF16String[0];
    const int newLength = WideCharToMultiByte(CP_UTF8, 0, originalString, originalLength, nullptr, 0, nullptr, nullptr);
    if (newLength <= 0) {
        return {};
    }
    std::string UTF8String(newLength, '\0');
    WideCharToMultiByte(CP_UTF8, 0, originalString, originalLength, &UTF8String[0], newLength, nullptr, nullptr);
    return UTF8String;
}

[[nodiscard]] static inline std::wstring UTF8ToUTF16(const std::string &UTF8String) noexcept
{
    if (UTF8String.empty()) {
        return {};
    }
    const auto originalLength = static_cast<int>(UTF8String.size());
    const auto originalString = &UTF8String[0];
    const int newLength = MultiByteToWideChar(CP_UTF8, 0, originalString, originalLength, nullptr, 0);
    if (newLength <= 0) {
        return {};
    }
    std::wstring UTF16String(newLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, originalString, originalLength, &UTF16String[0], newLength);
    return UTF16String;
}

class SystemLibraryPrivate
{
public:
    explicit SystemLibraryPrivate(SystemLibrary *q) noexcept;
    ~SystemLibraryPrivate() noexcept;

    void FileName(const std::wstring &fileName) noexcept;
    [[nodiscard]] std::wstring FileName() const noexcept;

    [[nodiscard]] bool Loaded() const noexcept;
    [[nodiscard]] bool Load(const bool load) noexcept;

    [[nodiscard]] FARPROC GetSymbol(const std::wstring &function) noexcept;

    [[nodiscard]] static FARPROC GetSymbolNoCache(const std::wstring &fileName, const std::wstring &function) noexcept;

private:
    explicit SystemLibraryPrivate(const SystemLibraryPrivate &) noexcept = delete;
    explicit SystemLibraryPrivate(SystemLibraryPrivate &&) noexcept = delete;

    SystemLibraryPrivate &operator=(const SystemLibraryPrivate &) const noexcept = delete;
    SystemLibraryPrivate &operator=(SystemLibraryPrivate &&) const noexcept = delete;

private:
    SystemLibrary *q_ptr = nullptr;
    bool m_failedToLoad = false;
    std::wstring m_fileName = {};
    HMODULE m_module = nullptr;
    std::unordered_map<std::wstring, FARPROC> m_resolvedSymbols = {};
};

SystemLibraryPrivate::SystemLibraryPrivate(SystemLibrary *q) noexcept
{
    q_ptr = q;
    if (!g_bSystemEnvironmentDetected) {
        g_bSystemEnvironmentDetected = true;
        if (IsWindows8OrGreater()) {
            g_bLoadFromSystem32Available = true;
        } else {
            g_bLoadFromSystem32Available = (GetSymbolNoCache(L"kernel32.dll", L"AddDllDirectory") != nullptr);
        }
        std::wstring dbgMsg = LR"("LOAD_LIBRARY_SEARCH_SYSTEM32" is )";
        if (!g_bLoadFromSystem32Available) {
            dbgMsg += L"not ";
        }
        dbgMsg += std::wstring(L"available on the current platform.") + std::wstring(__NEW_LINE);
        OutputDebugStringW(dbgMsg.c_str());
    }
}

SystemLibraryPrivate::~SystemLibraryPrivate() noexcept
{
    if (Loaded()) {
        const bool result = Load(false);
        // The result is not important here.
        UNREFERENCED_PARAMETER(result);
    }
}

void SystemLibraryPrivate::FileName(const std::wstring &fileName) noexcept
{
    if (Loaded()) {
        // You should unload the library first to be able to set a different filename.
        return;
    }
    if (fileName.empty()) {
        // To unload the library, call "Load(false)" explicitly.
        return;
    }
    if (m_fileName != fileName) {
        // We assume the given filename is in good form.
        m_fileName = fileName;
    }
}

std::wstring SystemLibraryPrivate::FileName() const noexcept
{
    return m_fileName;
}

bool SystemLibraryPrivate::Loaded() const noexcept
{
    return (m_module != nullptr);
}

bool SystemLibraryPrivate::Load(const bool load) noexcept
{
    if (load) {
        if (Loaded()) {
            // No need to reload an already loaded library.
            return true;
        }
        if (m_failedToLoad) {
            // Avoid loading a library which can't be loaded over and over again.
            return false;
        }
        if (m_fileName.empty()) {
            OutputDebugStringW(L"Can't load the system library now due to the file name has not been set yet.");
            return false;
        }
        {
            const std::wstring dbgMsg = std::wstring(LR"(Loading system library ")") + m_fileName + std::wstring(LR"(" ......)") + std::wstring(__NEW_LINE);
            OutputDebugStringW(dbgMsg.c_str());
        }
        HMODULE module = nullptr;
        if (g_bLoadFromSystem32Available) {
            module = LoadLibraryExW(m_fileName.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        } else {
            module = LoadLibraryW(m_fileName.c_str());
        }
        if (!module) {
            const std::wstring dbgMsg = std::wstring(L"Loading failed.") + std::wstring(__NEW_LINE);
            OutputDebugStringW(dbgMsg.c_str());
            m_failedToLoad = true;
            return false;
        }
        {
            const std::wstring dbgMsg = std::wstring(L"Loading finished successfully.") + std::wstring(__NEW_LINE);
            OutputDebugStringW(dbgMsg.c_str());
        }
        m_module = module;
    } else {
        if (!Loaded()) {
            // No need to unload a library which has not been loaded yet.
            return true;
        }
        {
            const std::wstring dbgMsg = std::wstring(LR"(Unloading system library ")") + m_fileName + std::wstring(LR"(" ......)") + std::wstring(__NEW_LINE);
            OutputDebugStringW(dbgMsg.c_str());
        }
        m_fileName = {};
        if (!m_resolvedSymbols.empty()) {
            bool hasContent = false;
            std::wstring dbgMsg = L"Cached symbols: [";
            for (auto &&symbol : std::as_const(m_resolvedSymbols)) {
                const std::wstring &name = symbol.first;
                // It may never be empty, but let's be safe.
                if (!name.empty()) {
                    dbgMsg += name + std::wstring(L"(), ");
                    if (!hasContent) {
                        hasContent = true;
                    }
                }
            }
            if (hasContent) {
                dbgMsg.erase(dbgMsg.cend() - 2);
            }
            dbgMsg += std::wstring(L"]") + std::wstring(__NEW_LINE);
            OutputDebugStringW(dbgMsg.c_str());
            m_resolvedSymbols = {};
        }
        // Reset it to "false" to avoid blocking us from re-use the current instance.
        m_failedToLoad = false;
        const BOOL result = FreeLibrary(m_module);
        m_module = nullptr;
        if (result == FALSE) {
            const std::wstring dbgMsg = std::wstring(L"Unloading failed.") + std::wstring(__NEW_LINE);
            OutputDebugStringW(dbgMsg.c_str());
            return false;
        }
        {
            const std::wstring dbgMsg = std::wstring(L"Unloading finished successfully.") + std::wstring(__NEW_LINE);
            OutputDebugStringW(dbgMsg.c_str());
        }
    }
    return true;
}

FARPROC SystemLibraryPrivate::GetSymbol(const std::wstring &function) noexcept
{
    if (m_failedToLoad) {
        // Don't try further if the library can't be loaded successfully.
        return nullptr;
    }
    // Return early if the parameter is not valid.
    if (function.empty()) {
        return nullptr;
    }
    if (!Loaded()) {
        if (!Load(true)) {
            return nullptr;
        }
    }
    bool shouldInsert = true;
    FARPROC address = nullptr;
    const auto resolve = [this](const std::wstring &nameWide) -> FARPROC {
        // "nameWide" may never be empty, but let's be safe.
        if (nameWide.empty()) {
            return nullptr;
        }
        const std::string nameMultiByte = UTF16ToUTF8(nameWide);
        // "nameMultiByte" may never be empty, but let's be safe.
        if (nameMultiByte.empty()) {
            return nullptr;
        }
        const FARPROC address = GetProcAddress(m_module, nameMultiByte.c_str());
        if (!address) {
            const std::wstring dbgMsg = std::wstring(LR"(Failed to resolve symbol ")") + nameWide + std::wstring(L"()\" from \"") + m_fileName + std::wstring(LR"(".)") + std::wstring(__NEW_LINE);
            OutputDebugStringW(dbgMsg.c_str());
            return nullptr;
        }
        return address;
    };
    if (m_resolvedSymbols.empty()) {
        address = resolve(function);
    } else {
        const auto search = m_resolvedSymbols.find(function);
        if (search == m_resolvedSymbols.cend()) {
            address = resolve(function);
        } else {
            shouldInsert = false;
            address = search->second;
        }
    }
    if (shouldInsert) {
        // We intend to append the symbol address to the cache list unconditionally even if
        // we failed to resolve it to avoid unneeded resolving operations afterwards.
        m_resolvedSymbols.insert({function, address});
    }
    return address;
}

FARPROC SystemLibraryPrivate::GetSymbolNoCache(const std::wstring &fileName, const std::wstring &function) noexcept
{
    if (fileName.empty() || function.empty()) {
        return nullptr;
    }
    const HMODULE module = GetModuleHandleW(fileName.c_str());
    if (!module) {
        const std::wstring dbgMsg = std::wstring(LR"(Failed to retrieve the module handle of ")") + function + std::wstring(LR"(".)") + std::wstring(__NEW_LINE);
        OutputDebugStringW(dbgMsg.c_str());
        return nullptr;
    }
    // "functionMultiByte" may never be empty, but let's be safe.
    const std::string functionMultiByte = UTF16ToUTF8(function);
    if (functionMultiByte.empty()) {
        return nullptr;
    }
    const FARPROC address = GetProcAddress(module, functionMultiByte.c_str());
    if (!address) {
        const std::wstring dbgMsg = std::wstring(LR"(Failed to resolve symbol ")") + function + std::wstring(L"()\" from \"") + fileName + std::wstring(LR"(".)") + std::wstring(__NEW_LINE);
        OutputDebugStringW(dbgMsg.c_str());
        return nullptr;
    }
    return address;
}

SystemLibrary::SystemLibrary() noexcept : d_ptr(std::make_unique<SystemLibraryPrivate>(this))
{
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

bool SystemLibrary::Loaded() const noexcept
{
    return d_ptr->Loaded();
}

bool SystemLibrary::Load(const bool load) noexcept
{
    return d_ptr->Load(load);
}

FARPROC SystemLibrary::GetSymbol(const std::wstring &function) noexcept
{
    return d_ptr->GetSymbol(function);
}

FARPROC SystemLibrary::GetSymbolNoCache(const std::wstring &fileName, const std::wstring &function) noexcept
{
    return SystemLibraryPrivate::GetSymbolNoCache(fileName, function);
}

[[nodiscard]] bool operator==(const SystemLibrary &lhs, const SystemLibrary &rhs) noexcept
{
    return (lhs.FileName() == rhs.FileName());
}

[[nodiscard]] bool operator!=(const SystemLibrary &lhs, const SystemLibrary &rhs) noexcept
{
    return (!(lhs == rhs));
}
