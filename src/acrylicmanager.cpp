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

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

/////////////////////////////////
/////     Global variables
/////////////////////////////////

static constexpr wchar_t g_am_ForceOfficialBlurEnvVar_p[] = L"ACRYLICMANAGER_FORCE_OFFICIAL_BLUR";
static constexpr wchar_t g_am_ForceXAMLIslandEnvVar_p[] = L"ACRYLICMANAGER_FORCE_XAML_ISLAND";
static constexpr wchar_t g_am_ForceDirect2DEnvVar_p[] = L"ACRYLICMANAGER_FORCE_DIRECT2D";

static bool g_am_AcrylicManagerInitialized_p = false;

static bool g_am_IsUsingOfficialBlur_p = false;
static bool g_am_IsUsingXAMLIsland_p = false;
static bool g_am_IsUsingDirect2D_p = false;

/////////////////////////////////
/////     Helper functions
/////////////////////////////////

[[nodiscard]] static inline HRESULT am_MoveWindowHelper_p(const HWND hWnd, const int x, const int y)
{
    if (!hWnd || (x <= 0) || (y <= 0)) {
        return E_INVALIDARG;
    }
    SIZE size = {};
    if (FAILED(am_GetWindowSizeHelper_p(hWnd, &size))) {
        return E_FAIL;
    }
    if (MoveWindow(hWnd, x, y, size.cx, size.cy, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(MoveWindow)
    }
    return S_OK;
}

[[nodiscard]] static inline HRESULT am_ResizeWindowHelper_p(const HWND hWnd, const int w, const int h)
{
    if (!hWnd || (w <= 0) || (h <= 0)) {
        return E_INVALIDARG;
    }
    RECT rect = {};
    if (FAILED(am_GetWindowGeometryHelper_p(hWnd, &rect))) {
        return E_FAIL;
    }
    if (MoveWindow(hWnd, rect.left, rect.top, w, h, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE_AND_RETURN(MoveWindow)
    }
    return S_OK;
}

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

[[nodiscard]] static inline HRESULT am_CleanupHelper_p()
{
    g_am_IsUsingOfficialBlur_p = false;

    // Host window
    if (g_am_HostWindowHandle_p && g_am_HostWindowProc_p) {
        SetWindowLongPtrW(g_am_HostWindowHandle_p, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_am_HostWindowProc_p));
        g_am_HostWindowProc_p = nullptr;
        g_am_HostWindowHandle_p = nullptr;
    }

    // Direct2D
    g_am_D2DBitmapProperties_p = {};
    g_am_DXGISwapChainDesc_p = {};
    SAFE_FREE_CHARARRAY(g_am_WallpaperFilePath_p)
    g_am_DesktopBackgroundColor_p = D2D1::ColorF(D2D1::ColorF::Black);
    g_am_WallpaperAspectStyle_p = WallpaperAspectStyle::Invalid;
    g_am_D3DFeatureLevel_p = D3D_FEATURE_LEVEL_1_0_CORE;
    g_am_IsUsingDirect2D_p = false;

    // XAML Island
    if (g_am_XAMLSource_p) {
        g_am_XAMLSource_p.Close();
        g_am_XAMLSource_p = nullptr;
    }
    g_am_RootGrid_p = nullptr;
    g_am_BackgroundBrush_p = nullptr;
    if (g_am_XAMLManager_p) {
        g_am_XAMLManager_p.Close();
        g_am_XAMLManager_p = nullptr;
    }
    g_am_BrushTheme_p = SystemTheme::Invalid;
    g_am_TintColor_p = {};
    g_am_TintOpacity_p = 0.0;
    g_am_TintLuminosityOpacity_p = nullptr;
    g_am_FallbackColor_p = {};
    g_am_IsUsingXAMLIsland_p = false;

    // Drag bar window
    if (g_am_DragBarWindowHandle_p) {
        if (DestroyWindow(g_am_DragBarWindowHandle_p) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow)
        }
        g_am_DragBarWindowHandle_p = nullptr;
    }
    if (g_am_DragBarWindowAtom_p != 0) {
        if (g_am_DragBarWindowClassName_p) {
            if (UnregisterClassW(g_am_DragBarWindowClassName_p, HINST_THISCOMPONENT) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW)
            }
            SAFE_FREE_CHARARRAY(g_am_DragBarWindowClassName_p)
        }
        g_am_DragBarWindowAtom_p = 0;
    }

    // Main window
    if (g_am_MainWindowHandle_p) {
        if (DestroyWindow(g_am_MainWindowHandle_p) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(DestroyWindow)
        }
        g_am_MainWindowHandle_p = nullptr;
    }
    if (g_am_MainWindowAtom_p != 0) {
        if (g_am_MainWindowClassName_p) {
            if (UnregisterClassW(g_am_MainWindowClassName_p, HINST_THISCOMPONENT) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(UnregisterClassW)
            }
            SAFE_FREE_CHARARRAY(g_am_MainWindowClassName_p)
        }
        g_am_MainWindowAtom_p = 0;
    }
    g_am_CurrentDpi_p = 0;
    g_am_CurrentDpr_p = 0.0;

    am_ReleaseAllLoadedModules_p();

    g_am_AcrylicManagerInitialized_p = false;

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

