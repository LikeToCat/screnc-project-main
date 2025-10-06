#include "stdafx.h"
#include "PngLoader.h"
#include <tchar.h>
#include <objidl.h>
#include <cstdio>
Gdiplus::Bitmap* LARPNG::LoadPngFromResource(HINSTANCE hInstance, LPCTSTR pszResourceName, LPCTSTR pszResourceType)
{
    HRSRC hResource = FindResource(hInstance, pszResourceName, pszResourceType);
    if (hResource == nullptr)
        return nullptr;
    DWORD imageSize = SizeofResource(hInstance, hResource);
    if (imageSize == 0)
        return nullptr;
    HGLOBAL hResourceData = LoadResource(hInstance, hResource);
    if (hResourceData == nullptr)
        return nullptr;
    const void* pResourceData = LockResource(hResourceData);
    if (pResourceData == nullptr)
        return nullptr;
    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (!hBuffer)
        return nullptr;
    void* pBuffer = GlobalLock(hBuffer);
    memcpy(pBuffer, pResourceData, imageSize);
    GlobalUnlock(hBuffer);
    IStream* pStream = nullptr;
    if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) != S_OK)
    {
        GlobalFree(hBuffer);
        return nullptr;
    }
    Bitmap* pBitmap = Bitmap::FromStream(pStream);
    pStream->Release();
    return pBitmap;
}
