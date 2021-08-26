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

#include "acrylicbrush_system.h"
#include "resource.h"
#include "customframe.h"
#include "utils.h"
#include <DwmApi.h>

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED (0x031E)
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED (0x0320)
#endif

using SetWindowCompositionAttributeSignature = BOOL(WINAPI *)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);

static const std::wstring g_mainWindowClassNameSuffix = L"@SystemMainWindow";
static const std::wstring g_mainWindowTitle = L"AcrylicManager System Main Window";

static std::wstring g_mainWindowClassName = nullptr;
static ATOM g_mainWindowAtom = 0;
static HWND g_mainWindowHandle = nullptr;
static UINT g_currentDpi = 0;
static double g_currentDpr = 0.0;

static HMODULE g_user32Module = nullptr;
static SetWindowCompositionAttributeSignature SetWindowCompositionAttribute = nullptr;

static inline void Cleanup()
{
    if (g_mainWindowHandle) {
        DestroyWindow(g_mainWindowHandle);
        g_mainWindowHandle = nullptr;
    }
    if (g_mainWindowAtom != 0) {
        UnregisterClassW(g_mainWindowClassName.c_str(), GET_CURRENT_INSTANCE);
        g_mainWindowAtom = 0;
        g_mainWindowClassName.clear();
    }
    if (g_currentDpi != 0) {
        g_currentDpi = 0;
    }
    if (g_currentDpr != 0.0) {
        g_currentDpr = 0.0;
    }
    if (SetWindowCompositionAttribute) {
        SetWindowCompositionAttribute = nullptr;
    }
    if (g_user32Module) {
        FreeLibrary(g_user32Module);
        g_user32Module = nullptr;
    }
}

