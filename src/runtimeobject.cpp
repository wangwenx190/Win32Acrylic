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

// Define these macros first before including their header files to avoid linking
// to their import libraries.

#ifndef _ROAPI_
#define _ROAPI_
#endif

#include "acrylicmanager_global.h"
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
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoActivateInstance, activatableClassId, instance)
}

HRESULT WINAPI
RoGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void    **factory
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetActivationFactory, activatableClassId, iid, factory)
}

HRESULT WINAPI
RoGetApartmentIdentifier(
    UINT64 *apartmentIdentifier
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetApartmentIdentifier, apartmentIdentifier)
}

HRESULT WINAPI
RoInitialize(
    RO_INIT_TYPE initType
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoInitialize, initType)
}

HRESULT WINAPI
RoRegisterActivationFactories(
    HSTRING                 *activatableClassIds,
    PFNGETACTIVATIONFACTORY *activationFactoryCallbacks,
    UINT32                  count,
    RO_REGISTRATION_COOKIE  *cookie
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoRegisterActivationFactories, activatableClassIds, activationFactoryCallbacks, count, cookie)
}

HRESULT WINAPI
RoRegisterForApartmentShutdown(
    IApartmentShutdown                     *callbackObject,
    UINT64                                 *apartmentIdentifier,
    APARTMENT_SHUTDOWN_REGISTRATION_COOKIE *regCookie
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoRegisterForApartmentShutdown, callbackObject, apartmentIdentifier, regCookie)
}

void WINAPI
RoRevokeActivationFactories(
    RO_REGISTRATION_COOKIE cookie
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoRevokeActivationFactories, cookie)
}

void WINAPI
RoUninitialize()
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoUninitialize)
}

HRESULT WINAPI
RoUnregisterForApartmentShutdown(
    APARTMENT_SHUTDOWN_REGISTRATION_COOKIE regCookie
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoUnregisterForApartmentShutdown, regCookie)
}

HRESULT WINAPI
GetRestrictedErrorInfo(
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(GetRestrictedErrorInfo, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoCaptureErrorContext(
    HRESULT hr
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoCaptureErrorContext, hr)
}

void WINAPI
RoFailFastWithErrorContext(
    HRESULT hrError
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoFailFastWithErrorContext, hrError)
}

HRESULT WINAPI
RoGetErrorReportingFlags(
    UINT32 *pFlags
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetErrorReportingFlags, pFlags)
}

BOOL WINAPI
RoOriginateError(
    HRESULT error,
    HSTRING message
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateError, error, message)
}

BOOL WINAPI
RoOriginateErrorW(
    HRESULT error,
    UINT    cchMax,
    PCWSTR  message
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateErrorW, error, cchMax, message)
}

HRESULT WINAPI
RoResolveRestrictedErrorInfoReference(
    PCWSTR               reference,
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoResolveRestrictedErrorInfoReference, reference, ppRestrictedErrorInfo)
}

HRESULT WINAPI
RoSetErrorReportingFlags(
    UINT32 flags
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoSetErrorReportingFlags, flags)
}

BOOL WINAPI
RoTransformError(
    HRESULT oldError,
    HRESULT newError,
    HSTRING message
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoTransformError, oldError, newError, message)
}

BOOL WINAPI
RoTransformErrorW(
    HRESULT oldError,
    HRESULT newError,
    UINT    cchMax,
    PCWSTR  message
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoTransformErrorW, oldError, newError, cchMax, message)
}

HRESULT WINAPI
SetRestrictedErrorInfo(
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(SetRestrictedErrorInfo, pRestrictedErrorInfo)
}

BOOL WINAPI
IsErrorPropagationEnabled()
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION(IsErrorPropagationEnabled)
}

void WINAPI
RoClearError()
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoClearError)
}

HRESULT WINAPI
RoGetMatchingRestrictedErrorInfo(
    HRESULT              hrIn,
    IRestrictedErrorInfo **ppRestrictedErrorInfo
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetMatchingRestrictedErrorInfo, hrIn, ppRestrictedErrorInfo)
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
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoInspectCapturedStackBackTrace, targetErrorInfoAddress, machine, readMemoryCallback, context, frameCount, targetBackTraceAddress)
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
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoInspectThreadErrorInfo, targetTebAddress, machine, readMemoryCallback, context, targetErrorInfoAddress)
}

