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

#include "utils.h"
#include <wininet.h>
#include <ShlObj_Core.h>
#include <ShellScalingApi.h>
#include <DwmApi.h>
#include <cmath>

static constexpr wchar_t g_personalizeRegistryKey[] = LR"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)";
static constexpr wchar_t g_dwmRegistryKey[] = LR"(Software\Microsoft\Windows\DWM)";
static constexpr wchar_t g_desktopRegistryKey[] = LR"(Control Panel\Desktop)";

bool Utils::CompareSystemVersion(const WindowsVersion ver, const VersionCompare comp)
{
    OSVERSIONINFOEXW osvi;
    SecureZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    switch (ver) {
    case WindowsVersion::WindowsVista: {
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 6000; // Windows Vista with Service Pack 1: 6001
    } break;
    case WindowsVersion::Windows7: {
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 1;
        osvi.dwBuildNumber = 7600; // Windows 7 with Service Pack 1: 7601
    } break;
    case WindowsVersion::Windows8: {
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 2;
        osvi.dwBuildNumber = 9200;
    } break;
    case WindowsVersion::Windows8_1: {
        osvi.dwMajorVersion = 6;
        osvi.dwMinorVersion = 3;
        osvi.dwBuildNumber = 9200; // Windows 8.1 with Update 1: 9600
    } break;
    case WindowsVersion::Windows10_1507: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 10240;
    } break;
    case WindowsVersion::Windows10_1511: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 10586;
    } break;
    case WindowsVersion::Windows10_1607: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 14393;
    } break;
    case WindowsVersion::Windows10_1703: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 15063;
    } break;
    case WindowsVersion::Windows10_1709: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 16299;
    } break;
    case WindowsVersion::Windows10_1803: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 17134;
    } break;
    case WindowsVersion::Windows10_1809: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 17763;
    } break;
    case WindowsVersion::Windows10_1903: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 18362;
    } break;
    case WindowsVersion::Windows10_1909: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 18363;
    } break;
    case WindowsVersion::Windows10_2004: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 19041;
    } break;
    case WindowsVersion::Windows10_20H2: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 19042;
    } break;
    case WindowsVersion::Windows10_21H1: {
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 19043;
    } break;
    case WindowsVersion::Windows11: {
        // FIXME: check the actual version number of Win11.
        osvi.dwMajorVersion = 11;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 0;
    } break;
    }
    BYTE op = 0;
    switch (comp) {
    case VersionCompare::Less:
        op = VER_LESS;
        break;
    case VersionCompare::Equal:
        op = VER_EQUAL;
        break;
    case VersionCompare::Greater:
        op = VER_GREATER;
        break;
    case VersionCompare::LessOrEqual:
        op = VER_LESS_EQUAL;
        break;
    case VersionCompare::GreaterOrEqual:
        op = VER_GREATER_EQUAL;
        break;
    }
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);
    VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, op);
    return (VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE);
}

bool Utils::IsWindows7OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows7, VersionCompare::GreaterOrEqual);
    return result;
}

bool Utils::IsWindows8OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows8, VersionCompare::GreaterOrEqual);
    return result;
}

bool Utils::IsWindows8Point1OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows8_1, VersionCompare::GreaterOrEqual);
    return result;
}

bool Utils::IsWindows10OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows10, VersionCompare::GreaterOrEqual);
    return result;
}

bool Utils::IsWindows10RS1OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows10_1607, VersionCompare::GreaterOrEqual);
    return result;
}

bool Utils::IsWindows10RS2OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows10_1703, VersionCompare::GreaterOrEqual);
    return result;
}

bool Utils::IsWindows10RS5OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows10_1809, VersionCompare::GreaterOrEqual);
    return result;
}

bool Utils::IsWindows1019H1OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows10_1903, VersionCompare::GreaterOrEqual);
    return result;
}

bool Utils::IsWindows11OrGreater()
{
    static const bool result = CompareSystemVersion(WindowsVersion::Windows11, VersionCompare::GreaterOrEqual);
    return result;
}

std::wstring Utils::GetCurrentDirectoryPath()
{
    auto buf = new wchar_t[MAX_PATH];
    SecureZeroMemory(buf, sizeof(buf));
    if (GetModuleFileNameW(nullptr, buf, MAX_PATH) == 0) {
        SAFE_FREE_CHARARRAY(buf)
        PRINT_WIN32_ERROR_MESSAGE(GetModuleFileNameW)
        return {};
    }
    *wcsrchr(buf, L'\\') = L'\0';
    const std::wstring result = buf;
    SAFE_FREE_CHARARRAY(buf)
    return result;
}

