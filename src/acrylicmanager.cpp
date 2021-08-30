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
#include "acrylicbrush.h"
#include "acrylicbrushfactory.h"
#include "utils.h"

static constexpr wchar_t g_forceSystemBrushEnvVar[] = L"ACRYLICMANAGER_FORCE_SYSTEM_BRUSH";
static constexpr wchar_t g_forceCompositionBrushEnvVar[] = L"ACRYLICMANAGER_FORCE_COMPOSITION_BRUSH";
static constexpr wchar_t g_forceWinUI3BrushEnvVar[] = L"ACRYLICMANAGER_FORCE_WINUI3_BRUSH";
static constexpr wchar_t g_forceWinUI2BrushEnvVar[] = L"ACRYLICMANAGER_FORCE_WINUI2_BRUSH";
static constexpr wchar_t g_forceDirect2DBrushEnvVar[] = L"ACRYLICMANAGER_FORCE_DIRECT2D_BRUSH";
static constexpr wchar_t g_forceNullBrushEnvVar[] = L"ACRYLICMANAGER_FORCE_NULL_BRUSH";
static constexpr wchar_t g_forceOverrideBrushEnvVar[] = L"ACRYLICMANAGER_BRUSH_OVERRIDE";

static constexpr wchar_t g_iniFileName[] = L".acrylicmanager.ini";
static constexpr wchar_t g_iniSectionName[] = L"Settings";
static constexpr wchar_t g_iniKeyName[] = L"Brush";

static std::unordered_map<LPCWSTR, AcrylicBrush *> g_brushList = {};

[[nodiscard]] static inline BrushType PickUpTheAppropriateBrushType()
{
    auto env_override_str = Utils::GetStringFromEnvironmentVariable(g_forceOverrideBrushEnvVar);
    const bool env_system = ((_wcsicmp(env_override_str, L"System") == 0)
                             || Utils::GetBoolFromEnvironmentVariable(g_forceSystemBrushEnvVar));
    const bool env_composition = ((_wcsicmp(env_override_str, L"Composition") == 0)
                                  || Utils::GetBoolFromEnvironmentVariable(g_forceCompositionBrushEnvVar));
    const bool env_winui3 = ((_wcsicmp(env_override_str, L"WinUI3") == 0)
                             || Utils::GetBoolFromEnvironmentVariable(g_forceWinUI3BrushEnvVar));
    const bool env_winui2 = ((_wcsicmp(env_override_str, L"WinUI2") == 0)
                             || Utils::GetBoolFromEnvironmentVariable(g_forceWinUI2BrushEnvVar));
    const bool env_direct2d = ((_wcsicmp(env_override_str, L"Direct2D") == 0)
                               || Utils::GetBoolFromEnvironmentVariable(g_forceDirect2DBrushEnvVar));
    const bool env_null = ((_wcsicmp(env_override_str, L"Null") == 0)
                           || Utils::GetBoolFromEnvironmentVariable(g_forceNullBrushEnvVar));
    SAFE_FREE_CHARARRAY(env_override_str)
    if (env_system) {
        return BrushType::System;
    } else if (env_composition) {
        return BrushType::Composition;
    } else if (env_winui3) {
        return BrushType::WinUI3;
    } else if (env_winui2) {
        return BrushType::WinUI2;
    } else if (env_direct2d) {
        return BrushType::Direct2D;
    } else if (env_null) {
        return BrushType::Null;
    }
    auto iniFilePath = new wchar_t[MAX_PATH];
    SecureZeroMemory(iniFilePath, sizeof(iniFilePath));
    auto currentDirPath = Utils::GetCurrentDirectoryPath();
    swprintf(iniFilePath, L"%s\\%s", currentDirPath, g_iniFileName);
    SAFE_FREE_CHARARRAY(currentDirPath)
    auto ini_override_str = Utils::GetStringFromIniFile(iniFilePath, g_iniSectionName, g_iniKeyName);
    SAFE_FREE_CHARARRAY(iniFilePath)
    const bool ini_system = (_wcsicmp(ini_override_str, L"System") == 0);
    const bool ini_composition = (_wcsicmp(ini_override_str, L"Composition") == 0);
    const bool ini_winui3 = (_wcsicmp(ini_override_str, L"WinUI3") == 0);
    const bool ini_winui2 = (_wcsicmp(ini_override_str, L"WinUI2") == 0);
    const bool ini_direct2d = (_wcsicmp(ini_override_str, L"Direct2D") == 0);
    const bool ini_null = (_wcsicmp(ini_override_str, L"Null") == 0);
    SAFE_FREE_CHARARRAY(ini_override_str)
    if (ini_system) {
        return BrushType::System;
    } else if (ini_composition) {
        return BrushType::Composition;
    } else if (ini_winui3) {
        return BrushType::WinUI3;
    } else if (ini_winui2) {
        return BrushType::WinUI2;
    } else if (ini_direct2d) {
        return BrushType::Direct2D;
    } else if (ini_null) {
        return BrushType::Null;
    }
    const bool os_system = Utils::IsWindows7OrGreater();
    const bool os_composition = Utils::IsWindows10OrGreater();
    const bool os_winui3 = Utils::IsWindows10RS5OrGreater();
    const bool os_winui2 = Utils::IsWindows10RS2OrGreater();
    const bool os_direct2d = Utils::IsWindows8OrGreater();
    if (os_system) {
        return BrushType::System;
    } else if (os_composition) {
        return BrushType::Composition;
    } else if (os_winui3) {
        return BrushType::WinUI3;
    } else if (os_winui2) {
        return BrushType::WinUI2;
    } else if (os_direct2d) {
        return BrushType::Direct2D;
    }
    return BrushType::Null;
}