BOOL WINAPI
RoOriginateLanguageException(
    HRESULT  error,
    HSTRING  message,
    IUnknown *languageException
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION(RoOriginateLanguageException, error, message, languageException)
}

HRESULT WINAPI
RoReportFailedDelegate(
    IUnknown             *punkDelegate,
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoReportFailedDelegate, punkDelegate, pRestrictedErrorInfo)
}

HRESULT WINAPI
RoReportUnhandledError(
    IRestrictedErrorInfo *pRestrictedErrorInfo
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoReportUnhandledError, pRestrictedErrorInfo)
}

HRESULT WINAPI
RoGetActivatableClassRegistration(
    HSTRING                       activatableClassId,
    PActivatableClassRegistration *activatableClassRegistration
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetActivatableClassRegistration, activatableClassId, activatableClassRegistration)
}

HRESULT WINAPI
RoGetServerActivatableClasses(
    HSTRING serverName,
    HSTRING **activatableClassIds,
    DWORD   *count
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetServerActivatableClasses, serverName, activatableClassIds, count)
}

void WINAPI
HSTRING_UserFree(
    ULONG   *pFlags,
    HSTRING *ppidl
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(HSTRING_UserFree, pFlags, ppidl)
}

void WINAPI
HSTRING_UserFree64(
    ULONG   *unnamedParam1,
    HSTRING *unnamedParam2
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(HSTRING_UserFree64, unnamedParam1, unnamedParam2)
}

UCHAR * WINAPI
HSTRING_UserMarshal(
    ULONG   *pFlags,
    UCHAR   *pBuffer,
    HSTRING *ppidl
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserMarshal, pFlags, pBuffer, ppidl)
}

UCHAR * WINAPI
HSTRING_UserMarshal64(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserMarshal64, unnamedParam1, unnamedParam2, unnamedParam3)
}

ULONG WINAPI
HSTRING_UserSize(
    ULONG   *unnamedParam1,
    ULONG   unnamedParam2,
    HSTRING *unnamedParam3
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_INT_FUNCTION(HSTRING_UserSize, unnamedParam1, unnamedParam2, unnamedParam3)
}

ULONG WINAPI
HSTRING_UserSize64(
    ULONG   *unnamedParam1,
    ULONG   unnamedParam2,
    HSTRING *unnamedParam3
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_INT_FUNCTION(HSTRING_UserSize64, unnamedParam1, unnamedParam2, unnamedParam3)
}

UCHAR * WINAPI
HSTRING_UserUnmarshal(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserUnmarshal, unnamedParam1, unnamedParam2, unnamedParam3)
}

UCHAR * WINAPI
HSTRING_UserUnmarshal64(
    ULONG   *unnamedParam1,
    UCHAR   *unnamedParam2,
    HSTRING *unnamedParam3
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_PTR_FUNCTION(HSTRING_UserUnmarshal64, unnamedParam1, unnamedParam2, unnamedParam3)
}

HRESULT WINAPI
WindowsCompareStringOrdinal(
    HSTRING string1,
    HSTRING string2,
    INT32   *result
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsCompareStringOrdinal, string1, string2, result)
}

HRESULT WINAPI
WindowsConcatString(
    HSTRING string1,
    HSTRING string2,
    HSTRING *newString
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsConcatString, string1, string2, newString)
}

HRESULT WINAPI
WindowsCreateString(
    PCNZWCH sourceString,
    UINT32  length,
    HSTRING *string
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsCreateString, sourceString, length, string)
}

HRESULT WINAPI
WindowsCreateStringReference(
    PCWSTR         sourceString,
    UINT32         length,
    HSTRING_HEADER *hstringHeader,
    HSTRING        *string
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsCreateStringReference, sourceString, length, hstringHeader, string)
}

HRESULT WINAPI
WindowsDeleteString(
    HSTRING string
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsDeleteString, string)
}

HRESULT WINAPI
WindowsDeleteStringBuffer(
    HSTRING_BUFFER bufferHandle
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsDeleteStringBuffer, bufferHandle)
}

