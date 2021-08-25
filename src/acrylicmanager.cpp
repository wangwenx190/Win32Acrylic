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

#include "acrylicmanager.h"

static constexpr wchar_t g_forceSystemBackendEnvVar[] = L"ACRYLICMANAGER_FORCE_SYSTEM";
static constexpr wchar_t g_forceCompositionBackendEnvVar[] = L"ACRYLICMANAGER_FORCE_COMPOSITION";
static constexpr wchar_t g_forceWinUI3BackendEnvVar[] = L"ACRYLICMANAGER_FORCE_WINUI3";
static constexpr wchar_t g_forceWinUI2BackendEnvVar[] = L"ACRYLICMANAGER_FORCE_WINUI2";
static constexpr wchar_t g_forceDirect2DBackendEnvVar[] = L"ACRYLICMANAGER_FORCE_DIRECT2D";

static bool g_isUsingSystemBackend = false;
static bool g_isUsingCompositionBackend = false;
static bool g_isUsingWinUI3Backend = false;
static bool g_isUsingWinUI2Backend = false;
static bool g_isUsingDirect2DBackend = false;

[[nodiscard]] static inline HRESULT am_CenterWindowHelper_p(const HWND hWnd)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    RECT windowRect = {0, 0, 0, 0};
    if (GetWindowRect(hWnd, &windowRect) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetWindowRect)
    }
    const int windowWidth = GET_RECT_WIDTH(windowRect);
    const int windowHeight = GET_RECT_HEIGHT(windowRect);
    RECT screenRect = {};
    if (FAILED(am_GetScreenGeometry_p(hWnd, &screenRect))) {
        return E_FAIL;
    }
    const int screenWidth = GET_RECT_WIDTH(screenRect);
    const int screenHeight = GET_RECT_HEIGHT(screenRect);
    const int newX = (screenWidth - windowWidth) / 2;
    const int newY = (screenHeight - windowHeight) / 2;
    if (MoveWindow(hWnd, newX, newY, windowWidth, windowHeight, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(MoveWindow)
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_IsWindowActiveHelper_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    *result = (GetActiveWindow() == hWnd);
    return S_OK;
}

[[nodiscard]] static inline LRESULT CALLBACK am_HookWindowProcHelper_p(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const auto getOriginalResult = [hWnd, uMsg, wParam, lParam]() {
        if (g_am_HostWindowProc_p) {
            return CallWindowProcW(g_am_HostWindowProc_p, hWnd, uMsg, wParam, lParam);
        } else {
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }
    };

    if (!g_am_MainWindowHandle_p) {
        return getOriginalResult();
    }

    switch (uMsg) {
    case WM_WINDOWPOSCHANGED: {
        const auto newPos = reinterpret_cast<LPWINDOWPOS>(lParam);
        // We'll also get the "WM_WINDOWPOSCHANGED" message if the Z order of our window changes.
        // So we have to make sure the background window's Z order is updated in time as well.
        if (SetWindowPos(g_am_MainWindowHandle_p, hWnd,
                         newPos->x, newPos->y, newPos->cx, newPos->cy,
                         SWP_NOACTIVATE | SWP_NOOWNERZORDER) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowPos)
        }
    } break;
    case WM_ACTIVATE: {
        // todo
    } break;
    case WM_CLOSE: {
        SendMessageW(g_am_MainWindowHandle_p, WM_CLOSE, 0, 0);
    } break;
    default:
        break;
    }

    return getOriginalResult();
}