EXTERN_C LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    SecureZeroMemory(&msg, sizeof(msg));
    msg.hwnd = hWnd;
    msg.message = message;
    msg.wParam = wParam;
    msg.lParam = lParam;
    const DWORD pos = GetMessagePos();
    msg.pt = {GET_X_LPARAM(pos), GET_Y_LPARAM(pos)};
    msg.time = GetMessageTime();
    LRESULT result = 0;
    if (CustomFrame::HandleWindowProc(&msg, &result)) {
        return result;
    }

    bool themeChanged = false;

    switch (message) {
    default:
        break;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

AcrylicBrushSystem::AcrylicBrushSystem()
{
}

AcrylicBrushSystem::~AcrylicBrushSystem()
{
}

bool AcrylicBrushSystem::IsSupportedByCurrentOS() const
{
    static const bool result = (Utils::IsWindows11OrGreater()
                                || (Utils::CompareSystemVersion(WindowsVersion::Windows10_RS2, VersionCompare::Greater)
                                    && Utils::CompareSystemVersion(WindowsVersion::Windows10_RS4, VersionCompare::Less)));
    return result;
}

HWND AcrylicBrushSystem::GetWindowHandle() const
{
    return g_mainWindowHandle;
}

int AcrylicBrushSystem::EventLoopExec() const
{
    MSG msg = {};

    while (GetMessageW(&msg, nullptr, 0, 0) != FALSE) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

void AcrylicBrushSystem::Release()
{
    --m_refCount;
    if (m_refCount <= 0) {
        m_refCount = 0;
        delete this;
    }
}

bool AcrylicBrushSystem::RegisterMainWindowClass() const
{
    g_mainWindowClassName = m_windowClassNamePrefix + Utils::GenerateGUID() + g_mainWindowClassNameSuffix;

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWindowProc;
    wcex.hInstance = GET_CURRENT_INSTANCE;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIconW(GET_CURRENT_INSTANCE, MAKEINTRESOURCEW(IDI_DEFAULTICON));
    wcex.hIconSm = LoadIconW(GET_CURRENT_INSTANCE, MAKEINTRESOURCEW(IDI_DEFAULTICONSM));
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wcex.lpszClassName = g_mainWindowClassName.c_str();

    g_mainWindowAtom = RegisterClassExW(&wcex);

    if (g_mainWindowAtom == 0) {
        PRINT_WIN32_ERROR_MESSAGE(RegisterClassExW)
        return false;
    }

    return true;
}

bool AcrylicBrushSystem::CreateMainWindow() const
{
    g_mainWindowHandle = CreateWindowExW(0L,
                                         g_mainWindowClassName.c_str(),
                                         g_mainWindowTitle.c_str(),
                                         WS_OVERLAPPEDWINDOW,
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                         nullptr, nullptr, GET_CURRENT_INSTANCE, nullptr);

    if (!g_mainWindowHandle) {
        PRINT_WIN32_ERROR_MESSAGE(CreateWindowExW)
        return false;
    }

    g_currentDpi = Utils::GetDotsPerInchForWindow(g_mainWindowHandle);
    if (g_currentDpi <= 0) {
        g_currentDpi = USER_DEFAULT_SCREEN_DPI;
    }
    g_currentDpr = Utils::GetDevicePixelRatioForWindow(g_mainWindowHandle);
    if (g_currentDpr <= 0.0) {
        g_currentDpr = 1.0;
    }

    // Ensure DWM still draws the top frame by extending the top frame.
    // This also ensures our window still has the frame shadow drawn by DWM.
    if (!Utils::UpdateFrameMargins(g_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to update main window's frame margins.");
        return false;
    }
    // Force a WM_NCCALCSIZE processing to make the window become frameless immediately.
    if (!Utils::TriggerFrameChangeForWindow(g_mainWindowHandle)) {
        OutputDebugStringW(L"Failed to trigger frame change event for main window.");
        return false;
    }
    // Ensure our window still has window transitions.
    if (!Utils::SetWindowTransitionsEnabled(g_mainWindowHandle, true)) {
        OutputDebugStringW(L"Failed to enable window transitions for main window.");
        return false;
    }

    return true;
}

bool AcrylicBrushSystem::InitializeInternalAPI() const
{
    g_user32Module = LoadLibraryExW(L"User32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!g_user32Module) {
        return false;
    }
    SetWindowCompositionAttribute = reinterpret_cast<SetWindowCompositionAttributeSignature>(GetProcAddress(g_user32Module, "SetWindowCompositionAttribute"));
    if (!SetWindowCompositionAttribute) {
        return false;
    }
    return true;
}

bool AcrylicBrushSystem::SetBlurBehindWindowEnabled(const bool enable, const winrt::Windows::UI::Color &color)
{
    // We prefer using DwmEnableBlurBehindWindow() on Windows 7.
    if (Utils::IsWindows8OrGreater()) {
        ACCENT_POLICY accent;
        SecureZeroMemory(&accent, sizeof(accent));
        WINDOWCOMPOSITIONATTRIBDATA data = {};
        data.Attrib = WCA_ACCENT_POLICY;
        data.pvData = &accent;
        data.cbData = sizeof(accent);
        if (enable) {
            // The gradient color must be set otherwise it'll look like a classic blur.
            // Use semi-transparent gradient color to get better appearance.
            if (gradientColor.isValid()) {
                accent.GradientColor = qRgba(gradientColor.blue(), gradientColor.green(), gradientColor.red(), gradientColor.alpha());
            } else {
                const QColor colorizationColor = DWM::getColorizationColor();
                accent.GradientColor =
                        RGB(qRound(colorizationColor.red() * (colorizationColor.alpha() / 255.0) + 255.0 - colorizationColor.alpha()),
                            qRound(colorizationColor.green() * (colorizationColor.alpha() / 255.0) + 255.0 - colorizationColor.alpha()),
                            qRound(colorizationColor.blue() * (colorizationColor.alpha() / 255.0) + 255.0 - colorizationColor.alpha()));
            }
            if (shouldUseOfficialAcrylicEffect()) {
                accent.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                if (!gradientColor.isValid()) {
                    accent.GradientColor = 0x01FFFFFF;
                }
            } else {
                accent.AccentState = ACCENT_ENABLE_BLURBEHIND;
            }
        } else {
            accent.AccentState = ACCENT_DISABLED;
        }
        if (SetWindowCompositionAttribute(g_mainWindowHandle, &data) == FALSE) {
            PRINT_WIN32_ERROR_MESSAGE(SetWindowCompositionAttribute)
            return false;
        }
        return true;
    } else {
        DWM_BLURBEHIND dwmBB;
        SecureZeroMemory(&dwmBB, sizeof(dwmBB));
        dwmBB.dwFlags = DWM_BB_ENABLE;
        dwmBB.fEnable = enable ? TRUE : FALSE;
        if (enable) {
            dwmBB.dwFlags |= DWM_BB_BLURREGION;
            dwmBB.hRgnBlur = CreateRectRgn(0, 0, -1, -1);
        }
        const HRESULT hr = DwmEnableBlurBehindWindow(g_mainWindowHandle, &dwmBB);
        if (dwmBB.hRgnBlur) {
            if (DeleteObject(dwmBB.hRgnBlur) == FALSE) {
                PRINT_WIN32_ERROR_MESSAGE(DeleteObject)
                return false;
            }
        }
        if (FAILED(hr)) {
            PRINT_HR_ERROR_MESSAGE(DwmEnableBlurBehindWindow, hr)
            return false;
        }
        return true;
    }
}