UINT Utils::GetDotsPerInchForWindow(const HWND hWnd)
{
    if (!hWnd) {
        return USER_DEFAULT_SCREEN_DPI;
    }
    {
        const UINT dpi = GetDpiForWindow(hWnd);
        if (dpi > 0) {
            return dpi;
        }
    }
    {
        const UINT dpi = GetSystemDpiForProcess(GetCurrentProcess());
        if (dpi > 0) {
            return dpi;
        }
    }
    {
        const UINT dpi = GetDpiForSystem();
        if (dpi > 0) {
            return dpi;
        }
    }
    {
        UINT dpiX = 0, dpiY = 0;
        if (SUCCEEDED(GetDpiForMonitor(GET_CURRENT_SCREEN(hWnd), static_cast<MONITOR_DPI_TYPE>(MonitorDpiType::EFFECTIVE_DPI), &dpiX, &dpiY))) {
            if ((dpiX > 0) && (dpiY > 0)) {
                return std::round(static_cast<double>(dpiX + dpiY) / 2.0);
            }
        }

    }
    {
        const HDC hdc = GetDC(nullptr);
        if (hdc) {
            const int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
            const int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
            ReleaseDC(nullptr, hdc);
            if ((dpiX > 0) && (dpiY > 0)) {
                return std::round(static_cast<double>(dpiX + dpiY) / 2.0);
            }
        }
    }
    return USER_DEFAULT_SCREEN_DPI;
}

double Utils::GetDevicePixelRatioForWindow(const HWND hWnd)
{
    if (!hWnd) {
        return 1.0;
    }
    const HMONITOR mon = GET_CURRENT_SCREEN(hWnd);
    if (!mon) {
        PRINT_WIN32_ERROR_MESSAGE(MonitorFromWindow)
        return 1.0;
    }
    DEVICE_SCALE_FACTOR dsf = DEVICE_SCALE_FACTOR_INVALID;
    const HRESULT hr = GetScaleFactorForMonitor(mon, &dsf);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(GetScaleFactorForMonitor, hr)
        return 1.0;
    }
    switch (dsf) {
    case SCALE_100_PERCENT:
        return 1.0;
    case SCALE_120_PERCENT:
        return 1.2;
    case SCALE_125_PERCENT:
        return 1.25;
    case SCALE_140_PERCENT:
        return 1.4;
    case SCALE_150_PERCENT:
        return 1.5;
    case SCALE_160_PERCENT:
        return 1.6;
    case SCALE_175_PERCENT:
        return 1.75;
    case SCALE_180_PERCENT:
        return 1.8;
    case SCALE_200_PERCENT:
        return 2.0;
    case SCALE_225_PERCENT:
        return 2.25;
    case SCALE_250_PERCENT:
        return 2.5;
    case SCALE_300_PERCENT:
        return 3.0;
    case SCALE_350_PERCENT:
        return 3.5;
    case SCALE_400_PERCENT:
        return 4.0;
    case SCALE_450_PERCENT:
        return 4.5;
    case SCALE_500_PERCENT:
        return 5.0;
    default:
        return 1.0;
    }
    return 1.0;
}

int Utils::GetResizeBorderThickness(const HWND hWnd)
{
    if (!hWnd) {
        // The padded border will disappear if DWM composition is disabled.
        return (IsCompositionEnabled() ? 8 : 4);
    }
    const UINT dpi = GetDotsPerInchForWindow(hWnd);
    const int result_dpi = (GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi)
                        + GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi));
    const int result_nondpi = (GetSystemMetrics(SM_CXPADDEDBORDER)
                               + GetSystemMetrics(SM_CXSIZEFRAME));
    return ((result_dpi > 0) ? result_dpi : result_nondpi);
}

int Utils::GetCaptionHeight(const HWND hWnd)
{
    if (!hWnd) {
        return 23;
    }
    const UINT dpi = GetDotsPerInchForWindow(hWnd);
    const int result_dpi = GetSystemMetricsForDpi(SM_CYCAPTION, dpi);
    const int result_nondpi = GetSystemMetrics(SM_CYCAPTION);
    return ((result_dpi > 0) ? result_dpi : result_nondpi);
}

int Utils::GetTitleBarHeight(const HWND hWnd)
{
    // The padded border will disappear if DWM composition is disabled.
    const int presetTitleBarHeight = (IsCompositionEnabled() ? 31 : 27);
    if (!hWnd) {
        return presetTitleBarHeight;
    }
    // todo adjustwindowrectfordpi
    const int result = (GetResizeBorderThickness(hWnd) + GetCaptionHeight(hWnd));
    const double dpr = GetDevicePixelRatioForWindow(hWnd);
    return ((result > 0) ? result : std::round(static_cast<double>(presetTitleBarHeight) * ((dpr > 0.0) ? dpr : 1.0)));
}

int Utils::GetWindowVisibleFrameBorderThickness(const HWND hWnd)
{
    if (!hWnd) {
        return 0;
    }
    if (!IsWindows10OrGreater()) {
        return 0;
    }
    UINT value = 0;
    const HRESULT hr = DwmGetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::VISIBLE_FRAME_BORDER_THICKNESS), &value, sizeof(value));
    if (SUCCEEDED(hr)) {
        return value;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    const double dpr = GetDevicePixelRatioForWindow(hWnd);
    return (IsWindowNoState(hWnd) ? std::round(1.0 * ((dpr > 0.0) ? dpr : 1.0)) : 0);
}