[[nodiscard]] static inline HRESULT am_InitializeAcrylicManagerHelper_p(const int x, const int y, const int w, const int h)
{
    if (g_am_AcrylicManagerInitialized_p) {
        return E_FAIL;
    }
    if (FAILED(am_RegisterMainWindowClassHelper_p())) {
        PRINT_AND_RETURN(L"Failed to register main window class.")
    }
    if (FAILED(am_CreateMainWindowHelper_p(x, y, w, h))) {
        PRINT_AND_RETURN(L"Failed to create main window.")
    }
    const auto official = []() -> HRESULT {
        // todo
        g_am_IsUsingOfficialBlur_p = false;
        return E_FAIL;
    };
    const auto xamlIsland = []() -> HRESULT {
        if (SUCCEEDED(am_CreateXAMLIslandHelper_p())) {
            if (SUCCEEDED(am_RegisterDragBarWindowClassHelper_p())) {
                if (SUCCEEDED(am_CreateDragBarWindowHelper_p())) {
                    g_am_IsUsingXAMLIsland_p = true;
                    g_am_AcrylicManagerInitialized_p = true;
                    return S_OK;
                } else {
                    PRINT_AND_RETURN(L"Failed to create drag bar window.")
                }
            } else {
                PRINT_AND_RETURN(L"Failed to register drag bar window class.")
            }
        } else {
            PRINT_AND_RETURN(L"Failed to create XAML Island.")
        }
    };
    const auto direct2d = []() -> HRESULT {
        if (SUCCEEDED(am_InitializeDirect2DInfrastructureHelper_p())) {
            g_am_IsUsingDirect2D_p = true;
            g_am_AcrylicManagerInitialized_p = true;
            return S_OK;
        } else {
            PRINT_AND_RETURN(L"Failed to initialize the Direct2D infrastructure.")
        }
    };
    if (g_am_IsForceOfficialBlur_p) {
        return official();
    }
    if (g_am_IsForceXAMLIsland_p) {
        return xamlIsland();
    }
    if (g_am_IsForceDirect2D_p) {
        return direct2d();
    }
    if (g_am_IsOfficialBlurAvailable_p) {
        return official();
    }
    if (g_am_IsXAMLIslandAvailable_p) {
        return xamlIsland();
    }
    if (g_am_IsDirect2DAvailable_p) {
        return direct2d();
    }
    // Just don't crash.
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_InstallHostWindowHookHelper_p(const HWND hWnd, const bool enable)
{
    if (!g_am_MainWindowHandle_p || !hWnd || g_am_HostWindowHandle_p || g_am_HostWindowProc_p) {
        return E_INVALIDARG;
    }
    const auto exStyle = static_cast<DWORD>(GetWindowLongPtrW(g_am_MainWindowHandle_p, GWL_EXSTYLE));
    if (enable) {
        const auto hostWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hWnd, GWLP_WNDPROC));
        if (!hostWndProc) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetWindowLongPtrW)
        }
        if (SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(am_HookWindowProcHelper_p)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
        }
        if (SetWindowLongPtrW(g_am_MainWindowHandle_p, GWL_EXSTYLE, static_cast<LONG_PTR>(exStyle | WS_EX_NOACTIVATE)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
        }
        g_am_HostWindowHandle_p = hWnd;
        g_am_HostWindowProc_p = hostWndProc;
    } else {
        if (SetWindowLongPtrW(g_am_MainWindowHandle_p, GWL_EXSTYLE, static_cast<LONG_PTR>(exStyle & ~WS_EX_NOACTIVATE)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
        }
        if (SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_am_HostWindowProc_p)) == 0) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
        }
        g_am_HostWindowProc_p = nullptr;
        g_am_HostWindowHandle_p = nullptr;
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_GetLibraryVersion_p(LPWSTR *ver)
{
    if (!ver) {
        return E_INVALIDARG;
    }
    const auto str = new wchar_t[20]; // 20 should be enough for a version string...
    SecureZeroMemory(str, sizeof(str));
    wcscpy(str, ACRYLICMANAGER_VERSION_STR);
    *ver = str;
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_CreateAcrylicWindow_p(const int x, const int y, const int w, const int h)
{
    if (!g_am_IsWindows7OrGreater_p) {
        PRINT_AND_RETURN(L"AcrylicManager won't be functional on such old systems.")
    }

    bool dwmComp = false;
    if (FAILED(am_IsCompositionEnabled_p(&dwmComp)) || !dwmComp) {
        PRINT_AND_RETURN(L"AcrylicManager won't be functional when DWM composition is disabled.")
    }

    return am_InitializeAcrylicManagerHelper_p(x, y, w, h);
}

[[nodiscard]] static inline HRESULT am_LocalFreeA_p(LPSTR str)
{
    if (!str) {
        return E_INVALIDARG;
    }
    SAFE_FREE_CHARARRAY(str)
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_LocalFreeW_p(LPWSTR str)
{
    if (!str) {
        return E_INVALIDARG;
    }
    SAFE_FREE_CHARARRAY(str)
    return S_OK;
}

HRESULT am_WideToMulti_p(LPCWSTR in, const UINT codePage, LPSTR *out)
{
    if (!in || !out) {
        return E_INVALIDARG;
    }
    const int required = WideCharToMultiByte(codePage, 0, in, -1, nullptr, 0, nullptr, nullptr);
    const auto result = new char[required];
    WideCharToMultiByte(codePage, 0, in, -1, result, required, nullptr, nullptr);
    *out = result;
    return S_OK;
}

HRESULT am_MultiToWide_p(LPCSTR in, const UINT codePage, LPWSTR *out)
{
    if (!in || !out) {
        return E_INVALIDARG;
    }
    const int required = MultiByteToWideChar(codePage, 0, in, -1, nullptr, 0);
    const auto result = new wchar_t[required];
    MultiByteToWideChar(codePage, 0, in, -1, result, required);
    *out = result;
    return S_OK;
}

HRESULT am_IsWindowBackgroundTranslucent_p(const HWND hWnd, bool *result)
{
    if (!hWnd || !result) {
        return E_INVALIDARG;
    }
    BOOL enabled = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::USE_HOSTBACKDROPBRUSH), &enabled, sizeof(enabled)))) {
        *result = (enabled != FALSE);
        return S_OK;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so querying it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    // todo: GetWindowCompositionAttribute()
    const DWORD exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    *result = (exStyle & WS_EX_LAYERED);
    return S_OK;
}

