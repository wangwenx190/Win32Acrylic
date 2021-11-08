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

class MainWindowPrivate
{
public:
    explicit MainWindowPrivate(MainWindow *q) noexcept;
    ~MainWindowPrivate() noexcept;

    [[nodiscard]] bool Initialize() noexcept;

private:
    MainWindowPrivate(const MainWindowPrivate &) = delete;
    MainWindowPrivate &operator=(const MainWindowPrivate &) = delete;
    MainWindowPrivate(MainWindowPrivate &&) = delete;
    MainWindowPrivate &operator=(MainWindowPrivate &&) = delete;

private:
    MainWindow *q_ptr = nullptr;
};

MainWindowPrivate::MainWindowPrivate(MainWindow *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"MainWindowPrivate's q is null.");
        std::exit(-1);
    }
    q_ptr = q;
    if (!Initialize()) {
        Utils::DisplayErrorDialog(L"Failed to initialize MainWindowPrivate.");
        std::exit(-1);
    }
}

MainWindowPrivate::~MainWindowPrivate() noexcept = default;

bool MainWindowPrivate::Initialize() noexcept
{
    ACCENT_POLICY ap;
    SecureZeroMemory(&ap, sizeof(ap));
    ap.State = ACCENT_ENABLE_ACRYLICBLURBEHIND;
    ap.GradientColor = 0x01FFFFFF;
    WINDOWCOMPOSITIONATTRIBDATA wcad = {};
    wcad.Attrib = WCA_ACCENT_POLICY;
    wcad.pvData = &ap;
    wcad.cbData = sizeof(ap);
    if (SetWindowCompositionAttribute(q_ptr->WindowHandle(), &wcad) == FALSE) {
        PRINT_WIN32_ERROR_MESSAGE(SetWindowCompositionAttribute, L"Failed to set the window composition attribute data.")
        return false;
    }
    return true;
}

MainWindow::MainWindow() noexcept : Window(true)
{
    d_ptr = std::make_unique<MainWindowPrivate>(this);
}

MainWindow::~MainWindow() noexcept = default;
