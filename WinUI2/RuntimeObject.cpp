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

#include <Windows.h>

#ifndef WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART1
#define WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART1(funcName) \
    static bool __tried = false; \
    using __sig = decltype(&::funcName); \
    static __sig __func = nullptr; \
    if (!__func) { \
        if (__tried) {
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART2
#define WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART2(libName) \
        } else { \
            __tried = true; \
            const HMODULE __module = LoadLibraryExW(L#libName ".dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); \
            if (!__module) { \
                OutputDebugStringW(L"Failed to load dynamic link library " #libName ".dll.");
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART3
#define WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART3(funcName) \
            } \
            __func = reinterpret_cast<__sig>(GetProcAddress(__module, #funcName)); \
            if (!__func) { \
                OutputDebugStringW(L"Failed to resolve symbol " #funcName "().");
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART4
#define WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART4 \
            } \
        } \
    }
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VOID
#define WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VOID \
            SetLastError(ERROR_SUCCESS); \
            return;
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VALUE
#define WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VALUE(value) \
            SetLastError(ERROR_SUCCESS); \
            return (value);
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_CALL_FUNC
#define WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_CALL_FUNC(...) __func(__VA_ARGS__);
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN
#define WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN(...) return __func(__VA_ARGS__);
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_VOID_FUNCTION
#define WIN32ACRYLIC_TRY_EXECUTE_VOID_FUNCTION(libName, funcName, ...) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART1(funcName) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VOID \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART2(libName) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VOID \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART3(funcName) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VOID \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART4 \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_CALL_FUNC(__VA_ARGS__)
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_RETURN_FUNCTION
#define WIN32ACRYLIC_TRY_EXECUTE_RETURN_FUNCTION(libName, funcName, defVal, ...) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART1(funcName) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART2(libName) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART3(funcName) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_RETURN_VALUE(defVal) \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_PART4 \
WIN32ACRYLIC_TRY_EXECUTE_FUNCTION_CALL_FUNC_RETURN(__VA_ARGS__)
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION_RETURN
#define WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, defVal, ...) WIN32ACRYLIC_TRY_EXECUTE_RETURN_FUNCTION(ComBase, funcName, defVal, ##__VA_ARGS__)
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION
#define WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(funcName, ...) WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION
#define WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION(funcName, ...) WIN32ACRYLIC_TRY_EXECUTE_VOID_FUNCTION(ComBase, funcName, ##__VA_ARGS__)
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_WINRT_INT_FUNCTION
#define WIN32ACRYLIC_TRY_EXECUTE_WINRT_INT_FUNCTION(funcName, ...) WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, 0, ##__VA_ARGS__)
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION
#define WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION(funcName, ...) WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, FALSE, ##__VA_ARGS__)
#endif

#ifndef WIN32ACRYLIC_TRY_EXECUTE_WINRT_PTR_FUNCTION
#define WIN32ACRYLIC_TRY_EXECUTE_WINRT_PTR_FUNCTION(funcName, ...) WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION_RETURN(funcName, nullptr, ##__VA_ARGS__)
#endif

// Define these macros first before including their header files to avoid linking
// to their import libraries.

#ifndef _ROAPI_
#define _ROAPI_
#endif

#include <RoApi.h>
#include <RoErrorApi.h>
#include <RoRegistrationApi.h>
#include <RoParameterizedIid.h>
#include <RoMetadataResolution.h>

#ifdef __cplusplus
EXTERN_C_START
#endif

HRESULT WINAPI
RoActivateInstance(
    HSTRING      activatableClassId,
    IInspectable **instance
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoActivateInstance, activatableClassId, instance)
}

HRESULT WINAPI
RoGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void    **factory
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetActivationFactory, activatableClassId, iid, factory)
}

HRESULT WINAPI
RoGetApartmentIdentifier(
    UINT64 *apartmentIdentifier
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetApartmentIdentifier, apartmentIdentifier)
}

HRESULT WINAPI
RoInitialize(
    RO_INIT_TYPE initType
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoInitialize, initType)
}