[[nodiscard]] static inline AcrylicBrush *ExtractBrushFromList(LPCWSTR id)
{
    if (!id) {
        return nullptr;
    }
    const auto search = g_brushList.find(id);
    return ((search == g_brushList.end()) ? nullptr : search->second);
}

static inline void Release()
{
    if (g_brushList.empty()) {
        return;
    }
    for (auto &&item : std::as_const(g_brushList)) {
        if (item.first) {
            delete [] item.first;
        }
        if (item.second) {
            delete item.second;
        }
    }
    g_brushList.clear();
}

bool am_CreateWindow(const BrushType type, LPCWSTR *id)
{
    if (!id) {
        return false;
    }
    BrushType realType = type;
    if (realType == BrushType::Auto) {
        realType = PickUpTheAppropriateBrushType();
    }
    assert(realType != BrushType::Auto);
    if (realType == BrushType::Auto) {
        return false;
    }
    HRESULT hr = E_FAIL;
    AcrylicBrush *brush = nullptr;
    switch (realType) {
    case BrushType::System:
        hr = AcrylicBrushFactory::CreateInstance(IID_ACRYLICBRUSH_SYSTEM, &brush);
        break;
    case BrushType::Composition:
        hr = AcrylicBrushFactory::CreateInstance(IID_ACRYLICBRUSH_COMPOSITION, &brush);
        break;
    case BrushType::WinUI3:
        hr = AcrylicBrushFactory::CreateInstance(IID_ACRYLICBRUSH_WINUI3, &brush);
        break;
    case BrushType::WinUI2:
        hr = AcrylicBrushFactory::CreateInstance(IID_ACRYLICBRUSH_WINUI2, &brush);
        break;
    case BrushType::Direct2D:
        hr = AcrylicBrushFactory::CreateInstance(IID_ACRYLICBRUSH_DIRECT2D, &brush);
        break;
    case BrushType::Null:
        hr = AcrylicBrushFactory::CreateInstance(IID_ACRYLICBRUSH_NULL, &brush);
        break;
    default:
        assert(false);
        return false;
    }
    if (FAILED(hr)) {
        PRINT_HR_ERROR_MESSAGE(CreateInstance, hr)
        return false;
    }
    assert(brush);
    if (!brush) {
        return false;
    }
    const auto cleanup = [&brush](){
        if (brush) {
            delete brush;
            brush = nullptr;
        }
    };
    if (!brush->IsSupportedByCurrentOS()) {
        OutputDebugStringW(L"The selected brush type is not supported by the current system.");
        cleanup();
        return false;
    }
    if (!brush->Initialize()) {
        OutputDebugStringW(L"Failed to initialize the acrylic brush.");
        cleanup();
        return false;
    }
    LPCWSTR guid = Utils::GenerateGUID();
    g_brushList.insert({guid, brush});
    *id = guid;
    return true;
}

bool am_DestroyWindow(LPCWSTR id)
{
    const auto brush = ExtractBrushFromList(id);
    if (!brush) {
        return false;
    }
    SendMessageW(brush->GetWindowHandle(), WM_SYSCOMMAND, SC_CLOSE, 0);
    // todo: delete id
    delete brush;
    g_brushList.erase(id);
    return true;
}

