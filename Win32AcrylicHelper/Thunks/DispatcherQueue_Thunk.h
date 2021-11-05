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

#include <Windows.System.h>

using PDISPATCHERQUEUE = ABI::Windows::System::IDispatcherQueue *;
using PDISPATCHERQUEUECONTROLLER = ABI::Windows::System::IDispatcherQueueController *;

using DISPATCHERQUEUE_THREAD_APARTMENTTYPE = enum _DISPATCHERQUEUE_THREAD_APARTMENTTYPE
{
    DQTAT_COM_NONE = 0,
    DQTAT_COM_ASTA = 1,
    DQTAT_COM_STA = 2
};

using DISPATCHERQUEUE_THREAD_TYPE = enum _DISPATCHERQUEUE_THREAD_TYPE
{
    DQTYPE_THREAD_DEDICATED = 1,
    DQTYPE_THREAD_CURRENT = 2,
};

using DispatcherQueueOptions = struct _DispatcherQueueOptions
{
    DWORD                                dwSize;        // Size of the struct
    DISPATCHERQUEUE_THREAD_TYPE          threadType;    // Thread affinity on which DispatcherQueueController is created.
    DISPATCHERQUEUE_THREAD_APARTMENTTYPE apartmentType; // Initialize COM apartment on the new thread as ASTA or STA. Only relevant if threadType is DQTYPE_THREAD_DEDICATED
};

EXTERN_C HRESULT WINAPI CreateDispatcherQueueController(DispatcherQueueOptions, PDISPATCHERQUEUECONTROLLER *);
