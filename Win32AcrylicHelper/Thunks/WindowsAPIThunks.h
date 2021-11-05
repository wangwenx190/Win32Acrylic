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

#pragma once

#include <SDKDDKVer.h>
#include <Windows.h>

#ifndef DEFAULT_INT
#define DEFAULT_INT (-1)
#endif // DEFAULT_INT

#ifndef DEFAULT_UINT
#define DEFAULT_UINT (0)
#endif // DEFAULT_UINT

#ifndef DEFAULT_PTR
#define DEFAULT_PTR (nullptr)
#endif // DEFAULT_PTR

#ifndef DEFAULT_BOOL
#define DEFAULT_BOOL (FALSE)
#endif // DEFAULT_BOOL

#ifndef DEFAULT_VOID
#define DEFAULT_VOID (static_cast<VOID>(0))
#endif // DEFAULT_VOID

#ifndef DEFAULT_HRESULT
#define DEFAULT_HRESULT (E_NOTIMPL)
#endif // DEFAULT_HRESULT

#ifndef _LCRT_DEFINE_IAT_SYMBOL_MAKE_NAME
#ifdef _M_IX86
#define _LCRT_DEFINE_IAT_SYMBOL_MAKE_NAME(function, size) _CRT_CONCATENATE( _CRT_CONCATENATE( _imp__ , function ), _CRT_CONCATENATE( _ , size ) )
#elif defined(_M_AMD64)
#define _LCRT_DEFINE_IAT_SYMBOL_MAKE_NAME(function, size) _CRT_CONCATENATE( __imp_ , function )
#else // !_M_IX86 && !_M_AMD64
#error "The current architecture is not supported!"
#endif // _M_IX86
#endif // _LCRT_DEFINE_IAT_SYMBOL_MAKE_NAME

#ifndef _LCRT_DEFINE_IAT_SYMBOL
#ifdef _M_IX86
// There is an "@" in x86 symbol names, we use the "identifier" feature to fix it.
#define _LCRT_DEFINE_IAT_SYMBOL(function, size) \
__pragma(warning(suppress:4483)) \
EXTERN_C __declspec(selectany) void const * const __identifier( _CRT_STRINGIZE_( _imp__ ## function ## @ ## size ) ) = reinterpret_cast< void const * >( function )
#else // _M_IX86
#define _LCRT_DEFINE_IAT_SYMBOL(function, size) \
EXTERN_C __declspec(selectany) void const * const _LCRT_DEFINE_IAT_SYMBOL_MAKE_NAME( function, size ) = reinterpret_cast< void const * >( function )
#endif // _M_IX86
#endif // _LCRT_DEFINE_IAT_SYMBOL

#ifndef __RESOLVE_API_INTERNAL
#define __RESOLVE_API_INTERNAL(library, symbol) ( reinterpret_cast< decltype( & ::symbol ) >( GetWindowsAPIByName( L#library , L#symbol ) ) )
#endif // __RESOLVE_API_INTERNAL

#ifndef __RESOLVE_API
#define __RESOLVE_API(library, symbol) static const auto symbol ## _API = __RESOLVE_API_INTERNAL( library , symbol )
#endif // __RESOLVE_API

#ifndef __THUNK_API
#define __THUNK_API(library, symbol, result_type, default_result, argument_signature, argument_list) \
EXTERN_C result_type WINAPI \
symbol \
argument_signature \
{ \
    __RESOLVE_API( library , symbol ); \
    return ( ( symbol ## _API ) ? ( symbol ## _API argument_list ) : ( default_result ) ); \
} \
_LCRT_DEFINE_IAT_SYMBOL( symbol , 0 );
#endif // __THUNK_API

EXTERN_C FARPROC WINAPI
GetWindowsAPIByName(
    _In_ LPCWSTR library,
    _In_ LPCWSTR symbol
) noexcept;
