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
#include "WindowsVersion.h"
#include "SystemLibraryManager.h"
#include <ComBaseApi.h>
#include <DwmApi.h>

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT (reinterpret_cast<HINSTANCE>(&__ImageBase))
#endif

static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
static constexpr DWORD g_DWMWA_USE_IMMERSIVE_DARK_MODE = 20;

static constexpr wchar_t g_personalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

HINSTANCE Utils::GetCurrentInstance() noexcept
{
    static const auto result = HINST_THISCOMPONENT;
    return result;
}

LPCWSTR Utils::GetWindowClassName(const ATOM atom) noexcept
{
    if (atom == INVALID_ATOM) {
        OutputDebugStringW(L"Failed to convert the given ATOM to window class name due to it's invalid.");
        return nullptr;
    } else {
        return reinterpret_cast<LPCWSTR>(static_cast<WORD>(MAKELONG(atom, 0)));
    }
}

LPCWSTR Utils::GetSystemErrorMessage(LPCWSTR function, const DWORD code) noexcept
{
    if (!function) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return nullptr;
    }
    if (code == ERROR_SUCCESS) {
        OutputDebugStringW(L"Operation succeeded.");
        return nullptr;
    }
    LPWSTR buf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 0, nullptr) == 0) {
        OutputDebugStringW(L"Failed to retrieve the system error message.");
        return nullptr;
    }
    const auto str = new wchar_t[MAX_PATH];
    SecureZeroMemory(str, sizeof(str));
    swprintf(str, L"%s failed with error %d: %s.", function, code, buf);
    LocalFree(buf);
    return str;
}

LPCWSTR Utils::GetSystemErrorMessage(LPCWSTR function, const HRESULT hr) noexcept
{
    if (!function) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return nullptr;
    }
    if (SUCCEEDED(hr)) {
        OutputDebugStringW(L"Operation succeeded.");
        return nullptr;
    }
    return GetSystemErrorMessage(function, HRESULT_CODE(hr));
}

LPCWSTR Utils::GetSystemErrorMessage(LPCWSTR function) noexcept
{
    if (!function) {
        OutputDebugStringW(L"Failed to retrieve the system error message due to the function name is empty.");
        return nullptr;
    } else {
        return GetSystemErrorMessage(function, GetLastError());
    }
}

void Utils::DisplayErrorDialog(LPCWSTR text) noexcept
{
    USER32_API(MessageBoxW);
    if (MessageBoxWFunc) {
        if (text) {
            OutputDebugStringW(text);
            MessageBoxWFunc(nullptr, text, L"Error", MB_ICONERROR | MB_OK);
        } else {
            OutputDebugStringW(L"Failed to show the message box due to the content is empty.");
        }
    } else {
        OutputDebugStringW(L"MessageBoxW() is not available.");
    }
}

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

bool Utils::ShouldAppsUseDarkMode() noexcept
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    // Starting from Windows 10 19H1, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    if (IsWindows1019H1OrGreater()) {
        ADVAPI32_API(RegOpenKeyExW);
        ADVAPI32_API(RegQueryValueExW);
        ADVAPI32_API(RegCloseKey);
        if (RegOpenKeyExWFunc && RegQueryValueExWFunc && RegCloseKeyFunc) {
            HKEY hKey = nullptr;
            if (RegOpenKeyExWFunc(HKEY_CURRENT_USER, g_personalizeRegistryKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                PRINT_WIN32_ERROR_MESSAGE(RegOpenKeyExW, L"Failed to open the registry key to read.")
                return false;
            }
            DWORD dwValue = 0;
            DWORD dwType = REG_DWORD;
            DWORD dwSize = sizeof(dwValue);
            const bool success = (RegQueryValueExWFunc(hKey, L"AppsUseLightTheme", nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwValue),&dwSize) == ERROR_SUCCESS);
            if (!success) {
                PRINT_WIN32_ERROR_MESSAGE(RegQueryValueExW, L"Failed to query the registry key value.")
            }
            if (RegCloseKeyFunc(hKey) != ERROR_SUCCESS) {
                PRINT_WIN32_ERROR_MESSAGE(RegCloseKey, L"Failed to close the registry key.")
            }
            return (success && (dwValue == 0));
        } else {
            OutputDebugStringW(L"RegOpenKeyExW(), RegQueryValueExW() and RegCloseKey() are not available.");
            return false;
        }
    } else {
        static const auto ShouldAppsUseDarkModeFunc = reinterpret_cast<BOOL(WINAPI *)()>(SystemLibraryManager::instance().GetSymbol(L"UxTheme.dll", MAKEINTRESOURCEW(132)));
        if (ShouldAppsUseDarkModeFunc) {
            return (ShouldAppsUseDarkModeFunc() != FALSE);
        } else {
            OutputDebugStringW(L"ShouldAppsUseDarkMode() is not available.");
            return false;
        }
    }
}