[[nodiscard]] static inline HRESULT am_MainWindowEventLoopHelper_p(int *result)
{
    if (!result || !g_am_MainWindowHandle_p) {
        return E_INVALIDARG;
    }

    MSG msg = {};

    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        BOOL filtered = FALSE;
        if (g_am_XAMLSource_p) {
            const auto interop = g_am_XAMLSource_p.as<IDesktopWindowXamlSourceNative2>();
            if (interop) {
                winrt::check_hresult(interop->PreTranslateMessage(&msg, &filtered));
            }
        }
        if (filtered == FALSE) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    *result = static_cast<int>(msg.wParam);

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

HRESULT am_SetWindowCompositionAttribute_p(const HWND hWnd, LPWINDOWCOMPOSITIONATTRIBDATA pwcad)
{
    if (!hWnd || !pwcad) {
        return E_INVALIDARG;
    }
    static bool tried = false;
    using sig = BOOL(WINAPI *)(HWND, LPWINDOWCOMPOSITIONATTRIBDATA);
    static sig func = nullptr;
    if (!func) {
        if (tried) {
            return E_FAIL;
        } else {
            tried = true;
            FARPROC address = nullptr;
            if (FAILED(am_GetSymbolAddressFromExecutable_p(L"User32.dll", L"SetWindowCompositionAttribute", true, &address))) {
                return E_FAIL;
            }
            func = reinterpret_cast<sig>(address);
            if (!func) {
                return E_FAIL;
            }
        }
    }
    return ((func(hWnd, pwcad) != FALSE) ? S_OK : E_FAIL);
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

HRESULT am_ReleaseAllLoadedModules_p()
{
    if (g_am_LoadedModuleList_p.empty()) {
        return S_OK;
    }
    for (auto &&module : std::as_const(g_am_LoadedModuleList_p)) {
        const HMODULE handle = module.second;
        if (!handle) {
            PRINT(L"The cached module handle is null.")
            continue;
        }
        if (FreeLibrary(handle) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(FreeLibrary)
        }
    }
    g_am_LoadedModuleList_p.clear();
    return S_OK;
}

/////////////////////////////////
/////     Public interface
/////////////////////////////////

HRESULT am_GetVersion(LPWSTR *ver)
{
    return am_GetLibraryVersion_p(ver);
}

HRESULT am_CreateWindow(const int x, const int y, const int w, const int h)
{
    return am_CreateAcrylicWindow_p(x, y, w, h);
}

HRESULT am_GetWindowGeometry(RECT *result)
{
    return am_GetWindowGeometryHelper_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_SetWindowGeometry(const int x, const int y, const int w, const int h)
{
    return am_SetWindowGeometryHelper_p(g_am_MainWindowHandle_p, x, y, w, h);
}

HRESULT am_MoveWindow(const int x, const int y)
{
    return am_MoveWindowHelper_p(g_am_MainWindowHandle_p, x, y);
}

HRESULT am_GetWindowSize(SIZE *result)
{
    return am_GetWindowSizeHelper_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_ResizeWindow(const int w, const int h)
{
    return am_ResizeWindowHelper_p(g_am_MainWindowHandle_p, w, h);
}

HRESULT am_CenterWindow()
{
    return am_CenterWindowHelper_p(g_am_MainWindowHandle_p);
}

HRESULT am_GetWindowState(WindowState *result)
{
    return am_GetWindowStateHelper_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_SetWindowState(const WindowState state)
{
    return am_SetWindowStateHelper_p(g_am_MainWindowHandle_p, state);
}

HRESULT am_CloseWindow()
{
    return am_Release();
}

HRESULT am_GetWindowHandle(HWND *result)
{
    return am_GetWindowHandleHelper_p(result);
}

HRESULT am_GetBrushTheme(SystemTheme *result)
{
    return am_GetBrushThemeHelper_p(result);
}

HRESULT am_SetBrushTheme(const SystemTheme theme)
{
    return am_SwitchAcrylicBrushThemeHelper_p(theme);
}

HRESULT am_GetTintColor(int *r, int *g, int *b, int *a)
{
    return am_GetTintColorHelper_p(r, g, b, a);
}

HRESULT am_SetTintColor(const int r, const int g, const int b, const int a)
{
    return am_SetTintColorHelper_p(r, g, b, a);
}

HRESULT am_GetTintOpacity(double *opacity)
{
    return am_GetTintOpacityHelper_p(opacity);
}

HRESULT am_SetTintOpacity(const double opacity)
{
    return am_SetTintOpacityHelper_p(opacity);
}

HRESULT am_GetTintLuminosityOpacity(double *opacity)
{
    return am_GetTintLuminosityOpacityHelper_p(opacity);
}

HRESULT am_SetTintLuminosityOpacity(const double *opacity)
{
    return am_SetTintLuminosityOpacityHelper_p(opacity);
}

HRESULT am_GetFallbackColor(int *r, int *g, int *b, int *a)
{
    return am_GetFallbackColorHelper_p(r, g, b, a);
}

HRESULT am_SetFallbackColor(const int r, const int g, const int b, const int a)
{
    return am_SetFallbackColorHelper_p(r, g, b, a);
}

HRESULT am_EventLoopExec(int *result)
{
    return am_MainWindowEventLoopHelper_p(result);
}

HRESULT am_IsWindowActive(bool *result)
{
    return am_IsWindowActiveHelper_p(g_am_MainWindowHandle_p, result);
}

HRESULT am_FreeStringA(LPSTR str)
{
    return am_LocalFreeA_p(str);
}

HRESULT am_FreeStringW(LPWSTR str)
{
    return am_LocalFreeW_p(str);
}

HRESULT am_Release()
{
    return am_CleanupHelper_p();
}

HRESULT am_SetHostWindow(const HWND hWnd)
{
    return am_InstallHostWindowHookHelper_p(hWnd, true);
}

HRESULT am_GetHostWindow(HWND *result)
{
    return am_GetHostWindow_p(result);
}

HRESULT am_RemoveHostWindow()
{
    return am_RemoveHostWindow_p();
}

/////////////////////////////////
/////     DLL entry point
/////////////////////////////////

#ifndef ACRYLICMANAGER_STATIC

[[nodiscard]] ACRYLICMANAGER_API BOOL APIENTRY
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
        am_CleanupHelper_p();
        break;
    }
    return TRUE;
}

#endif
