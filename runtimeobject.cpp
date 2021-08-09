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

#ifndef _DWMAPI_
#define _DWMAPI_
#endif

#ifndef _ROAPI_
#define _ROAPI_
#endif

#include <dwmapi.h>
#include <roapi.h>
#include <roerrorapi.h>
#include <roregistrationapi.h>
#include <roparameterizediid.h>
#include <rometadataresolution.h>

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI
DwmExtendFrameIntoClientArea(
    HWND hWnd,
    const MARGINS *pMarInset
)
{
    using DwmExtendFrameIntoClientAreaPrototype = HRESULT(WINAPI *)(HWND, const MARGINS *);
    static bool tried = false;
    static DwmExtendFrameIntoClientAreaPrototype DwmExtendFrameIntoClientAreaPfn = nullptr;
    if (!DwmExtendFrameIntoClientAreaPfn) {
        if (tried) {
            return E_NOTIMPL;
        } else {
            tried = true;
            const HMODULE dll = LoadLibraryExW(L"dwmapi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (dll) {
                DwmExtendFrameIntoClientAreaPfn = reinterpret_cast<DwmExtendFrameIntoClientAreaPrototype>(GetProcAddress(dll, "DwmExtendFrameIntoClientArea"));
                FreeLibrary(dll);
                if (!DwmExtendFrameIntoClientAreaPfn) {
                    return E_NOTIMPL;
                }
            } else {
                return E_NOTIMPL;
            }
        }
    }
    return DwmExtendFrameIntoClientAreaPfn(hWnd, pMarInset);
}

