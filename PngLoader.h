#pragma once
#include <afxwin.h>
#include <Gdiplus.h>
#pragma comment ( lib, "Gdiplus.lib")
using namespace Gdiplus;
namespace LARPNG 
{
	// 从资源中加载 PNG 图片
// hInstance：应用实例句柄
// pszResourceName：资源名称或 MAKEINTRESOURCE(ID)
// pszResourceType：资源类型，默认为 "PNG"
	Gdiplus::Bitmap* LoadPngFromResource(HINSTANCE hInstance, LPCTSTR pszResourceName, LPCTSTR pszResourceType = _T("PNG"));
}