bool Utils::ShouldAppsUseDarkMode()
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    const auto resultFromRegistry = []() -> bool {
        const int value = GetIntFromRegistry(HKEY_CURRENT_USER, g_personalizeRegistryKey, L"AppsUseLightTheme");
        return (value == 0);
    };
    // Starting from Windows 10 19H1, ShouldAppsUseDarkMode() always return "TRUE"
    // (actually, a random non-zero number at runtime), so we can't use it due to
    // this unreliability. In this case, we just simply read the user's setting from
    // the registry instead, it's not elegant but at least it works well.
    if (IsWindows1019H1OrGreater()) {
        return resultFromRegistry();
    } else {
        static bool tried = false;
        using sig = BOOL(WINAPI *)();
        static sig func = nullptr;
        if (!func) {
            if (tried) {
                return resultFromRegistry();
            } else {
                tried = true;
                const HMODULE dll = LoadLibraryExW(L"UxTheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (!dll) {
                    PRINT_WIN32_ERROR_MESSAGE(LoadLibraryExW)
                    return resultFromRegistry();
                }
                func = reinterpret_cast<sig>(GetProcAddress(dll, MAKEINTRESOURCEA(132)));
                if (!func) {
                    PRINT_WIN32_ERROR_MESSAGE(GetProcAddress)
                    return resultFromRegistry();
                }
            }
        }
        return (func() != FALSE);
    }
}

bool Utils::ShouldSystemUsesDarkMode()
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    const auto resultFromRegistry = []() -> bool {
        const int value = GetIntFromRegistry(HKEY_CURRENT_USER, g_personalizeRegistryKey, L"SystemUsesLightTheme");
        return (value == 0);
    };
    if (false) {
        return resultFromRegistry();
    } else {
        static bool tried = false;
        using sig = BOOL(WINAPI *)();
        static sig func = nullptr;
        if (!func) {
            if (tried) {
                return resultFromRegistry();
            } else {
                tried = true;
                const HMODULE dll = LoadLibraryExW(L"UxTheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
                if (!dll) {
                    PRINT_WIN32_ERROR_MESSAGE(LoadLibraryExW)
                    return resultFromRegistry();
                }
                func = reinterpret_cast<sig>(GetProcAddress(dll, MAKEINTRESOURCEA(138)));
                if (!func) {
                    PRINT_WIN32_ERROR_MESSAGE(GetProcAddress)
                    return resultFromRegistry();
                }
            }
        }
        return (func() != FALSE);
    }
}

COLORREF Utils::GetColorizationColor()
{
    COLORREF color = RGB(0, 0, 0);
    BOOL opaque = FALSE;
    const HRESULT hr = DwmGetColorizationColor(&color, &opaque);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DwmGetColorizationColor, hr)
        color = static_cast<COLORREF>(GetIntFromRegistry(HKEY_CURRENT_USER, g_dwmRegistryKey, L"ColorizationColor"));
        if (color == 0) {
            color = RGB(128, 128, 128); // Dark gray
        }
    }
    return color;
}

ColorizationArea Utils::GetColorizationArea()
{
    // todo: check which specific win10.
    if (!IsWindows10OrGreater()) {
        return ColorizationArea::None;
    }
    const HKEY rootKey = HKEY_CURRENT_USER;
    const std::wstring keyName = L"ColorPrevalence";
    const int themeValue = GetIntFromRegistry(rootKey, g_personalizeRegistryKey, keyName);
    const int dwmValue = GetIntFromRegistry(rootKey, g_dwmRegistryKey, keyName);
    const bool theme = (themeValue != 0);
    const bool dwm = (dwmValue != 0);
    if (theme && dwm) {
        return ColorizationArea::All;
    } else if (theme) {
        return ColorizationArea::StartMenu_TaskBar_ActionCenter;
    } else if (dwm) {
        return ColorizationArea::TitleBar_WindowBorder;
    }
    return ColorizationArea::None;
}

bool Utils::IsHighContrastModeEnabled()
{
    HIGHCONTRASTW hc;
    SecureZeroMemory(&hc, sizeof(hc));
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SystemParametersInfoW)
        return false;
    }
    return (hc.dwFlags & HCF_HIGHCONTRASTON);
}

bool Utils::IsWindowDarkFrameBorderEnabled(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    BOOL enabled = FALSE;
    HRESULT hr = DwmGetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::USE_IMMERSIVE_DARK_MODE_BEFORE_20H1), &enabled, sizeof(enabled));
    if (SUCCEEDED(hr)) {
        return (enabled != FALSE);
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    hr = DwmGetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::USE_IMMERSIVE_DARK_MODE), &enabled, sizeof(enabled));
    if (SUCCEEDED(hr)) {
        return (enabled != FALSE);
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    return false;
}

bool Utils::SetWindowDarkFrameBorderEnabled(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return false;
    }
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    const BOOL enabled = (enable ? TRUE : FALSE);
    HRESULT hr = DwmSetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::USE_IMMERSIVE_DARK_MODE_BEFORE_20H1), &enabled, sizeof(enabled));
    if (SUCCEEDED(hr)) {
        return true;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so changing it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    hr = DwmSetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::USE_IMMERSIVE_DARK_MODE), &enabled, sizeof(enabled));
    if (SUCCEEDED(hr)) {
        return true;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so changing it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    return false;
}

