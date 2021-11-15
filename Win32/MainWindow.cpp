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

#include "MainWindow.h"
#include "Utils.h"
#include "OperationResult.h"
#include "Undocumented.h"
#include "WindowsVersion.h"

static constexpr const DWORD PresetTintColor_Light = 0x01FFFFFF;
static constexpr const DWORD PresetTintColor_Dark = 0; // ### TODO

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *q) noexcept;
    ~MainWindowPrivate() noexcept;

    [[nodiscard]] bool Initialize() noexcept;

private:
    [[nodiscard]] DWORD GetAppropriateAccentColor() const noexcept;
    [[nodiscard]] bool SetBlurBehindParameters(const ACCENT_STATE state, const DWORD color) const noexcept;
    [[nodiscard]] bool MainWindowMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) const noexcept;

    void OnThemeChanged(const WindowTheme arg) noexcept;

private:
    MainWindowPrivate(const MainWindowPrivate &) = delete;
    MainWindowPrivate &operator=(const MainWindowPrivate &) = delete;
    MainWindowPrivate(MainWindowPrivate &&) = delete;
    MainWindowPrivate &operator=(MainWindowPrivate &&) = delete;

private:
    MainWindow *q_ptr = nullptr;
    static inline bool m_osEnvDetected = false;
    static inline bool m_isAPIWorkingWell = false;
};

MainWindowPrivate::MainWindowPrivate(MainWindow *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"MainWindowPrivate's q is null.");
        std::exit(-1);
    }
    q_ptr = q;
    if (Initialize()) {
        if (!m_osEnvDetected) {
            m_osEnvDetected = true;
            // The workaround we mentioned below becomes unusable on Windows 11, the window
            // will flicker a lot during move, and there's some laggy at the same time.
            // But the undocumented API seems to be fixed in some degree, so don't apply
            // the workaround on Windows 11, the laggy is still there, but the flicker will
            // gone.
            const VersionNumber &curOsVer = WindowsVersion::CurrentVersion();
            static constexpr const VersionNumber goodVersionStart = VersionNumber(10, 0, 16190);
            m_isAPIWorkingWell = ((curOsVer >= WindowsVersion::Windows11) || ((curOsVer >= goodVersionStart) && (curOsVer < WindowsVersion::Windows10_RedStone4)));
        }
        q_ptr->CustomMessageHandler(std::bind(&MainWindowPrivate::MainWindowMessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        q_ptr->ThemeChangeHandler(std::bind(&MainWindowPrivate::OnThemeChanged, this, std::placeholders::_1));
    } else {
        Utils::DisplayErrorDialog(L"Failed to initialize MainWindowPrivate.");
        std::exit(-1);
    }
}

MainWindowPrivate::~MainWindowPrivate() noexcept = default;

DWORD MainWindowPrivate::GetAppropriateAccentColor() const noexcept
{
    if (!q_ptr) {
        return 0;
    }
    return ((q_ptr->Theme() == WindowTheme::Light) ? PresetTintColor_Light : PresetTintColor_Dark);
}

bool MainWindowPrivate::Initialize() noexcept
{
    if (!SetBlurBehindParameters(ACCENT_ENABLE_ACRYLICBLURBEHIND, GetAppropriateAccentColor())) {
        Utils::DisplayErrorDialog(L"Failed to enable blur behind window.");
        return false;
    }
    return true;
}

bool MainWindowPrivate::SetBlurBehindParameters(const ACCENT_STATE state, const DWORD color) const noexcept
{
    if (!q_ptr) {
        return false;
    }
    const HWND hWnd = q_ptr->WindowHandle();
    if (!hWnd) {
        return false;
    }
    ACCENT_POLICY ap = {};
    ap.State = state;
    ap.Flags = 2; // Magic number
    ap.GradientColor = color;
    WINDOWCOMPOSITIONATTRIBDATA wcad = {};
    wcad.Attrib = WCA_ACCENT_POLICY;
    wcad.pvData = &ap;
    wcad.cbData = sizeof(ap);
    if (SetWindowCompositionAttribute(hWnd, &wcad) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowCompositionAttribute, L"Failed to set the window composition attribute data.")
        return false;
    }
    if (InvalidateRect(hWnd, nullptr, TRUE) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(InvalidateRect, L"Failed to add the whole client area to the window's update region.")
        return false;
    }
    return true;
}

bool MainWindowPrivate::MainWindowMessageHandler(const UINT message, const WPARAM wParam, const LPARAM lParam, LRESULT *result) const noexcept
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(result);
    if (m_isAPIWorkingWell) {
        return false;
    }
    switch (message) {
    case WM_ENTERSIZEMOVE: {
        // The window will be super laggy during move and resize, we workaround this by
        // switching to the traditional blur temporarily. The undocumented API is buggy
        // and thus we can't truly fix this issue unless Microsoft itself fixes it.
        if (!SetBlurBehindParameters(ACCENT_ENABLE_BLURBEHIND, GetAppropriateAccentColor())) {
            Utils::DisplayErrorDialog(L"Failed to change blur behind parameters.");
        }
    } break;
    case WM_EXITSIZEMOVE: {
        // Switch back to the real acrylic blur.
        if (!SetBlurBehindParameters(ACCENT_ENABLE_ACRYLICBLURBEHIND, GetAppropriateAccentColor())) {
            Utils::DisplayErrorDialog(L"Failed to change blur behind parameters.");
        }
    } break;
    default:
        break;
    }
    return false;
}

void MainWindowPrivate::OnThemeChanged(const WindowTheme arg) noexcept
{
    UNREFERENCED_PARAMETER(arg);
    if (!SetBlurBehindParameters(ACCENT_ENABLE_ACRYLICBLURBEHIND, GetAppropriateAccentColor())) {
        Utils::DisplayErrorDialog(L"Failed to change blur behind parameters.");
    }
}

MainWindow::MainWindow() noexcept : Window(WINDOW_USE_ALTERNATIVE_RENDERING)
{
    d_ptr = std::make_unique<MainWindowPrivate>(this);
}

MainWindow::~MainWindow() noexcept = default;
