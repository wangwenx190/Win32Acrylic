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
#include "OperationResult.h"
#include <ShellScalingApi.h>
#include <ComBaseApi.h>

void Utils::DisplayErrorDialog(const std::wstring &text) noexcept
{
    USER32_API(MessageBoxW);
    if (MessageBoxW_API) {
        if (!text.empty()) {
            OutputDebugStringW(text.c_str());
            MessageBoxW_API(nullptr, text.c_str(), L"Error", MB_ICONERROR | MB_OK);
        }
    } else {
        OutputDebugStringW(L"Can't display the error dialog due to MessageBoxW() is not available.");
    }
}

std::wstring Utils::GenerateGUID() noexcept
{
    OLE32_API(CoCreateGuid);
    OLE32_API(StringFromGUID2);
    if (CoCreateGuid_API && StringFromGUID2_API) {
        GUID guid = {};
        const HRESULT hr = CoCreateGuid_API(&guid);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(CoCreateGuid, hr, L"Failed to generate a new GUID.")
            return {};
        }
        wchar_t buf[MAX_PATH] = { L'\0' };
        if (StringFromGUID2_API(guid, buf, MAX_PATH) == 0) {
            PRINT_WIN32_ERROR_MESSAGE(StringFromGUID2, L"Failed to convert GUID to string.")
            return {};
        }
        return buf;
    } else {
        DisplayErrorDialog(L"Can't generate a new GUID due to CoCreateGuid() and StringFromGUID2() are not available.");
        return {};
    }
}

ProcessDPIAwareness Utils::GetProcessDPIAwareness() noexcept
{
    USER32_API(GetThreadDpiAwarenessContext);
    USER32_API(GetAwarenessFromDpiAwarenessContext);
    if (GetThreadDpiAwarenessContext_API && GetAwarenessFromDpiAwarenessContext_API) {
        const DPI_AWARENESS_CONTEXT context = GetThreadDpiAwarenessContext_API();
        if (context) {
            const auto awareness = static_cast<int>(GetAwarenessFromDpiAwarenessContext_API(context));
            switch (awareness) {
            case DPI_AWARENESS_PER_MONITOR_AWARE_V2: {
                return ProcessDPIAwareness::PerMonitorV2;
            } break;
            case DPI_AWARENESS_PER_MONITOR_AWARE: {
                return ProcessDPIAwareness::PerMonitor;
            } break;
            case DPI_AWARENESS_SYSTEM_AWARE: {
                return ProcessDPIAwareness::System;
            } break;
            case DPI_AWARENESS_UNAWARE_GDISCALED: {
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
        if (GetProcessDpiAwareness_API) {
            int awareness = 0;
            const HRESULT hr = GetProcessDpiAwareness_API(nullptr, reinterpret_cast<PROCESS_DPI_AWARENESS *>(&awareness));
            if (SUCCEEDED(hr)) {
                switch (awareness) {
                case PROCESS_PER_MONITOR_DPI_AWARE_V2: {
                    return ProcessDPIAwareness::PerMonitorV2;
                } break;
                case PROCESS_PER_MONITOR_DPI_AWARE: {
                    return ProcessDPIAwareness::PerMonitor;
                } break;
                case PROCESS_SYSTEM_DPI_AWARE: {
                    return ProcessDPIAwareness::System;
                } break;
                case PROCESS_DPI_UNAWARE_GDISCALED: {
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
            if (IsProcessDPIAware_API) {
                if (IsProcessDPIAware_API() == FALSE) {
                    return ProcessDPIAwareness::Unaware;
                } else {
                    return ProcessDPIAwareness::System;
                }
            } else {
                DisplayErrorDialog(L"Can't retrieve the DPI awareness of the current process due to IsProcessDPIAware() is not available.");
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
        pda = static_cast<PROCESS_DPI_AWARENESS>(PROCESS_PER_MONITOR_DPI_AWARE_V2);
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
        pda = static_cast<PROCESS_DPI_AWARENESS>(PROCESS_DPI_UNAWARE_GDISCALED);
    } break;
    case ProcessDPIAwareness::Unaware: {
        dac = DPI_AWARENESS_CONTEXT_UNAWARE;
        pda = PROCESS_DPI_UNAWARE;
    } break;
    }
    USER32_API(SetProcessDpiAwarenessContext);
    if (SetProcessDpiAwarenessContext_API) {
        if (SetProcessDpiAwarenessContext_API(dac) == FALSE) {
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
        if (SetProcessDpiAwareness_API) {
            const HRESULT hr = SetProcessDpiAwareness_API(pda);
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
            if (SetProcessDPIAware_API) {
                if (SetProcessDPIAware_API() == FALSE) {
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
                DisplayErrorDialog(L"Can't set the DPI awareness of the current process due to SetProcessDPIAware() is not available.");
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
