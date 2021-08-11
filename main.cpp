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

EXTERN_C int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (!Is19H1OrGreater()) {
        DisplayErrorMessage(L"This application only supports Windows 10 19H1 and onwards.");
        return -1;
    }

    WNDCLASSEXW wcex;
    SecureZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = BackgroundWindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = BACKGROUND_BRUSH;
    wcex.lpszClassName = g_windowClassName;

    if (!RegisterClassExW(&wcex)) {
        DisplayErrorMessage(L"Failed to register the window class.");
        return -1;
    }

    const HWND mainWindowHandle = CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW, g_windowClassName, g_windowTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr
    );
    if (!mainWindowHandle) {
        DisplayErrorMessage(L"Failed to create the main window.");
        return -1;
    }

    g_instance = hInstance;
    g_mainWindowHandle = mainWindowHandle;

    // XAML Island section:
    // The call to winrt::init_apartment initializes COM; by default, in a multithreaded apartment.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // Initialize the XAML framework's core window for the current thread.
    // We need this "windowsXamlManager" live through out the whole application's life-cycle,
    // don't remove it, otherwise the application will crash.
    const auto windowsXamlManager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource desktopWindowXamlSource = {};
    // Get handle to the core window.
    const auto interop = desktopWindowXamlSource.as<IDesktopWindowXamlSourceNative>();
    // Parent the DesktopWindowXamlSource object to the current window.
    winrt::check_hresult(interop->AttachToWindow(mainWindowHandle));
    // Get the new child window's HWND.
    HWND xamlIslandHandle = nullptr;
    interop->get_WindowHandle(&xamlIslandHandle);
    if (!xamlIslandHandle) {
        DisplayErrorMessage(L"Failed to retrieve the XAML Island window handle.");
        return -1;
    }
    g_xamlIslandHandle = xamlIslandHandle;
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    GetClientRect(mainWindowHandle, &rect);
    SetWindowPos(xamlIslandHandle, nullptr, 0, 0, rect.right, rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    // Create the XAML content.
    winrt::Windows::UI::Xaml::Controls::Grid xamlGrid = {};
    winrt::Windows::UI::Xaml::Media::AcrylicBrush acrylicBrush = {};
    acrylicBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    xamlGrid.Background(acrylicBrush);
    //xamlGrid.Children().Clear();
    //xamlGrid.Children().Append(/* some UWP control */);
    //xamlGrid.UpdateLayout();
    desktopWindowXamlSource.Content(xamlGrid);
    // End XAML Island section.

    UpdateFrameMargins(mainWindowHandle);
    TriggerFrameChange(mainWindowHandle);

    ShowWindow(xamlIslandHandle, nCmdShow);
    UpdateWindow(xamlIslandHandle);

    // Message loop:
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
