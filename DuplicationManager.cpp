#include "DuplicationManager.h"

DUPLICATIONMANAGER::DUPLICATIONMANAGER()
{
	m_Interface_deskDup = nullptr;
	m_Device = nullptr;
	m_MetaBuffer = nullptr;
	m_deskTexture = nullptr;
	m_OutputNum = 0;
	m_MetaBufferSize = 0;
	RtlZeroMemory(&m_descInfo, sizeof(m_descInfo));
}

DUPLICATIONMANAGER::~DUPLICATIONMANAGER()
{
	if (m_Interface_deskDup) {
		m_Interface_deskDup->Release();
		m_Interface_deskDup = nullptr;
	}
	if (m_MetaBuffer) {
		delete[]m_MetaBuffer;
		m_MetaBuffer = nullptr;
	}
	if (m_Device) {
		m_Device->Release();
		m_Device = nullptr;
	}
	if (m_deskTexture) {
		m_deskTexture->Release();
		m_deskTexture = nullptr;
	}
}

DUPL_RETURN DUPLICATIONMANAGER::InitDup(ID3D11Device* device, UINT output)
{
	//获取IDXGIDevice接口
	m_Device = device;
	m_OutputNum = output;
	m_Device->AddRef();//增加对象引用，防止使用过程中被外部释放
	IDXGIDevice* DXGIDv_device = nullptr;
	HRESULT hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DXGIDv_device));
	if (FAILED(hr)) {
		return ProcessFailure(nullptr, L"显卡接口获取失败", L"Error", hr);
	}

	//获取显卡接口
	IDXGIAdapter* DXGIAd_adapter = nullptr;
	hr = DXGIDv_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DXGIAd_adapter));
	DXGIDv_device->Release();
	DXGIDv_device = nullptr;
	if (FAILED(hr)){
		return ProcessFailure(nullptr, L"显卡接口获取失败", L"Error", hr);
	}

	//获取连接到显卡的显示器的接口，并将其接口扩展为IDXGIOutput1接口
	IDXGIOutput* DXGIOp_output = nullptr;
	IDXGIOutput1* DXGIOp1_output = nullptr;
	hr = DXGIAd_adapter->EnumOutputs(output, &DXGIOp_output);
	if (FAILED(hr)) {
		return ProcessFailure(nullptr, L"获取指定显示输出失败", L"Error", hr);
	}
	DXGIOp_output->GetDesc(&m_descInfo);//存储获取到的显示器详细信息
	hr = DXGIOp_output->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&DXGIOp1_output));
	DXGIOp_output->Release();
	DXGIOp_output = nullptr;
	if (FAILED(hr)){
		return ProcessFailure(nullptr, L"显示器接口扩展失败", L"Error", hr);
	}

	//获取桌面复制接口
	hr = DXGIOp1_output->DuplicateOutput(m_Device, &m_Interface_deskDup);
	DXGIOp1_output->Release();
	DXGIOp1_output = nullptr;
	if (FAILED(hr)){
		if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
			return ProcessFailure(nullptr, L"桌面复制接口被占用，获取失败", L"Error", hr);
		}
		return ProcessFailure(nullptr, L"桌面复制接口获取失败", L"Error", hr);
	}
	return DUPL_RETURN_SUCCESS;
}

DUPL_RETURN DUPLICATIONMANAGER::GetMouse(_PTR_INFO* ptrInfo, DXGI_OUTDUPL_FRAME_INFO* frameInfo, INT OffsetX, INT OffsetY)
{
	//判断当前捕获帧是否需要更新鼠标
	if (frameInfo->LastMouseUpdateTime.QuadPart == 0) {
		return DUPL_RETURN_SUCCESS;
	}
	bool isUpdateMouse = true;

	//排除多显示器时的特殊情况
	//排除当前显示器报告鼠标不可见，但最后更新位置的是另一个显示器
	if (!frameInfo->PointerPosition.Visible && ptrInfo->WhoUpdatedPositionLast != m_OutputNum)	{
		isUpdateMouse = false;
	}
	//两个显示器都报告鼠标可见，但已记录的时间戳更新（更近）
	if (frameInfo->PointerPosition.Visible && ptrInfo->Visible && ptrInfo->LastTimeStamp.QuadPart > frameInfo->LastMouseUpdateTime.QuadPart) {
		isUpdateMouse = false;
	}

	//更新鼠标的位置以及元信息
	if (isUpdateMouse) {
		ptrInfo->Position.x = frameInfo->PointerPosition.Position.x + m_descInfo.DesktopCoordinates.left- OffsetX;
		ptrInfo->Position.y = frameInfo->PointerPosition.Position.y + m_descInfo.DesktopCoordinates.top - OffsetY;
		ptrInfo->WhoUpdatedPositionLast = m_OutputNum;
		ptrInfo->LastTimeStamp = frameInfo->LastMouseUpdateTime;
		ptrInfo->Visible = frameInfo->PointerPosition.Visible != 0;
	}

	//判断是否需要更新鼠标的形状
	if (frameInfo->PointerShapeBufferSize == 0) {
		return DUPL_RETURN_SUCCESS;
	}

	//判断是否需要重新申请更大的形状数据内存缓冲区
	if (frameInfo->PointerShapeBufferSize > ptrInfo->BufferSize) {
		if (ptrInfo->PtrShapeBuffer) {
			delete[]ptrInfo->PtrShapeBuffer;
			ptrInfo->PtrShapeBuffer = nullptr;
		}
		ptrInfo->PtrShapeBuffer = new (std::nothrow) BYTE[frameInfo->PointerShapeBufferSize];
		ptrInfo->BufferSize = frameInfo->PointerShapeBufferSize;
	}

	//通过桌面复制接口API将鼠标形状数据赋值给形状数据缓冲区
	UINT realBufferSize;//实际写入的数据大小
	HRESULT hr = m_Interface_deskDup->GetFramePointerShape(frameInfo->PointerShapeBufferSize, reinterpret_cast<VOID*>(ptrInfo->PtrShapeBuffer),
		&realBufferSize, &(ptrInfo->ShapeInfo));
	if (FAILED(hr)) {
		delete[]ptrInfo->PtrShapeBuffer;
		ptrInfo->PtrShapeBuffer = nullptr;
		ptrInfo->BufferSize = 0;
		return ProcessFailure(nullptr, L"写入新的形状数据失败", L"Error", hr);
	}
	return DUPL_RETURN_SUCCESS;
}