std::wstring Utils::GetWallpaperFilePath(const int screen)
{
    if (IsWindows8OrGreater()) {
        HRESULT hr = CoInitialize(nullptr);
        if (SUCCEEDED(hr)) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
            hr = CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pDesktopWallpaper));
            if (SUCCEEDED(hr)) {
                UINT monitorCount = 0;
                hr = pDesktopWallpaper->GetMonitorDevicePathCount(&monitorCount);
                if (SUCCEEDED(hr)) {
                    if (screen < monitorCount) {
                        LPWSTR monitorId = nullptr;
                        hr = pDesktopWallpaper->GetMonitorDevicePathAt(screen, &monitorId);
                        if (SUCCEEDED(hr)) {
                            LPWSTR wallpaperPath = nullptr;
                            hr = pDesktopWallpaper->GetWallpaper(monitorId, &wallpaperPath);
                            if (SUCCEEDED(hr)) {
                                CoTaskMemFree(monitorId);
                                const std::wstring result = wallpaperPath;
                                CoTaskMemFree(wallpaperPath);
                                COM_SAFE_RELEASE(pDesktopWallpaper)
                                CoUninitialize();
                                return result;
                            } else {
                                CoTaskMemFree(monitorId);
                                COM_SAFE_RELEASE(pDesktopWallpaper)
                                PRINT_HR_ERROR_MESSAGE(GetWallpaper, hr)
                            }
                        } else {
                            COM_SAFE_RELEASE(pDesktopWallpaper)
                            PRINT_HR_ERROR_MESSAGE(GetMonitorDevicePathAt, hr)
                        }
                    } else {
                        COM_SAFE_RELEASE(pDesktopWallpaper)
                        OutputDebugStringW(L"The given screen ID is beyond total screen count.");
                    }
                } else {
                    COM_SAFE_RELEASE(pDesktopWallpaper)
                    PRINT_HR_ERROR_MESSAGE(GetMonitorDevicePathCount, hr)
                }
            } else {
                COM_SAFE_RELEASE(pDesktopWallpaper)
                PRINT_HR_ERROR_MESSAGE(CoCreateInstance, hr)
            }
            CoUninitialize();
        } else {
            PRINT_HR_ERROR_MESSAGE(CoInitialize, hr)
        }
    }
    HRESULT hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        IActiveDesktop *pActiveDesktop = nullptr;
        hr = CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pActiveDesktop));
        if (SUCCEEDED(hr)) {
            auto wallpaperPath = new wchar_t[MAX_PATH];
            SecureZeroMemory(wallpaperPath, sizeof(wallpaperPath));
            // TODO: AD_GETWP_BMP, AD_GETWP_IMAGE, AD_GETWP_LAST_APPLIED. What's the difference?
            hr = pActiveDesktop->GetWallpaper(wallpaperPath, MAX_PATH, AD_GETWP_LAST_APPLIED);
            if (SUCCEEDED(hr)) {
                const std::wstring result = wallpaperPath;
                SAFE_FREE_CHARARRAY(wallpaperPath)
                COM_SAFE_RELEASE(pActiveDesktop)
                CoUninitialize();
                return result;
            } else {
                SAFE_FREE_CHARARRAY(wallpaperPath)
                COM_SAFE_RELEASE(pActiveDesktop)
                PRINT_HR_ERROR_MESSAGE(GetWallpaper, hr)
            }
        } else {
            COM_SAFE_RELEASE(pActiveDesktop)
            PRINT_HR_ERROR_MESSAGE(CoCreateInstance, hr)
        }
        CoUninitialize();
    } else {
        PRINT_HR_ERROR_MESSAGE(CoInitialize, hr)
    }
    auto wallpaperPath = new wchar_t[MAX_PATH];
    SecureZeroMemory(wallpaperPath, sizeof(wallpaperPath));
    if (SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, wallpaperPath, 0) != FALSE) {
        const std::wstring result = wallpaperPath;
        SAFE_FREE_CHARARRAY(wallpaperPath)
        return result;
    } else {
        SAFE_FREE_CHARARRAY(wallpaperPath)
        PRINT_WIN32_ERROR_MESSAGE(SystemParametersInfoW)
    }
    const std::wstring result = GetStringFromRegistry(HKEY_CURRENT_USER, g_desktopRegistryKey, L"WallPaper");
    return result;
}

