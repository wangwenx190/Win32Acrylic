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

#include <Windows.h>
#include <cstdio>

static LPCWSTR g_applicationName = L"AcrylicManager Demo Application";

static const int WindowState_Shown = 5;

using am_GetVersion_ptr = HRESULT(WINAPI *)(LPWSTR *);
using am_FreeStringW_ptr = HRESULT(WINAPI *)(LPWSTR);
using am_CreateWindow_ptr = HRESULT(WINAPI *)(const int, const int, const int, const int);
using am_CenterWindow_ptr = HRESULT(WINAPI *)();
using am_SetWindowState_ptr = HRESULT(WINAPI *)(const int);
using am_EventLoopExec_ptr = HRESULT(WINAPI *)(int *);
using am_CanUnloadDll_ptr = HRESULT(WINAPI *)(bool *);
using am_Release_ptr = HRESULT(WINAPI *)();

static am_GetVersion_ptr am_GetVersion_pfn = nullptr;
static am_FreeStringW_ptr am_FreeStringW_pfn = nullptr;
static am_CreateWindow_ptr am_CreateWindow_pfn = nullptr;
static am_CenterWindow_ptr am_CenterWindow_pfn = nullptr;
static am_SetWindowState_ptr am_SetWindowState_pfn = nullptr;
static am_EventLoopExec_ptr am_EventLoopExec_pfn = nullptr;
static am_CanUnloadDll_ptr am_CanUnloadDll_pfn = nullptr;
static am_Release_ptr am_Release_pfn = nullptr;

[[nodiscard]] static inline bool InitializeAcrylicManagerLibrary()
{
    static bool tried = false;
    if (tried) {
        return false;
    }
    tried = true;
    HMODULE dll = LoadLibraryExW(L"AcrylicManager.dll", nullptr, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
    if (!dll) {
        dll = LoadLibraryExW(L"AcrylicManagerd.dll", nullptr, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
        if (!dll) {
            OutputDebugStringW(L"Failed to load AcrylicManager library.");
            return false;
        }
    }
    am_GetVersion_pfn = reinterpret_cast<am_GetVersion_ptr>(GetProcAddress(dll, "am_GetVersion"));
    if (!am_GetVersion_pfn) {
        OutputDebugStringW(L"Failed to resolve am_GetVersion().");
        return false;
    }
    am_FreeStringW_pfn = reinterpret_cast<am_FreeStringW_ptr>(GetProcAddress(dll, "am_FreeStringW"));
    if (!am_FreeStringW_pfn) {
        OutputDebugStringW(L"Failed to resolve am_FreeStringW().");
        return false;
    }
    am_CreateWindow_pfn = reinterpret_cast<am_CreateWindow_ptr>(GetProcAddress(dll, "am_CreateWindow"));
    if (!am_CreateWindow_pfn) {
        OutputDebugStringW(L"Failed to resolve am_CreateWindow().");
        return false;
    }
    am_CenterWindow_pfn = reinterpret_cast<am_CenterWindow_ptr>(GetProcAddress(dll, "am_CenterWindow"));
    if (!am_CenterWindow_pfn) {
        OutputDebugStringW(L"Failed to resolve am_CenterWindow().");
        return false;
    }
    am_SetWindowState_pfn = reinterpret_cast<am_SetWindowState_ptr>(GetProcAddress(dll, "am_SetWindowState"));
    if (!am_SetWindowState_pfn) {
        OutputDebugStringW(L"Failed to resolve am_SetWindowState().");
        return false;
    }
    am_EventLoopExec_pfn = reinterpret_cast<am_EventLoopExec_ptr>(GetProcAddress(dll, "am_EventLoopExec"));
    if (!am_EventLoopExec_pfn) {
        OutputDebugStringW(L"Failed to resolve am_EventLoopExec().");
        return false;
    }
    am_CanUnloadDll_pfn = reinterpret_cast<am_CanUnloadDll_ptr>(GetProcAddress(dll, "am_CanUnloadDll"));
    if (!am_CanUnloadDll_pfn) {
        OutputDebugStringW(L"Failed to resolve am_CanUnloadDll().");
        return false;
    }
    am_Release_pfn = reinterpret_cast<am_Release_ptr>(GetProcAddress(dll, "am_Release"));
    if (!am_Release_pfn) {
        OutputDebugStringW(L"Failed to resolve am_Release().");
        return false;
    }
    return true;
}

EXTERN_C int APIENTRY
wWinMain(
    _In_ HINSTANCE     hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR        lpCmdLine,
    _In_ int           nCmdShow
)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    if (!InitializeAcrylicManagerLibrary()) {
        MessageBoxW(nullptr, L"Failed to initialize AcrylicManager library.", g_applicationName, MB_ICONERROR | MB_OK);
        return -1;
    }

    LPWSTR ver = nullptr;
    if (SUCCEEDED(am_GetVersion_pfn(&ver))) {
        const auto str = new wchar_t[MAX_PATH];
        SecureZeroMemory(str, sizeof(str));
        swprintf(str, L"AcrylicManager version: %s", ver);
        OutputDebugStringW(str);
        delete [] str;
        am_FreeStringW_pfn(ver);
    }

    int result = -1;

    if (SUCCEEDED(am_CreateWindow_pfn(-1, -1, -1, -1))) {
        if (SUCCEEDED(am_CenterWindow_pfn())) {
            if (SUCCEEDED(am_SetWindowState_pfn(WindowState_Shown))) {
                am_EventLoopExec_pfn(&result);
            }
        }
    }

    am_Release_pfn();

    return result;
}