DUPL_RETURN DUPLICATIONMANAGER::GetFrame(_Out_ _FRAME_DATA* data, _Out_ bool* timeout)
{
	//获取新的帧
	IDXGIResource* deskResourse = nullptr;
	DXGI_OUTDUPL_FRAME_INFO frameInfo;
	HRESULT hr =  m_Interface_deskDup->AcquireNextFrame(500, &frameInfo, &deskResourse);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
		*timeout = true;	//设置超时标志
		return DUPL_RETURN_SUCCESS;
	}
	*timeout = false;

	if (FAILED(hr)) {
		return ProcessFailure(nullptr, L"获取新的帧失败", L"Error", hr);
	}

	//资源接口转换
	if (m_deskTexture) {//释放上一帧的资源
		m_deskTexture->Release();
		m_deskTexture = nullptr;
	}
	hr = deskResourse->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_deskTexture));
	deskResourse->Release();
	deskResourse = nullptr;
	if (FAILED(hr)) {
		return ProcessFailure(nullptr, L"资源接口转换失败", L"Error", hr);
	}

	//获取移动矩形数据和脏数矩形数据
	if (frameInfo.TotalMetadataBufferSize) {
		if (frameInfo.TotalMetadataBufferSize > m_MetaBufferSize) {//如果需要扩大缓冲区大小
			if (m_MetaBuffer) {
				delete[] m_MetaBuffer;
				m_MetaBuffer = nullptr;
			}
			m_MetaBuffer = new (std::nothrow) BYTE[frameInfo.TotalMetadataBufferSize];
			if (!m_MetaBuffer) {
				m_MetaBufferSize = 0;
				data->DirtyCount = 0;
				data->MoveCount = 0;
				return ProcessFailure(nullptr, L"扩展缓冲区失败", L"Error", E_OUTOFMEMORY);
			}
			m_MetaBufferSize = frameInfo.TotalMetadataBufferSize;
		}
		
		UINT bufSize;
		bufSize = frameInfo.TotalMetadataBufferSize;
		hr = m_Interface_deskDup->GetFrameMoveRects(bufSize, (DXGI_OUTDUPL_MOVE_RECT*)m_MetaBuffer, &bufSize);
		if (FAILED(hr)) {
			data->DirtyCount = 0;
			data->MoveCount = 0;
			return ProcessFailure(nullptr, L"获取移动矩形数据失败", L"Error", hr);
		}
		data->MoveCount = bufSize / sizeof(DXGI_OUTDUPL_MOVE_RECT);
		BYTE* dirRect = m_MetaBuffer + bufSize;
		bufSize = frameInfo.TotalMetadataBufferSize - bufSize;

		hr = m_Interface_deskDup->GetFrameDirtyRects(bufSize, (RECT*)dirRect, &bufSize);
		if (FAILED(hr)) {
			data->DirtyCount = 0;
			data->MoveCount = 0;
			return ProcessFailure(nullptr, L"获取脏矩形数据失败", L"Error", hr);
		}

		//传出参数的赋值
		data->DirtyCount = bufSize / sizeof(RECT);
		data->MetaData = m_MetaBuffer;
	}
	data->FrameInfo = frameInfo;
	data->Frame = m_deskTexture;
	return DUPL_RETURN_SUCCESS;
}

DUPL_RETURN DUPLICATIONMANAGER::DoneWithFrame()
{
	HRESULT hr = m_Interface_deskDup->ReleaseFrame();
	if (FAILED(hr)) {
		return ProcessFailure(nullptr, L"释放帧失败", L"Error", hr);
	}
	if (m_deskTexture) {
		m_deskTexture->Release();
		m_deskTexture = nullptr;
	}
	return DUPL_RETURN_SUCCESS;
}

void DUPLICATIONMANAGER::GetOutputDesc(_Out_ DXGI_OUTPUT_DESC* desc)
{
	*desc = m_descInfo;
}


