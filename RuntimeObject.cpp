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

#include "SystemLibraryManager.h"

#ifndef OLEAUT32_API
#define OLEAUT32_API(symbol) __RESOLVE_API(OleAut32.dll, symbol)
#endif

#ifndef COMBASE_API
#define COMBASE_API(symbol) __RESOLVE_API(ComBase.dll, symbol)
#endif

#ifndef RUNTIMEOBJECT_API_VOID
#define RUNTIMEOBJECT_API_VOID(symbol, ...) \
    COMBASE_API(symbol); \
    if (symbol##_API) { \
        symbol##_API(__VA_ARGS__); \
    } else { \
        OutputDebugStringW(L#symbol L"() is not available."); \
    }
#endif

#ifndef RUNTIMEOBJECT_API_RETURN
#define RUNTIMEOBJECT_API_RETURN(symbol, defVal, ...) \
    COMBASE_API(symbol); \
    if (symbol##_API) { \
        return symbol##_API(__VA_ARGS__); \
    } else { \
        OutputDebugStringW(L#symbol L"() is not available."); \
        return defVal; \
    }
#endif

// Define these macros first before including their header files to avoid linking
// to their import libraries.

#ifndef _OLE32_
#define _OLE32_
#endif

#ifndef _OLEAUT32_
#define _OLEAUT32_
#endif

#ifndef _COMBASEAPI_
#define _COMBASEAPI_
#endif

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

// RuntimeObject

HRESULT WINAPI
RoActivateInstance(
    HSTRING      activatableClassId,
    IInspectable **instance
)
{
    RUNTIMEOBJECT_API_RETURN(RoActivateInstance, E_NOTIMPL, activatableClassId, instance)
}

HRESULT WINAPI
RoGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void    **factory
)
{
    RUNTIMEOBJECT_API_RETURN(RoGetActivationFactory, E_NOTIMPL, activatableClassId, iid, factory)
}

HRESULT WINAPI
RoGetApartmentIdentifier(
    UINT64 *apartmentIdentifier
)
{
    RUNTIMEOBJECT_API_RETURN(RoGetApartmentIdentifier, E_NOTIMPL, apartmentIdentifier)
}

HRESULT WINAPI
RoInitialize(
    RO_INIT_TYPE initType
)
{
    RUNTIMEOBJECT_API_RETURN(RoInitialize, E_NOTIMPL, initType)
}

HRESULT WINAPI
RoRegisterActivationFactories(
    HSTRING                 *activatableClassIds,
    PFNGETACTIVATIONFACTORY *activationFactoryCallbacks,
    UINT32                  count,
    RO_REGISTRATION_COOKIE  *cookie
)
{
    RUNTIMEOBJECT_API_RETURN(RoRegisterActivationFactories, E_NOTIMPL, activatableClassIds, activationFactoryCallbacks, count, cookie)
}

HRESULT WINAPI
RoRegisterForApartmentShutdown(
    IApartmentShutdown                     *callbackObject,
    UINT64                                 *apartmentIdentifier,
    APARTMENT_SHUTDOWN_REGISTRATION_COOKIE *regCookie
)
{
    RUNTIMEOBJECT_API_RETURN(RoRegisterForApartmentShutdown, E_NOTIMPL, callbackObject, apartmentIdentifier, regCookie)
}

void WINAPI
RoRevokeActivationFactories(
    RO_REGISTRATION_COOKIE cookie
)
{
    RUNTIMEOBJECT_API_VOID(RoRevokeActivationFactories, cookie)
}

void WINAPI
RoUninitialize()
{
    RUNTIMEOBJECT_API_VOID(RoUninitialize)
}

HRESULT WINAPI
RoUnregisterForApartmentShutdown(
    APARTMENT_SHUTDOWN_REGISTRATION_COOKIE regCookie
)
{
    RUNTIMEOBJECT_API_RETURN(RoUnregisterForApartmentShutdown, E_NOTIMPL, regCookie)
}

HRESULT WINAPI
GetRestrictedErrorInfo(
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_API_RETURN(GetRestrictedErrorInfo, E_NOTIMPL, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoCaptureErrorContext(
    HRESULT hr
)
{
    RUNTIMEOBJECT_API_RETURN(RoCaptureErrorContext, E_NOTIMPL, hr)
}

void WINAPI
RoFailFastWithErrorContext(
    HRESULT hrError
)
{
    RUNTIMEOBJECT_API_VOID(RoFailFastWithErrorContext, hrError)
}

HRESULT WINAPI
RoGetErrorReportingFlags(
    UINT32 *pFlags
)
{
    RUNTIMEOBJECT_API_RETURN(RoGetErrorReportingFlags, E_NOTIMPL, pFlags)
}

BOOL WINAPI
RoOriginateError(
    HRESULT error,
    HSTRING message
)
{
    RUNTIMEOBJECT_API_RETURN(RoOriginateError, FALSE, error, message)
}

BOOL WINAPI
RoOriginateErrorW(
    HRESULT error,
    UINT    cchMax,
    PCWSTR  message
)
{
    RUNTIMEOBJECT_API_RETURN(RoOriginateErrorW, FALSE, error, cchMax, message)
}

HRESULT WINAPI
RoResolveRestrictedErrorInfoReference(
    PCWSTR               reference,
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_API_RETURN(RoResolveRestrictedErrorInfoReference, E_NOTIMPL, reference, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoSetErrorReportingFlags(
    UINT32 flags
)
{
    RUNTIMEOBJECT_API_RETURN(RoSetErrorReportingFlags, E_NOTIMPL, flags)
}

BOOL WINAPI
RoTransformError(
    HRESULT oldError,
    HRESULT newError,
    HSTRING message
)
{
    RUNTIMEOBJECT_API_RETURN(RoTransformError, FALSE, oldError, newError, message)
}

BOOL WINAPI
RoTransformErrorW(
    HRESULT oldError,
    HRESULT newError,
    UINT    cchMax,
    PCWSTR  message
)
{
    RUNTIMEOBJECT_API_RETURN(RoTransformErrorW, FALSE, oldError, newError, cchMax, message)
}

HRESULT WINAPI
SetRestrictedErrorInfo(
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_API_RETURN(SetRestrictedErrorInfo, E_NOTIMPL, pRestrictedErrorInfo)
}

BOOL WINAPI
IsErrorPropagationEnabled()
{
    RUNTIMEOBJECT_API_RETURN(IsErrorPropagationEnabled, FALSE)
}

void WINAPI
RoClearError()
{
    RUNTIMEOBJECT_API_VOID(RoClearError)
}

HRESULT WINAPI
RoGetMatchingRestrictedErrorInfo(
    HRESULT              hrIn,
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_API_RETURN(RoGetMatchingRestrictedErrorInfo, E_NOTIMPL, hrIn, ppRestrictedErrorInfo)
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
    RUNTIMEOBJECT_API_RETURN(RoInspectCapturedStackBackTrace, E_NOTIMPL, targetErrorInfoAddress, machine, readMemoryCallback, context, frameCount, targetBackTraceAddress)
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
    RUNTIMEOBJECT_API_RETURN(RoInspectThreadErrorInfo, E_NOTIMPL, targetTebAddress, machine, readMemoryCallback, context, targetErrorInfoAddress)
}

BOOL WINAPI
RoOriginateLanguageException(
    HRESULT  error,
    HSTRING  message,
    IUnknown *languageException
)
{
    RUNTIMEOBJECT_API_RETURN(RoOriginateLanguageException, FALSE, error, message, languageException)
}

HRESULT WINAPI
RoReportFailedDelegate(
    IUnknown             *punkDelegate,
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_API_RETURN(RoReportFailedDelegate, E_NOTIMPL, punkDelegate, pRestrictedErrorInfo)
}

HRESULT WINAPI
RoReportUnhandledError(
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    RUNTIMEOBJECT_API_RETURN(RoReportUnhandledError, E_NOTIMPL, pRestrictedErrorInfo)
}

HRESULT WINAPI
RoGetActivatableClassRegistration(
    HSTRING                       activatableClassId,
    PActivatableClassRegistration *activatableClassRegistration
)
{
    RUNTIMEOBJECT_API_RETURN(RoGetActivatableClassRegistration, E_NOTIMPL, activatableClassId, activatableClassRegistration)
}

HRESULT WINAPI
RoGetServerActivatableClasses(
    HSTRING serverName,
    HSTRING **activatableClassIds,
    DWORD   *count
)
{
    RUNTIMEOBJECT_API_RETURN(RoGetServerActivatableClasses, E_NOTIMPL, serverName, activatableClassIds, count)
}

void WINAPI
HSTRING_UserFree(
    ULONG   *pFlags,
    HSTRING *ppidl
)
{
    RUNTIMEOBJECT_API_VOID(HSTRING_UserFree, pFlags, ppidl)
}

void WINAPI
HSTRING_UserFree64(
    ULONG   *unnamedParam1,
    HSTRING *unnamedParam2
)
{
    RUNTIMEOBJECT_API_VOID(HSTRING_UserFree64, unnamedParam1, unnamedParam2)
}

UCHAR * WINAPI
HSTRING_UserMarshal(
    ULONG   *pFlags,
    UCHAR   *pBuffer,
    HSTRING *ppidl
)
{
    RUNTIMEOBJECT_API_RETURN(HSTRING_UserMarshal, nullptr, pFlags, pBuffer, ppidl)
}

UCHAR * WINAPI
HSTRING_UserMarshal64(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_API_RETURN(HSTRING_UserMarshal64, nullptr, unnamedParam1, unnamedParam2, unnamedParam3)
}

ULONG WINAPI
HSTRING_UserSize(
    ULONG   *unnamedParam1,
    ULONG   unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_API_RETURN(HSTRING_UserSize, 0, unnamedParam1, unnamedParam2, unnamedParam3)
}

ULONG WINAPI
HSTRING_UserSize64(
    ULONG   *unnamedParam1,
    ULONG   unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_API_RETURN(HSTRING_UserSize64, 0, unnamedParam1, unnamedParam2, unnamedParam3)
}

UCHAR * WINAPI
HSTRING_UserUnmarshal(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_API_RETURN(HSTRING_UserUnmarshal, nullptr, unnamedParam1, unnamedParam2, unnamedParam3)
}

UCHAR * WINAPI
HSTRING_UserUnmarshal64(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    RUNTIMEOBJECT_API_RETURN(HSTRING_UserUnmarshal64, nullptr, unnamedParam1, unnamedParam2, unnamedParam3)
}

HRESULT WINAPI
WindowsCompareStringOrdinal(
    HSTRING string1,
    HSTRING string2,
    INT32   *result
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsCompareStringOrdinal, E_NOTIMPL, string1, string2, result)
}

HRESULT WINAPI
WindowsConcatString(
    HSTRING string1,
    HSTRING string2,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsConcatString, E_NOTIMPL, string1, string2, newString)
}

HRESULT WINAPI
WindowsCreateString(
    PCNZWCH sourceString,
    UINT32  length,
    HSTRING *string
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsCreateString, E_NOTIMPL, sourceString, length, string)
}

HRESULT WINAPI
WindowsCreateStringReference(
    PCWSTR         sourceString,
    UINT32         length,
    HSTRING_HEADER *hstringHeader,
    HSTRING        *string
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsCreateStringReference, E_NOTIMPL, sourceString, length, hstringHeader, string)
}

HRESULT WINAPI
WindowsDeleteString(
    HSTRING string
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsDeleteString, E_NOTIMPL, string)
}

