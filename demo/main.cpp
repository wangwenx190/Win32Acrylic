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
#include <cstdio>

static constexpr int max_window_count = 1;

static inline void PrintWindowInformation(const int index, LPCWSTR id)
{
    auto str = new wchar_t[MAX_PATH];
    SecureZeroMemory(str, sizeof(str));
    swprintf(str, L"Window index: %d, ID: %s.", index, id);
    OutputDebugStringW(str);
    SAFE_FREE_CHARARRAY(str)
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

    int result = -1;

    for (int i = 0; i != max_window_count; ++i) {
        LPCWSTR id = nullptr;
        if (!am_CreateWindow(BrushType::Auto, &id)) {
            OutputDebugStringW(L"Failed to create the acrylic window.");
            continue;
        }
        PrintWindowInformation((i + 1), id);
        if (!am_MoveToScreenCenter(id)) {
            OutputDebugStringW(L"Failed to move the acrylic window to screen center.");
            continue;
        }
        if (!am_SetWindowState(id, WindowState::Shown)) {
            OutputDebugStringW(L"Failed to show the acrylic window.");
            continue;
        }
        result = am_GetMessageLoopResult(id);
    }

    return result;
}