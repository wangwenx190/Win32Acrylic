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

#include "Utils.h"
#include "SystemLibraryManager.h"
#include <ShellScalingApi.h>
#include <ComBaseApi.h>

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT (reinterpret_cast<HINSTANCE>(&__ImageBase))
#endif

static constexpr int g_DPI_AWARENESS_PER_MONITOR_AWARE_V2 = 3;
static constexpr int g_PROCESS_PER_MONITOR_DPI_AWARE_V2 = 3;
static constexpr int g_DPI_AWARENESS_UNAWARE_GDISCALED = 4;
static constexpr int g_PROCESS_DPI_UNAWARE_GDISCALED = 4;

bool Utils::IsHighContrastModeEnabled() noexcept
{
    USER32_API(SystemParametersInfoW);
    if (SystemParametersInfoWFunc) {
        HIGHCONTRASTW hc;
        SecureZeroMemory(&hc, sizeof(hc));
        hc.cbSize = sizeof(hc);
        if (SystemParametersInfoWFunc(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SystemParametersInfoW, L"Failed to retrieve the high contrast mode state.")
            return false;
        }
        return (hc.dwFlags & HCF_HIGHCONTRASTON);
    } else {
        OutputDebugStringW(L"SystemParametersInfoW() is not available.");
        return false;
    }
}

DWORD Utils::GetDWORDFromRegistry(const HKEY rootKey, const std::wstring &subKey, const std::wstring &keyName) noexcept
{
    ADVAPI32_API(RegOpenKeyExW);
    ADVAPI32_API(RegQueryValueExW);
    ADVAPI32_API(RegCloseKey);
    if (RegOpenKeyExWFunc && RegQueryValueExWFunc && RegCloseKeyFunc) {
        if (!rootKey || subKey.empty() || keyName.empty()) {
            OutputDebugStringW(L"Can't query the registry due to invalid parameters are passed.");
            return 0;
        }
        HKEY hKey = nullptr;
        if (RegOpenKeyExWFunc(rootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            PRINT_WIN32_ERROR_MESSAGE(RegOpenKeyExW, L"Failed to open the registry key to read.")
            return 0;
        }
        DWORD dwValue = 0;
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(dwValue);
        const bool success = (RegQueryValueExWFunc(hKey, keyName.c_str(), nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwValue), &dwSize) == ERROR_SUCCESS);
        if (!success) {
            PRINT_WIN32_ERROR_MESSAGE(RegQueryValueExW, L"Failed to query the registry key value.")
            // Don't return early here because we have to close the opened registry key later.
        }
        if (RegCloseKeyFunc(hKey) != ERROR_SUCCESS) {
            PRINT_WIN32_ERROR_MESSAGE(RegCloseKey, L"Failed to close the registry key.")
            return 0;
        }
        return dwValue;
    } else {
        OutputDebugStringW(L"RegOpenKeyExW(), RegQueryValueExW() and RegCloseKey() are not available.");
        return 0;
    }
}

HINSTANCE Utils::GetCurrentModuleInstance() noexcept
{
    static const auto result = HINST_THISCOMPONENT;
    return result;
}

HINSTANCE Utils::GetWindowInstance(const HWND hWnd) noexcept
{
    USER32_API(GetWindowLongPtrW);
    if (GetWindowLongPtrWFunc) {
        if (!hWnd) {
            return nullptr;
        }
        const LONG_PTR result = GetWindowLongPtrWFunc(hWnd, GWLP_HINSTANCE);
        if (result == 0) {
            PRINT_WIN32_ERROR_MESSAGE(GetWindowLongPtrW, L"Failed to retrieve the window's HINSTANCE.")
            return nullptr;
        } else {
            return reinterpret_cast<HINSTANCE>(result);
        }
    } else {
        OutputDebugStringW(L"GetWindowLongPtrW() is not available.");
        return nullptr;
    }
}

std::wstring Utils::GetSystemErrorMessage(const std::wstring &function, const DWORD code) noexcept
{
    if (function.empty()) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return {};
    }
    if (code == ERROR_SUCCESS) {
        OutputDebugStringW(L"Operation succeeded.");
        return {};
    }
    LPWSTR buf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 0, nullptr) == 0) {
        OutputDebugStringW(L"Failed to retrieve the system error message.");
        return {};
    }
    const std::wstring result = L"\"" + function + L"\" failed with error " + IntegerToString(code, 10) + L": " + buf + L".";
    LocalFree(buf);
    return result;
}

std::wstring Utils::GetSystemErrorMessage(const std::wstring &function, const HRESULT hr) noexcept
{
    if (function.empty()) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return {};
    }
    if (SUCCEEDED(hr)) {
        OutputDebugStringW(L"Operation succeeded.");
        return {};
    }
    const DWORD dwError = HRESULT_CODE(hr);
    return GetSystemErrorMessage(function, dwError);
}

std::wstring Utils::GetSystemErrorMessage(const std::wstring &function) noexcept
{
    if (function.empty()) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return {};
    } else {
        const DWORD dwError = GetLastError();
        return GetSystemErrorMessage(function, dwError);
    }
}

