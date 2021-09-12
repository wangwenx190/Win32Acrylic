# Win32Acrylic WinUI2 Demo

This demo uses the XAML Island technology from WinUI2, and thus it can only be run on Windows 10 19H1 (Win10 1903, 10.0.18362) and onwards. This is the limitation of XAML Island itself. However, you don't need to download the WinUI2 runtime dlls seperately because this demo only uses very limited features of WinRT so it can use the runtime which is shipped by Windows itself. This demo uses the official acrylic brush from UWP so their appearance and behavior are exactly the same.

## Screenshots

TODO

## Build

### Prerequisites

1. Visual Studio 2017 or later
2. C++ workload for desktop
3. Windows 10 SDK, as new as possible

No NuGet packages are needed.

### Compilation

```bat
md build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -GNinja ..
cmake --build . --config release --target all --parallel
cmake --install .
```
