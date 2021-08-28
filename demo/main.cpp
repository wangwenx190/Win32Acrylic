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
#include <acrylicmanager.h>

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
        OutputDebugStringW(L"Failed to initialize AcrylicManager library.");
        return -1;
    }

    LPWSTR ver = nullptr;
    if (SUCCEEDED(am_GetVersion(&ver))) {
        const auto str = new wchar_t[MAX_PATH];
        SecureZeroMemory(str, sizeof(str));
        swprintf(str, L"AcrylicManager version: %s", ver);
        OutputDebugStringW(str);
        am_FreeStringW(ver);
        delete [] str;
    }

    int result = -1;

    if (SUCCEEDED(am_CreateWindow(-1, -1, -1, -1))) {
        am_CenterWindow();
        am_SetWindowState(g_am_WindowState_Shown);
        am_EventLoopExec(&result);
    }

    am_Release();

    return result;
}
