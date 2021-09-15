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
#include <UxTheme.h>
#include <DwmApi.h>
#include <Unknwn.h>
#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>
#include "Resource.h"

namespace Constants {
namespace Light {
static constexpr winrt::Windows::UI::Color TintColor = {255, 252, 252, 252};
static constexpr double TintOpacity = 0.0;
static constexpr double LuminosityOpacity = 0.85;
static constexpr winrt::Windows::UI::Color FallbackColor = {255, 249, 249, 249};
} // namespace Light
namespace Dark {
static constexpr winrt::Windows::UI::Color TintColor = {255, 44, 44, 44};
static constexpr double TintOpacity = 0.15;
static constexpr double LuminosityOpacity = 0.96;
static constexpr winrt::Windows::UI::Color FallbackColor = {255, 44, 44, 44};
} // namespace Dark
namespace HighContrast {
// ### TO BE IMPLEMENTED
} // namespace HighContrast
} // namespace Constants

static constexpr DWORD _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1 = 19;
static constexpr DWORD _DWMWA_USE_IMMERSIVE_DARK_MODE = 20;

static constexpr wchar_t g_personalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";

static constexpr wchar_t g_defaultWindowClassName[] = LR"(wangwenx190\Win32Acrylic\WindowClass\0000-0000-0000)";
static constexpr wchar_t g_defaultWindowTitle[] = L"Win32Acrylic WinUI2 Demo";

static ATOM g_mainWindowAtom = INVALID_ATOM;
static HWND g_mainWindowHandle = nullptr;
static HWND g_xamlIslandWindowHandle = nullptr;
static HINSTANCE g_instance = nullptr;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_manager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_source = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_backgroundBrush = nullptr;

[[nodiscard]] static inline bool IsWindowsOrGreater(const int major, const int minor, const int build)
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = major;
    osvi.dwMinorVersion = minor;
    osvi.dwBuildNumber = build;
    const BYTE op = VER_GREATER_EQUAL;
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

[[nodiscard]] static inline bool IsWindows10RS1OrGreater()
{
    // Windows 10 Version 1607 (Anniversary Update)
    // Code name: Red Stone 1
    return IsWindowsOrGreater(10, 0, 14393);
}

[[nodiscard]] static inline bool IsWindows1019H1OrGreater()
{
    // Windows 10 Version 1903 (May 2019 Update)
    // Code name: 19H1
    return IsWindowsOrGreater(10, 0, 18362);
}

