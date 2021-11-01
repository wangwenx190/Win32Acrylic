# Using the Official Acrylic Brush in Win32 Applications

This demo is based on the XAML Island technology and thus it can only be run on Windows 10 19H1 (Win10 1903, 10.0.18362) and onwards. This is the limitation of XAML Island itself, I really can't do anything about it. And you don't need to download any runtime libraries seperately because this demo only uses very limited features of WinRT so it can use the runtime which is shipped by Windows itself. Due to this demo uses the official acrylic brush from UWP, their appearance and behavior should be exactly the same. This demo also has official support for Windows 11.

## Screenshots

[!Light](/Screenshots/Light.png)

[!Dark](/Screenshots/Dark.png)

## Build

### Prerequisites

1. Visual Studio 2017 or later
2. C++ workload for desktop
3. Windows 10 SDK, as new as possible
4. CMake, as new as possible
5. Any make tool. Ninja is recommended. MSBuild is also fine.

Don't need any NuGet packages.

Don't need the XAML toolchain.

Don't need the UWP workload.

### Compilation

```bat
cmake -DCMAKE_BUILD_TYPE=Release -GNinja .
cmake --build . --config Release --target all --parallel
```

## License

```text
MIT License

Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

```