HRESULT WINAPI
WindowsDeleteStringBuffer(
    HSTRING_BUFFER bufferHandle
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsDeleteStringBuffer, E_NOTIMPL, bufferHandle)
}

HRESULT WINAPI
WindowsDuplicateString(
    HSTRING string,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsDuplicateString, E_NOTIMPL, string, newString)
}

UINT32 WINAPI
WindowsGetStringLen(
    HSTRING string
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsGetStringLen, 0, string)
}

PCWSTR WINAPI
WindowsGetStringRawBuffer(
    HSTRING string,
    UINT32  *length
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsGetStringRawBuffer, nullptr, string, length)
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
    RUNTIMEOBJECT_API_RETURN(WindowsInspectString, E_NOTIMPL, targetHString, machine, callback, context, length, targetStringAddress)
}

BOOL WINAPI
WindowsIsStringEmpty(
    HSTRING string
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsIsStringEmpty, FALSE, string)
}

HRESULT WINAPI
WindowsPreallocateStringBuffer(
    UINT32         length,
    WCHAR          **charBuffer,
    HSTRING_BUFFER *bufferHandle
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsPreallocateStringBuffer, E_NOTIMPL, length, charBuffer, bufferHandle)
}

HRESULT WINAPI
WindowsPromoteStringBuffer(
    HSTRING_BUFFER bufferHandle,
    HSTRING        *string
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsPromoteStringBuffer, E_NOTIMPL, bufferHandle, string)
}