void Utils::DisplayErrorDialog(const std::wstring &text) noexcept
{
    USER32_API(MessageBoxW);
    if (MessageBoxWFunc) {
        if (text.empty()) {
            OutputDebugStringW(L"Failed to show the message box due to the content is empty.");
        } else {
            OutputDebugStringW(text.c_str());
            MessageBoxWFunc(nullptr, text.c_str(), L"Error", MB_ICONERROR | MB_OK);
        }
    } else {
        OutputDebugStringW(L"MessageBoxW() is not available.");
    }
}

std::wstring Utils::GenerateGUID() noexcept
{
    OLE32_API(CoCreateGuid);
    OLE32_API(StringFromGUID2);
    if (CoCreateGuidFunc && StringFromGUID2Func) {
        GUID guid = {};
        const HRESULT hr = CoCreateGuidFunc(&guid);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(CoCreateGuid, hr, L"Failed to generate a new GUID.")
            return {};
        }
        wchar_t buf[MAX_PATH] = { L'\0' };
        if (StringFromGUID2Func(guid, buf, MAX_PATH) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(StringFromGUID2, L"Failed to convert GUID to string.")
            return {};
        }
        return buf;
    } else {
        OutputDebugStringW(L"CoCreateGuid() and StringFromGUID2() are not available.");
        return {};
    }
}

ProcessDPIAwareness Utils::GetProcessDPIAwareness() noexcept
{
    USER32_API(GetThreadDpiAwarenessContext);
    USER32_API(GetAwarenessFromDpiAwarenessContext);
    if (GetThreadDpiAwarenessContextFunc && GetAwarenessFromDpiAwarenessContextFunc) {
        const DPI_AWARENESS_CONTEXT context = GetThreadDpiAwarenessContextFunc();
        if (context) {
            const auto awareness = static_cast<int>(GetAwarenessFromDpiAwarenessContextFunc(context));
            switch (awareness) {
            case g_DPI_AWARENESS_PER_MONITOR_AWARE_V2: {
                return ProcessDPIAwareness::PerMonitorV2;
            } break;
            case DPI_AWARENESS_PER_MONITOR_AWARE: {
                return ProcessDPIAwareness::PerMonitor;
            } break;
            case DPI_AWARENESS_SYSTEM_AWARE: {
                return ProcessDPIAwareness::System;
            } break;
            case g_DPI_AWARENESS_UNAWARE_GDISCALED: {
                return ProcessDPIAwareness::GdiScaled;
            } break;
            case DPI_AWARENESS_UNAWARE: {
                return ProcessDPIAwareness::Unaware;
            } break;
            case DPI_AWARENESS_INVALID: {
                PRINT_WIN32_ERROR_MESSAGE(GetAwarenessFromDpiAwarenessContext, L"Failed to extract the DPI awareness from the context.")
                return ProcessDPIAwareness::Unaware;
            } break;
            }
            return ProcessDPIAwareness::Unaware;
        } else {
            PRINT_WIN32_ERROR_MESSAGE(GetThreadDpiAwarenessContext, L"Failed to retrieve the DPI awareness context of the current thread.")
            return ProcessDPIAwareness::Unaware;
        }
    } else {
        OutputDebugStringW(L"GetThreadDpiAwarenessContext() and GetAwarenessFromDpiAwarenessContext() are not available.");
        SHCORE_API(GetProcessDpiAwareness);
        if (GetProcessDpiAwarenessFunc) {
            int awareness = 0;
            const HRESULT hr = GetProcessDpiAwarenessFunc(nullptr, reinterpret_cast<PROCESS_DPI_AWARENESS *>(&awareness));
            if (SUCCEEDED(hr)) {
                switch (awareness) {
                case g_PROCESS_PER_MONITOR_DPI_AWARE_V2: {
                    return ProcessDPIAwareness::PerMonitorV2;
                } break;
                case PROCESS_PER_MONITOR_DPI_AWARE: {
                    return ProcessDPIAwareness::PerMonitor;
                } break;
                case PROCESS_SYSTEM_DPI_AWARE: {
                    return ProcessDPIAwareness::System;
                } break;
                case g_PROCESS_DPI_UNAWARE_GDISCALED: {
                    return ProcessDPIAwareness::GdiScaled;
                } break;
                case PROCESS_DPI_UNAWARE: {
                    return ProcessDPIAwareness::Unaware;
                } break;
                }
                return ProcessDPIAwareness::Unaware;
            } else {
                PRINT_HR_ERROR_MESSAGE(GetProcessDpiAwareness, hr, L"Failed to retrieve the DPI awareness of the current process.")
                return ProcessDPIAwareness::Unaware;
            }
        } else {
            OutputDebugStringW(L"GetProcessDpiAwareness() is not available.");
            USER32_API(IsProcessDPIAware);
            if (IsProcessDPIAwareFunc) {
                if (IsProcessDPIAwareFunc() == FALSE) {
                    return ProcessDPIAwareness::Unaware;
                } else {
                    return ProcessDPIAwareness::System;
                }
            } else {
                OutputDebugStringW(L"IsProcessDPIAware() is not available.");
                return ProcessDPIAwareness::Unaware;
            }
        }
    }
}