bool Utils::ShouldSystemUsesDarkMode() noexcept
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    static const auto ShouldSystemUsesDarkModeFunc = reinterpret_cast<BOOL(WINAPI *)()>(SystemLibraryManager::instance().GetSymbol(L"UxTheme.dll", MAKEINTRESOURCEW(138)));
    if (ShouldSystemUsesDarkModeFunc) {
        return (ShouldSystemUsesDarkModeFunc() != FALSE);
    } else {
        OutputDebugStringW(L"ShouldSystemUsesDarkMode() is not available.");
        return false;
    }
}

bool Utils::GenerateGUID(LPCWSTR *str) noexcept
{
    OLE32_API(CoCreateGuid);
    OLE32_API(StringFromGUID2);
    if (CoCreateGuidFunc && StringFromGUID2Func) {
        if (!str) {
            OutputDebugStringW(L"Failed to generate GUID due to the given string address is null.");
            return false;
        }
        GUID guid = {};
        const HRESULT hr = CoCreateGuidFunc(&guid);
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(CoCreateGuid, hr, L"Failed to generate a new GUID.")
            return false;
        }
        auto buf = new wchar_t[MAX_PATH];
        SecureZeroMemory(buf, sizeof(buf));
        if (StringFromGUID2Func(guid, buf, MAX_PATH) == 0) {
            delete [] buf;
            buf = nullptr;
            PRINT_WIN32_ERROR_MESSAGE(StringFromGUID2, L"Failed to convert GUID to string.")
            return false;
        }
        *str = buf;
        return true;
    } else {
        OutputDebugStringW(L"CoCreateGuid() and StringFromGUID2() are not available.");
        return false;
    }
}

bool Utils::RefreshWindowTheme(const HWND hWnd) noexcept
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    DWMAPI_API(DwmSetWindowAttribute);
    UXTHEME_API(SetWindowTheme);
    if (DwmSetWindowAttributeFunc && SetWindowThemeFunc) {
        if (!hWnd) {
            OutputDebugStringW(L"Failed to refresh the window theme due to the given window handle is null.");
            return false;
        }
        BOOL useDarkFrame = FALSE;
        LPCWSTR themeName = nullptr;
        if (IsHighContrastModeEnabled()) {
            // ### TO BE IMPLEMENTED
        } else if (ShouldAppsUseDarkMode()) {
            useDarkFrame = TRUE;
            themeName = L"Dark_Explorer";
        } else {
            useDarkFrame = FALSE;
            themeName = L"Explorer";
        }
        const HRESULT hr1 = DwmSetWindowAttributeFunc(hWnd, g_DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &useDarkFrame, sizeof(useDarkFrame));
        const HRESULT hr2 = DwmSetWindowAttributeFunc(hWnd, g_DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkFrame, sizeof(useDarkFrame));
        const HRESULT hr3 = SetWindowThemeFunc(hWnd, themeName, nullptr);
        if (FAILED(hr1) && FAILED(hr2)) {
            PRINT_HR_ERROR_MESSAGE(DwmSetWindowAttribute, hr2, L"Failed to set the window dark mode state.")
            return false;
        }
        if (FAILED(hr3)) {
            PRINT_HR_ERROR_MESSAGE(SetWindowTheme, hr3, L"Failed to set the window theme.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DwmSetWindowAttribute() and SetWindowTheme() are not available.");
        return false;
    }
}

bool Utils::CloseWindow(const HWND hWnd, const ATOM atom) noexcept
{
    USER32_API(DestroyWindow);
    USER32_API(UnregisterClassW);
    if (DestroyWindowFunc && UnregisterClassWFunc) {
        if (!hWnd) {
            OutputDebugStringW(L"Failed to close the window due to the given window handle is null.");
            return false;
        }
        if (atom == INVALID_ATOM) {
            OutputDebugStringW(L"Failed to close the window due to the given window ATOM is invalid.");
            return false;
        }
        if (DestroyWindowFunc(hWnd) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow, L"Failed to destroy the window.")
            return false;
        }
        if (UnregisterClassWFunc(GetWindowClassName(atom), GetCurrentInstance()) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW, L"Failed to unregister the window class.")
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DestroyWindow() and UnregisterClassW() are not available.");
        return false;
    }
}
