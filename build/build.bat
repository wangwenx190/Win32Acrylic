:: MIT License
::
:: Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
::
:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files (the "Software"), to deal
:: in the Software without restriction, including without limitation the rights
:: to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
:: copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions:
::
:: The above copyright notice and this permission notice shall be included in all
:: copies or substantial portions of the Software.
::
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
:: SOFTWARE.

@echo off
cls
setlocal enabledelayedexpansion
color
title Building AcrylicManager ...
set PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer;%PATH%
where /q vswhere
if %errorlevel% neq 0 (
    color 4f
    echo Can't locate vswhere's location.
    goto Finish
)
set _vs_dir=
for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "_vs_dir=%%i"
)
if not exist "%_vs_dir%" (
    color 4f
    echo Can't locate Visual Studio's install location.
    goto Finish
)
set _target_arch=x64
if exist "%~dp0BUILD_X86" set _target_arch=x86
if /i "%_target_arch%" == "x86" (
    call "%_vs_dir%\VC\Auxiliary\Build\vcvars32.bat"
) else (
    call "%_vs_dir%\VC\Auxiliary\Build\vcvars64.bat"
)
set PATH=%ProgramFiles%\CMake\bin;%PATH%
where /q cmake
if %errorlevel% neq 0 (
    color 4f
    echo Can't locate CMake's install location.
    goto Finish
)
cmake --version
if defined NINJA_BUILD_DIR set PATH=%NINJA_BUILD_DIR%;%PATH%
set _ninja_exists=false
where /q ninja
if %errorlevel% equ 0 (
    set _ninja_exists=true
    echo Ninja build version:
    ninja --version
) else (
    set _ninja_exists=false
    echo Can't locate Ninja build's location, using the default generator instead ...
)
cd /d "%~dp0"
set _build_dir=%~dp0_cache_
if exist "%_build_dir%" rd /s /q "%_build_dir%"
md "%_build_dir%"
cd "%_build_dir%"
:: -Wno-dev: Suppress annoying developer warnings from CMake.
set _cmake_config_params=-DACRYLICMANAGER_TARGET_ARCHITECTURE=%_target_arch% -DCMAKE_TOOLCHAIN_FILE="%~dp0toolchain-msvc.cmake" -DBUILD_DEMO=ON -Wno-dev
if /i "%_ninja_exists%" == "true" (
    set _cmake_config_params=%_cmake_config_params% -GNinja
)
set _binary_type=shared
if exist "%~dp0BUILD_STATIC" set _binary_type=static
if /i "%_binary_type%" == "static" (
    set _cmake_config_params=%_cmake_config_params% -DBUILD_SHARED_LIBS=OFF
) else (
    set _cmake_config_params=%_cmake_config_params% -DBUILD_SHARED_LIBS=ON
)
set _build_type=release
if exist "%~dp0BUILD_DEBUG" set _build_type=debug
if /i "%_build_type%" == "debug" (
    set _cmake_config_params=%_cmake_config_params% -DCMAKE_BUILD_TYPE=Debug
) else (
    set _cmake_config_params=%_cmake_config_params% -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
)
cmake %_cmake_config_params% "%~dp0.."
set _cmake_build_params=--build . --target all --parallel
if /i "%_build_type%" == "debug" (
    set _cmake_build_params=%_cmake_build_params% --config debug
) else (
    set _cmake_build_params=%_cmake_build_params% --config release
)
cmake %_cmake_build_params%
::cmake --install .
set _bin_dir=%_build_dir%\bin
del /f /s /q "%_bin_dir%\*.lib"
::del /f /s /q "%_bin_dir%\*.pdb"
del /f /s /q "%_bin_dir%\*.obj"
del /f /s /q "%_bin_dir%\*.ilk"
del /f /s /q "%_bin_dir%\*.manifest"
copy /y "%~dp0README.md" "%_bin_dir%"
set _license_dir=%_bin_dir%\licenses
md "%_license_dir%"
copy /y "%~dp0LICENSE" "%_license_dir%\AcrylicManager.txt"
copy /y "%~dp0licenses\*" "%_license_dir%"
set _new_bin_name=AcrylicManager
ren bin %_new_bin_name%
set PATH=%ProgramFiles%\7-Zip;%PATH%
where /q 7z
if %errorlevel% equ 0 (
    7z a %_new_bin_name%.7z %_new_bin_name%\ -mx -myx -ms=on -mmt=on -m0=LZMA2:d=1g:fb=273
    rd /s /q %_new_bin_name%
    move /y %_new_bin_name%.7z "%~dp0"
)
if exist %_new_bin_name% move /y %_new_bin_name% "%~dp0"
color 2f
echo *****************************************
echo *               FINISHED                *
echo *****************************************
goto Finish

:Finish
cd /d "%~dp0"
if exist "%_build_dir%" rd /s /q "%_build_dir%"
endlocal
pause
exit /b