RECT am_GetWindowGeometry(LPCWSTR id)
{
    const auto brush = ExtractBrushFromList(id);
    return (brush ? GET_WINDOW_RECT(brush->GetWindowHandle()) : RECT{});
}

bool am_SetWindowGeometry(LPCWSTR id, const RECT geometry)
{
    const auto brush = ExtractBrushFromList(id);
    if (!brush) {
        return false;
    }
    if (MoveWindow(brush->GetWindowHandle(), geometry.left, geometry.top,
                   GET_RECT_WIDTH(geometry), GET_RECT_HEIGHT(geometry), TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(MoveWindow)
        return false;
    }
    return true;
}

POINT am_GetWindowPos(LPCWSTR id)
{
    const auto brush = ExtractBrushFromList(id);
    if (!brush) {
        return {};
    }
    const RECT rect = am_GetWindowGeometry(id);
    return {rect.left, rect.top};
}

bool am_SetWindowPos(LPCWSTR id, const POINT pos)
{
    const auto brush = ExtractBrushFromList(id);
    if (!brush) {
        return false;
    }
    const SIZE size = am_GetWindowSize(id);
    return am_SetWindowGeometry(id, {pos.x, pos.y, size.cx, size.cy});
}

SIZE am_GetWindowSize(LPCWSTR id)
{
    const auto brush = ExtractBrushFromList(id);
    return (brush ? GET_WINDOW_SIZE(brush->GetWindowHandle()) : SIZE{});
}

bool am_SetWindowSize(LPCWSTR id, const SIZE size)
{
    const auto brush = ExtractBrushFromList(id);
    if (!brush) {
        return false;
    }
    const POINT pos = am_GetWindowPos(id);
    return am_SetWindowGeometry(id, {pos.x, pos.y, size.cx, size.cy});
}

WindowState am_GetWindowState(LPCWSTR id)
{
    const auto brush = ExtractBrushFromList(id);
    if (!brush) {
        return WindowState::Invalid;
    }
    const HWND hWnd = brush->GetWindowHandle();
    if (IsMinimized(hWnd)) {
        return WindowState::Minimized;
    } else if (IsWindowNoState(hWnd)) {
        return WindowState::Normal;
    } else if (IsMaximized(hWnd)) {
        return WindowState::Maximized;
    } else if (IsFullScreened(hWnd)) {
        return WindowState::FullScreened;
    } else if (IsWindowHidden(hWnd)) {
        return WindowState::Hidden;
    } else if (IsWindowShown(hWnd)) {
        return WindowState::Shown;
    } else {
        return WindowState::Invalid;
    }
}

bool am_SetWindowState(LPCWSTR id, const WindowState state)
{
    const auto brush = ExtractBrushFromList(id);
    if (!brush) {
        return false;
    }
    const HWND hWnd = brush->GetWindowHandle();
    switch (state) {
    case WindowState::Minimized:
        ShowWindow(hWnd, SW_MINIMIZE);
        break;
    case WindowState::Normal:
        ShowWindow(hWnd, SW_RESTORE);
        break;
    case WindowState::Maximized:
        ShowWindow(hWnd, SW_MAXIMIZE);
        break;
    case WindowState::FullScreened:
        // todo
        break;
    case WindowState::Hidden:
        ShowWindow(hWnd, SW_HIDE);
        break;
    case WindowState::Shown:
        ShowWindow(hWnd, SW_SHOW);
        break;
    default:
        assert(false);
        return false;
    }
    return true;
}

bool am_MoveToScreenCenter(LPCWSTR id)
{
    const auto brush = ExtractBrushFromList(id);
    if (!brush) {
        return false;
    }
    const HWND hWnd = brush->GetWindowHandle();
    const SIZE windowSize = am_GetWindowSize(id);
    const SIZE screenSize = GET_RECT_SIZE(GET_SCREEN_GEOMETRY(GET_CURRENT_SCREEN(hWnd)));
    const auto newX = static_cast<int>(std::round(static_cast<double>(screenSize.cx - windowSize.cx) / 2.0));
    const auto newY = static_cast<int>(std::round(static_cast<double>(screenSize.cy - windowSize.cy) / 2.0));
    return am_SetWindowPos(id, {newX, newY});
}

int am_GetMessageLoopResult(LPCWSTR id)
{
    const auto brush = ExtractBrushFromList(id);
    return (brush ? brush->MessageLoop() : -1);
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
        Release();
        break;
    }
    return TRUE;
}

#endif