COLORREF Utils::GetDesktopBackgroundColor(const int screen)
{
    if (IsWindows8OrGreater()) {
        HRESULT hr = CoInitialize(nullptr);
        if (SUCCEEDED(hr)) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
            hr = CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pDesktopWallpaper));
            if (SUCCEEDED(hr)) {
                COLORREF color = RGB(0, 0, 0);
                hr = pDesktopWallpaper->GetBackgroundColor(&color);
                if (SUCCEEDED(hr)) {
                    COM_SAFE_RELEASE(pDesktopWallpaper)
                    CoUninitialize();
                    return color;
                } else {
                    COM_SAFE_RELEASE(pDesktopWallpaper)
                    PRINT_HR_ERROR_MESSAGE(GetBackgroundColor, hr)
                }
            } else {
                COM_SAFE_RELEASE(pDesktopWallpaper)
                PRINT_HR_ERROR_MESSAGE(CoCreateInstance, hr)
            }
            CoUninitialize();
        } else {
            PRINT_HR_ERROR_MESSAGE(CoInitialize, hr)
        }
    }
    // TODO: Is there any other way to get the background color? Traditional Win32 API? Registry?
    // Is there a COM API for Win7?
    return RGB(0, 0, 0);
}

WallpaperAspectStyle Utils::GetWallpaperAspectStyle(const int screen)
{
    if (IsWindows8OrGreater()) {
        HRESULT hr = CoInitialize(nullptr);
        if (SUCCEEDED(hr)) {
            IDesktopWallpaper *pDesktopWallpaper = nullptr;
            hr = CoCreateInstance(CLSID_DesktopWallpaper, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pDesktopWallpaper));
            if (SUCCEEDED(hr)) {
                DESKTOP_WALLPAPER_POSITION position = DWPOS_FILL;
                hr = pDesktopWallpaper->GetPosition(&position);
                if (SUCCEEDED(hr)) {
                    WallpaperAspectStyle result = WallpaperAspectStyle::Invalid;
                    switch (position) {
                    case DWPOS_CENTER:
                        result = WallpaperAspectStyle::Central;
                        break;
                    case DWPOS_TILE:
                        result = WallpaperAspectStyle::Tiled;
                        break;
                    case DWPOS_STRETCH:
                        result = WallpaperAspectStyle::IgnoreRatioFit;
                        break;
                    case DWPOS_FIT:
                        result = WallpaperAspectStyle::KeepRatioFit;
                        break;
                    case DWPOS_FILL:
                        result = WallpaperAspectStyle::KeepRatioByExpanding;
                        break;
                    case DWPOS_SPAN:
                        result = WallpaperAspectStyle::Span;
                        break;
                    }
                    COM_SAFE_RELEASE(pDesktopWallpaper)
                    CoUninitialize();
                    return result;
                } else {
                    COM_SAFE_RELEASE(pDesktopWallpaper)
                    PRINT_HR_ERROR_MESSAGE(GetPosition, hr)
                }
            } else {
                COM_SAFE_RELEASE(pDesktopWallpaper)
                PRINT_HR_ERROR_MESSAGE(CoCreateInstance, hr)
            }
            CoUninitialize();
        } else {
            PRINT_HR_ERROR_MESSAGE(CoInitialize, hr)
        }
    }
    HRESULT hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr)) {
        IActiveDesktop *pActiveDesktop = nullptr;
        hr = CoCreateInstance(CLSID_ActiveDesktop, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pActiveDesktop));
        if (SUCCEEDED(hr)) {
            WALLPAPEROPT opt;
            SecureZeroMemory(&opt, sizeof(opt));
            opt.dwSize = sizeof(opt);
            hr = pActiveDesktop->GetWallpaperOptions(&opt, 0);
            if (SUCCEEDED(hr)) {
                WallpaperAspectStyle result = WallpaperAspectStyle::Invalid;
                switch (opt.dwStyle) {
                case WPSTYLE_CENTER:
                    result = WallpaperAspectStyle::Central;
                    break;
                case WPSTYLE_TILE:
                    result = WallpaperAspectStyle::Tiled;
                    break;
                case WPSTYLE_STRETCH:
                    result = WallpaperAspectStyle::IgnoreRatioFit;
                    break;
                case WPSTYLE_KEEPASPECT:
                    result = WallpaperAspectStyle::KeepRatioFit;
                    break;
                case WPSTYLE_CROPTOFIT:
                    result = WallpaperAspectStyle::KeepRatioByExpanding;
                    break;
                case WPSTYLE_SPAN:
                    result = WallpaperAspectStyle::Span;
                    break;
                }
                COM_SAFE_RELEASE(pActiveDesktop)
                CoUninitialize();
                return result;
            } else {
                COM_SAFE_RELEASE(pActiveDesktop)
                PRINT_HR_ERROR_MESSAGE(GetWallpaperOptions, hr)
            }
        } else {
            COM_SAFE_RELEASE(pActiveDesktop)
            PRINT_HR_ERROR_MESSAGE(CoCreateInstance, hr)
        }
        CoUninitialize();
    } else {
        PRINT_HR_ERROR_MESSAGE(CoInitialize, hr)
    }
    const HKEY rootKey = HKEY_CURRENT_USER;
    const int styleValue = GetIntFromRegistry(rootKey, g_desktopRegistryKey, L"WallpaperStyle");
    switch (styleValue) {
    case 0: {
        const int tileValue = GetIntFromRegistry(rootKey, g_desktopRegistryKey, L"TileWallpaper");
        if (tileValue != 0) {
            return WallpaperAspectStyle::Tiled;
        } else {
            return WallpaperAspectStyle::Central;
        }
    }
    case 2:
        return WallpaperAspectStyle::IgnoreRatioFit;
    case 6:
        return WallpaperAspectStyle::KeepRatioFit;
    case 10:
        return WallpaperAspectStyle::KeepRatioByExpanding;
    case 22:
        return WallpaperAspectStyle::Span;
    default:
        return WallpaperAspectStyle::Invalid;
    }
}

