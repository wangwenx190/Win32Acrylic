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

#include <SDKDDKVer.h>
#include <Windows.h>
#pragma push_macro("GetCurrentTime")
#pragma push_macro("TRY")
#undef GetCurrentTime
#undef TRY
#include <Unknwn.h>
#include <WinRT\Windows.Foundation.Collections.h>
#include <WinRT\Windows.UI.Xaml.Hosting.h>
#include <WinRT\Windows.UI.Xaml.Controls.h>
#include <WinRT\Windows.UI.Xaml.Media.h>
#include <Windows.UI.Xaml.Hosting.DesktopWindowXamlSource.h>
#pragma pop_macro("TRY")
#pragma pop_macro("GetCurrentTime")
#include "Resource.h"
#include "WindowsVersion.h"
#include "SystemLibraryManager.h"
#include "Utils.h"

namespace Constants {
namespace Light {
static constexpr winrt::Windows::UI::Color TintColor = {255, 252, 252, 252};
static constexpr double TintOpacity = 0.0;
static constexpr double LuminosityOpacity = 0.85;
static constexpr winrt::Windows::UI::Color FallbackColor = {255, 249, 249, 249};
} // namespace Light
namespace Dark {
static constexpr winrt::Windows::UI::Color TintColor = {255, 44, 44, 44};
static constexpr double TintOpacity = 0.15;
static constexpr double LuminosityOpacity = 0.96;
static constexpr winrt::Windows::UI::Color FallbackColor = {255, 44, 44, 44};
} // namespace Dark
namespace HighContrast {
// ### TO BE IMPLEMENTED
} // namespace HighContrast
} // namespace Constants

static constexpr wchar_t g_defaultWindowTitle[] = L"Win32AcrylicHelper Application Main Window";

static ATOM g_mainWindowAtom = INVALID_ATOM;
static HWND g_mainWindowHandle = nullptr;
static HWND g_xamlIslandWindowHandle = nullptr;

static winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager g_manager = nullptr;
static winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource g_source = nullptr;
static winrt::Windows::UI::Xaml::Controls::Grid g_rootGrid = nullptr;
static winrt::Windows::UI::Xaml::Media::AcrylicBrush g_backgroundBrush = nullptr;

[[nodiscard]] static inline bool RefreshBackgroundBrush() noexcept
{
    if (!IsWindows10RS1OrGreater()) {
        return false;
    }
    if (g_backgroundBrush == nullptr) {
        OutputDebugStringW(L"Failed to refresh the background brush due to the brush is null.");
        return false;
    }
    if (Utils::IsHighContrastModeEnabled()) {
        // ### TO BE IMPLEMENTED
    } else if (Utils::ShouldAppsUseDarkMode()) {
        g_backgroundBrush.TintColor(Constants::Dark::TintColor);
        g_backgroundBrush.TintOpacity(Constants::Dark::TintOpacity);
        g_backgroundBrush.TintLuminosityOpacity(Constants::Dark::LuminosityOpacity);
        g_backgroundBrush.FallbackColor(Constants::Dark::FallbackColor);
    } else {
        g_backgroundBrush.TintColor(Constants::Light::TintColor);
        g_backgroundBrush.TintOpacity(Constants::Light::TintOpacity);
        g_backgroundBrush.TintLuminosityOpacity(Constants::Light::LuminosityOpacity);
        g_backgroundBrush.FallbackColor(Constants::Light::FallbackColor);
    }
    return true;
}