HRESULT WINAPI
RoActivateInstance(
    HSTRING      activatableClassId,
    IInspectable **instance
)
{
    using RoActivateInstancePrototype = HRESULT(WINAPI *)(HSTRING, IInspectable **);
    static bool tried = false;
    static RoActivateInstancePrototype RoActivateInstancePfn = nullptr;
    if (!RoActivateInstancePfn) {
        if (tried) {
            return E_NOTIMPL;
        } else {
            tried = true;
            const HMODULE dll = LoadLibraryExW(L"", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (dll) {
                RoActivateInstancePfn = reinterpret_cast<RoActivateInstancePrototype>(GetProcAddress(dll, "RoActivateInstance"));
                FreeLibrary(dll);
                if (!RoActivateInstancePfn) {
                    return E_NOTIMPL;
                }
            } else {
                return E_NOTIMPL;
            }
        }
    }
    return RoActivateInstancePfn(activatableClassId, instance);
}

HRESULT WINAPI
RoGetActivationFactory(
    HSTRING activatableClassId,
    REFIID  iid,
    void    **factory
)
{
    using RoGetActivationFactoryPrototype = HRESULT(WINAPI *)(HSTRING, REFIID, void **);
    static bool tried = false;
    static RoGetActivationFactoryPrototype RoGetActivationFactoryPfn = nullptr;
    if (!RoGetActivationFactoryPfn) {
        if (tried) {
            return E_NOTIMPL;
        } else {
            tried = true;
            const HMODULE dll = LoadLibraryExW(L"", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (dll) {
                RoGetActivationFactoryPfn = reinterpret_cast<RoGetActivationFactoryPrototype>(GetProcAddress(dll, "RoGetActivationFactory"));
                FreeLibrary(dll);
                if (!RoGetActivationFactoryPfn) {
                    return E_NOTIMPL;
                }
            } else {
                return E_NOTIMPL;
            }
        }
    }
    return RoGetActivationFactoryPfn(activatableClassId, iid, factory);
}

HRESULT WINAPI
RoGetApartmentIdentifier(
    UINT64 *apartmentIdentifier
)
{
    using RoGetApartmentIdentifierPrototype = HRESULT(WINAPI *)(UINT64 *);
    static bool tried = false;
    static RoGetApartmentIdentifierPrototype RoGetApartmentIdentifierPfn = nullptr;
    if (!RoGetApartmentIdentifierPfn) {
        if (tried) {
            return E_NOTIMPL;
        } else {
            tried = true;
            const HMODULE dll = LoadLibraryExW(L"", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (dll) {
                RoGetApartmentIdentifierPfn = reinterpret_cast<RoGetApartmentIdentifierPrototype>(GetProcAddress(dll, "RoGetApartmentIdentifier"));
                FreeLibrary(dll);
                if (!RoGetApartmentIdentifierPfn) {
                    return E_NOTIMPL;
                }
            } else {
                return E_NOTIMPL;
            }
        }
    }
    return RoGetApartmentIdentifierPfn(apartmentIdentifier);
}

HRESULT WINAPI
RoInitialize(
    RO_INIT_TYPE initType
)
{
    using RoInitializePrototype = HRESULT(WINAPI *)(RO_INIT_TYPE);
    static bool tried = false;
    static RoInitializePrototype RoInitializePfn = nullptr;
    if (!RoInitializePfn) {
        if (tried) {
            return E_NOTIMPL;
        } else {
            tried = true;
            const HMODULE dll = LoadLibraryExW(L"", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (dll) {
                RoInitializePfn = reinterpret_cast<RoInitializePrototype>(GetProcAddress(dll, "RoInitialize"));
                FreeLibrary(dll);
                if (!RoInitializePfn) {
                    return E_NOTIMPL;
                }
            } else {
                return E_NOTIMPL;
            }
        }
    }
    return RoInitializePfn(initType);
}

HRESULT WINAPI RoRegisterActivationFactories(
  HSTRING                 *activatableClassIds,
  PFNGETACTIVATIONFACTORY *activationFactoryCallbacks,
  UINT32                  count,
  RO_REGISTRATION_COOKIE  *cookie
);

HRESULT WINAPI RoRegisterForApartmentShutdown(
  IApartmentShutdown                     *callbackObject,
  UINT64                                 *apartmentIdentifier,
  APARTMENT_SHUTDOWN_REGISTRATION_COOKIE *regCookie
);

void WINAPI RoRevokeActivationFactories(
  RO_REGISTRATION_COOKIE cookie
);

void WINAPI RoUninitialize();

HRESULT WINAPI RoUnregisterForApartmentShutdown(
  APARTMENT_SHUTDOWN_REGISTRATION_COOKIE regCookie
);

HRESULT WINAPI GetRestrictedErrorInfo(
  IRestrictedErrorInfo **ppRestrictedErrorInfo
);

HRESULT WINAPI RoCaptureErrorContext(
  HRESULT hr
);

void WINAPI RoFailFastWithErrorContext(
  HRESULT hrError
);

HRESULT WINAPI RoGetErrorReportingFlags(
  UINT32 *pflags
);

BOOL WINAPI RoOriginateError(
  HRESULT error,
  HSTRING message
);

BOOL WINAPI RoOriginateErrorW(
  HRESULT error,
  UINT    cchMax,
  PCWSTR  message
);

HRESULT WINAPI RoResolveRestrictedErrorInfoReference(
  PCWSTR               reference,
  IRestrictedErrorInfo **ppRestrictedErrorInfo
);

HRESULT WINAPI RoSetErrorReportingFlags(
  UINT32 flags
);

BOOL WINAPI RoTransformError(
  HRESULT oldError,
  HRESULT newError,
  HSTRING message
);

BOOL WINAPI RoTransformErrorW(
  HRESULT oldError,
  HRESULT newError,
  UINT    cchMax,
  PCWSTR  message
);

HRESULT WINAPI SetRestrictedErrorInfo(
  IRestrictedErrorInfo *pRestrictedErrorInfo
);

BOOL WINAPI IsErrorPropagationEnabled();

void WINAPI RoClearError();

HRESULT WINAPI RoGetMatchingRestrictedErrorInfo(
    HRESULT hrIn,
    IRestrictedErrorInfo** ppRestrictedErrorInfo
);

HRESULT WINAPI RoInspectCapturedStackBackTrace(
  UINT_PTR                 targetErrorInfoAddress,
  USHORT                   machine,
  PINSPECT_MEMORY_CALLBACK readMemoryCallback,
  PVOID                    context,
  UINT32                   *frameCount,
  UINT_PTR                 *targetBackTraceAddress
);

HRESULT WINAPI RoInspectThreadErrorInfo(
  UINT_PTR                 targetTebAddress,
  USHORT                   machine,
  PINSPECT_MEMORY_CALLBACK readMemoryCallback,
  PVOID                    context,
  UINT_PTR                 *targetErrorInfoAddress
);

BOOL WINAPI RoOriginateLanguageException(
  HRESULT  error,
  HSTRING  message,
  IUnknown *languageException
);

HRESULT WINAPI RoReportFailedDelegate(
  IUnknown             *punkDelegate,
  IRestrictedErrorInfo *pRestrictedErrorInfo
);

HRESULT WINAPI RoReportUnhandledError(
  IRestrictedErrorInfo *pRestrictedErrorInfo
);

HRESULT WINAPI RoGetActivatableClassRegistration(
  HSTRING                       activatableClassId,
  PActivatableClassRegistration *activatableClassRegistration
);

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
