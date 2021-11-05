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

#ifndef _ROAPI_
#define _ROAPI_
#endif // _ROAPI_

#include "WindowsAPIThunks.h"
#include "SystemLibraryManager.h"

#include <RoApi.h>
#include <RoErrorApi.h>
#include <RoRegistrationApi.h>
#include <RoParameterizedIid.h>
#include <RoMetadataResolution.h>

#ifndef __COMBASE_DLL_FILENAME
#define __COMBASE_DLL_FILENAME combase.dll
#endif // __COMBASE_DLL_FILENAME

__THUNK_API(__COMBASE_DLL_FILENAME, RoActivateInstance, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, IInspectable **arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetActivationFactory, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, REFIID arg2, void **arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetApartmentIdentifier, HRESULT, DEFAULT_HRESULT, (UINT64 *arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoInitialize, HRESULT, DEFAULT_HRESULT, (RO_INIT_TYPE arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoRegisterActivationFactories, HRESULT, DEFAULT_HRESULT, (HSTRING *arg1, PFNGETACTIVATIONFACTORY *arg2, UINT32 arg3, RO_REGISTRATION_COOKIE *arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__COMBASE_DLL_FILENAME, RoRegisterForApartmentShutdown, HRESULT, DEFAULT_HRESULT, (IApartmentShutdown *arg1, UINT64 *arg2, APARTMENT_SHUTDOWN_REGISTRATION_COOKIE *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, RoRevokeActivationFactories, void, DEFAULT_VOID, (RO_REGISTRATION_COOKIE arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoUninitialize, void, DEFAULT_VOID, (VOID), ())
__THUNK_API(__COMBASE_DLL_FILENAME, RoUnregisterForApartmentShutdown, HRESULT, DEFAULT_HRESULT, (APARTMENT_SHUTDOWN_REGISTRATION_COOKIE arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, GetRestrictedErrorInfo, HRESULT, DEFAULT_HRESULT, (IRestrictedErrorInfo **arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoCaptureErrorContext, HRESULT, DEFAULT_HRESULT, (HRESULT arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoFailFastWithErrorContext, void, DEFAULT_VOID, (HRESULT arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetErrorReportingFlags, HRESULT, DEFAULT_HRESULT, (UINT32 *arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoOriginateError, BOOL, DEFAULT_BOOL, (HRESULT arg1, HSTRING arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, RoOriginateErrorW, BOOL, DEFAULT_BOOL, (HRESULT arg1, UINT arg2, PCWSTR arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, RoResolveRestrictedErrorInfoReference, HRESULT, DEFAULT_HRESULT, (PCWSTR arg1, IRestrictedErrorInfo **arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, RoSetErrorReportingFlags, HRESULT, DEFAULT_HRESULT, (UINT32 arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoTransformError, BOOL, DEFAULT_BOOL, (HRESULT arg1, HRESULT arg2, HSTRING arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, RoTransformErrorW, BOOL, DEFAULT_BOOL, (HRESULT arg1, HRESULT arg2, UINT arg3, PCWSTR arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__COMBASE_DLL_FILENAME, SetRestrictedErrorInfo, HRESULT, DEFAULT_HRESULT, (IRestrictedErrorInfo *arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, IsErrorPropagationEnabled, BOOL, DEFAULT_BOOL, (VOID), ())
__THUNK_API(__COMBASE_DLL_FILENAME, RoClearError, void, DEFAULT_VOID, (VOID), ())
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetMatchingRestrictedErrorInfo, HRESULT, DEFAULT_HRESULT, (HRESULT arg1, IRestrictedErrorInfo **arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, RoInspectCapturedStackBackTrace, HRESULT, DEFAULT_HRESULT, (UINT_PTR arg1, USHORT arg2, PINSPECT_MEMORY_CALLBACK arg3, PVOID arg4, UINT32 *arg5, UINT_PTR *arg6), (arg1, arg2, arg3, arg4, arg5, arg6))
__THUNK_API(__COMBASE_DLL_FILENAME, RoInspectThreadErrorInfo, HRESULT, DEFAULT_HRESULT, (UINT_PTR arg1, USHORT arg2, PINSPECT_MEMORY_CALLBACK arg3, PVOID arg4, UINT_PTR *arg5), (arg1, arg2, arg3, arg4, arg5))
__THUNK_API(__COMBASE_DLL_FILENAME, RoOriginateLanguageException, BOOL, DEFAULT_BOOL, (HRESULT arg1, HSTRING arg2, IUnknown *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, RoReportFailedDelegate, HRESULT, DEFAULT_HRESULT, (IUnknown *arg1, IRestrictedErrorInfo *arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, RoReportUnhandledError, HRESULT, DEFAULT_HRESULT, (IRestrictedErrorInfo *arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetActivatableClassRegistration, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, PActivatableClassRegistration *arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetServerActivatableClasses, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, HSTRING **arg2, DWORD *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, HSTRING_UserFree, void, DEFAULT_VOID, (ULONG *arg1, HSTRING *arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, HSTRING_UserFree64, void, DEFAULT_VOID, (ULONG *arg1, HSTRING *arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, HSTRING_UserMarshal, UCHAR *, DEFAULT_PTR, (ULONG *arg1, UCHAR *arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, HSTRING_UserMarshal64, UCHAR *, DEFAULT_PTR, (ULONG *arg1, UCHAR *arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, HSTRING_UserSize, ULONG, DEFAULT_UINT, (ULONG *arg1, ULONG arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, HSTRING_UserSize64, ULONG, DEFAULT_UINT, (ULONG *arg1, ULONG arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, HSTRING_UserUnmarshal, UCHAR *, DEFAULT_PTR, (ULONG *arg1, UCHAR *arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, HSTRING_UserUnmarshal64, UCHAR *, DEFAULT_PTR, (ULONG *arg1, UCHAR *arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsCompareStringOrdinal, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, HSTRING arg2, INT32 *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsConcatString, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, HSTRING arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsCreateString, HRESULT, DEFAULT_HRESULT, (PCNZWCH arg1, UINT32 arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsCreateStringReference, HRESULT, DEFAULT_HRESULT, (PCWSTR arg1, UINT32 arg2, HSTRING_HEADER *arg3, HSTRING *arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsDeleteString, HRESULT, DEFAULT_HRESULT, (HSTRING arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsDeleteStringBuffer, HRESULT, DEFAULT_HRESULT, (HSTRING_BUFFER arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsDuplicateString, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, HSTRING *arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsGetStringLen, UINT32, DEFAULT_UINT, (HSTRING arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsGetStringRawBuffer, PCWSTR, DEFAULT_PTR, (HSTRING arg1, UINT32 *arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsInspectString, HRESULT, DEFAULT_HRESULT, (UINT_PTR arg1, USHORT arg2, PINSPECT_HSTRING_CALLBACK arg3, void *arg4, UINT32 *arg5, UINT_PTR *arg6), (arg1, arg2, arg3, arg4, arg5, arg6))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsIsStringEmpty, BOOL, DEFAULT_BOOL, (HSTRING arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsPreallocateStringBuffer, HRESULT, DEFAULT_HRESULT, (UINT32 arg1, WCHAR **arg2, HSTRING_BUFFER *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsPromoteStringBuffer, HRESULT, DEFAULT_HRESULT, (HSTRING_BUFFER arg1, HSTRING *arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsReplaceString, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, HSTRING arg2, HSTRING arg3, HSTRING *arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsStringHasEmbeddedNull, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, BOOL *arg2), (arg1, arg2))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsSubstring, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, UINT32 arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsSubstringWithSpecifiedLength, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, UINT32 arg2, UINT32 arg3, HSTRING *arg4), (arg1, arg2, arg3, arg4))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsTrimStringEnd, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, HSTRING arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, WindowsTrimStringStart, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, HSTRING arg2, HSTRING *arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetBufferMarshaler, HRESULT, DEFAULT_HRESULT, (IMarshal **arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoFreeParameterizedTypeExtra, void, DEFAULT_VOID, (ROPARAMIIDHANDLE arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetParameterizedTypeInstanceIID, HRESULT, DEFAULT_HRESULT, (UINT32 arg1, PCWSTR *arg2, const IRoMetaDataLocator &arg3, IID *arg4, ROPARAMIIDHANDLE *arg5), (arg1, arg2, arg3, arg4, arg5))
__THUNK_API(__COMBASE_DLL_FILENAME, RoParameterizedTypeExtraGetTypeSignature, PCSTR, DEFAULT_PTR, (ROPARAMIIDHANDLE arg1), (arg1))
__THUNK_API(__COMBASE_DLL_FILENAME, RoGetMetaDataFile, HRESULT, DEFAULT_HRESULT, (const HSTRING arg1, IMetaDataDispenserEx *arg2, HSTRING *arg3, IMetaDataImport2 **arg4, mdTypeDef *arg5), (arg1, arg2, arg3, arg4, arg5))
__THUNK_API(__COMBASE_DLL_FILENAME, RoParseTypeName, HRESULT, DEFAULT_HRESULT, (HSTRING arg1, DWORD *arg2, HSTRING **arg3), (arg1, arg2, arg3))
__THUNK_API(__COMBASE_DLL_FILENAME, RoResolveNamespace, HRESULT, DEFAULT_HRESULT, (const HSTRING arg1, const HSTRING arg2, const DWORD arg3, const HSTRING *arg4, DWORD *arg5, HSTRING **arg6, DWORD *arg7, HSTRING **arg8), (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