HRESULT WINAPI
RoRegisterActivationFactories(
    HSTRING                 *activatableClassIds,
    PFNGETACTIVATIONFACTORY *activationFactoryCallbacks,
    UINT32                  count,
    RO_REGISTRATION_COOKIE  *cookie
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoRegisterActivationFactories, activatableClassIds, activationFactoryCallbacks, count, cookie)
}

HRESULT WINAPI
RoRegisterForApartmentShutdown(
    IApartmentShutdown                     *callbackObject,
    UINT64                                 *apartmentIdentifier,
    APARTMENT_SHUTDOWN_REGISTRATION_COOKIE *regCookie
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoRegisterForApartmentShutdown, callbackObject, apartmentIdentifier, regCookie)
}

void WINAPI
RoRevokeActivationFactories(
    RO_REGISTRATION_COOKIE cookie
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoRevokeActivationFactories, cookie)
}

void WINAPI
RoUninitialize()
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoUninitialize)
}

HRESULT WINAPI
RoUnregisterForApartmentShutdown(
    APARTMENT_SHUTDOWN_REGISTRATION_COOKIE regCookie
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoUnregisterForApartmentShutdown, regCookie)
}

HRESULT WINAPI
GetRestrictedErrorInfo(
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(GetRestrictedErrorInfo, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoCaptureErrorContext(
    HRESULT hr
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoCaptureErrorContext, hr)
}

void WINAPI
RoFailFastWithErrorContext(
    HRESULT hrError
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoFailFastWithErrorContext, hrError)
}

HRESULT WINAPI
RoGetErrorReportingFlags(
    UINT32 *pFlags
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetErrorReportingFlags, pFlags)
}

BOOL WINAPI
RoOriginateError(
    HRESULT error,
    HSTRING message
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateError, error, message)
}

BOOL WINAPI
RoOriginateErrorW(
    HRESULT error,
    UINT    cchMax,
    PCWSTR  message
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateErrorW, error, cchMax, message)
}

HRESULT WINAPI
RoResolveRestrictedErrorInfoReference(
    PCWSTR               reference,
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoResolveRestrictedErrorInfoReference, reference, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoSetErrorReportingFlags(
    UINT32 flags
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoSetErrorReportingFlags, flags)
}

BOOL WINAPI
RoTransformError(
    HRESULT oldError,
    HRESULT newError,
    HSTRING message
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoTransformError, oldError, newError, message)
}

BOOL WINAPI
RoTransformErrorW(
    HRESULT oldError,
    HRESULT newError,
    UINT    cchMax,
    PCWSTR  message
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoTransformErrorW, oldError, newError, cchMax, message)
}

HRESULT WINAPI
SetRestrictedErrorInfo(
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(SetRestrictedErrorInfo, pRestrictedErrorInfo)
}

BOOL WINAPI
IsErrorPropagationEnabled()
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION(IsErrorPropagationEnabled)
}

void WINAPI
RoClearError()
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoClearError)
}

HRESULT WINAPI
RoGetMatchingRestrictedErrorInfo(
    HRESULT              hrIn,
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetMatchingRestrictedErrorInfo, hrIn, ppRestrictedErrorInfo)
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
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoInspectCapturedStackBackTrace, targetErrorInfoAddress, machine, readMemoryCallback, context, frameCount, targetBackTraceAddress)
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
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoInspectThreadErrorInfo, targetTebAddress, machine, readMemoryCallback, context, targetErrorInfoAddress)
}

BOOL WINAPI
RoOriginateLanguageException(
    HRESULT  error,
    HSTRING  message,
    IUnknown *languageException
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateLanguageException, error, message, languageException)
}

HRESULT WINAPI
RoReportFailedDelegate(
    IUnknown             *punkDelegate,
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoReportFailedDelegate, punkDelegate, pRestrictedErrorInfo)
}