HRESULT am_SetWindowTranslucentBackgroundEnabled_p(const HWND hWnd, const bool enable)
{
    if (!hWnd) {
        return E_INVALIDARG;
    }
    const BOOL enabled = enable ? TRUE : FALSE;
    if (SUCCEEDED(DwmSetWindowAttribute(hWnd, static_cast<DWORD>(DwmWindowAttribute::USE_HOSTBACKDROPBRUSH), &enabled, sizeof(enabled)))) {
        return S_OK;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so changing it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
    }
    ACCENT_POLICY policy;
    SecureZeroMemory(&policy, sizeof(policy));
    policy.AccentState = enable ? ACCENT_ENABLE_HOSTBACKDROP : ACCENT_DISABLED;
    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &policy;
    data.cbData = sizeof(policy);
    if (SUCCEEDED(am_SetWindowCompositionAttribute_p(hWnd, &data))) {
        return S_OK;
    } else {
        // We just eat this error because this enum value was introduced in a very
        // late Windows 10 version, so changing it's value will always result in
        // a "parameter error" (code: 87) on systems before that value was introduced.
        policy.AccentState = enable ? ACCENT_INVALID_STATE : ACCENT_DISABLED;
        if (SUCCEEDED(am_SetWindowCompositionAttribute_p(hWnd, &data))) {
            return S_OK;
        } else {
            // We just eat this error because this enum value was introduced in a very
            // late Windows 10 version, so changing it's value will always result in
            // a "parameter error" (code: 87) on systems before that value was introduced.
        }
    }
    const DWORD exStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    const COLORREF colorKey = RGB(0, 0, 0);
    const BYTE alpha = enable ? 0 : 255;
    if (!enable) {
        if (!(exStyle & WS_EX_LAYERED)) {
            PRINT_AND_RETURN(L"Can't change alpha channel for non-layered window.")
        }
        if (SetLayeredWindowAttributes(hWnd, colorKey, alpha, LWA_COLORKEY) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetLayeredWindowAttributes)
        }
    }
    if (SetWindowLongPtrW(hWnd, GWL_EXSTYLE, (enable ? (exStyle | WS_EX_LAYERED) : (exStyle & ~WS_EX_LAYERED))) == 0) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetWindowLongPtrW)
    }
    if (enable) {
        if (SetLayeredWindowAttributes(hWnd, colorKey, alpha, LWA_COLORKEY) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(SetLayeredWindowAttributes)
        }
    }
    return S_OK;
}

HRESULT am_GetSymbolAddressFromExecutable_p(LPCWSTR path, LPCWSTR function, const bool system, FARPROC *result)
{
    if (!path || !function || !result) {
        return E_INVALIDARG;
    }
    HMODULE module = nullptr;
    const auto search = g_am_LoadedModuleList_p.find(path);
    if (search == g_am_LoadedModuleList_p.end()) {
        const DWORD flag = (system ? LOAD_LIBRARY_SEARCH_SYSTEM32 : LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        module = LoadLibraryExW(path, nullptr, flag);
#else
        static bool tried = false;
        using sig = decltype(&::LoadLibraryExW);
        static sig func = nullptr;
        if (!func) {
            if (tried) {
                return E_FAIL;
            } else {
                tried = true;
                MEMORY_BASIC_INFORMATION mbi;
                SecureZeroMemory(&mbi, sizeof(mbi));
                if (VirtualQuery(reinterpret_cast<LPCVOID>(&VirtualQuery), &mbi, sizeof(mbi)) == 0) {
                    PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(VirtualQuery)
                }
                const auto kernel32 = static_cast<HMODULE>(mbi.AllocationBase);
                if (!kernel32) {
                    PRINT_AND_RETURN(L"Failed to retrieve the Kernel32.dll's base address.")
                }
                func = reinterpret_cast<sig>(GetProcAddress(kernel32, "LoadLibraryExW"));
                if (!func) {
                    PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetProcAddress)
                }
            }
        }
        module = func(path, nullptr, flag);
#endif
        if (module) {
            g_am_LoadedModuleList_p.insert({path, module});
        } else {
            PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(LoadLibraryExW)
        }
    } else {
        module = search->second;
        if (!module) {
            PRINT_AND_RETURN(L"The cached module handle is null.")
        }
    }
    LPSTR functionNameAnsi = nullptr;
    if (FAILED(am_WideToMulti_p(function, CP_UTF8, &functionNameAnsi))) {
        PRINT_AND_RETURN(L"Failed to convert a wide string to multibyte string.")
    }
    const FARPROC address = GetProcAddress(module, functionNameAnsi);
    SAFE_FREE_CHARARRAY(functionNameAnsi)
    if (!address) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(GetProcAddress)
    }
    *result = address;
    return S_OK;
}

#ifndef ACRYLICMANAGER_STATIC

EXTERN_C ACRYLICMANAGER_API BOOL APIENTRY
DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#endif
