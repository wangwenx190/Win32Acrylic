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

#include "Application.h"
#include "MainWindow.h"
#include "Utils.h"
#include "WindowsVersion.h"

static constexpr const wchar_t __NEW_LINE[] = L"\r\n";

class ApplicationPrivate
{
public:
    explicit ApplicationPrivate(Application *q) noexcept;
    ~ApplicationPrivate() noexcept;

    [[nodiscard]] bool Initialize() noexcept;
    [[nodiscard]] int Run() const noexcept;

private:
    ApplicationPrivate(const ApplicationPrivate &) = delete;
    ApplicationPrivate &operator=(const ApplicationPrivate &) = delete;
    ApplicationPrivate(ApplicationPrivate &&) = delete;
    ApplicationPrivate &operator=(ApplicationPrivate &&) = delete;

private:
    Application *q_ptr = nullptr;
    std::unique_ptr<MainWindow> m_window;
};

ApplicationPrivate::ApplicationPrivate(Application *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"ApplicationPrivate's q is null.");
        std::exit(-1);
    }
    q_ptr = q;
    if (!Initialize()) {
        Utils::DisplayErrorDialog(L"Failed to initialize the Direct Composition application.");
        std::exit(-1);
    }
}

ApplicationPrivate::~ApplicationPrivate() noexcept = default;

bool ApplicationPrivate::Initialize() noexcept
{
    const VersionNumber &curOsVer = WindowsVersion::CurrentVersion();
    const std::wstring osVerDbgMsg = std::wstring(L"Current operating system version: ") + WindowsVersion::ToHumanReadableString(curOsVer) + std::wstring(__NEW_LINE);
    OutputDebugStringW(osVerDbgMsg.c_str());
    // ### FIXME: Check the exact minimum supported version.
    static constexpr const VersionNumber win10 = VersionNumber(10, 0, 0);
    if (curOsVer < win10) {
        Utils::DisplayErrorDialog(L"This application only supports running on Windows 10 and onwards.");
        return false;
    }
    if (!Utils::SetProcessDPIAwareness(ProcessDPIAwareness::PerMonitorVersion2)) {
        if (!Utils::SetProcessDPIAwareness(ProcessDPIAwareness::PerMonitor)) {
            if (!Utils::SetProcessDPIAwareness(ProcessDPIAwareness::System)) {
                if (!Utils::SetProcessDPIAwareness(ProcessDPIAwareness::GdiScaled)) {
                    Utils::DisplayErrorDialog(L"Failed to enable high DPI scaling for the current process.");
                    return false;
                }
            }
        }
    }
    const ProcessDPIAwareness curPcDPIAwareness = Utils::GetProcessDPIAwareness();
    const std::wstring curPcDPIAwarenessDbgMsg = std::wstring(L"Current process's DPI awareness: ") + Utils::DPIAwarenessToString(curPcDPIAwareness) + std::wstring(__NEW_LINE);
    OutputDebugStringW(curPcDPIAwarenessDbgMsg.c_str());
    m_window = std::make_unique<MainWindow>();
    m_window->StartupLocation(WindowStartupLocation::ScreenCenter);
    m_window->Visibility(WindowState::Windowed);
    m_window->Active(true);
    return true;
}

int ApplicationPrivate::Run() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't run the Direct Composition application due to the q_ptr is null.");
        return -1;
    }
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Can't run the Direct Composition application due to the main window has not been created yet.");
        return -1;
    }
    return MainWindow::MessageLoop();
}

Application::Application() noexcept
{
    d_ptr = std::make_unique<ApplicationPrivate>(this);
}

Application::~Application() noexcept = default;

int Application::Run() const noexcept
{
    return d_ptr->Run();
}