std::wstring Utils::GetStringFromEnvironmentVariable(const std::wstring &name)
{
    if (name.empty()) {
        return {};
    }
    auto buf = new wchar_t[MAX_PATH];
    SecureZeroMemory(buf, sizeof(buf));
    if (GetEnvironmentVariableW(name.c_str(), buf, sizeof(buf)) == 0) {
        // We eat this error because the given environment variable may not exist.
        SAFE_FREE_CHARARRAY(buf)
        return {};
    }
    const std::wstring result = buf;
    SAFE_FREE_CHARARRAY(buf)
    return result;
}

int Utils::GetIntFromEnvironmentVariable(const std::wstring &name)
{
    if (name.empty()) {
        return 0;
    }
    const std::wstring str = GetStringFromEnvironmentVariable(name);
    return _wtoi(str.c_str());
}

bool Utils::GetBoolFromEnvironmentVariable(const std::wstring &name)
{
    if (name.empty()) {
        return false;
    }
    const std::wstring str = GetStringFromEnvironmentVariable(name);
    return ((_wcsicmp(str.c_str(), L"True") == 0)
            || (_wcsicmp(str.c_str(), L"Enable") == 0)
            || (_wcsicmp(str.c_str(), L"On") == 0)
            || (_wcsicmp(str.c_str(), L"Enabled") == 0)
            || (_wcsicmp(str.c_str(), L"Yes") == 0));
}

std::wstring Utils::GetStringFromIniFile(const std::wstring &file, const std::wstring &section, const std::wstring &key)
{
    if (file.empty() || section.empty() || key.empty()) {
        return {};
    }
    auto buf = new wchar_t[MAX_PATH];
    SecureZeroMemory(buf, sizeof(buf));
    if (GetPrivateProfileStringW(section.c_str(), key.c_str(), nullptr, buf, MAX_PATH, file.c_str()) == 0) {
        SAFE_FREE_CHARARRAY(buf)
        PRINT_WIN32_ERROR_MESSAGE(GetPrivateProfileStringW)
        return {};
    }
    const std::wstring result = buf;
    SAFE_FREE_CHARARRAY(buf)
    return result;
}

int Utils::GetIntFromIniFile(const std::wstring &file, const std::wstring &section, const std::wstring &key)
{
    if (file.empty() || section.empty() || key.empty()) {
        return 0;
    }
    const int result = GetPrivateProfileIntW(section.c_str(), key.c_str(), 0, file.c_str());
    if (GetLastError() != ERROR_SUCCESS) {
        PRINT_WIN32_ERROR_MESSAGE(GetPrivateProfileIntW)
        return 0;
    }
    return result;
}

bool Utils::GetBoolFromIniFile(const std::wstring &file, const std::wstring &section, const std::wstring &key)
{
    if (file.empty() || section.empty() || key.empty()) {
        return false;
    }
    const std::wstring str = GetStringFromIniFile(file, section, key);
    return ((_wcsicmp(str.c_str(), L"True") == 0)
            || (_wcsicmp(str.c_str(), L"Enable") == 0)
            || (_wcsicmp(str.c_str(), L"On") == 0)
            || (_wcsicmp(str.c_str(), L"Yes") == 0)
            || (_wcsicmp(str.c_str(), L"Enabled") == 0));
}

std::wstring Utils::GetStringFromRegistry(const HKEY rootKey, const std::wstring &subKey, const std::wstring &key)
{
    if (!rootKey || subKey.empty() || key.empty()) {
        return {};
    }
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(rootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        PRINT_WIN32_ERROR_MESSAGE(RegOpenKeyExW)
        return {};
    }
    auto buf = new wchar_t[MAX_PATH];
    SecureZeroMemory(buf, sizeof(buf));
    DWORD dwType = REG_SZ;
    DWORD dwSize = sizeof(buf);
    const bool success = (RegQueryValueExW(hKey, key.c_str(), nullptr, &dwType,
                                reinterpret_cast<LPBYTE>(buf), &dwSize) == ERROR_SUCCESS);
    if (!success) {
        // We eat this error because the given registry key and value may not exist.
        SAFE_FREE_CHARARRAY(buf)
    }
    if (RegCloseKey(hKey) != ERROR_SUCCESS) {
        SAFE_FREE_CHARARRAY(buf)
        PRINT_WIN32_ERROR_MESSAGE(RegCloseKey)
        return {};
    }
    const std::wstring result = (success ? buf : std::wstring{});
    SAFE_FREE_CHARARRAY(buf)
    return result;
}

