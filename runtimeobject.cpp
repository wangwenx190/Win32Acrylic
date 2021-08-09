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

#ifndef _DWMAPI_
#define _DWMAPI_
#endif

#ifndef _ROAPI_
#define _ROAPI_
#endif

#include <Windows.h>
#include <DWMAPI.h>
#include <roapi.h>
#include <roerrorapi.h>
#include <roregistrationapi.h>
#include <roparameterizediid.h>
#include <rometadataresolution.h>

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION(funcName, libName, defVal, ...) \
using sig = decltype(&::funcName); \
static bool tried = false; \
static sig func = nullptr; \
if (!func) { \
    if (tried) { \
        return defVal; \
    } else { \
        tried = true; \
        const HMODULE dll = LoadLibraryExW(L#libName ".dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); \
        if (!dll) { \
            OutputDebugStringW(L"Failed to load " #libName ".dll."); \
            return defVal; \
        } \
        func = reinterpret_cast<sig>(GetProcAddress(dll, #funcName)); \
        FreeLibrary(dll); \
        if (!func) { \
            OutputDebugStringW(L"Failed to resolve " #funcName "()."); \
            return defVal; \
        } \
    } \
} \
return func(__VA_ARGS__);
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_USER_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_USER_FUNCTION(funcName, defVal, ...) RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION(funcName, User32, defVal, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_DWM_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_DWM_FUNCTION(funcName, defVal, ...) RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION(funcName, DWMAPI, defVal, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_WINRT_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION(funcName, ComBase, E_NOTIMPL, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_WINRT_VOID_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION(funcName, ComBase, void, ##__VA_ARGS__)
#endif

#ifndef RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION
#define RUNTIMEOBJECT_TRY_EXECUTE_WINRT_BOOL_FUNCTION(funcName, ...) RUNTIMEOBJECT_TRY_EXECUTE_FUNCTION(funcName, ComBase, FALSE, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int WINAPI
GetSystemMetricsForDpi(
    int  nIndex,
    UINT dpi
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_USER_FUNCTION(GetSystemMetricsForDpi, 0, nIndex, dpi)
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
    RUNTIMEOBJECT_TRY_EXECUTE_USER_FUNCTION(AdjustWindowRectExForDpi, FALSE, lpRect, dwStyle, bMenu, dwExStyle, dpi)
}

HRESULT WINAPI
DwmExtendFrameIntoClientArea(
    HWND          hWnd,
    const MARGINS *pMarInset
)
{
    RUNTIMEOBJECT_TRY_EXECUTE_DWM_FUNCTION(DwmExtendFrameIntoClientArea, E_NOTIMPL, hWnd, pMarInset)
}

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

HRESULT WINAPI RoGetServerActivatableClasses(
  HSTRING serverName,
  HSTRING **activatableClassIds,
  DWORD   *count
);

void WINAPI HSTRING_UserFree(
  ULONG   *pFlags,
  HSTRING *ppidl
);

void WINAPI HSTRING_UserFree64(
  unsigned long *,
  HSTRING       *
);

UCHAR * WINAPI HSTRING_UserMarshal(
  ULONG   *pFlags,
  UCHAR   *pBuffer,
  HSTRING *ppidl
);

unsigned char * WINAPI HSTRING_UserMarshal64(
  unsigned long *,
  unsigned char *,
  HSTRING       *
);

unsigned long WINAPI HSTRING_UserSize(
  unsigned long *,
  unsigned long ,
  HSTRING       *
);

unsigned long WINAPI HSTRING_UserSize64(
  unsigned long *,
  unsigned long ,
  HSTRING       *
);

unsigned char * WINAPI HSTRING_UserUnmarshal(
  unsigned long *,
  unsigned char *,
  HSTRING       *
);

unsigned char * WINAPI HSTRING_UserUnmarshal64(
  unsigned long *,
  unsigned char *,
  HSTRING       *
);

HRESULT WINAPI WindowsCompareStringOrdinal(
  HSTRING string1,
  HSTRING string2,
  INT32   *result
);

HRESULT WINAPI WindowsConcatString(
  HSTRING string1,
  HSTRING string2,
  HSTRING *newString
);

HRESULT WINAPI WindowsCreateString(
  PCNZWCH sourceString,
  UINT32  length,
  HSTRING *string
);

HRESULT WINAPI WindowsCreateStringReference(
  PCWSTR         sourceString,
  UINT32         length,
  HSTRING_HEADER *hstringHeader,
  HSTRING        *string
);

HRESULT WINAPI WindowsDeleteString(
  HSTRING string
);

HRESULT WINAPI WindowsDeleteStringBuffer(
  HSTRING_BUFFER bufferHandle
);

HRESULT WINAPI WindowsDuplicateString(
  HSTRING string,
  HSTRING *newString
);

UINT32 WINAPI WindowsGetStringLen(
  HSTRING string
);

PCWSTR WINAPI WindowsGetStringRawBuffer(
  HSTRING string,
  UINT32  *length
);

HRESULT WINAPI WindowsInspectString(
  UINT_PTR                  targetHString,
  USHORT                    machine,
  PINSPECT_HSTRING_CALLBACK callback,
  void                      *context,
  UINT32                    *length,
  UINT_PTR                  *targetStringAddress
);

BOOL WINAPI WindowsIsStringEmpty(
  HSTRING string
);

HRESULT WINAPI WindowsPreallocateStringBuffer(
  UINT32         length,
  WCHAR          **charBuffer,
  HSTRING_BUFFER *bufferHandle
);

HRESULT WINAPI WindowsPromoteStringBuffer(
  HSTRING_BUFFER bufferHandle,
  HSTRING        *string
);

HRESULT WINAPI WindowsReplaceString(
  HSTRING string,
  HSTRING stringReplaced,
  HSTRING stringReplaceWith,
  HSTRING *newString
);

HRESULT WINAPI WindowsStringHasEmbeddedNull(
  HSTRING string,
  BOOL    *hasEmbedNull
);

HRESULT WINAPI WindowsSubstring(
  HSTRING string,
  UINT32  startIndex,
  HSTRING *newString
);

HRESULT WINAPI WindowsSubstringWithSpecifiedLength(
  HSTRING string,
  UINT32  startIndex,
  UINT32  length,
  HSTRING *newString
);

HRESULT WINAPI WindowsTrimStringEnd(
  HSTRING string,
  HSTRING trimString,
  HSTRING *newString
);

HRESULT WINAPI WindowsTrimStringStart(
  HSTRING string,
  HSTRING trimString,
  HSTRING *newString
);

HRESULT WINAPI RoGetBufferMarshaler(
  IMarshal **bufferMarshaler
);

void WINAPI RoFreeParameterizedTypeExtra(
  ROPARAMIIDHANDLE extra
);

HRESULT WINAPI RoGetParameterizedTypeInstanceIID(
  UINT32                     nameElementCount,
  PCWSTR                     *nameElements,
  const IRoMetaDataLocator & metaDataLocator,
  GUID                       *iid,
  ROPARAMIIDHANDLE           *pExtra
);

PCSTR WINAPI RoParameterizedTypeExtraGetTypeSignature(
  ROPARAMIIDHANDLE extra
);

HRESULT WINAPI RoGetMetaDataFile(
  const HSTRING        name,
  IMetaDataDispenserEx *metaDataDispenser,
  HSTRING              *metaDataFilePath,
  IMetaDataImport2     **metaDataImport,
  mdTypeDef            *typeDefToken
);

HRESULT WINAPI RoParseTypeName(
  HSTRING typeName,
  DWORD   *partsCount,
  HSTRING **typeNameParts
);

HRESULT WINAPI RoResolveNamespace(
  const HSTRING name,
  const HSTRING windowsMetaDataDir,
  const DWORD   packageGraphDirsCount,
  const HSTRING *packageGraphDirs,
  DWORD         *metaDataFilePathsCount,
  HSTRING       **metaDataFilePaths,
  DWORD         *subNamespacesCount,
  HSTRING       **subNamespaces
);

#ifdef __cplusplus
}
#endif