[[nodiscard]] static inline bool SyncXAMLIslandPosition(const UINT width, const UINT height) noexcept
{
    USER32_API(SetWindowPos);
    if (SetWindowPosFunc) {
        if (!g_xamlIslandWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
            return false;
        }
        if ((width == 0) || (height == 0)) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to invalid width and height.");
            return false;
        }
        if (SetWindowPosFunc(g_xamlIslandWindowHandle, nullptr, 0, 0, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER) == FALSE) {
            auto msg = Utils::GetSystemErrorMessage(L"SetWindowPos");
            if (msg) {
                Utils::DisplayErrorDialog(msg);
                delete [] msg;
                msg = nullptr;
            } else {
                OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window.");
            }
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"SetWindowPos() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool SyncXAMLIslandPosition() noexcept
{
    USER32_API(GetClientRect);
    if (GetClientRectFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to the main window handle is null.");
            return false;
        }
        if (!g_xamlIslandWindowHandle) {
            OutputDebugStringW(L"Failed to sync the geometry of the XAML Island window due to its window handle is null.");
            return false;
        }
        RECT clientRect = {0, 0, 0, 0};
        if (GetClientRectFunc(g_mainWindowHandle, &clientRect) == FALSE) {
            auto msg = Utils::GetSystemErrorMessage(L"GetClientRect");
            if (msg) {
                Utils::DisplayErrorDialog(msg);
                delete [] msg;
                msg = nullptr;
            } else {
                OutputDebugStringW(L"Failed to retrieve the client rect of the window.");
            }
            return false;
        }
        return SyncXAMLIslandPosition(clientRect.right, clientRect.bottom);
    } else {
        OutputDebugStringW(L"GetClientRect() is not available.");
        return false;
    }
}

[[nodiscard]] static inline LRESULT CALLBACK MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (message) {
    case WM_CLOSE: {
        if (Utils::CloseWindow(g_mainWindowHandle, g_mainWindowAtom)) {
            g_mainWindowHandle = nullptr;
            g_mainWindowAtom = INVALID_ATOM;
            return 0;
        } else {
            Utils::DisplayErrorDialog(L"Failed to close the main window.");
        }
    } break;
    case WM_DESTROY: {
        g_backgroundBrush = nullptr;
        g_rootGrid = nullptr;
        if (g_source != nullptr) {
            g_source.Close();
            g_source = nullptr;
        }
        if (g_manager != nullptr) {
            g_manager.Close();
            g_manager = nullptr;
        }
        g_xamlIslandWindowHandle = nullptr;
        USER32_API(PostQuitMessage);
        if (PostQuitMessageFunc) {
            PostQuitMessageFunc(0);
            return 0;
        } else {
            OutputDebugStringW(L"PostQuitMessage() is not available.");
        }
    } break;
    case WM_SIZE: {
        if (g_xamlIslandWindowHandle) {
            const UINT width = LOWORD(lParam);
            const UINT height = HIWORD(lParam);
            if (!SyncXAMLIslandPosition(width, height)) {
                Utils::DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
            }
        }
    } break;
    case WM_SETTINGCHANGE: {
        if (!IsWindows10RS1OrGreater()) {
            break;
        }
        // wParam == 0: User-wide setting change
        // wParam == 1: System-wide setting change
        if (((wParam == 0) || (wParam == 1)) && (_wcsicmp(reinterpret_cast<LPCWSTR>(lParam), L"ImmersiveColorSet") == 0)) {
            if (!Utils::RefreshWindowTheme(hWnd)) {
                Utils::DisplayErrorDialog(L"Failed to refresh the main window theme.");
            }
            if (!RefreshBackgroundBrush()) {
                Utils::DisplayErrorDialog(L"Failed to refresh the background brush.");
            }
        }
    } break;
    default:
        break;
    }
    USER32_API(DefWindowProcW);
    if (DefWindowProcWFunc) {
        return DefWindowProcWFunc(hWnd, message, wParam, lParam);
    } else {
        OutputDebugStringW(L"DefWindowProcW() is not available.");
        return 0;
    }
}

