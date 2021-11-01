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

    [[nodiscard]] FARPROC GetSymbol(const std::wstring &fileName, const std::wstring &symbolName) noexcept;

    void Release() noexcept;

private:
    explicit SystemLibraryManagerPrivate(const SystemLibraryManagerPrivate &) noexcept = delete;
    explicit SystemLibraryManagerPrivate(SystemLibraryManagerPrivate &&) noexcept = delete;

    SystemLibraryManagerPrivate &operator=(const SystemLibraryManagerPrivate &) const noexcept = delete;
    SystemLibraryManagerPrivate &operator=(SystemLibraryManagerPrivate &&) const noexcept = delete;

private:
    SystemLibraryManager *q_ptr = nullptr;
    std::unordered_map<std::wstring, std::shared_ptr<SystemLibrary>> m_loadedLibraries = {};
};

SystemLibraryManagerPrivate::SystemLibraryManagerPrivate(SystemLibraryManager *q) noexcept
{
    q_ptr = q;
}

SystemLibraryManagerPrivate::~SystemLibraryManagerPrivate() noexcept
{
    Release();
}

FARPROC SystemLibraryManagerPrivate::GetSymbol(const std::wstring &fileName, const std::wstring &symbolName) noexcept
{
    if (fileName.empty() || symbolName.empty()) {
        return nullptr;
    }
    bool shouldInsert = true;
    std::shared_ptr<SystemLibrary> library = nullptr;
    if (m_loadedLibraries.empty()) {
        library = std::make_shared<SystemLibrary>(fileName);
    } else {
        const auto search = m_loadedLibraries.find(fileName);
        if (search == m_loadedLibraries.cend()) {
            library = std::make_shared<SystemLibrary>(fileName);
        } else {
            shouldInsert = false;
            library = search->second;
        }
    }
    if (shouldInsert) {
        m_loadedLibraries.insert({fileName, library});
    }
    // "library" may never be null, but let's be safe.
    return (library ? library->GetSymbol(symbolName) : nullptr);
}

void SystemLibraryManagerPrivate::Release() noexcept
{
    if (m_loadedLibraries.empty()) {
        return;
    }
    for (auto &&library : std::as_const(m_loadedLibraries)) {
        auto pLibrary = library.second;
        // It may never be null, but let's be safe.
        if (pLibrary) {
            const bool result = pLibrary->Load(false);
            // The result is not important here.
            UNREFERENCED_PARAMETER(result);
            pLibrary.reset();
        }
    }
    m_loadedLibraries = {};
}

SystemLibraryManager::SystemLibraryManager() noexcept : d_ptr(std::make_unique<SystemLibraryManagerPrivate>(this))
{
}

SystemLibraryManager::~SystemLibraryManager() noexcept = default;

SystemLibraryManager &SystemLibraryManager::instance() noexcept
{
    static SystemLibraryManager manager;
    return manager;
}

FARPROC SystemLibraryManager::GetSymbol(const std::wstring &fileName, const std::wstring &symbolName) noexcept
{
    return d_ptr->GetSymbol(fileName, symbolName);
}

void SystemLibraryManager::Release() noexcept
{
    d_ptr->Release();
}