HRESULT WINAPI
WindowsReplaceString(
    HSTRING string,
    HSTRING stringReplaced,
    HSTRING stringReplaceWith,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsReplaceString, E_NOTIMPL, string, stringReplaced, stringReplaceWith, newString)
}

HRESULT WINAPI
WindowsStringHasEmbeddedNull(
    HSTRING string,
    BOOL    *hasEmbedNull
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsStringHasEmbeddedNull, E_NOTIMPL, string, hasEmbedNull)
}

HRESULT WINAPI
WindowsSubstring(
    HSTRING string,
    UINT32  startIndex,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsSubstring, E_NOTIMPL, string, startIndex, newString)
}

HRESULT WINAPI
WindowsSubstringWithSpecifiedLength(
    HSTRING string,
    UINT32  startIndex,
    UINT32  length,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsSubstringWithSpecifiedLength, E_NOTIMPL, string, startIndex, length, newString)
}

HRESULT WINAPI
WindowsTrimStringEnd(
    HSTRING string,
    HSTRING trimString,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsTrimStringEnd, E_NOTIMPL, string, trimString, newString)
}

HRESULT WINAPI
WindowsTrimStringStart(
    HSTRING string,
    HSTRING trimString,
    HSTRING *newString
)
{
    RUNTIMEOBJECT_API_RETURN(WindowsTrimStringStart, E_NOTIMPL, string, trimString, newString)
}

