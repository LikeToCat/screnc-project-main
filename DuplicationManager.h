#ifndef _DUPLICATIONMANAGER_H_
#define _DUPLICATIONMANAGER_H_
#include "CommonTypes.h"

class DUPLICATIONMANAGER
{
public:
	DUPLICATIONMANAGER();
	~DUPLICATIONMANAGER();
	DUPL_RETURN InitDup(ID3D11Device* device, UINT output);
	DUPL_RETURN GetMouse(_PTR_INFO* ptrInfo, DXGI_OUTDUPL_FRAME_INFO* frameInfo, INT OffsetX, INT OffsetY);
	DUPL_RETURN GetFrame(_Out_ _FRAME_DATA* data, _Out_ bool* timeout);
	DUPL_RETURN DoneWithFrame();
	void GetOutputDesc(_Out_ DXGI_OUTPUT_DESC* desc);
private:
	ID3D11Device* m_Device;			
	UINT m_OutputNum;								//当前显示器序号
	DXGI_OUTPUT_DESC m_descInfo;					//显示器详细信息
	IDXGIOutputDuplication* m_Interface_deskDup;	//桌面复制接口
	BYTE* m_MetaBuffer;								//移动矩形数据和脏数据存储的缓冲区
	UINT m_MetaBufferSize;							//移动矩形数据和脏数据大小
	ID3D11Texture2D* m_deskTexture;					//桌面纹理资源
};

#endif