[[nodiscard]] static inline bool RegisterMainWindowClass(LPCWSTR name) noexcept
{
    USER32_API(LoadCursorW);
    USER32_API(LoadIconW);
    USER32_API(RegisterClassExW);
    USER32_API(GetClassInfoExW);
    if (LoadCursorWFunc && LoadIconWFunc && RegisterClassExWFunc && GetClassInfoExWFunc) {
        bool exists = false;
        WNDCLASSEXW wcex = {};
        if (name) {
            exists = (GetClassInfoExWFunc(Utils::GetCurrentInstance(), name, &wcex) != FALSE);
        }
        if (!exists) {
            if (g_mainWindowAtom != INVALID_ATOM) {
                exists = (GetClassInfoExWFunc(Utils::GetCurrentInstance(), Utils::GetWindowClassName(g_mainWindowAtom), &wcex) != FALSE);
            }
        }
        LPCWSTR guid = nullptr;
        if (!exists) {
            if (!Utils::GenerateGUID(&guid)) {
                OutputDebugStringW(L"Failed to register the window class due to can't generate a new GUID.");
                return false;
            }
            SecureZeroMemory(&wcex, sizeof(wcex));
            wcex.cbSize = sizeof(wcex);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = MessageHandler;
            wcex.hInstance = Utils::GetCurrentInstance();
            wcex.lpszClassName = (name ? name : guid);
            wcex.hCursor = LoadCursorWFunc(nullptr, IDC_ARROW);
            wcex.hIcon = LoadIconWFunc(Utils::GetCurrentInstance(), MAKEINTRESOURCEW(IDI_APPICON));
            wcex.hIconSm = LoadIconWFunc(Utils::GetCurrentInstance(), MAKEINTRESOURCEW(IDI_APPICON_SMALL));
        }
        g_mainWindowAtom = RegisterClassExWFunc(&wcex);
        if (guid) {
            delete [] guid;
            guid = nullptr;
        }
        if (g_mainWindowAtom == INVALID_ATOM) {
            auto msg = Utils::GetSystemErrorMessage(L"RegisterClassExW");
            if (msg) {
                Utils::DisplayErrorDialog(msg);
                delete [] msg;
                msg = nullptr;
            } else {
                OutputDebugStringW(L"Failed to register the window class for the main window.");
            }
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"LoadCursorW(), LoadIconW(), RegisterClassExW() and GetClassInfoExW() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool CreateMainWindow(LPCWSTR title) noexcept
{
    USER32_API(CreateWindowExW);
    if (CreateWindowExWFunc) {
        if (g_mainWindowAtom == INVALID_ATOM) {
            OutputDebugStringW(L"Failed to create main window due to the main window ATOM is invalid.");
            return false;
        }
        g_mainWindowHandle = CreateWindowExWFunc(
            WS_EX_NOREDIRECTIONBITMAP,
            Utils::GetWindowClassName(g_mainWindowAtom),
            (title ? title : g_defaultWindowTitle),
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            nullptr, nullptr, Utils::GetCurrentInstance(), nullptr);
        if (!g_mainWindowHandle) {
            auto msg = Utils::GetSystemErrorMessage(L"CreateWindowExW");
            if (msg) {
                Utils::DisplayErrorDialog(msg);
                delete [] msg;
                msg = nullptr;
            } else {
                OutputDebugStringW(L"Failed to create the main window.");
            }
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"CreateWindowExW() is not available.");
        return false;
    }
}

[[nodiscard]] static inline bool ShowMainWindow(const int nCmdShow) noexcept
{
    USER32_API(ShowWindow);
    USER32_API(UpdateWindow);
    if (ShowWindowFunc && UpdateWindowFunc) {
        if (!g_mainWindowHandle) {
            OutputDebugStringW(L"Failed to show the main window due to the main window handle is null.");
            return false;
        }
        ShowWindowFunc(g_mainWindowHandle, nCmdShow);
        if (UpdateWindowFunc(g_mainWindowHandle) == FALSE) {
            auto msg = Utils::GetSystemErrorMessage(L"UpdateWindow");
            if (msg) {
                Utils::DisplayErrorDialog(msg);
                delete [] msg;
                msg = nullptr;
            } else {
                OutputDebugStringW(L"Failed to update the main window.");
            }
            return false;
        }
        return true;
    } else {
        OutputDebugStringW(L"ShowWindow() and UpdateWindow() are not available.");
        return false;
    }
}

[[nodiscard]] static inline bool InitializeXAMLIsland() noexcept
{
    // The call to winrt::init_apartment() initializes COM. By default, in a multithreaded apartment.
    winrt::init_apartment(winrt::apartment_type::single_threaded);
    // Initialize the XAML framework's core window for the current thread.
    g_manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    if (g_manager == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to initialize the Windows XAML Manager for the current thread.");
        return false;
    }
    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    g_source = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
    if (g_source == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the Desktop Window XAML Source.");
        return false;
    }
    // Get handle to the core window.
    const auto interop = g_source.as<IDesktopWindowXamlSourceNative>();
    if (!interop) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the IDesktopWindowXamlSourceNative.");
        return false;
    }
    // Parent the DesktopWindowXamlSource object to the current window.
    HRESULT hr = interop->AttachToWindow(g_mainWindowHandle);
    if (FAILED(hr)) {
        auto msg = Utils::GetSystemErrorMessage(L"AttachToWindow", hr);
        if (msg) {
            Utils::DisplayErrorDialog(msg);
            delete [] msg;
            msg = nullptr;
        } else {
            OutputDebugStringW(L"Failed to attach the XAML Island window to the main window.");
        }
        return false;
    }
    // Get the new child window's HWND.
    hr = interop->get_WindowHandle(&g_xamlIslandWindowHandle);
    if (FAILED(hr)) {
        auto msg = Utils::GetSystemErrorMessage(L"get_WindowHandle", hr);
        if (msg) {
            Utils::DisplayErrorDialog(msg);
            delete [] msg;
            msg = nullptr;
        } else {
            OutputDebugStringW(L"Failed to retrieve the XAML Island window's handle.");
        }
        return false;
    }
    if (!g_xamlIslandWindowHandle) {
        Utils::DisplayErrorDialog(L"Failed to retrieve the window handle of XAML Island's core window.");
        return false;
    }
    // Update the XAML Island window size because initially it is 0x0.
    if (!SyncXAMLIslandPosition()) {
        Utils::DisplayErrorDialog(L"Failed to sync the geometry of the XAML Island window.");
        return false;
    }
    // Create the XAML content.
    g_rootGrid = winrt::Windows::UI::Xaml::Controls::Grid();
    if (g_rootGrid == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the XAML Grid.");
        return false;
    }
    g_backgroundBrush = winrt::Windows::UI::Xaml::Media::AcrylicBrush();
    if (g_backgroundBrush == nullptr) {
        Utils::DisplayErrorDialog(L"Failed to create the XAML AcrylicBrush");
        return false;
    }
    if (!RefreshBackgroundBrush()) {
        Utils::DisplayErrorDialog(L"Failed to refresh the background brush.");
        return false;
    }
    g_backgroundBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    g_rootGrid.Background(g_backgroundBrush);
    //g_rootGrid.Children().Clear();
    //g_rootGrid.Children().Append(/* some UWP control */);
    //g_rootGrid.UpdateLayout();
    g_source.Content(g_rootGrid);
    return true;
}

[[nodiscard]] static inline int MessageLoop() noexcept
{
    USER32_API(GetMessageW);
    USER32_API(TranslateMessage);
    USER32_API(DispatchMessageW);
    if (GetMessageWFunc && TranslateMessageFunc && DispatchMessageWFunc) {
        MSG msg = {};
        while (GetMessageWFunc(&msg, nullptr, 0, 0) != FALSE) {
            BOOL filtered = FALSE;
            if (g_source != nullptr) {
                const auto interop2 = g_source.as<IDesktopWindowXamlSourceNative2>();
                if (interop2) {
                    const HRESULT hr = interop2->PreTranslateMessage(&msg, &filtered);
                    if (FAILED(hr)) {
                        auto errMsg = Utils::GetSystemErrorMessage(L"PreTranslateMessage", hr);
                        if (errMsg) {
                            Utils::DisplayErrorDialog(errMsg);
                            delete [] errMsg;
                            errMsg = nullptr;
                        } else {
                            OutputDebugStringW(L"Failed to pre-translate win32 messages.");
                        }
                    }
                } else {
                    OutputDebugStringW(L"Failed to retrieve the IDesktopWindowXamlSourceNative2.");
                }
            }
            if (filtered == FALSE) {
                TranslateMessageFunc(&msg);
                DispatchMessageWFunc(&msg);
            }
        }
        return static_cast<int>(msg.wParam);
    } else {
        OutputDebugStringW(L"GetMessageW(), TranslateMessage() and DispatchMessageW() are not available.");
        return -1;
    }
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

    if (!IsWindows1019H1OrGreater()) {
        Utils::DisplayErrorDialog(L"This application only supports Windows 10 19H1 and onwards.");
        return -1;
    }

    if (!RegisterMainWindowClass(nullptr)) {
        Utils::DisplayErrorDialog(L"Failed to register the main window class.");
        return -1;
    }

    if (!CreateMainWindow(nullptr)) {
        Utils::DisplayErrorDialog(L"Failed to create the main window.");
        return -1;
    }

    if (!Utils::RefreshWindowTheme(g_mainWindowHandle)) {
        Utils::DisplayErrorDialog(L"Failed to refresh the main window theme.");
        return -1;
    }

    if (!InitializeXAMLIsland()) {
        Utils::DisplayErrorDialog(L"Failed to initialize the XAML Island.");
        return -1;
    }

    if (!ShowMainWindow(nCmdShow)) {
        Utils::DisplayErrorDialog(L"Failed to show the main window.");
        return -1;
    }

    return MessageLoop();
}