int Utils::GetIntFromRegistry(const HKEY rootKey, const std::wstring &subKey, const std::wstring &key)
{
    if (!rootKey || subKey.empty() || key.empty()) {
        return 0;
    }
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(rootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        PRINT_WIN32_ERROR_MESSAGE(RegOpenKeyExW)
        return 0;
    }
    DWORD dwValue = 0;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(dwValue);
    const bool success = (RegQueryValueExW(hKey, key.c_str(), nullptr, &dwType,
                                reinterpret_cast<LPBYTE>(&dwValue), &dwSize) == ERROR_SUCCESS);
    if (!success) {
        // We eat this error because the given registry key and value may not exist.
    }
    if (RegCloseKey(hKey) != ERROR_SUCCESS) {
        PRINT_WIN32_ERROR_MESSAGE(RegCloseKey)
        return 0;
    }
    return static_cast<int>(dwValue);
}

DpiAwareness Utils::GetDpiAwarenessForWindow(const HWND hWnd)
{
    if (!hWnd) {
        return DpiAwareness::Invalid;
    }
    static DpiAwareness result = DpiAwareness::Invalid;
    if (result != DpiAwareness::Invalid) {
        return result;
    }
    const auto context = GetWindowDpiAwarenessContext(hWnd);
    if (context) {
        const auto awareness = GetAwarenessFromDpiAwarenessContext(context);
        if (awareness != DPI_AWARENESS_INVALID) {
            result = static_cast<DpiAwareness>(awareness);
            return result;
        } else {
            PRINT_WIN32_ERROR_MESSAGE(GetAwarenessFromDpiAwarenessContext)
        }
    } else {
        PRINT_WIN32_ERROR_MESSAGE(GetWindowDpiAwarenessContext)
    }
    PROCESS_DPI_AWARENESS awareness = PROCESS_DPI_UNAWARE;
    const HRESULT hr = GetProcessDpiAwareness(nullptr, &awareness);
    if (SUCCEEDED(hr)) {
        result = static_cast<DpiAwareness>(awareness);
        return result;
    } else {
        PRINT_HR_ERROR_MESSAGE(GetProcessDpiAwareness, hr)
    }
    result = ((IsProcessDPIAware() == FALSE) ? DpiAwareness::Unaware : DpiAwareness::System);
    return result;
}

bool Utils::SetDpiAwarenessForWindow(const HWND hWnd, const DpiAwareness awareness)
{
    if (!hWnd) {
        return false;
    }
    switch (awareness) {
    case DpiAwareness::PerMonitorV2: {
        if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != FALSE) {
            return true;
        }
        PRINT_WIN32_ERROR_MESSAGE(SetProcessDpiAwarenessContext)
        return false;
    }
    case DpiAwareness::PerMonitor: {
        if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE) != FALSE) {
            return true;
        }
        PRINT_WIN32_ERROR_MESSAGE(SetProcessDpiAwarenessContext)
        const HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        if (SUCCEEDED(hr)) {
            return true;
        }
        PRINT_HR_ERROR_MESSAGE(SetProcessDpiAwareness, hr)
        return false;
    }
    case DpiAwareness::System: {
        if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE) != FALSE) {
            return true;
        }
        PRINT_WIN32_ERROR_MESSAGE(SetProcessDpiAwarenessContext)
        const HRESULT hr = SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
        if (SUCCEEDED(hr)) {
            return true;
        }
        PRINT_HR_ERROR_MESSAGE(SetProcessDpiAwareness, hr)
        return false;
    }
    case DpiAwareness::Unaware: {
        if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE) != FALSE) {
            return true;
        }
        PRINT_WIN32_ERROR_MESSAGE(SetProcessDpiAwarenessContext)
        const HRESULT hr = SetProcessDpiAwareness(PROCESS_DPI_UNAWARE);
        if (SUCCEEDED(hr)) {
            return true;
        }
        PRINT_HR_ERROR_MESSAGE(SetProcessDpiAwareness, hr)
        return false;
    }
    default:
        break;
    }
    return false;
}

std::wstring Utils::TranslateErrorCodeToMessage(const std::wstring &function, const HRESULT hr)
{
    if (function.empty() || SUCCEEDED(hr)) {
        return {};
    }
    const DWORD dwError = HRESULT_CODE(hr);
    LPWSTR buf = nullptr;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 0, nullptr) == 0) {
        return {};
    }
    auto str = new wchar_t[MAX_PATH];
    SecureZeroMemory(str, sizeof(str));
    swprintf(str, L"%s failed with error %d: %s", function.c_str(), dwError, buf);
    const std::wstring result = str;
    LocalFree(buf);
    SAFE_FREE_CHARARRAY(str)
    return result;
}

