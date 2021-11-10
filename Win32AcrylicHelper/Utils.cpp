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

#include <SDKDDKVer.h>
#include <Windows.h>
#include "Utils.h"
#include "OperationResult.h"

static constexpr const wchar_t __NEW_LINE[] = L"\r\n";

void Utils::DisplayErrorDialog(const std::wstring &text) noexcept
{
    if (!text.empty()) {
        const std::wstring textWithNewLine = text + std::wstring(__NEW_LINE);
        OutputDebugStringW(textWithNewLine.c_str());
        MessageBoxW(nullptr, text.c_str(), L"Error", MB_ICONERROR | MB_OK);
    }
}

ProcessDPIAwareness Utils::GetProcessDPIAwareness() noexcept
{
    const VersionNumber &curOsVer = WindowsVersion::CurrentVersion();
    // Win10 1607
    if (curOsVer >= WindowsVersion::Windows10_RedStone1) {
        const DPI_AWARENESS_CONTEXT context = GetThreadDpiAwarenessContext();
        if (context) {
            const auto awareness = static_cast<int>(GetAwarenessFromDpiAwarenessContext(context));
            switch (awareness) {
            case DPI_AWARENESS_PER_MONITOR_AWARE_V2: {
                return ProcessDPIAwareness::PerMonitorVersion2;
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
            default:
                break;
            }
        }
    }
    if (curOsVer >= WindowsVersion::Windows8_1) {
        PROCESS_DPI_AWARENESS pda = PROCESS_DPI_UNAWARE;
        const HRESULT hr = GetProcessDpiAwareness(nullptr, &pda);
        if (SUCCEEDED(hr)) {
            const auto awareness = static_cast<int>(pda);
            switch (awareness) {
            case PROCESS_PER_MONITOR_DPI_AWARE_V2: {
                return ProcessDPIAwareness::PerMonitorVersion2;
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
            default:
                break;
            }
        }
    }
    return (IsProcessDPIAware() ? ProcessDPIAwareness::System : ProcessDPIAwareness::Unaware);
}

bool Utils::SetProcessDPIAwareness(const ProcessDPIAwareness dpiAwareness) noexcept
{
    const VersionNumber &curOsVer = WindowsVersion::CurrentVersion();
    // Win10 1607
    if (curOsVer >= WindowsVersion::Windows10_RedStone1) {
        DPI_AWARENESS_CONTEXT dac = DPI_AWARENESS_CONTEXT_UNAWARE;
        switch (dpiAwareness) {
        case ProcessDPIAwareness::PerMonitorVersion2: {
            dac = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
        } break;
        case ProcessDPIAwareness::PerMonitor: {
            dac = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
        } break;
        case ProcessDPIAwareness::System: {
            dac = DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
        } break;
        case ProcessDPIAwareness::GdiScaled: {
            dac = DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED;
        } break;
        case ProcessDPIAwareness::Unaware: {
            dac = DPI_AWARENESS_CONTEXT_UNAWARE;
        } break;
        }
        if (SetProcessDpiAwarenessContext(dac) == FALSE) {
            const DWORD dwError = GetLastError();
            if (dwError == ERROR_ACCESS_DENIED) {
                // ERROR_ACCESS_DENIED means set externally (MSVC manifest or external application loading our library).
                // We assume it's the most appropriate DPI awareness.
                return true;
            }
        } else {
            return true;
        }
    }
    if (curOsVer >= WindowsVersion::Windows8_1) {
        PROCESS_DPI_AWARENESS pda = PROCESS_DPI_UNAWARE;
        switch (dpiAwareness) {
        case ProcessDPIAwareness::PerMonitorVersion2: {
            pda = PROCESS_PER_MONITOR_DPI_AWARE_V2;
        } break;
        case ProcessDPIAwareness::PerMonitor: {
            pda = PROCESS_PER_MONITOR_DPI_AWARE;
        } break;
        case ProcessDPIAwareness::System: {
            pda = PROCESS_SYSTEM_DPI_AWARE;
        } break;
        case ProcessDPIAwareness::GdiScaled: {
            pda = PROCESS_DPI_UNAWARE_GDISCALED;
        } break;
        case ProcessDPIAwareness::Unaware: {
            pda = PROCESS_DPI_UNAWARE;
        } break;
        }
        const HRESULT hr = SetProcessDpiAwareness(pda);
        if (FAILED(hr)) {
            if (hr == E_ACCESSDENIED) {
                // E_ACCESSDENIED means set externally (MSVC manifest or external application loading our library).
                // We assume it's the most appropriate DPI awareness.
                return true;
            }
        } else {
            return true;
        }
    }
    if (dpiAwareness == ProcessDPIAwareness::Unaware) {
        return true;
    } else {
        if (SetProcessDPIAware() != FALSE) {
            return true;
        }
    }
    return false;
}

std::wstring Utils::DPIAwarenessToString(const ProcessDPIAwareness value) noexcept
{
    switch (value) {
    case ProcessDPIAwareness::PerMonitorVersion2: {
        return L"Per Monitor Version 2";
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