HRESULT WINAPI
RoGetBufferMarshaler(
    IMarshal **bufferMarshaler
)
{
    RUNTIMEOBJECT_API_RETURN(RoGetBufferMarshaler, E_NOTIMPL, bufferMarshaler)
}

void WINAPI
RoFreeParameterizedTypeExtra(
    ROPARAMIIDHANDLE extra
)
{
    RUNTIMEOBJECT_API_VOID(RoFreeParameterizedTypeExtra, extra)
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
    RUNTIMEOBJECT_API_RETURN(RoGetParameterizedTypeInstanceIID, E_NOTIMPL, nameElementCount, nameElements, metaDataLocator, iid, pExtra)
}

PCSTR WINAPI
RoParameterizedTypeExtraGetTypeSignature(
    ROPARAMIIDHANDLE extra
)
{
    RUNTIMEOBJECT_API_RETURN(RoParameterizedTypeExtraGetTypeSignature, nullptr, extra)
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
    RUNTIMEOBJECT_API_RETURN(RoGetMetaDataFile, E_NOTIMPL, name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken)
}

HRESULT WINAPI
RoParseTypeName(
    HSTRING typeName,
    DWORD   *partsCount,
    HSTRING **typeNameParts
)
{
    RUNTIMEOBJECT_API_RETURN(RoParseTypeName, E_NOTIMPL, typeName, partsCount, typeNameParts)
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
    RUNTIMEOBJECT_API_RETURN(RoResolveNamespace, E_NOTIMPL, name, windowsMetaDataDir, packageGraphDirsCount, packageGraphDirs, metaDataFilePathsCount, metaDataFilePaths, subNamespacesCount, subNamespaces)
}

// ComBaseApi

HRESULT WINAPI
IIDFromString(
    LPCOLESTR lpsz,
    LPIID     lpiid
)
{
    OLE32_API(IIDFromString);
    if (IIDFromString_API) {
        return IIDFromString_API(lpsz, lpiid);
    } else {
        OutputDebugStringW(L"IIDFromString() is not available.");
        return E_NOTIMPL;
    }
}

HRESULT WINAPI
CoIncrementMTAUsage(
    CO_MTA_USAGE_COOKIE *pCookie
)
{
    OLE32_API(CoIncrementMTAUsage);
    if (CoIncrementMTAUsage_API) {
        return CoIncrementMTAUsage_API(pCookie);
    } else {
        OutputDebugStringW(L"CoIncrementMTAUsage() is not available.");
        return E_NOTIMPL;
    }
}

// COM Automation

void WINAPI
SysFreeString(
    BSTR bstrString
)
{
    OLEAUT32_API(SysFreeString);
    if (SysFreeString_API) {
        SysFreeString_API(bstrString);
    } else {
        OutputDebugStringW(L"SysFreeString() is not available.");
    }
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

HRESULT WINAPI
WINRT_CoIncrementMTAUsage(
    CO_MTA_USAGE_COOKIE *pCookie
)
{
    return CoIncrementMTAUsage(pCookie);
}

#ifdef __cplusplus
EXTERN_C_END
#endif