HRESULT WINAPI
RoReportUnhandledError(
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoReportUnhandledError, pRestrictedErrorInfo)
}

HRESULT WINAPI
RoGetActivatableClassRegistration(
    HSTRING                       activatableClassId,
    PActivatableClassRegistration *activatableClassRegistration
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetActivatableClassRegistration, activatableClassId, activatableClassRegistration)
}

HRESULT WINAPI
RoGetServerActivatableClasses(
    HSTRING serverName,
    HSTRING **activatableClassIds,
    DWORD   *count
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetServerActivatableClasses, serverName, activatableClassIds, count)
}

void WINAPI
HSTRING_UserFree(
    ULONG   *pFlags,
    HSTRING *ppidl
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION(HSTRING_UserFree, pFlags, ppidl)
}

void WINAPI
HSTRING_UserFree64(
    ULONG   *unnamedParam1,
    HSTRING *unnamedParam2
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION(HSTRING_UserFree64, unnamedParam1, unnamedParam2)
}

UCHAR * WINAPI
HSTRING_UserMarshal(
    ULONG   *pFlags,
    UCHAR   *pBuffer,
    HSTRING *ppidl
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserMarshal, pFlags, pBuffer, ppidl)
}

UCHAR * WINAPI
HSTRING_UserMarshal64(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserMarshal64, unnamedParam1, unnamedParam2, unnamedParam3)
}

ULONG WINAPI
HSTRING_UserSize(
    ULONG   *unnamedParam1,
    ULONG   unnamedParam2,
    HSTRING *unnamedParam3
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_INT_FUNCTION(HSTRING_UserSize, unnamedParam1, unnamedParam2, unnamedParam3)
}

ULONG WINAPI
HSTRING_UserSize64(
    ULONG   *unnamedParam1,
    ULONG   unnamedParam2,
    HSTRING *unnamedParam3
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_INT_FUNCTION(HSTRING_UserSize64, unnamedParam1, unnamedParam2, unnamedParam3)
}

UCHAR * WINAPI
HSTRING_UserUnmarshal(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserUnmarshal, unnamedParam1, unnamedParam2, unnamedParam3)
}

UCHAR * WINAPI
HSTRING_UserUnmarshal64(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserUnmarshal64, unnamedParam1, unnamedParam2, unnamedParam3)
}

HRESULT WINAPI
WindowsCompareStringOrdinal(
    HSTRING string1,
    HSTRING string2,
    INT32   *result
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsCompareStringOrdinal, string1, string2, result)
}

HRESULT WINAPI
WindowsConcatString(
    HSTRING string1,
    HSTRING string2,
    HSTRING *newString
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsConcatString, string1, string2, newString)
}

HRESULT WINAPI
WindowsCreateString(
    PCNZWCH sourceString,
    UINT32  length,
    HSTRING *string
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsCreateString, sourceString, length, string)
}

HRESULT WINAPI
WindowsCreateStringReference(
    PCWSTR         sourceString,
    UINT32         length,
    HSTRING_HEADER *hstringHeader,
    HSTRING        *string
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsCreateStringReference, sourceString, length, hstringHeader, string)
}

HRESULT WINAPI
WindowsDeleteString(
    HSTRING string
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsDeleteString, string)
}

HRESULT WINAPI
WindowsDeleteStringBuffer(
    HSTRING_BUFFER bufferHandle
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsDeleteStringBuffer, bufferHandle)
}

HRESULT WINAPI
WindowsDuplicateString(
    HSTRING string,
    HSTRING *newString
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsDuplicateString, string, newString)
}

UINT32 WINAPI
WindowsGetStringLen(
    HSTRING string
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_INT_FUNCTION(WindowsGetStringLen, string)
}

PCWSTR WINAPI
WindowsGetStringRawBuffer(
    HSTRING string,
    UINT32  *length
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_PTR_FUNCTION(WindowsGetStringRawBuffer, string, length)
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
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsInspectString, targetHString, machine, callback, context, length, targetStringAddress)
}

