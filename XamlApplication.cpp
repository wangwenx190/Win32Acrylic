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

#include "pch.h"
#include "XamlApplication.h"
#include "XamlWindow.h"
#include "WindowsVersion.h"
#include "Utils.h"

class XamlApplicationPrivate
{
public:
    explicit XamlApplicationPrivate(XamlApplication *q) noexcept;
    ~XamlApplicationPrivate() noexcept;

    [[nodiscard]] bool Initialize() noexcept;
    [[nodiscard]] int Run() const noexcept;

private:
    XamlApplicationPrivate(const XamlApplicationPrivate &) = delete;
    XamlApplicationPrivate &operator=(const XamlApplicationPrivate &) = delete;
    XamlApplicationPrivate(XamlApplicationPrivate &&) = delete;
    XamlApplicationPrivate &operator=(XamlApplicationPrivate &&) = delete;

private:
    XamlApplication *q_ptr = nullptr;
    static bool m_comInitialized;
    static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager m_xamlManager;
    std::unique_ptr<XamlWindow> m_window;
};

bool XamlApplicationPrivate::m_comInitialized = false;
winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager XamlApplicationPrivate::m_xamlManager = nullptr;

XamlApplicationPrivate::XamlApplicationPrivate(XamlApplication *q) noexcept
{
    if (!q) {
        Utils::DisplayErrorDialog(L"XamlApplicationPrivate's q is null.");
        return;
    }
    q_ptr = q;
}

XamlApplicationPrivate::~XamlApplicationPrivate() noexcept
{
    if (m_window) {
        m_window.release();
    }
    if (m_xamlManager != nullptr) {
        m_xamlManager.Close();
        m_xamlManager = nullptr;
    }
}

bool XamlApplicationPrivate::Initialize() noexcept
{
    const VersionNumber &curOsVer = WindowsVersion::CurrentVersion();
    const std::wstring osVerDbgMsg = L"Current operating system version: " + WindowsVersion::ToHumanReadableString(curOsVer);
    OutputDebugStringW(osVerDbgMsg.c_str());
    if (curOsVer < WindowsVersion::Windows10_19Half1) {
        Utils::DisplayErrorDialog(L"This application only supports running on Windows 10 19H1 and onwards.");
        return false;
    }
    if (!m_comInitialized) {
        // The call to winrt::init_apartment() initializes COM. By default, in a multi-threaded apartment.
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        m_comInitialized = true;
    }
    if (m_xamlManager == nullptr) {
        // Initialize the XAML framework's core window for the current thread.
        m_xamlManager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
        if (m_xamlManager == nullptr) {
            Utils::DisplayErrorDialog(L"Failed to initialize the XAML framework.");
            return false;
        }
    }
    m_window = std::make_unique<XamlWindow>();
    // ### TODO: move to screen center.
    m_window->Visibility(WindowState::Normal);
    return true;
}

int XamlApplicationPrivate::Run() const noexcept
{
    if (!q_ptr) {
        Utils::DisplayErrorDialog(L"Can't run the XAML application due to the q_ptr is null.");
        return -1;
    }
    if (!m_window) {
        Utils::DisplayErrorDialog(L"Can't run the XAML application due to the XAML window has not been created yet.");
        return -1;
    }
    return m_window->MessageLoop();
}

XamlApplication::XamlApplication() noexcept
{
    d_ptr = std::make_unique<XamlApplicationPrivate>(this);
    if (!d_ptr->Initialize()) {
        Utils::DisplayErrorDialog(L"Failed to initialize the XAML application.");
    }
}

XamlApplication::~XamlApplication() noexcept = default;

int XamlApplication::Run() const noexcept
{
    return d_ptr->Run();
}
