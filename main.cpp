#include <Unknwn.h> // This header file must be placed before any other header files.
#include <Windows.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

static CONST WCHAR szWindowClassName[] = L"Win32DesktopApp";
static CONST WCHAR szWindowTitle[] = L"Windows C++ Win32 Desktop App";

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static HWND g_mainWindowHandle = nullptr;
static HWND g_xamlIslandHandle = nullptr;
static HINSTANCE g_instance = nullptr;

INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    WNDCLASSEXW wndcls;
    SecureZeroMemory(&wndcls, sizeof(wndcls));
    wndcls.cbSize = sizeof(wndcls);
    wndcls.style = CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = WndProc;
    wndcls.hInstance = hInstance;
    wndcls.lpszClassName = szWindowClassName;
    wndcls.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wndcls.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassExW(&wndcls);

    CONST HWND mainWindowHwnd = CreateWindow(
        szWindowClassName,
        szWindowTitle,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // "WS_VISIBLE" is needed if we don't call ShowWindow().
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    if (!mainWindowHwnd) {
        return FALSE;
    }

    g_instance = hInstance;
    g_mainWindowHandle = mainWindowHwnd;

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
    winrt::check_hresult(interop->AttachToWindow(mainWindowHwnd));
    // Get the new child window's HWND.
    HWND xamlIslandHwnd = nullptr;
    interop->get_WindowHandle(&xamlIslandHwnd);
    if (!xamlIslandHwnd) {
        return FALSE;
    }
    g_xamlIslandHandle = xamlIslandHwnd;
    // Update the XAML Island window size because initially it is 0x0.
    RECT rect = {0, 0, 0, 0};
    GetClientRect(mainWindowHwnd, &rect);
    SetWindowPos(xamlIslandHwnd, nullptr, 0, 0, rect.right, rect.bottom, SWP_SHOWWINDOW);
    // Create the XAML content.
    winrt::Windows::UI::Xaml::Controls::Grid xamlContainer = {};
    winrt::Windows::UI::Xaml::Media::AcrylicBrush acrylicBrush = {};
    acrylicBrush.BackgroundSource(winrt::Windows::UI::Xaml::Media::AcrylicBackgroundSource::HostBackdrop);
    xamlContainer.Background(acrylicBrush);
    //xamlContainer.Children().Append(/* some UWP control */);
    //xamlContainer.UpdateLayout();
    desktopWindowXamlSource.Content(xamlContainer);
    // End XAML Island section.

    ShowWindow(xamlIslandHwnd, nCmdShow);
    UpdateWindow(xamlIslandHwnd);

    // Message loop:
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT messageCode, WPARAM wParam, LPARAM lParam)
{
    switch (messageCode)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE: {
        RECT rect = {0, 0, 0, 0};
        GetClientRect(g_mainWindowHandle, &rect);
        MoveWindow(g_xamlIslandHandle, 0, 0, rect.right, rect.bottom, TRUE);
    } break;
    default:
        break;
    }
    return DefWindowProcW(hWnd, messageCode, wParam, lParam);
}
