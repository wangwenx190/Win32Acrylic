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

#ifndef _USER32_
#define _USER32_
#endif

#ifndef _UXTHEME_
#define _UXTHEME_
#endif

#ifndef _DWMAPI_
#define _DWMAPI_
#endif

#ifndef _OLE32_
#define _OLE32_
#endif

#ifndef _COMBASEAPI_
#define _COMBASEAPI_
#endif

#ifndef _ROAPI_
#define _ROAPI_
#endif

#include <Windows.h>
#include <UxTheme.h>
#include <DwmApi.h>
#include <RoApi.h>
#include <RoErrorApi.h>
#include <RoRegistrationApi.h>
#include <RoParameterizedIid.h>
#include <RoMetadataResolution.h>

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART1
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART1(funcName) \
using sig = decltype(&::funcName); \
static bool tried = false; \
static sig func = nullptr; \
if (!func) { \
    if (tried) {
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART2
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART2(libName) \
    } else { \
        tried = true; \
        const HMODULE dll = LoadLibraryExW(L#libName ".dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); \
        if (!dll) { \
            OutputDebugStringW(L"Failed to load " #libName ".dll.");
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART3
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART3(funcName) \
        } \
        func = reinterpret_cast<sig>(GetProcAddress(dll, #funcName)); \
        FreeLibrary(dll); \
        if (!func) { \
            OutputDebugStringW(L"Failed to resolve " #funcName "().");
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART4
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART4 \
        } \
    } \
}
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VOID
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VOID return;
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VALUE
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VALUE(value) return (value);
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_CALL_FUNC
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_CALL_FUNC(...) func(__VA_ARGS__);
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN(...) return func(__VA_ARGS__);
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_VOID_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_VOID_FUNCTION(funcName, libName, ...) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART1(funcName) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VOID \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART2(libName) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VOID \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART3(funcName) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VOID \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART4 \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_CALL_FUNC(__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, libName, defVal, ...) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART1(funcName) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART2(libName) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART3(funcName) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_PART4 \
RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN(__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_USER_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_USER_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, User32, FALSE, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_USER_INT_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_USER_INT_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, User32, 0, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_THEME_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_THEME_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, UxTheme, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_THEME_PTR_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_THEME_PTR_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, UxTheme, nullptr, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_DWM_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_DWM_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, DwmApi, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, ComBase, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_VOID_FUNCTION(funcName, ComBase, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_WINRT_INT_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_WINRT_INT_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, ComBase, 0, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, ComBase, FALSE, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_WINRT_PTR_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_WINRT_PTR_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, ComBase, nullptr, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_OLE_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_OLE_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_RETURN_FUNCTION(funcName, Ole32, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
EXTERN_C_START
#endif

/////////////////////////////////
/////     User32
/////////////////////////////////

int WINAPI
GetSystemMetricsForDpi(
    int  nIndex,
    UINT dpi
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_USER_INT_FUNCTION(GetSystemMetricsForDpi, nIndex, dpi)
}

BOOL WINAPI
AdjustWindowRectExForDpi(
    LPRECT lpRect,
    DWORD  dwStyle,
    BOOL   bMenu,
    DWORD  dwExStyle,
    UINT   dpi
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_USER_FUNCTION(AdjustWindowRectExForDpi, lpRect, dwStyle, bMenu, dwExStyle, dpi)
}

/////////////////////////////////
/////     UxTheme
/////////////////////////////////

HPAINTBUFFER WINAPI
BeginBufferedPaint(
    HDC             hdcTarget,
    const RECT      *prcTarget,
    BP_BUFFERFORMAT dwFormat,
    BP_PAINTPARAMS  *pPaintParams,
    HDC             *phdc
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_THEME_PTR_FUNCTION(BeginBufferedPaint, hdcTarget, prcTarget, dwFormat, pPaintParams, phdc)
}

HRESULT WINAPI
EndBufferedPaint(
    HPAINTBUFFER hBufferedPaint,
    BOOL         fUpdateTarget
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_THEME_FUNCTION(EndBufferedPaint, hBufferedPaint, fUpdateTarget)
}

HRESULT WINAPI
BufferedPaintSetAlpha(
    HPAINTBUFFER hBufferedPaint,
    const RECT   *prc,
    BYTE         alpha
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_THEME_FUNCTION(BufferedPaintSetAlpha, hBufferedPaint, prc, alpha)
}

/////////////////////////////////
/////     DwmApi
/////////////////////////////////

HRESULT WINAPI
DwmExtendFrameIntoClientArea(
    HWND          hWnd,
    const MARGINS *pMarInset
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_DWM_FUNCTION(DwmExtendFrameIntoClientArea, hWnd, pMarInset)
}

/////////////////////////////////
/////     Windows Runtime
/////////////////////////////////

HRESULT WINAPI
RoActivateInstance(
    HSTRING      activatableClassId,
    IInspectable **instance
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoActivateInstance, activatableClassId, instance)
}

HRESULT WINAPI
RoGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void    **factory
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetActivationFactory, activatableClassId, iid, factory)
}

HRESULT WINAPI
RoGetApartmentIdentifier(
    UINT64 *apartmentIdentifier
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetApartmentIdentifier, apartmentIdentifier)
}

HRESULT WINAPI
RoInitialize(
    RO_INIT_TYPE initType
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoInitialize, initType)
}

HRESULT WINAPI
RoRegisterActivationFactories(
    HSTRING                 *activatableClassIds,
    PFNGETACTIVATIONFACTORY *activationFactoryCallbacks,
    UINT32                  count,
    RO_REGISTRATION_COOKIE  *cookie
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoRegisterActivationFactories, activatableClassIds, activationFactoryCallbacks, count, cookie)
}

HRESULT WINAPI
RoRegisterForApartmentShutdown(
    IApartmentShutdown                     *callbackObject,
    UINT64                                 *apartmentIdentifier,
    APARTMENT_SHUTDOWN_REGISTRATION_COOKIE *regCookie
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoRegisterForApartmentShutdown, callbackObject, apartmentIdentifier, regCookie)
}

void WINAPI
RoRevokeActivationFactories(
    RO_REGISTRATION_COOKIE cookie
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoRevokeActivationFactories, cookie)
}