bool Utils::SetProcessDPIAwareness(const ProcessDPIAwareness dpiAwareness) noexcept
{
    DPI_AWARENESS_CONTEXT dac = DPI_AWARENESS_CONTEXT_UNAWARE;
    PROCESS_DPI_AWARENESS pda = PROCESS_DPI_UNAWARE;
    switch (dpiAwareness) {
    case ProcessDPIAwareness::PerMonitorV2: {
        dac = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
        pda = static_cast<PROCESS_DPI_AWARENESS>(g_PROCESS_PER_MONITOR_DPI_AWARE_V2);
    } break;
    case ProcessDPIAwareness::PerMonitor: {
        dac = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
        pda = PROCESS_PER_MONITOR_DPI_AWARE;
    } break;
    case ProcessDPIAwareness::System: {
        dac = DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
        pda = PROCESS_SYSTEM_DPI_AWARE;
    } break;
    case ProcessDPIAwareness::GdiScaled: {
        dac = DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED;
        pda = static_cast<PROCESS_DPI_AWARENESS>(g_PROCESS_DPI_UNAWARE_GDISCALED);
    } break;
    case ProcessDPIAwareness::Unaware: {
        dac = DPI_AWARENESS_CONTEXT_UNAWARE;
        pda = PROCESS_DPI_UNAWARE;
    } break;
    }
    USER32_API(SetProcessDpiAwarenessContext);
    if (SetProcessDpiAwarenessContextFunc) {
        if (SetProcessDpiAwarenessContextFunc(dac) == FALSE) {
            const DWORD dwError = GetLastError();
            if (dwError == ERROR_ACCESS_DENIED) {
                // ERROR_ACCESS_DENIED means set externally (MSVC manifest or external application loading our library).
                // We assume it's the most appropriate DPI awareness.
                return true;
            } else {
                PRINT_WIN32_ERROR_MESSAGE(SetProcessDpiAwarenessContext, L"Failed to set DPI awareness for the process: SetProcessDpiAwarenessContext() returned FALSE.")
                return false;
            }
        } else {
            return true;
        }
    } else {
        OutputDebugStringW(L"SetProcessDpiAwarenessContext() is not available.");
        SHCORE_API(SetProcessDpiAwareness);
        if (SetProcessDpiAwarenessFunc) {
            const HRESULT hr = SetProcessDpiAwarenessFunc(pda);
            if (FAILED(hr)) {
                if (hr == E_ACCESSDENIED) {
                    // E_ACCESSDENIED means set externally (MSVC manifest or external application loading our library).
                    // We assume it's the most appropriate DPI awareness.
                    return true;
                } else {
                    PRINT_HR_ERROR_MESSAGE(SetProcessDpiAwareness, hr, L"Failed to set DPI awareness for the process: SetProcessDpiAwareness() reported failed.")
                    return false;
                }
            } else {
                return true;
            }
        } else {
            OutputDebugStringW(L"SetProcessDpiAwareness() is not available.");
            USER32_API(SetProcessDPIAware);
            if (SetProcessDPIAwareFunc) {
                if (SetProcessDPIAwareFunc() == FALSE) {
                    const DWORD dwError = GetLastError();
                    if (dwError == ERROR_ACCESS_DENIED) {
                        // ERROR_ACCESS_DENIED means set externally (MSVC manifest or external application loading our library).
                        // We assume it's the most appropriate DPI awareness.
                        return true;
                    } else {
                        PRINT_WIN32_ERROR_MESSAGE(SetProcessDPIAware, L"Failed to set DPI awareness for the process: SetProcessDPIAware() returned FALSE.")
                        return false;
                    }
                } else {
                    return true;
                }
            } else {
                OutputDebugStringW(L"SetProcessDPIAware() is not available.");
                return false;
            }
        }
    }
}

std::wstring Utils::IntegerToString(const int num, const int radix) noexcept
{
    wchar_t buf[MAX_PATH] = { L'\0' };
    _itow(num, buf, radix);
    return buf;
}

std::wstring Utils::DPIAwarenessToString(const ProcessDPIAwareness value) noexcept
{
    switch (value) {
    case ProcessDPIAwareness::PerMonitorV2: {
        return L"Per Monitor V2";
    } break;
    case ProcessDPIAwareness::PerMonitor: {
        return L"Per Monitor";
    } break;
    case ProcessDPIAwareness::System: {
        return L"System";
    } break;
    case ProcessDPIAwareness::GdiScaled: {
        return L"Unaware (GDI Scaled)";
    } break;
    case ProcessDPIAwareness::Unaware: {
        return L"Unaware";
    } break;
    }
    return L"Unknown";
}

std::wstring Utils::ThemeToString(const WindowTheme value) noexcept
{
    switch (value) {
    case WindowTheme::Light: {
        return L"Light";
    } break;
    case WindowTheme::Dark: {
        return L"Dark";
    } break;
    case WindowTheme::HighContrast: {
        return L"High Contrast";
    } break;
    }
    return L"Unknown";
}
