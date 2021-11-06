# Acrylic Demo Applications

Acrylic demo applications based on different technology stack.

## Screenshots

### UWP

![Light Theme](/Screenshots/Light/UWP.png "Light Theme")

![Dark Theme](/Screenshots/Dark/UWP.png "Dark Theme")

### Windows.UI.Composition

![Light Theme](/Screenshots/Light/Windows.UI.Composition.png "Light Theme")

![Dark Theme](/Screenshots/Dark/Windows.UI.Composition.png "Dark Theme")

### Direct Composition

![Light Theme](/Screenshots/Light/DirectComposition.png "Light Theme")

![Dark Theme](/Screenshots/Dark/DirectComposition.png "Dark Theme")

### Direct2D

![Light Theme](/Screenshots/Light/Direct2D.png "Light Theme")

![Dark Theme](/Screenshots/Dark/Direct2D.png "Dark Theme")

## Build

### Prerequisites

1. Visual Studio 2017 or later
2. C++ workload for desktop
3. Windows 10/11 SDK, as new as possible
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
