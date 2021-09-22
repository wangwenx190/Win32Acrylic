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

#include "SystemLibraryManager.h"
#include "SystemLibrary.h"
#include <unordered_map>

class SystemLibraryManagerPrivate
{
public:
    explicit SystemLibraryManagerPrivate(SystemLibraryManager *q) noexcept;
    ~SystemLibraryManagerPrivate() noexcept;

    [[nodiscard]] FARPROC GetSymbol(LPCWSTR fileName, LPCWSTR symbolName) noexcept;

private:
    SystemLibraryManagerPrivate(const SystemLibraryManagerPrivate &) = delete;
    SystemLibraryManagerPrivate &operator=(const SystemLibraryManagerPrivate &) = delete;
    SystemLibraryManagerPrivate(SystemLibraryManagerPrivate &&) = delete;
    SystemLibraryManagerPrivate &operator=(SystemLibraryManagerPrivate &&) = delete;

private:
    SystemLibraryManager *q_ptr = nullptr;
    std::unordered_map<LPCWSTR, SystemLibrary *> m_loadedLibraries = {};
};

SystemLibraryManagerPrivate::SystemLibraryManagerPrivate(SystemLibraryManager *q) noexcept
{
    q_ptr = q;
}

SystemLibraryManagerPrivate::~SystemLibraryManagerPrivate() noexcept
{
    if (!m_loadedLibraries.empty()) {
        for (auto &&library : std::as_const(m_loadedLibraries)) {
            auto pLibrary = library.second;
            if (pLibrary) {
                if (pLibrary->IsLoaded()) {
                    pLibrary->Unload();
                }
                delete pLibrary;
                pLibrary = nullptr;
            }
        }
        m_loadedLibraries.clear();
    }
}

FARPROC SystemLibraryManagerPrivate::GetSymbol(LPCWSTR fileName, LPCWSTR symbolName) noexcept
{
    if (!fileName || (wcscmp(fileName, L"") == 0)) {
        OutputDebugStringW(L"Failed to resolve symbol from library due to the given file name is empty.");
        return nullptr;
    }
    if (!symbolName || (wcscmp(symbolName, L"") == 0)) {
        OutputDebugStringW(L"Failed to resolve symbol from library due to the given symbol name is empty.");
        return nullptr;
    }
    const auto search = m_loadedLibraries.find(fileName);
    if (search == m_loadedLibraries.cend()) {
        const auto library = new SystemLibrary(fileName);
        m_loadedLibraries.insert({fileName, library});
        return library->GetSymbol(symbolName);
    } else {
        return search->second->GetSymbol(symbolName);
    }
}

SystemLibraryManager::SystemLibraryManager() noexcept
{
    d_ptr = std::make_unique<SystemLibraryManagerPrivate>(this);
}

SystemLibraryManager::~SystemLibraryManager() noexcept = default;

SystemLibraryManager &SystemLibraryManager::instance() noexcept
{
    static SystemLibraryManager manager;
    return manager;
}

FARPROC SystemLibraryManager::GetSymbol(LPCWSTR fileName, LPCWSTR symbolName) noexcept
{
    return d_ptr->GetSymbol(fileName, symbolName);
}