void WINAPI
RoUninitialize()
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoUninitialize)
}

HRESULT WINAPI
RoUnregisterForApartmentShutdown(
    APARTMENT_SHUTDOWN_REGISTRATION_COOKIE regCookie
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoUnregisterForApartmentShutdown, regCookie)
}

HRESULT WINAPI
GetRestrictedErrorInfo(
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(GetRestrictedErrorInfo, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoCaptureErrorContext(
    HRESULT hr
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoCaptureErrorContext, hr)
}

void WINAPI
RoFailFastWithErrorContext(
    HRESULT hrError
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoFailFastWithErrorContext, hrError)
}

HRESULT WINAPI
RoGetErrorReportingFlags(
    UINT32 *pFlags
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetErrorReportingFlags, pFlags)
}

BOOL WINAPI
RoOriginateError(
    HRESULT error,
    HSTRING message
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateError, error, message)
}

BOOL WINAPI
RoOriginateErrorW(
    HRESULT error,
    UINT    cchMax,
    PCWSTR  message
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateErrorW, error, cchMax, message)
}

HRESULT WINAPI
RoResolveRestrictedErrorInfoReference(
    PCWSTR               reference,
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoResolveRestrictedErrorInfoReference, reference, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoSetErrorReportingFlags(
    UINT32 flags
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoSetErrorReportingFlags, flags)
}

BOOL WINAPI
RoTransformError(
    HRESULT oldError,
    HRESULT newError,
    HSTRING message
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoTransformError, oldError, newError, message)
}

BOOL WINAPI
RoTransformErrorW(
    HRESULT oldError,
    HRESULT newError,
    UINT    cchMax,
    PCWSTR  message
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoTransformErrorW, oldError, newError, cchMax, message)
}

HRESULT WINAPI
SetRestrictedErrorInfo(
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(SetRestrictedErrorInfo, pRestrictedErrorInfo)
}

BOOL WINAPI
IsErrorPropagationEnabled()
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(IsErrorPropagationEnabled)
}

void WINAPI
RoClearError()
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoClearError)
}

HRESULT WINAPI
RoGetMatchingRestrictedErrorInfo(
    HRESULT              hrIn,
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetMatchingRestrictedErrorInfo, hrIn, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoInspectCapturedStackBackTrace(
    UINT_PTR                 targetErrorInfoAddress,
    USHORT                   machine,
    PINSPECT_MEMORY_CALLBACK readMemoryCallback,
    PVOID                    context,
    UINT32                   *frameCount,
    UINT_PTR                 *targetBackTraceAddress
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoInspectCapturedStackBackTrace, targetErrorInfoAddress, machine, readMemoryCallback, context, frameCount, targetBackTraceAddress)
}

