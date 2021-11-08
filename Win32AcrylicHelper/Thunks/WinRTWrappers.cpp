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

#ifndef _OLE32_
#define _OLE32_
#endif // _OLE32_

#ifndef _COMBASEAPI_
#define _COMBASEAPI_
#endif // _COMBASEAPI_

#ifndef _OLEAUT32_
#define _OLEAUT32_
#endif // _OLEAUT32_

#include <ComBaseApi.h>
#include <OleAuto.h>

#ifdef __cplusplus
EXTERN_C_START
#endif // __cplusplus

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

HRESULT WINAPI
WINRT_SetErrorInfo(
    ULONG      dwReserved,
    IErrorInfo *perrinfo
)
{
    return SetErrorInfo(dwReserved, perrinfo);
}

HRESULT WINAPI
WINRT_GetErrorInfo(
    ULONG      dwReserved,
    IErrorInfo **pperrinfo
)
{
    return GetErrorInfo(dwReserved, pperrinfo);
}

BSTR WINAPI
WINRT_SysAllocString(
    const OLECHAR *psz
)
{
    return SysAllocString(psz);
}

UINT WINAPI
WINRT_SysStringLen(
    BSTR pbstr
)
{
    return SysStringLen(pbstr);
}

#ifdef __cplusplus
EXTERN_C_END
#endif // __cplusplus
