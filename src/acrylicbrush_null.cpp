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

#include "acrylicbrush_null.h"
#include "customframe.h"

class AcrylicBrushNullPrivate final : public CustomFrameT<AcrylicBrushNullPrivate>
{
    ACRYLICMANAGER_DISABLE_COPY_MOVE(AcrylicBrushNullPrivate)

public:
    explicit AcrylicBrushNullPrivate(AcrylicBrushNull *q);
    ~AcrylicBrushNullPrivate() override;

    [[nodiscard]] bool Initialize();
    [[nodiscard]] HWND GetWindowHandle() const;
    void ReloadBrushParameters();

private:
    AcrylicBrushNull *q_ptr = nullptr;
};

AcrylicBrushNullPrivate::AcrylicBrushNullPrivate(AcrylicBrushNull *q)
{
    q_ptr = q;
}

AcrylicBrushNullPrivate::~AcrylicBrushNullPrivate()
{
}

bool AcrylicBrushNullPrivate::Initialize()
{
    if (!CreateThisWindow()) {
        OutputDebugStringW(L"Failed to create the background window.");
        return false;
    }
    return true;
}

HWND AcrylicBrushNullPrivate::GetWindowHandle() const
{
    return GetHandle();
}

void AcrylicBrushNullPrivate::ReloadBrushParameters()
{
    // Nothing to do.
}

AcrylicBrushNull::AcrylicBrushNull()
{
    d_ptr = std::make_unique<AcrylicBrushNullPrivate>(this);
}

AcrylicBrushNull::~AcrylicBrushNull()
{
}

bool AcrylicBrushNull::IsSupportedByCurrentOS() const
{
    static const bool result = true;
    return result;
}

bool AcrylicBrushNull::Initialize() const
{
    return d_ptr->Initialize();
}

bool AcrylicBrushNull::RefreshBrush() const
{
    d_ptr->ReloadBrushParameters();
    return true;
}

HWND AcrylicBrushNull::GetWindowHandle() const
{
    return d_ptr->GetWindowHandle();
}

int AcrylicBrushNull::MessageLoop() const
{
    return AcrylicBrushNullPrivate::MessageLoop();
}