HRESULT WINAPI
RoInspectThreadErrorInfo(
    UINT_PTR                 targetTebAddress,
    USHORT                   machine,
    PINSPECT_MEMORY_CALLBACK readMemoryCallback,
    PVOID                    context,
    UINT_PTR                 *targetErrorInfoAddress
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoInspectThreadErrorInfo, targetTebAddress, machine, readMemoryCallback, context, targetErrorInfoAddress)
}

BOOL WINAPI
RoOriginateLanguageException(
    HRESULT  error,
    HSTRING  message,
    IUnknown *languageException
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateLanguageException, error, message, languageException)
}

HRESULT WINAPI
RoReportFailedDelegate(
    IUnknown             *punkDelegate,
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoReportFailedDelegate, punkDelegate, pRestrictedErrorInfo)
}

HRESULT WINAPI
RoReportUnhandledError(
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoReportUnhandledError, pRestrictedErrorInfo)
}

HRESULT WINAPI
RoGetActivatableClassRegistration(
    HSTRING                       activatableClassId,
    PActivatableClassRegistration *activatableClassRegistration
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetActivatableClassRegistration, activatableClassId, activatableClassRegistration)
}

HRESULT WINAPI
RoGetServerActivatableClasses(
    HSTRING serverName,
    HSTRING **activatableClassIds,
    DWORD   *count
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetServerActivatableClasses, serverName, activatableClassIds, count)
}

void WINAPI
HSTRING_UserFree(
    ULONG   *pFlags,
    HSTRING *ppidl
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(HSTRING_UserFree, pFlags, ppidl)
}

void WINAPI
HSTRING_UserFree64(
    ULONG   *unnamedParam1,
    HSTRING *unnamedParam2
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(HSTRING_UserFree64, unnamedParam1, unnamedParam2)
}

UCHAR * WINAPI
HSTRING_UserMarshal(
    ULONG   *pFlags,
    UCHAR   *pBuffer,
    HSTRING *ppidl
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserMarshal, pFlags, pBuffer, ppidl)
}

UCHAR * WINAPI
HSTRING_UserMarshal64(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserMarshal64, unnamedParam1, unnamedParam2, unnamedParam3)
}

ULONG WINAPI
HSTRING_UserSize(
    ULONG   *unnamedParam1,
    ULONG   unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_INT_FUNCTION(HSTRING_UserSize, unnamedParam1, unnamedParam2, unnamedParam3)
}

ULONG WINAPI
HSTRING_UserSize64(
    ULONG   *unnamedParam1,
    ULONG   unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_INT_FUNCTION(HSTRING_UserSize64, unnamedParam1, unnamedParam2, unnamedParam3)
}

UCHAR * WINAPI
HSTRING_UserUnmarshal(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserUnmarshal, unnamedParam1, unnamedParam2, unnamedParam3)
}

UCHAR * WINAPI
HSTRING_UserUnmarshal64(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserUnmarshal64, unnamedParam1, unnamedParam2, unnamedParam3)
}

HRESULT WINAPI
WindowsCompareStringOrdinal(
    HSTRING string1,
    HSTRING string2,
    INT32   *result
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsCompareStringOrdinal, string1, string2, result)
}

HRESULT WINAPI
WindowsConcatString(
    HSTRING string1,
    HSTRING string2,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsConcatString, string1, string2, newString)
}

HRESULT WINAPI
WindowsCreateString(
    PCNZWCH sourceString,
    UINT32  length,
    HSTRING *string
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsCreateString, sourceString, length, string)
}

HRESULT WINAPI
WindowsCreateStringReference(
    PCWSTR         sourceString,
    UINT32         length,
    HSTRING_HEADER *hstringHeader,
    HSTRING        *string
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsCreateStringReference, sourceString, length, hstringHeader, string)
}

HRESULT WINAPI
WindowsDeleteString(
    HSTRING string
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsDeleteString, string)
}

HRESULT WINAPI
WindowsDeleteStringBuffer(
    HSTRING_BUFFER bufferHandle
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsDeleteStringBuffer, bufferHandle)
}

HRESULT WINAPI
WindowsDuplicateString(
    HSTRING string,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsDuplicateString, string, newString)
}

UINT32 WINAPI
WindowsGetStringLen(
    HSTRING string
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_INT_FUNCTION(WindowsGetStringLen, string)
}

PCWSTR WINAPI
WindowsGetStringRawBuffer(
    HSTRING string,
    UINT32  *length
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_PTR_FUNCTION(WindowsGetStringRawBuffer, string, length)
}

