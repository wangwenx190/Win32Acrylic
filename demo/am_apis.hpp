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

#pragma once

#include <Windows.h>

static LPCWSTR g_acrylicManagerDllName = L"AcrylicManager.dll";
static LPCWSTR g_acrylicManagerDllNameDebug = L"AcrylicManagerd.dll";

static const int g_am_WindowState_Shown = 5;

using am_GetVersion_sig = HRESULT(WINAPI *)(LPWSTR *);
static am_GetVersion_sig am_GetVersion = nullptr;

using am_FreeStringW_sig = HRESULT(WINAPI *)(LPWSTR);
static am_FreeStringW_sig am_FreeStringW = nullptr;

using am_CreateWindow_sig = HRESULT(WINAPI *)(const int, const int, const int, const int);
static am_CreateWindow_sig am_CreateWindow = nullptr;

using am_CenterWindow_sig = HRESULT(WINAPI *)();
static am_CenterWindow_sig am_CenterWindow = nullptr;

using am_SetWindowState_sig = HRESULT(WINAPI *)(const int);
static am_SetWindowState_sig am_SetWindowState = nullptr;

using am_SetHostWindow_sig = HRESULT(WINAPI *)(const HWND);
static am_SetHostWindow_sig am_SetHostWindow = nullptr;

using am_SetWindowTranslucentBackgroundEnabled_sig = HRESULT(WINAPI *)(const HWND, const bool);
static am_SetWindowTranslucentBackgroundEnabled_sig am_SetWindowTranslucentBackgroundEnabled = nullptr;

using am_EventLoopExec_sig = HRESULT(WINAPI *)(int *);
static am_EventLoopExec_sig am_EventLoopExec = nullptr;

using am_CanUnloadDll_sig = HRESULT(WINAPI *)(bool *);
static am_CanUnloadDll_sig am_CanUnloadDll = nullptr;

using am_Release_sig = HRESULT(WINAPI *)();
static am_Release_sig am_Release = nullptr;

[[nodiscard]] static inline bool InitializeAcrylicManagerLibrary()
{
    static bool tried = false;
    if (tried) {
        return false;
    }
    tried = true;
    HMODULE dll = LoadLibraryExW(g_acrylicManagerDllName, nullptr, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
    if (!dll) {
        dll = LoadLibraryExW(g_acrylicManagerDllNameDebug, nullptr, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
        if (!dll) {
            OutputDebugStringW(L"Failed to load AcrylicManager library.");
            return false;
        }
    }
    am_GetVersion = reinterpret_cast<am_GetVersion_sig>(GetProcAddress(dll, "am_GetVersion"));
    if (!am_GetVersion) {
        OutputDebugStringW(L"Failed to resolve am_GetVersion().");
        return false;
    }
    am_FreeStringW = reinterpret_cast<am_FreeStringW_sig>(GetProcAddress(dll, "am_FreeStringW"));
    if (!am_FreeStringW) {
        OutputDebugStringW(L"Failed to resolve am_FreeStringW().");
        return false;
    }
    am_CreateWindow = reinterpret_cast<am_CreateWindow_sig>(GetProcAddress(dll, "am_CreateWindow"));
    if (!am_CreateWindow) {
        OutputDebugStringW(L"Failed to resolve am_CreateWindow().");
        return false;
    }
    am_CenterWindow = reinterpret_cast<am_CenterWindow_sig>(GetProcAddress(dll, "am_CenterWindow"));
    if (!am_CenterWindow) {
        OutputDebugStringW(L"Failed to resolve am_CenterWindow().");
        return false;
    }
    am_SetWindowState = reinterpret_cast<am_SetWindowState_sig>(GetProcAddress(dll, "am_SetWindowState"));
    if (!am_SetWindowState) {
        OutputDebugStringW(L"Failed to resolve am_SetWindowState().");
        return false;
    }
    am_SetHostWindow = reinterpret_cast<am_SetHostWindow_sig>(GetProcAddress(dll, "am_SetHostWindow"));
    if (!am_SetHostWindow) {
        OutputDebugStringW(L"Failed to resolve am_SetHostWindow().");
        return false;
    }
    am_SetWindowTranslucentBackgroundEnabled = reinterpret_cast<am_SetWindowTranslucentBackgroundEnabled_sig>(GetProcAddress(dll, "am_SetWindowTranslucentBackgroundEnabled_p"));
    if (!am_SetWindowTranslucentBackgroundEnabled) {
        OutputDebugStringW(L"Failed to resolve am_SetWindowTranslucentBackgroundEnabled_p().");
        return false;
    }
    am_EventLoopExec = reinterpret_cast<am_EventLoopExec_sig>(GetProcAddress(dll, "am_EventLoopExec"));
    if (!am_EventLoopExec) {
        OutputDebugStringW(L"Failed to resolve am_EventLoopExec().");
        return false;
    }
    am_CanUnloadDll = reinterpret_cast<am_CanUnloadDll_sig>(GetProcAddress(dll, "am_CanUnloadDll"));
    if (!am_CanUnloadDll) {
        OutputDebugStringW(L"Failed to resolve am_CanUnloadDll().");
        return false;
    }
    am_Release = reinterpret_cast<am_Release_sig>(GetProcAddress(dll, "am_Release"));
    if (!am_Release) {
        OutputDebugStringW(L"Failed to resolve am_Release().");
        return false;
    }
    return true;
}