HRESULT WINAPI
WindowsDuplicateString(
    HSTRING string,
    HSTRING *newString
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsDuplicateString, string, newString)
}

UINT32 WINAPI
WindowsGetStringLen(
    HSTRING string
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_INT_FUNCTION(WindowsGetStringLen, string)
}

PCWSTR WINAPI
WindowsGetStringRawBuffer(
    HSTRING string,
    UINT32  *length
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_PTR_FUNCTION(WindowsGetStringRawBuffer, string, length)
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
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsInspectString, targetHString, machine, callback, context, length, targetStringAddress)
}

BOOL WINAPI
WindowsIsStringEmpty(
    HSTRING string
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_BOOL_FUNCTION(WindowsIsStringEmpty, string)
}

HRESULT WINAPI
WindowsPreallocateStringBuffer(
    UINT32         length,
    WCHAR          **charBuffer,
    HSTRING_BUFFER *bufferHandle
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsPreallocateStringBuffer, length, charBuffer, bufferHandle)
}

HRESULT WINAPI
WindowsPromoteStringBuffer(
    HSTRING_BUFFER bufferHandle,
    HSTRING        *string
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsPromoteStringBuffer, bufferHandle, string)
}

HRESULT WINAPI
WindowsReplaceString(
    HSTRING string,
    HSTRING stringReplaced,
    HSTRING stringReplaceWith,
    HSTRING *newString
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsReplaceString, string, stringReplaced, stringReplaceWith, newString)
}

HRESULT WINAPI
WindowsStringHasEmbeddedNull(
    HSTRING string,
    BOOL    *hasEmbedNull
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsStringHasEmbeddedNull, string, hasEmbedNull)
}

HRESULT WINAPI
WindowsSubstring(
    HSTRING string,
    UINT32  startIndex,
    HSTRING *newString
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsSubstring, string, startIndex, newString)
}

HRESULT WINAPI
WindowsSubstringWithSpecifiedLength(
    HSTRING string,
    UINT32  startIndex,
    UINT32  length,
    HSTRING *newString
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsSubstringWithSpecifiedLength, string, startIndex, length, newString)
}

HRESULT WINAPI
WindowsTrimStringEnd(
    HSTRING string,
    HSTRING trimString,
    HSTRING *newString
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsTrimStringEnd, string, trimString, newString)
}

HRESULT WINAPI
WindowsTrimStringStart(
    HSTRING string,
    HSTRING trimString,
    HSTRING *newString
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(WindowsTrimStringStart, string, trimString, newString)
}

HRESULT WINAPI
RoGetBufferMarshaler(
    IMarshal **bufferMarshaler
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetBufferMarshaler, bufferMarshaler)
}

void WINAPI
RoFreeParameterizedTypeExtra(
    ROPARAMIIDHANDLE extra
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_VOID_FUNCTION(RoFreeParameterizedTypeExtra, extra)
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
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetParameterizedTypeInstanceIID, nameElementCount, nameElements, metaDataLocator, iid, pExtra)
}

PCSTR WINAPI
RoParameterizedTypeExtraGetTypeSignature(
    ROPARAMIIDHANDLE extra
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_PTR_FUNCTION(RoParameterizedTypeExtraGetTypeSignature, extra)
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
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoGetMetaDataFile, name, metaDataDispenser, metaDataFilePath, metaDataImport, typeDefToken)
}

HRESULT WINAPI
RoParseTypeName(
    HSTRING typeName,
    DWORD   *partsCount,
    HSTRING **typeNameParts
)
{
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoParseTypeName, typeName, partsCount, typeNameParts)
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
    ACRYLICMANAGER_TRY_EXECUTE_WINRT_FUNCTION(RoResolveNamespace, name, windowsMetaDataDir, packageGraphDirsCount, packageGraphDirs, metaDataFilePathsCount, metaDataFilePaths, subNamespacesCount, subNamespaces)
}

HRESULT WINAPI
CoIncrementMTAUsage(
    CO_MTA_USAGE_COOKIE *pCookie
)
{
    ACRYLICMANAGER_TRY_EXECUTE_OLE_FUNCTION(CoIncrementMTAUsage, pCookie)
}

#ifdef __cplusplus
EXTERN_C_END
#endif