BOOL WINAPI
WindowsIsStringEmpty(
    HSTRING string
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_BOOL_FUNCTION(WindowsIsStringEmpty, string)
}

HRESULT WINAPI
WindowsPreallocateStringBuffer(
    UINT32         length,
    WCHAR          **charBuffer,
    HSTRING_BUFFER *bufferHandle
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsPreallocateStringBuffer, length, charBuffer, bufferHandle)
}

HRESULT WINAPI
WindowsPromoteStringBuffer(
    HSTRING_BUFFER bufferHandle,
    HSTRING        *string
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsPromoteStringBuffer, bufferHandle, string)
}

HRESULT WINAPI
WindowsReplaceString(
    HSTRING string,
    HSTRING stringReplaced,
    HSTRING stringReplaceWith,
    HSTRING *newString
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsReplaceString, string, stringReplaced, stringReplaceWith, newString)
}

HRESULT WINAPI
WindowsStringHasEmbeddedNull(
    HSTRING string,
    BOOL    *hasEmbedNull
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsStringHasEmbeddedNull, string, hasEmbedNull)
}

HRESULT WINAPI
WindowsSubstring(
    HSTRING string,
    UINT32  startIndex,
    HSTRING *newString
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsSubstring, string, startIndex, newString)
}

HRESULT WINAPI
WindowsSubstringWithSpecifiedLength(
    HSTRING string,
    UINT32  startIndex,
    UINT32  length,
    HSTRING *newString
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsSubstringWithSpecifiedLength, string, startIndex, length, newString)
}

HRESULT WINAPI
WindowsTrimStringEnd(
    HSTRING string,
    HSTRING trimString,
    HSTRING *newString
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsTrimStringEnd, string, trimString, newString)
}

HRESULT WINAPI
WindowsTrimStringStart(
    HSTRING string,
    HSTRING trimString,
    HSTRING *newString
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(WindowsTrimStringStart, string, trimString, newString)
}

HRESULT WINAPI
RoGetBufferMarshaler(
    IMarshal **bufferMarshaler
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetBufferMarshaler, bufferMarshaler)
}

void WINAPI
RoFreeParameterizedTypeExtra(
    ROPARAMIIDHANDLE extra
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoFreeParameterizedTypeExtra, extra)
}

HRESULT WINAPI
RoGetParameterizedTypeInstanceIID(
    UINT32                     nameElementCount,
    PCWSTR                     *nameElements,
    const IRoMetaDataLocator   &metaDataLocator,
    IID                        *iid,
    ROPARAMIIDHANDLE           *pExtra
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetParameterizedTypeInstanceIID, nameElementCount, nameElements, metaDataLocator, iid, pExtra)
}

PCSTR WINAPI
RoParameterizedTypeExtraGetTypeSignature(
    ROPARAMIIDHANDLE extra
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_PTR_FUNCTION(RoParameterizedTypeExtraGetTypeSignature, extra)
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
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoGetMetaDataFile, name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken)
}

HRESULT WINAPI
RoParseTypeName(
    HSTRING typeName,
    DWORD   *partsCount,
    HSTRING **typeNameParts
)
{
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoParseTypeName, typeName, partsCount, typeNameParts)
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
    WIN32ACRYLIC_TRY_EXECUTE_WINRT_FUNCTION(RoResolveNamespace, name, windowsMetaDataDir, packageGraphDirsCount, packageGraphDirs, metaDataFilePathsCount, metaDataFilePaths, subNamespacesCount, subNamespaces)
}

// Wrappers

void WINAPI
WINRT_SysFreeString(
    BSTR bstrString
)
{
    SysFreeString(bstrString);
}

HRESULT WINAPI
WINRT_IIDFromString(
    LPCOLESTR lpsz,
    LPIID     lpiid
)
{
    return IIDFromString(lpsz, lpiid);
}

#ifdef __cplusplus
EXTERN_C_END
#endif