HRESULT WINAPI
WindowsInspectString(
    UINT_PTR                  targetHString,
    USHORT                    machine,
    PINSPECT_HSTRING_CALLBACK callback,
    void                      *context,
    UINT32                    *length,
    UINT_PTR                  *targetStringAddress
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsInspectString, targetHString, machine, callback, context, length, targetStringAddress)
}

BOOL WINAPI
WindowsIsStringEmpty(
    HSTRING string
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(WindowsIsStringEmpty, string)
}

HRESULT WINAPI
WindowsPreallocateStringBuffer(
    UINT32         length,
    WCHAR          **charBuffer,
    HSTRING_BUFFER *bufferHandle
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsPreallocateStringBuffer, length, charBuffer, bufferHandle)
}

HRESULT WINAPI
WindowsPromoteStringBuffer(
    HSTRING_BUFFER bufferHandle,
    HSTRING        *string
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsPromoteStringBuffer, bufferHandle, string)
}

HRESULT WINAPI
WindowsReplaceString(
    HSTRING string,
    HSTRING stringReplaced,
    HSTRING stringReplaceWith,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsReplaceString, string, stringReplaced, stringReplaceWith, newString)
}

HRESULT WINAPI
WindowsStringHasEmbeddedNull(
    HSTRING string,
    BOOL    *hasEmbedNull
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsStringHasEmbeddedNull, string, hasEmbedNull)
}

HRESULT WINAPI
WindowsSubstring(
    HSTRING string,
    UINT32  startIndex,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsSubstring, string, startIndex, newString)
}

HRESULT WINAPI
WindowsSubstringWithSpecifiedLength(
    HSTRING string,
    UINT32  startIndex,
    UINT32  length,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsSubstringWithSpecifiedLength, string, startIndex, length, newString)
}

HRESULT WINAPI
WindowsTrimStringEnd(
    HSTRING string,
    HSTRING trimString,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsTrimStringEnd, string, trimString, newString)
}

HRESULT WINAPI
WindowsTrimStringStart(
    HSTRING string,
    HSTRING trimString,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(WindowsTrimStringStart, string, trimString, newString)
}

HRESULT WINAPI
RoGetBufferMarshaler(
    IMarshal **bufferMarshaler
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetBufferMarshaler, bufferMarshaler)
}

void WINAPI
RoFreeParameterizedTypeExtra(
    ROPARAMIIDHANDLE extra
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoFreeParameterizedTypeExtra, extra)
}

HRESULT WINAPI
RoGetParameterizedTypeInstanceIID(
    UINT32                     nameElementCount,
    PCWSTR                     *nameElements,
    const IRoMetaDataLocator   &metaDataLocator,
    GUID                       *iid,
    ROPARAMIIDHANDLE           *pExtra
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetParameterizedTypeInstanceIID, nameElementCount, nameElements, metaDataLocator, iid, pExtra)
}

PCSTR WINAPI
RoParameterizedTypeExtraGetTypeSignature(
    ROPARAMIIDHANDLE extra
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_PTR_FUNCTION(RoParameterizedTypeExtraGetTypeSignature, extra)
}

HRESULT WINAPI
RoGetMetaDataFile(
    const HSTRING        name,
    IMetaDataDispenserEx *metaDataDispenser,
    HSTRING              *metaDataFilePath,
    IMetaDataImport2     **metaDataImport,
    mdTypeDef            *typeDefToken
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoGetMetaDataFile, name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken)
}

HRESULT WINAPI
RoParseTypeName(
    HSTRING typeName,
    DWORD   *partsCount,
    HSTRING **typeNameParts
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoParseTypeName, typeName, partsCount, typeNameParts)
}

HRESULT WINAPI
RoResolveNamespace(
    const HSTRING name,
    const HSTRING windowsMetaDataDir,
    const DWORD   packageGraphDirsCount,
    const HSTRING *packageGraphDirs,
    DWORD         *metaDataFilePathsCount,
    HSTRING       **metaDataFilePaths,
    DWORD         *subNamespacesCount,
    HSTRING       **subNamespaces
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(RoResolveNamespace, name, windowsMetaDataDir, packageGraphDirsCount, packageGraphDirs, metaDataFilePathsCount, metaDataFilePaths, subNamespacesCount, subNamespaces)
}

/////////////////////////////////
/////     Ole32 / ComBaseApi
/////////////////////////////////

HRESULT WINAPI
CoIncrementMTAUsage(
    CO_MTA_USAGE_COOKIE *pCookie
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_OLE_FUNCTION(CoIncrementMTAUsage, pCookie)
}

#ifdef __cplusplus
EXTERN_C_END
#endif
