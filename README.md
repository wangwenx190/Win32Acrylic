# Use the official Acrylic brush in Win32 C++ applications

This simple demo application shows how to use the official acrylic brush in native Win32 C++ applications. It uses **XAML Islands** to embed native UWP controls into pure C++ applications.

## Screenshots

![Screenshot](/screenshot.png)

## Build

### Prerequisites

- The latest version of Visual Studio 2019
- The C++ toolset, including the latest version of Windows SDK
- The latest version of C++/WinRT nuget package

### Compilation

```bash
cmake .
cmake --build .
cmake --install .
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