static inline void DisplayErrorDialog(LPCWSTR text)
{
    static bool tried = false;
    using MessageBoxWSig = decltype(&::MessageBoxW);
    static MessageBoxWSig MessageBoxWFunc = nullptr;
    if (!MessageBoxWFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                MessageBoxWFunc = reinterpret_cast<MessageBoxWSig>(GetProcAddress(User32DLL, "MessageBoxW"));
                if (!MessageBoxWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol MessageBoxW().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (MessageBoxWFunc) {
        if (text && (wcslen(text) > 0)) {
            OutputDebugStringW(text);
            MessageBoxWFunc(nullptr, text, L"Error", MB_ICONERROR | MB_OK);
        } else {
            OutputDebugStringW(L"Failed to show the message box due to the content is empty.");
        }
    } else {
        OutputDebugStringW(L"MessageBoxW() is not available.");
    }
}

[[nodiscard]] static inline bool IsHighContrastModeEnabled()
{
    static bool tried = false;
    using SystemParametersInfoWSig = decltype(&::SystemParametersInfoW);
    static SystemParametersInfoWSig SystemParametersInfoWFunc = nullptr;
    if (!SystemParametersInfoWFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                SystemParametersInfoWFunc = reinterpret_cast<SystemParametersInfoWSig>(GetProcAddress(User32DLL, "SystemParametersInfoW"));
                if (!SystemParametersInfoWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol SystemParametersInfoW().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (SystemParametersInfoWFunc) {
        HIGHCONTRASTW hc;
        SecureZeroMemory(&hc, sizeof(hc));
        hc.cbSize = sizeof(hc);
        if (SystemParametersInfoWFunc(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
            OutputDebugStringW(L"Failed to retrieve the high contrast mode state.");
            return false;
        }
        return (hc.dwFlags & HCF_HIGHCONTRASTON);
    } else {
        OutputDebugStringW(L"SystemParametersInfoW() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool ShouldAppsUseDarkMode()
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    // Starting from Windows 10 19H1, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    if (IsWindows1019H1OrGreater()) {
        static bool tried = false;
        using RegOpenKeyExWSig = decltype(&::RegOpenKeyExW);
        static RegOpenKeyExWSig RegOpenKeyExWFunc = nullptr;
        using RegQueryValueExWSig = decltype(&::RegQueryValueExW);
        static RegQueryValueExWSig RegQueryValueExWFunc = nullptr;
        using RegCloseKeySig = decltype(&::RegCloseKey);
        static RegCloseKeySig RegCloseKeyFunc = nullptr;
        if (!RegOpenKeyExWFunc || !RegQueryValueExWFunc || !RegCloseKeyFunc) {
            if (!tried) {
                tried = true;
                const HMODULE AdvApi32DLL = LoadLibraryExW(L"AdvApi32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (AdvApi32DLL) {
                    RegOpenKeyExWFunc = reinterpret_cast<RegOpenKeyExWSig>(GetProcAddress(AdvApi32DLL, "RegOpenKeyExW"));
                    if (!RegOpenKeyExWFunc) {
                        OutputDebugStringW(L"Failed to resolve symbol RegOpenKeyExW().");
                    }
                    RegQueryValueExWFunc = reinterpret_cast<RegQueryValueExWSig>(GetProcAddress(AdvApi32DLL, "RegQueryValueExW"));
                    if (!RegQueryValueExWFunc) {
                        OutputDebugStringW(L"Failed to resolve symbol RegQueryValueExW().");
                    }
                    RegCloseKeyFunc = reinterpret_cast<RegCloseKeySig>(GetProcAddress(AdvApi32DLL, "RegCloseKey"));
                    if (!RegCloseKeyFunc) {
                        OutputDebugStringW(L"Failed to resolve symbol RegCloseKey().");
                    }
                } else {
                    OutputDebugStringW(L"Failed to load dynamic link library AdvApi32.dll.");
                }
            }
        }
        if (RegOpenKeyExWFunc && RegQueryValueExWFunc && RegCloseKeyFunc) {
            HKEY hKey = nullptr;
            if (RegOpenKeyExWFunc(HKEY_CURRENT_USER, g_personalizeRegistryKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                OutputDebugStringW(L"Failed to open the registry key to read.");
                return false;
            }
            DWORD dwValue = 0;
            DWORD dwType = REG_DWORD;
            DWORD dwSize = sizeof(dwValue);
            const bool success = (RegQueryValueExWFunc(hKey, L"AppsUseLightTheme", nullptr, &dwType, reinterpret_cast<LPBYTE>(&dwValue),&dwSize) == ERROR_SUCCESS);
            if (!success) {
                OutputDebugStringW(L"Failed to query the registry key value.");
            }
            if (RegCloseKeyFunc(hKey) != ERROR_SUCCESS) {
                OutputDebugStringW(L"Failed to close the registry key.");
            }
            return (success && (dwValue == 0));
        } else {
            OutputDebugStringW(L"RegOpenKeyExW(), RegQueryValueExW() and RegCloseKey() are not available.");
            return false;
        }
    } else {
        static bool tried = false;
        using ShouldAppsUseDarkModeSig = BOOL(WINAPI *)();
        static ShouldAppsUseDarkModeSig ShouldAppsUseDarkModeFunc = nullptr;
        if (!ShouldAppsUseDarkModeFunc) {
            if (!tried) {
                tried = true;
                const HMODULE UxThemeDLL = LoadLibraryExW(L"UxTheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (UxThemeDLL) {
                    ShouldAppsUseDarkModeFunc = reinterpret_cast<ShouldAppsUseDarkModeSig>(GetProcAddress(UxThemeDLL, MAKEINTRESOURCEA(132)));
                    if (!ShouldAppsUseDarkModeFunc) {
                        OutputDebugStringW(L"Failed to resolve symbol ShouldAppsUseDarkMode().");
                    }
                } else {
                    OutputDebugStringW(L"Failed to load dynamic link library UxTheme.dll.");
                }
            }
        }
        if (ShouldAppsUseDarkModeFunc) {
            return (ShouldAppsUseDarkModeFunc() != FALSE);
        } else {
            OutputDebugStringW(L"ShouldAppsUseDarkMode() is not available.");
            return false;
        }
    }
}

[[nodiscard]] static inline bool RefreshWindowTheme(const HWND hWnd)
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    static bool tried = false;
    using DwmSetWindowAttributeSig = decltype(&::DwmSetWindowAttribute);
    static DwmSetWindowAttributeSig DwmSetWindowAttributeFunc = nullptr;
    using SetWindowThemeSig = decltype(&::SetWindowTheme);
    static SetWindowThemeSig SetWindowThemeFunc = nullptr;
    if (!DwmSetWindowAttributeFunc || !SetWindowThemeFunc) {
        if (!tried) {
            tried = true;
            const HMODULE DwmApiDLL = LoadLibraryExW(L"DwmApi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (DwmApiDLL) {
                DwmSetWindowAttributeFunc = reinterpret_cast<DwmSetWindowAttributeSig>(GetProcAddress(DwmApiDLL, "DwmSetWindowAttribute"));
                if (!DwmSetWindowAttributeFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol DwmSetWindowAttribute().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library DwmApi.dll.");
            }
            const HMODULE UxThemeDLL = LoadLibraryExW(L"UxTheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (UxThemeDLL) {
                SetWindowThemeFunc = reinterpret_cast<SetWindowThemeSig>(GetProcAddress(UxThemeDLL, "SetWindowTheme"));
                if (!SetWindowThemeFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol SetWindowTheme().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic library UxTheme.dll.");
            }
        }
    }
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
        const HRESULT hr1 = DwmSetWindowAttributeFunc(hWnd, _DWMWA_USE_IMMERSIVE_DARK_MODE_BEFORE_20H1, &useDarkFrame, sizeof(useDarkFrame));
        const HRESULT hr2 = DwmSetWindowAttributeFunc(hWnd, _DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkFrame, sizeof(useDarkFrame));
        const HRESULT hr3 = SetWindowThemeFunc(hWnd, themeName, nullptr);
        if (FAILED(hr1) && FAILED(hr2)) {
            OutputDebugStringW(L"Failed to change the window dark mode state.");
            return false;
        }
        if (FAILED(hr3)) {
            OutputDebugStringW(L"Failed to change the window theme.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"DwmSetWindowAttribute() and SetWindowTheme() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool RefreshBackgroundBrush()
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    if (g_backgroundBrush == nullptr) {
        OutputDebugStringW(L"Failed to refresh the background brush due to the brush is null.");
        return false;
    }
    if (IsHighContrastModeEnabled()) {
        // ### TO BE IMPLEMENTED
    } else if (ShouldAppsUseDarkMode()) {
        g_backgroundBrush.TintColor(Constants::Dark::TintColor);
        g_backgroundBrush.TintOpacity(Constants::Dark::TintOpacity);
        g_backgroundBrush.TintLuminosityOpacity(Constants::Dark::LuminosityOpacity);
        g_backgroundBrush.FallbackColor(Constants::Dark::FallbackColor);
    } else {
        g_backgroundBrush.TintColor(Constants::Light::TintColor);
        g_backgroundBrush.TintOpacity(Constants::Light::TintOpacity);
        g_backgroundBrush.TintLuminosityOpacity(Constants::Light::LuminosityOpacity);
        g_backgroundBrush.FallbackColor(Constants::Light::FallbackColor);
    }
    return true;
}

[[nodiscard]] static inline bool CloseMainWindow(const HWND hWnd)
{
    static bool tried = false;
    using GetClassLongPtrWSig = decltype(&::GetClassLongPtrW);
    static GetClassLongPtrWSig GetClassLongPtrWFunc = nullptr;
    using GetWindowLongPtrWSig = decltype(&::GetWindowLongPtrW);
    static GetWindowLongPtrWSig GetWindowLongPtrWFunc = nullptr;
    using DestroyWindowSig = decltype(&::DestroyWindow);
    static DestroyWindowSig DestroyWindowFunc = nullptr;
    using UnregisterClassWSig = decltype(&::UnregisterClassW);
    static UnregisterClassWSig UnregisterClassWFunc = nullptr;
    if (!GetClassLongPtrWFunc || !GetWindowLongPtrWFunc || !DestroyWindowFunc || !UnregisterClassWFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                GetClassLongPtrWFunc = reinterpret_cast<GetClassLongPtrWSig>(GetProcAddress(User32DLL, "GetClassLongPtrW"));
                if (!GetClassLongPtrWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol GetClassLongPtrW().");
                }
                GetWindowLongPtrWFunc = reinterpret_cast<GetWindowLongPtrWSig>(GetProcAddress(User32DLL, "GetWindowLongPtrW"));
                if (!GetWindowLongPtrWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol GetWindowLongPtrW().");
                }
                DestroyWindowFunc = reinterpret_cast<DestroyWindowSig>(GetProcAddress(User32DLL, "DestroyWindow"));
                if (!DestroyWindowFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol DestroyWindow().");
                }
                UnregisterClassWFunc = reinterpret_cast<UnregisterClassWSig>(GetProcAddress(User32DLL, "UnregisterClassW"));
                if (!UnregisterClassWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol UnregisterClassW().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (GetClassLongPtrWFunc && GetWindowLongPtrWFunc && DestroyWindowFunc && UnregisterClassWFunc) {
        if (!hWnd) {
            OutputDebugStringW(L"Failed to close the window due to the given window handle is null.");
            return false;
        }
        if (DestroyWindowFunc(hWnd) == FALSE) {
            DisplayErrorDialog(L"Failed to destroy the window.");
            return false;
        }
        const auto atom = static_cast<ATOM>(GetClassLongPtrWFunc(hWnd, GCW_ATOM));
        if (atom == INVALID_ATOM) {
            DisplayErrorDialog(L"Failed to retrieve the ATOM of the window.");
            return false;
        }
        const auto instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtrWFunc(hWnd, GWLP_HINSTANCE));
        if (!instance) {
            DisplayErrorDialog(L"Failed to retrieve the HINSTANCE of the window.");
            return false;
        }
        if (UnregisterClassWFunc(reinterpret_cast<LPCWSTR>(MAKEWORD(atom, 0)), instance) == FALSE) {
            DisplayErrorDialog(L"Failed to unregister the window class.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"GetClassLongPtrW(), GetWindowLongPtrW(), DestroyWindow() and UnregisterClassW() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool SyncXAMLIslandPosition(const UINT width, const UINT height)
{
    static bool tried = false;
    using MoveWindowSig = decltype(&::MoveWindow);
    static MoveWindowSig MoveWindowFunc = nullptr;
    if (!MoveWindowFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                MoveWindowFunc = reinterpret_cast<MoveWindowSig>(GetProcAddress(User32DLL, "MoveWindow"));
                if (!MoveWindowFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol MoveWindow().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (MoveWindowFunc) {
        if (!g_xamlIslandWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
            return false;
        }
        if ((width == 0) || (height == 0)) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to invalid width and height.");
            return false;
        }
        if (MoveWindowFunc(g_xamlIslandWindowHandle, 0, 0, width, height, TRUE) == FALSE) {
            DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"MoveWindow() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool SyncXAMLIslandPosition()
{
    static bool tried = false;
    using GetClientRectSig = decltype(&::GetClientRect);
    static GetClientRectSig GetClientRectFunc = nullptr;
    if (!GetClientRectFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                GetClientRectFunc = reinterpret_cast<GetClientRectSig>(GetProcAddress(User32DLL, "GetClientRect"));
                if (!GetClientRectFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol GetClientRect().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (GetClientRectFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to main window handle is null.");
            return false;
        }
        if (!g_xamlIslandWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
            return false;
        }
        RECT clientRect = {0, 0, 0, 0};
        if (GetClientRectFunc(g_mainWindowHandle, &clientRect) == FALSE) {
            DisplayErrorDialog(L"Failed to retrieve the client rect of the window.");
            return false;
        }
        return SyncXAMLIslandPosition(clientRect.right, clientRect.bottom);
    } else {
        OutputDebugStringW(L"GetClientRect() is not available.");
        return false;
    }
}

static inline void DisposeResources()
{
    if (g_source != nullptr) {
        g_source.Close();
        g_source = nullptr;
    }
    if (g_manager != nullptr) {
        g_manager.Close();
        g_manager = nullptr;
    }
}

[[nodiscard]] static inline bool GenerateGUID(LPCWSTR *str)
{
    static bool tried = false;
    using CoCreateGuidSig = decltype(&::CoCreateGuid);
    static CoCreateGuidSig CoCreateGuidFunc = nullptr;
    using StringFromGUID2Sig = decltype(&::StringFromGUID2);
    static StringFromGUID2Sig StringFromGUID2Func = nullptr;
    if (!CoCreateGuidFunc || !StringFromGUID2Func) {
        if (!tried) {
            tried = true;
            const HMODULE OLE32DLL = LoadLibraryExW(L"OLE32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (OLE32DLL) {
                CoCreateGuidFunc = reinterpret_cast<CoCreateGuidSig>(GetProcAddress(OLE32DLL, "CoCreateGuid"));
                if (!CoCreateGuidFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol CoCreateGuid().");
                }
                StringFromGUID2Func = reinterpret_cast<StringFromGUID2Sig>(GetProcAddress(OLE32DLL, "StringFromGUID2"));
                if (!StringFromGUID2Func) {
                    OutputDebugStringW(L"Failed to resolve symbol StringFromGUID2().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library OLE32.dll.");
            }
        }
    }
    if (CoCreateGuidFunc && StringFromGUID2Func) {
        if (!str) {
            OutputDebugStringW(L"Failed to generate GUID due to the given string address is null.");
            return false;
        }
        GUID guid = {};
        if (FAILED(CoCreateGuidFunc(&guid))) {
            DisplayErrorDialog(L"Failed to generate GUID.");
            return false;
        }
        auto buf = new wchar_t[MAX_PATH];
        SecureZeroMemory(buf, sizeof(buf));
        if (StringFromGUID2Func(guid, buf, MAX_PATH) == 0) {
            delete [] buf;
            buf = nullptr;
            DisplayErrorDialog(L"Failed to convert GUID to string.");
            return false;
        }
        *str = buf;
        return true;
    } else {
        OutputDebugStringW(L"CoCreateGuid() and StringFromGUID2() are not available.");
        return false;
    }
}

[[nodiscard]] static inline LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool tried = false;
    using PostQuitMessageSig = decltype(&::PostQuitMessage);
    static PostQuitMessageSig PostQuitMessageFunc = nullptr;
    using DefWindowProcWSig = decltype(&::DefWindowProcW);
    static DefWindowProcWSig DefWindowProcWFunc = nullptr;
    if (!PostQuitMessageFunc || !DefWindowProcWFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                PostQuitMessageFunc = reinterpret_cast<PostQuitMessageSig>(GetProcAddress(User32DLL, "PostQuitMessage"));
                if (!PostQuitMessageFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol PostQuitMessage().");
                }
                DefWindowProcWFunc = reinterpret_cast<DefWindowProcWSig>(GetProcAddress(User32DLL, "DefWindowProcW"));
                if (!DefWindowProcWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol DefWindowProcW().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    switch (message) {
    case WM_CLOSE: {
        if (!CloseMainWindow(hWnd)) {
            DisplayErrorDialog(L"Failed to close the window.");
            break;
        }
        return 0;
    } break;
    case WM_DESTROY: {
        DisposeResources();
        if (PostQuitMessageFunc) {
            PostQuitMessageFunc(0);
            return 0;
        } else {
            OutputDebugStringW(L"PostQuitMessage() is not available.");
            break;
        }
    } break;
    case WM_SIZE: {
        if (g_xamlIslandWindowHandle) {
            const UINT width = LOWORD(lParam);
            const UINT height = HIWORD(lParam);
            if (!SyncXAMLIslandPosition(width, height)) {
                DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
                break;
            }
        }
    } break;
    case WM_SETTINGCHANGE: {
        if (!IsWindows10RS1OrGreater()) {
            break;
        }
        // wParam == 0: User-wide setting change
        // wParam == 1: System-wide setting change
        if (((wParam == 0) || (wParam == 1)) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            if (!RefreshWindowTheme(hWnd)) {
                DisplayErrorDialog(L"Failed to refresh the window theme.");
                break;
            }
            if (!RefreshBackgroundBrush()) {
                DisplayErrorDialog(L"Failed to refresh the background brush.");
                break;
            }
        }
    } break;
    default:
        break;
    }
    if (DefWindowProcWFunc) {
        return DefWindowProcWFunc(hWnd, message, wParam, lParam);
    } else {
        OutputDebugStringW(L"DefWindowProcW() is not available.");
        return 0;
    }
}

[[nodiscard]] static inline bool RegisterMainWindowClass(LPCWSTR name, const WNDPROC WndProc, const HINSTANCE instance)
{
    static bool tried = false;
    using LoadCursorWSig = decltype(&::LoadCursorW);
    static LoadCursorWSig LoadCursorWFunc = nullptr;
    using LoadIconWSig = decltype(&::LoadIconW);
    static LoadIconWSig LoadIconWFunc = nullptr;
    using RegisterClassExWSig = decltype(&::RegisterClassExW);
    static RegisterClassExWSig RegisterClassExWFunc = nullptr;
    if (!LoadCursorWFunc || !LoadIconWFunc || !RegisterClassExWFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                LoadCursorWFunc = reinterpret_cast<LoadCursorWSig>(GetProcAddress(User32DLL, "LoadCursorW"));
                if (!LoadCursorWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol LoadCursorW().");
                }
                LoadIconWFunc = reinterpret_cast<LoadIconWSig>(GetProcAddress(User32DLL, "LoadIconW"));
                if (!LoadIconWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol LoadIconW().");
                }
                RegisterClassExWFunc = reinterpret_cast<RegisterClassExWSig>(GetProcAddress(User32DLL, "RegisterClassExW"));
                if (!RegisterClassExWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol RegisterClassExW().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (LoadCursorWFunc && LoadIconWFunc && RegisterClassExWFunc) {
        if (!WndProc) {
            OutputDebugStringW(L"Failed to register the window class due to the given WNDPROC is null.");
            return false;
        }
        if (!instance) {
            OutputDebugStringW(L"Failed to register the window class due to the given HINSTANCE is null.");
            return false;
        }
        WNDCLASSEXW wcex;
        SecureZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = instance;
        wcex.lpszClassName = [name]{
            if (name && (wcslen(name) > 0)) {
                return name;
            } else {
                LPCWSTR guid = nullptr;
                if (GenerateGUID(&guid)) {
                    return guid;
                } else {
                    return g_defaultWindowClassName;
                }
            }
        }();
        wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
        wcex.hIcon = LoadIconWFunc(instance, MAKEINTRESOURCEW(IDI_APPICON));
        wcex.hIconSm = LoadIconWFunc(instance, MAKEINTRESOURCEW(IDI_APPICON_SMALL));
        g_mainWindowAtom = RegisterClassExWFunc(&wcex);
        return (g_mainWindowAtom != INVALID_ATOM);
    } else {
        OutputDebugStringW(L"LoadCursorW(), LoadIconW() and RegisterClassExW() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool CreateMainWindow(LPCWSTR title, const ATOM atom, const HINSTANCE instance)
{
    static bool tried = false;
    using CreateWindowExWSig = decltype(&::CreateWindowExW);
    static CreateWindowExWSig CreateWindowExWFunc = nullptr;
    if (!CreateWindowExWFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                CreateWindowExWFunc = reinterpret_cast<CreateWindowExWSig>(GetProcAddress(User32DLL, "CreateWindowExW"));
                if (!CreateWindowExWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol CreateWindowExW().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (CreateWindowExWFunc) {
        if (atom == INVALID_ATOM) {
            OutputDebugStringW(L"Failed to create the window due to the given ATOM is invalid.");
            return false;
        }
        if (!instance) {
            OutputDebugStringW(L"Failed to create the window due to the given HINSTANCE is null.");
            return false;
        }
        g_mainWindowHandle = CreateWindowExWFunc(
            WS_EX_NOREDIRECTIONBITMAP,
            reinterpret_cast<LPCWSTR>(MAKEWORD(atom, 0)),
            [title]{
                if (title && (wcslen(title) > 0)) {
                    return title;
                } else {
                    return g_defaultWindowTitle;
                }
            }(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            nullptr, nullptr, instance, nullptr);
        return (g_mainWindowHandle != nullptr);
    } else {
        OutputDebugStringW(L"CreateWindowExW() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool ShowAndUpdateWindow(const HWND hWnd, const int nCmdShow)
{
    static bool tried = false;
    using ShowWindowSig = decltype(&::ShowWindow);
    static ShowWindowSig ShowWindowFunc = nullptr;
    using UpdateWindowSig = decltype(&::UpdateWindow);
    static UpdateWindowSig UpdateWindowFunc = nullptr;
    if (!ShowWindowFunc || !UpdateWindowFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                ShowWindowFunc = reinterpret_cast<ShowWindowSig>(GetProcAddress(User32DLL, "ShowWindow"));
                if (!ShowWindowFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol ShowWindow().");
                }
                UpdateWindowFunc = reinterpret_cast<UpdateWindowSig>(GetProcAddress(User32DLL, "UpdateWindow"));
                if (!UpdateWindowFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol UpdateWindow().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (ShowWindowFunc && UpdateWindowFunc) {
        if (!hWnd) {
            OutputDebugStringW(L"Failed to show the window due to the given window handle is null.");
            return false;
        }
        ShowWindowFunc(hWnd, nCmdShow);
        if (UpdateWindowFunc(hWnd) == FALSE) {
            DisplayErrorDialog(L"Failed to update the window.");
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"ShowWindow() and UpdateWindow() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool InitializeXAMLIsland()
{
    // The call to winrt::init_apartment() initializes COM. By default, in a multithreaded apartment.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // Initialize the XAML framework's core window for the current thread.
    g_manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    g_source = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
    // Get handle to the core window.
    const auto interop = g_source.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        DisplayErrorDialog(L"Failed to retrieve XAML Island's core window.");
        return false;
    }
    // Parent the DesktopWindowXamlSource object to the current window.
    winrt::check_hresult(interop->AttachToWindow(g_mainWindowHandle));
    // Get the new child window's HWND.
    winrt::check_hresult(interop->get_WindowHandle(&g_xamlIslandWindowHandle));
    if (!g_xamlIslandWindowHandle) {
        DisplayErrorDialog(L"Failed to retrieve the window handle of XAML Island's core window.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    if (!SyncXAMLIslandPosition()) {
        DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
        return false;
    }
    // Show the XAML Island window.
    if (!ShowAndUpdateWindow(g_xamlIslandWindowHandle, SW_SHOW)) {
        DisplayErrorDialog(L"Failed to show and update the XAML Island window.");
        return false;
    }
    // Create the XAML content.
    g_rootGrid = winrt::Windows::UI::Xaml::Controls::Grid();
    g_backgroundBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
    if (!RefreshBackgroundBrush()) {
        DisplayErrorDialog(L"Failed to refresh the background brush.");
        return false;
    }
    g_backgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_rootGrid.Background(g_backgroundBrush);
    //g_rootGrid.Children().Clear();
    //g_rootGrid.Children().Append(/* some UWP control */);
    //g_rootGrid.UpdateLayout();
    g_source.Content(g_rootGrid);
    return true;
}

[[nodiscard]] static inline int MessageLoop()
{
    static bool tried = false;
    using GetMessageWSig = decltype(&::GetMessageW);
    static GetMessageWSig GetMessageWFunc = nullptr;
    using TranslateMessageSig = decltype(&::TranslateMessage);
    static TranslateMessageSig TranslateMessageFunc = nullptr;
    using DispatchMessageWSig = decltype(&::DispatchMessageW);
    static DispatchMessageWSig DispatchMessageWFunc = nullptr;
    if (!GetMessageWFunc || !TranslateMessageFunc || !DispatchMessageWFunc) {
        if (!tried) {
            tried = true;
            const HMODULE User32DLL = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (User32DLL) {
                GetMessageWFunc = reinterpret_cast<GetMessageWSig>(GetProcAddress(User32DLL, "GetMessageW"));
                if (!GetMessageWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol GetMessageW().");
                }
                TranslateMessageFunc = reinterpret_cast<TranslateMessageSig>(GetProcAddress(User32DLL, "TranslateMessage"));
                if (!TranslateMessageFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol TranslateMessage().");
                }
                DispatchMessageWFunc = reinterpret_cast<DispatchMessageWSig>(GetProcAddress(User32DLL, "DispatchMessageW"));
                if (!DispatchMessageWFunc) {
                    OutputDebugStringW(L"Failed to resolve symbol DispatchMessageW().");
                }
            } else {
                OutputDebugStringW(L"Failed to load dynamic link library User32.dll.");
            }
        }
    }
    if (GetMessageWFunc && TranslateMessageFunc && DispatchMessageWFunc) {
        if (g_source == nullptr) {
            DisplayErrorDialog(L"Failed to do the message loop due to the XAML Island is not initialized.");
            return -1;
        }
        MSG msg = {};
        while (GetMessageWFunc(&msg, nullptr, 0, 0) != FALSE) {
            const auto interop2 = g_source.as<IDesktopWindowXamlSourceNative2>();
            if (interop2) {
                BOOL filtered = FALSE;
                winrt::check_hresult(interop2->PreTranslateMessage(&msg, &filtered));
                if (filtered == FALSE) {
                    TranslateMessageFunc(&msg);
                    DispatchMessageWFunc(&msg);
                }
            } else {
                DisplayErrorDialog(L"Failed to retrieve the window handle of XAML Island's core window.");
                return -1;
            }
        }
        return static_cast<int>(msg.wParam);
    } else {
        OutputDebugStringW(L"GetMessageW(), TranslateMessage() and DispatchMessageW() are not available.");
        return -1;
    }
}

EXTERN_C int APIENTRY
wWinMain(
    _In_ HINSTANCE     hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR        lpCmdLine,
    _In_ int           nCmdShow
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (!IsWindows1019H1OrGreater()) {
        DisplayErrorDialog(L"This application only supports Windows 10 19H1 and onwards.");
        return -1;
    }

    g_instance = hInstance;

    if (!RegisterMainWindowClass(nullptr, WindowProc, hInstance)) {
        DisplayErrorDialog(L"Failed to register the window class.");
        return -1;
    }

    if (!CreateMainWindow(nullptr, g_mainWindowAtom, hInstance)) {
        DisplayErrorDialog(L"Failed to create the window.");
        return -1;
    }

    if (!RefreshWindowTheme(g_mainWindowHandle)) {
        DisplayErrorDialog(L"Failed to refresh the window theme.");
        return -1;
    }

    if (!InitializeXAMLIsland()) {
        DisplayErrorDialog(L"Failed to initialize XAML Island.");
        return -1;
    }

    if (!ShowAndUpdateWindow(g_mainWindowHandle, nCmdShow)) {
        DisplayErrorDialog(L"Failed to show and update the window.");
        return -1;
    }

    return MessageLoop();
}