std::wstring Utils::GenerateGUID()
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CoInitialize, hr)
        return {};
    }
    GUID guid = {};
    hr = CoCreateGuid(&guid);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CoCreateGuid, hr)
        CoUninitialize();
        return {};
    }
    auto buf = new wchar_t[MAX_PATH];
    SecureZeroMemory(buf, sizeof(buf));
    if (StringFromGUID2(guid, buf, MAX_PATH) == 0) {
        SAFE_FREE_CHARARRAY(buf)
        PRINT_WIN32_ERROR_MESSAGE(StringFromGUID2)
        CoUninitialize();
        return {};
    }
    CoUninitialize();
    const std::wstring result = buf;
    SAFE_FREE_CHARARRAY(buf)
    return result;
}

bool Utils::UpdateFrameMargins(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    // DwmExtendFrameIntoClientArea() will always fail if DWM composition is disabled.
    if (!IsCompositionEnabled()) {
        return false;
    }
    const int topFrameMargin = (IsWindowNoState(hWnd) ? Utils::GetWindowVisibleFrameBorderThickness(hWnd) : 0);
    const int leftFrameMargin = (IsWindows10OrGreater() ? 0 : 1);
    // We removed the whole top part of the frame (see handling of
    // WM_NCCALCSIZE) so the top border is missing now. We add it back here.
    // Note #1: You might wonder why we don't remove just the title bar instead
    //  of removing the whole top part of the frame and then adding the little
    //  top border back. I tried to do this but it didn't work: DWM drew the
    //  whole title bar anyways on top of the window. It seems that DWM only
    //  wants to draw either nothing or the whole top part of the frame.
    // Note #2: For some reason if you try to set the top margin to just the
    //  top border height (what we want to do), then there is a transparency
    //  bug when the window is inactive, so I've decided to add the whole top
    //  part of the frame instead and then we will hide everything that we
    //  don't need (that is, the whole thing but the little 1 pixel wide border
    //  at the top) in the WM_PAINT handler. This eliminates the transparency
    //  bug and it's what a lot of Win32 apps that customize the title bar do
    //  so it should work fine.
    const MARGINS margins = {leftFrameMargin, leftFrameMargin, topFrameMargin, leftFrameMargin};
    const HRESULT hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DwmExtendFrameIntoClientArea, hr)
        return false;
    }
    return true;
}

bool OpenSystemMenu(const HWND hWnd, const POINT pos)
{
    if (!hWnd) {
        return false;
    }
    const HMENU menu = GetSystemMenu(hWnd, FALSE);
    if (!menu) {
        PRINT_WIN32_ERROR_MESSAGE(GetSystemMenu)
        return false;
    }
    // Update the options based on window state.
    MENUITEMINFOW mii;
    SecureZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fType = MFT_STRING;
    const auto setState = [&mii, menu](const UINT item, const bool enabled) -> bool {
        mii.fState = (enabled ? MF_ENABLED : MF_DISABLED);
        return (SetMenuItemInfoW(menu, item, FALSE, &mii) != FALSE);
    };
    const bool max = IsMaximized(hWnd);
    if (!setState(SC_RESTORE, max)) {
        PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW)
        return false;
    }
    if (!setState(SC_MOVE, !max)) {
        PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW)
        return false;
    }
    if (!setState(SC_SIZE, !max)) {
        PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW)
        return false;
    }
    if (!setState(SC_MINIMIZE, true)) {
        PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW)
        return false;
    }
    if (!setState(SC_MAXIMIZE, !max)) {
        PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW)
        return false;
    }
    if (!setState(SC_CLOSE, true)) {
        PRINT_WIN32_ERROR_MESSAGE(SetMenuItemInfoW)
        return false;
    }
    if (SetMenuDefaultItem(menu, UINT_MAX, FALSE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetMenuDefaultItem)
        return false;
    }
    // ### TODO: support RTL layout.
    const auto ret = TrackPopupMenu(menu, TPM_RETURNCMD, pos.x, pos.y, 0, hWnd, nullptr);
    if (ret != 0) {
        if (PostMessageW(hWnd, WM_SYSCOMMAND, ret, 0) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(PostMessageW)
            return false;
        }
    }
    return true;
}

bool Utils::IsCompositionEnabled()
{
    // DWM composition is always enabled and can't be disabled since Windows 8.
    if (IsWindows8OrGreater()) {
        return true;
    }
    BOOL enabled = FALSE;
    const HRESULT hr = DwmIsCompositionEnabled(&enabled);
    if (SUCCEEDED(hr)) {
        return (enabled != FALSE);
    } else {
        PRINT_HR_ERROR_MESSAGE(DwmIsCompositionEnabled, hr)
    }
    const int dwmComp = GetIntFromRegistry(HKEY_CURRENT_USER, g_dwmRegistryKey, L"Composition");
    return (dwmComp != 0);
}

bool Utils::SetWindowExcludedFromLivePreview(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return false;
    }
    // todo: is it needed to check the dwm composition state?
    const BOOL value = (enable ? TRUE : FALSE);
    const HRESULT hr = DwmSetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::EXCLUDED_FROM_PEEK), &value, sizeof(value));
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(DwmSetWindowAttribute, hr)
        return false;
    }
    return true;
}

bool Utils::RemoveWindowFromTaskListAndTaskBar(const HWND hWnd)
{
    if (!hWnd) {
        return false;
    }
    // todo
    return false;
}
