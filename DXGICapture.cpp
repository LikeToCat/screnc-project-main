#include "stdafx.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;
#ifdef TARGET_WIN10
#include "DXGICapture.h"
#include <string>

DXGICapture::DXGICapture(
	int AreaWidth,
	int AreaHeight,
	int frameRate,
	float captureLeft,    // 捕获区域左边界(归一化坐标)
	float captureTop,     // 捕获区域上边界(归一化坐标)
	float captureRight,   // 捕获区域右边界(归一化坐标)
	float captureBottom,  // 捕获区域下边界(归一化坐标
	bool captureCursor	  // 是否捕获鼠标
)
{
	m_isInitSuccees = false;
	m_pDevice = nullptr;
	m_deviceContext = nullptr;
	RtlZeroMemory(&m_Outputdesc, sizeof(m_Outputdesc));
	m_Outputdup = nullptr;
	m_pImageData = new unsigned char[AreaWidth * AreaHeight * 4];
	m_IsAttach = false;
	m_maxWidth = AreaWidth;
	m_maxHeight = AreaHeight;
	m_ThreadRunning = false;
	m_CaptureSize = 0;
	m_newFrameAvailable = false;
	m_Capturebuffer = new char[AreaWidth * AreaHeight * 4];
	m_frameDurationMs = 1000 / frameRate;
	m_stageTexture = nullptr;
	m_dxgiSurface = nullptr;
	m_sourceTexture = nullptr;
	m_renderTargetView = nullptr;
	m_shaderResourceView = nullptr;
	m_samplerState = nullptr;
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_vertexBuffer = nullptr;
	m_inputLayout = nullptr;
	
	// 存储捕获区域参数
	m_captureLeft = captureLeft;
	m_captureTop = captureTop;
	m_captureRight = captureRight;
	m_captureBottom = captureBottom;

	// 初始化鼠标捕获相关成员
	m_captureCursor = captureCursor;
	m_cursorShapeBuffer = nullptr;
	m_cursorShapeBufferSize = 0;
	m_cursorShapeChanged = false;
	ZeroMemory(&m_cursor, sizeof(m_cursor));
	m_cursor.cursorInfo.cbSize = sizeof(CURSORINFO);
	m_cursor.visible = false;
	m_cursor.shapeBuffer = nullptr;
}

DXGICapture::~DXGICapture()
{
	//确保线程终止
	StopCaptureThread();

	//资源释放
	if (m_stageTexture) {
		m_stageTexture->Release();
		m_stageTexture = nullptr;
	}
	if (m_dxgiSurface) {
		m_dxgiSurface->Release();
		m_dxgiSurface = nullptr;
	}
	if (m_targetTexture) {
		m_targetTexture->Release();
		m_targetTexture = nullptr;
	}
	if (m_samplerState) {
		m_samplerState->Release();
		m_samplerState = nullptr;
	}
	if (m_vertexShader) {
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}
	if (m_pixelShader) {
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}
	if (m_vertexBuffer) {
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
	if (m_inputLayout) {
		m_inputLayout->Release();
		m_inputLayout = nullptr;
	}
	if (m_renderTargetView) {  
		m_renderTargetView->Release();
		m_renderTargetView = nullptr;
	}
	if (m_shaderResourceView) { 
		m_shaderResourceView->Release();
		m_shaderResourceView = nullptr;
	}
	if (m_sourceTexture) {     
		m_sourceTexture->Release();
		m_sourceTexture = nullptr;
	}
	if (m_pDevice) {
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
	if (m_deviceContext) {
		m_deviceContext->Release();
		m_deviceContext = nullptr;
	}
	if (m_pImageData) {
		delete m_pImageData;
		m_pImageData = nullptr;
	}
	if (m_Outputdup) {
		m_Outputdup->Release();
		m_Outputdup = nullptr;
	}
	if (m_Capturebuffer) {
		delete[] m_Capturebuffer;
		m_Capturebuffer = nullptr;
	}
	if (m_cursorShapeBuffer) {
		delete[] m_cursorShapeBuffer;
		m_cursorShapeBuffer = nullptr;
	}
	if (m_cursor.shapeBuffer) {
		delete[] m_cursor.shapeBuffer;
		m_cursor.shapeBuffer = nullptr;
	}
	if (m_Capturebuffer) {
		delete[] m_Capturebuffer;
		m_Capturebuffer = nullptr;
	}
}

BOOL DXGICapture::ProcessCursorInfo(DXGI_OUTDUPL_FRAME_INFO& frameInfo)
{
	if (!m_captureCursor || !m_Outputdup)
		return FALSE;

	// 更新光标位置信息
	if (frameInfo.LastMouseUpdateTime.QuadPart != 0)
	{
		m_cursor.position.x = frameInfo.PointerPosition.Position.x;
		m_cursor.position.y = frameInfo.PointerPosition.Position.y;
		m_cursor.visible = frameInfo.PointerPosition.Visible != 0;

		DEBUG_CONSOLE_FMT(ConsoleHandle, L"光标位置更新: X=%d, Y=%d, 可见=%d",
			m_cursor.position.x, m_cursor.position.y, m_cursor.visible);
	}

	// 更新光标形状信息
	if (frameInfo.PointerShapeBufferSize > 0 &&
		frameInfo.PointerShapeBufferSize != m_cursorShapeBufferSize)
	{
		// 重新分配光标形状缓冲区
		if (m_cursorShapeBuffer) {
			delete[] m_cursorShapeBuffer;
			m_cursorShapeBuffer = nullptr;
		}

		m_cursorShapeBuffer = new BYTE[frameInfo.PointerShapeBufferSize];
		m_cursorShapeBufferSize = frameInfo.PointerShapeBufferSize;
		m_cursorShapeChanged = true;

		// 获取光标形状信息
		UINT bufferSize;
		DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo;
		HRESULT hr = m_Outputdup->GetFramePointerShape(
			m_cursorShapeBufferSize,
			m_cursorShapeBuffer,
			&bufferSize,
			&shapeInfo);

		if (SUCCEEDED(hr)) {
			m_cursor.width = shapeInfo.Width;
			m_cursor.height = shapeInfo.Height;
			m_cursor.shapeInfo = shapeInfo;

			// 如果已有光标形状缓冲区，释放它
			if (m_cursor.shapeBuffer) {
				delete[] m_cursor.shapeBuffer;
			}

			// 创建新的光标形状缓冲区并复制数据
			m_cursor.shapeBuffer = new BYTE[bufferSize];
			memcpy(m_cursor.shapeBuffer, m_cursorShapeBuffer, bufferSize);

			DEBUG_CONSOLE_FMT(ConsoleHandle, L"光标形状更新: 宽度=%d, 高度=%d, 类型=%d",
				shapeInfo.Width, shapeInfo.Height, shapeInfo.Type);
		}
		else {
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"获取光标形状失败, hr=0x%08x", hr);
			m_cursorShapeChanged = false;
			return FALSE;
		}
	}

	return TRUE;
}

BOOL DXGICapture::DrawCursorOnFrame(char* buffer, int width, int height)
{
	if (!m_captureCursor || !m_cursor.visible || !buffer)
		return FALSE;

	// 如果光标不在捕获区域内，无需绘制
	int screenWidth = m_Outputdesc.DesktopCoordinates.right - m_Outputdesc.DesktopCoordinates.left;
	int screenHeight = m_Outputdesc.DesktopCoordinates.bottom - m_Outputdesc.DesktopCoordinates.top;

	// 计算光标在归一化坐标系中的位置
	float normalizedCursorX = static_cast<float>(m_cursor.position.x) / screenWidth;
	float normalizedCursorY = static_cast<float>(m_cursor.position.y) / screenHeight;

	// 检查光标是否在捕获区域内
	if (normalizedCursorX < m_captureLeft || normalizedCursorX > m_captureRight ||
		normalizedCursorY < m_captureTop || normalizedCursorY > m_captureBottom) {
		return FALSE;  // 光标在捕获区域外
	}

	// 计算光标在输出图像中的位置
	float relativeX = (normalizedCursorX - m_captureLeft) / (m_captureRight - m_captureLeft);
	float relativeY = (normalizedCursorY - m_captureTop) / (m_captureBottom - m_captureTop);

	int cursorX = static_cast<int>(relativeX * width);
	int cursorY = static_cast<int>(relativeY * height);

	// 绘制光标
	if (m_cursor.shapeBuffer && m_cursor.width > 0 && m_cursor.height > 0) {
		switch (m_cursor.shapeInfo.Type) {
		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
		{
			// 处理单色光标（黑白，带掩码）
			int maskHeight = m_cursor.height / 2;
			BYTE* andMask = m_cursor.shapeBuffer;
			BYTE* xorMask = m_cursor.shapeBuffer + ((m_cursor.width + 7) / 8) * maskHeight;

			// 绘制单色光标
			for (int y = 0; y < maskHeight && (cursorY + y) < height; y++) {
				for (int x = 0; x < m_cursor.width && (cursorX + x) < width; x++) {
					if (cursorX + x >= 0 && cursorY + y >= 0) {
						int byteIndex = y * ((m_cursor.width + 7) / 8) + x / 8;
						int bitIndex = 7 - (x % 8);

						BYTE andMaskBit = (andMask[byteIndex] >> bitIndex) & 1;
						BYTE xorMaskBit = (xorMask[byteIndex] >> bitIndex) & 1;

						// 计算目标像素索引
						int pixelIndex = ((cursorY + y) * width + (cursorX + x)) * 4;

						// 应用AND和XOR操作
						if (andMaskBit == 0 && xorMaskBit == 0) {
							// 黑色 (0, 0, 0)
							buffer[pixelIndex] = 0;       // B
							buffer[pixelIndex + 1] = 0;   // G
							buffer[pixelIndex + 2] = 0;   // R
							buffer[pixelIndex + 3] = 255; // A
						}
						else if (andMaskBit == 0 && xorMaskBit == 1) {
							// 白色 (255, 255, 255)
							buffer[pixelIndex] = 255;     // B
							buffer[pixelIndex + 1] = 255; // G
							buffer[pixelIndex + 2] = 255; // R
							buffer[pixelIndex + 3] = 255; // A
						}
						else if (andMaskBit == 1 && xorMaskBit == 1) {
							// 反色，这里简化为灰色
							buffer[pixelIndex] ^= 0xFF;     // B
							buffer[pixelIndex + 1] ^= 0xFF; // G
							buffer[pixelIndex + 2] ^= 0xFF; // R
						}
						// andMaskBit == 1 && xorMaskBit == 0 表示透明，不做处理
					}
				}
			}
		}
		break;

		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
		{
			// 处理彩色光标 (ARGB格式)
			BYTE* colorData = m_cursor.shapeBuffer;

			for (int y = 0; y < m_cursor.height && (cursorY + y) < height; y++) {
				for (int x = 0; x < m_cursor.width && (cursorX + x) < width; x++) {
					if (cursorX + x >= 0 && cursorY + y >= 0) {
						// 计算源像素索引
						int srcIndex = (y * m_cursor.width + x) * 4;

						// 读取BGRA值
						BYTE b = colorData[srcIndex];
						BYTE g = colorData[srcIndex + 1];
						BYTE r = colorData[srcIndex + 2];
						BYTE a = colorData[srcIndex + 3];

						if (a > 0) {  // 只处理非完全透明的像素
							// 计算目标像素索引
							int dstIndex = ((cursorY + y) * width + (cursorX + x)) * 4;

							// 根据alpha值混合颜色
							if (a == 255) {
								// 完全不透明
								buffer[dstIndex] = b;     // B
								buffer[dstIndex + 1] = g; // G
								buffer[dstIndex + 2] = r; // R
								buffer[dstIndex + 3] = a; // A
							}
							else {
								// Alpha混合
								float alpha = a / 255.0f;
								buffer[dstIndex] = (BYTE)(b * alpha + buffer[dstIndex] * (1 - alpha));     // B
								buffer[dstIndex + 1] = (BYTE)(g * alpha + buffer[dstIndex + 1] * (1 - alpha)); // G
								buffer[dstIndex + 2] = (BYTE)(r * alpha + buffer[dstIndex + 2] * (1 - alpha)); // R
							}
						}
					}
				}
			}
		}
		break;

		case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
		{
			// 处理带掩码的彩色光标
			BYTE* colorData = m_cursor.shapeBuffer;

			for (int y = 0; y < m_cursor.height && (cursorY + y) < height; y++) {
				for (int x = 0; x < m_cursor.width && (cursorX + x) < width; x++) {
					if (cursorX + x >= 0 && cursorY + y >= 0) {
						// 计算源像素索引
						int srcIndex = (y * m_cursor.width + x) * 4;

						// 前三个字节是BGR，第四个字节是掩码
						BYTE b = colorData[srcIndex];
						BYTE g = colorData[srcIndex + 1];
						BYTE r = colorData[srcIndex + 2];
						BYTE mask = colorData[srcIndex + 3]; // 1为透明，0为不透明

						if (mask == 0) {  // 不透明像素
							// 计算目标像素索引
							int dstIndex = ((cursorY + y) * width + (cursorX + x)) * 4;

							buffer[dstIndex] = b;     // B
							buffer[dstIndex + 1] = g; // G
							buffer[dstIndex + 2] = r; // R
							buffer[dstIndex + 3] = 255; // A (完全不透明)
						}
					}
				}
			}
		}
		break;

		default:
			DEBUG_CONSOLE_STR(ConsoleHandle, L"未知的光标类型");
			return FALSE;
		}

		return TRUE;
	}

	// 如果没有光标形状数据，则尝试使用系统API获取并绘制光标
	if (!GetCursorInfo(&m_cursor.cursorInfo)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"GetCursorInfo失败");
		return FALSE;
	}

	if (!(m_cursor.cursorInfo.flags & CURSOR_SHOWING)) {
		m_cursor.visible = false;
		return FALSE;  // 光标不可见
	}

	// 获取当前光标
	m_cursor.hCursor = m_cursor.cursorInfo.hCursor;
	if (!m_cursor.hCursor) {
		return FALSE;
	}

	// 获取光标图标信息
	if (!GetIconInfo(m_cursor.hCursor, &m_cursor.iconInfo)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"GetIconInfo失败");
		return FALSE;
	}
	// 释放资源
	if (m_cursor.iconInfo.hbmColor)
		DeleteObject(m_cursor.iconInfo.hbmColor);
	if (m_cursor.iconInfo.hbmMask)
		DeleteObject(m_cursor.iconInfo.hbmMask);

	return TRUE;
}

BOOL DXGICapture::CaptureImage(char*& pImageData, int* nLen)
{
	if (!m_ThreadRunning) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"捕获线程未开启");
		return false;
	}
	//等待新帧
	std::unique_lock<std::mutex> lock(m_CaptureMutex);
	if (!m_newFrameAvailable) {
		if (!m_CaptureCV.wait_for(lock, std::chrono::milliseconds(1000), [this] {
			return m_newFrameAvailable;
			})) {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"等待新帧超时");
			return FALSE;
		}
	}
	//有新帧可用，复制数据
	memcpy(pImageData, m_Capturebuffer, m_CaptureSize);
	*nLen = m_CaptureSize;
	m_newFrameAvailable = false;
	return TRUE;
}

void DXGICapture::initDup()
{
	//判断是否已经初始化过
	if (m_isInitSuccees) {
		return;
	}

	//准备驱动列表和功能列表
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,	//硬件驱动，硬件支持所有Direct3D功能
		D3D_DRIVER_TYPE_WARP,		//WARP驱动，这是一个搞性能软件光栅，这个软件光栅支持9_1到10.1的功能级别
		D3D_DRIVER_TYPE_REFERENCE	//参考驱动程序，是支持每个Direct3D功能的软件实现
	};
	unsigned int  numDriverTypes = ARRAYSIZE(driverTypes);	
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,		//Direct3D 11.0支持的目标功能，包括着色器模型5
		D3D_FEATURE_LEVEL_10_1,		//Direct3D 10.1支持的目标功能，包括着色器模型4
		D3D_FEATURE_LEVEL_10_0,		//Direct3D 10.0支持的目标功能，包括着色器模型4
		D3D_FEATURE_LEVEL_9_1		//目标功能[功能级别]（/ windows / desktop / direct3d11 / overviews-direct3d-11-devices-downlevel-intro）9.1支持，包括着色器模型2
	};
	unsigned int numFeatureLevels = ARRAYSIZE(featureLevels);

	//自动选择匹配用户电脑的驱动和功能
	HRESULT hr = S_OK;
	D3D_FEATURE_LEVEL featureLevel;
	for (int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		hr = D3D11CreateDevice(NULL, driverTypes[driverTypeIndex], NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, 
			featureLevels,numFeatureLevels, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_deviceContext);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"匹配不到对于的驱动和功能");
		return;
	}

	//创建DXGI设备
	IDXGIDevice* pDxgiDevice = NULL;
	hr = m_pDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDxgiDevice));
	if (FAILED(hr)){
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建DXGi设备失败");
		return;
	}
	//3.获取DXGI adapter
	IDXGIAdapter* pDxgiAdapter = NULL;
	hr = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDxgiAdapter));
	pDxgiDevice->Release();
	pDxgiDevice = nullptr;
	if (FAILED(hr)){
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建DXGI adapter设备失败");
		return;
	}

	//4.获取IDXGIOutput
	int nOutput = 0;
	IDXGIOutput* pDxgiOutput = NULL;
	hr = pDxgiAdapter->EnumOutputs(nOutput, &pDxgiOutput);
	pDxgiAdapter->Release();
	pDxgiAdapter = nullptr;

	//5.获取DXGI_OUTPUT_DESC 参数
	hr = pDxgiOutput->GetDesc(&m_Outputdesc);
	if (FAILED(hr)){
		DEBUG_CONSOLE_STR(ConsoleHandle, L"获取DXGI_OUTPUT_DESC 参数失败");
		return;
	}
	//6.获取IDXGIOutput1
	IDXGIOutput1* pDxgiOutput1 = NULL;
	hr = pDxgiOutput->QueryInterface(_uuidof(pDxgiOutput1), reinterpret_cast<void**>(&pDxgiOutput1));
	pDxgiOutput->Release();
	pDxgiOutput = nullptr;
	if (FAILED(hr)){
		DEBUG_CONSOLE_STR(ConsoleHandle, L"获取IDXGIOutput1 参数失败");
		return;
	}
	//7.创建复制桌面
	hr = pDxgiOutput1->DuplicateOutput(m_pDevice, &m_Outputdup);
	pDxgiOutput1->Release();
	pDxgiOutput1 = nullptr;
	if (FAILED(hr)){
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建复制桌面失败");
		return;
	}

	//创建源纹理 (原始分辨率)
	D3D11_TEXTURE2D_DESC sourceDesc = {};
	sourceDesc.Width = m_Outputdesc.DesktopCoordinates.right - m_Outputdesc.DesktopCoordinates.left;
	sourceDesc.Height = m_Outputdesc.DesktopCoordinates.bottom - m_Outputdesc.DesktopCoordinates.top;
	sourceDesc.MipLevels = 1;
	sourceDesc.ArraySize = 1;
	sourceDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sourceDesc.SampleDesc.Count = 1;
	sourceDesc.SampleDesc.Quality = 0;
	sourceDesc.Usage = D3D11_USAGE_DEFAULT;
	sourceDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	sourceDesc.CPUAccessFlags = 0;
	sourceDesc.MiscFlags = 0;
	hr = m_pDevice->CreateTexture2D(&sourceDesc, NULL, &m_sourceTexture);
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建源纹理失败");
		return;
	}

	D3D11_TEXTURE2D_DESC targetDesc = {};
	targetDesc.Width = m_maxWidth;
	targetDesc.Height = m_maxHeight;
	targetDesc.MipLevels = 1;
	targetDesc.ArraySize = 1;
	targetDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	targetDesc.SampleDesc.Count = 1;
	targetDesc.SampleDesc.Quality = 0;
	targetDesc.Usage = D3D11_USAGE_DEFAULT;
	targetDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	targetDesc.CPUAccessFlags = 0;
	targetDesc.MiscFlags = 0;

	hr = m_pDevice->CreateTexture2D(&targetDesc, NULL, &m_targetTexture);
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建渲染目标纹理失败");
		return;
	}

	// 创建渲染目标视图
	hr = m_pDevice->CreateRenderTargetView(m_targetTexture, NULL, &m_renderTargetView);
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建渲染目标视图失败");
		return;
	}
	
	//创建着色器资源视图
	hr = m_pDevice->CreateShaderResourceView(m_sourceTexture, NULL, &m_shaderResourceView);
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建着色器资源视图失败");
		return;
	}
	
	//预创建CPU可访问的纹理资源(目标分辨率)
	m_frameTextureDesc = {};
	m_frameTextureDesc.Width = m_maxWidth;
	m_frameTextureDesc.Height = m_maxHeight;
	m_frameTextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	m_frameTextureDesc.Usage = D3D11_USAGE_STAGING;
	m_frameTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	m_frameTextureDesc.BindFlags = 0;
	m_frameTextureDesc.MiscFlags = 0;
	m_frameTextureDesc.MipLevels = 1;
	m_frameTextureDesc.ArraySize = 1;
	m_frameTextureDesc.SampleDesc.Count = 1;
	m_frameTextureDesc.SampleDesc.Quality = 0;
	
	hr = m_pDevice->CreateTexture2D(&m_frameTextureDesc, NULL, &m_stageTexture);
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"纹理资源创建失败");
		return;
	}

	if (!InitializeRenderResources(m_captureLeft, m_captureTop, m_captureRight, m_captureBottom)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"渲染资源初始化失败");
		return;
	}
	m_isInitSuccees = true;
}

void DXGICapture::DelInitDup()
{
	if (!m_isInitSuccees) {
		return;
	}
	m_isInitSuccees = FALSE;

	// 释放所有渲染资源
	if (m_sourceTexture) {
		m_sourceTexture->Release();
		m_sourceTexture = nullptr;
	}
	if (m_targetTexture) {
		m_targetTexture->Release();
		m_targetTexture = nullptr;
	}
	if (m_renderTargetView) {
		m_renderTargetView->Release();
		m_renderTargetView = nullptr;
	}
	if (m_shaderResourceView) {
		m_shaderResourceView->Release();
		m_shaderResourceView = nullptr;
	}
	if (m_stageTexture) {
		m_stageTexture->Release();
		m_stageTexture = nullptr;
	}
	if (m_dxgiSurface) {
		m_dxgiSurface->Release();
		m_dxgiSurface = nullptr;
	}
	if (m_deviceContext) {
		m_deviceContext->Release();
		m_deviceContext = nullptr;
	}
	if (m_Outputdup) {
		m_Outputdup->Release();
		m_Outputdup = nullptr;
	}
	if (m_pDevice) {
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
}

BOOL DXGICapture::AttachToThread()
{
	//如果已经绑定线程则直接退出
	if (m_IsAttach)
	{
		return TRUE;
	}
	//获取桌面句柄,绑定给线程
	HDESK hCurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!hCurrentDesktop)
	{
		return FALSE;
	}
	BOOL isDesktopAttached = SetThreadDesktop(hCurrentDesktop);
	DWORD lastError = GetLastError();
	CloseDesktop(hCurrentDesktop);
	hCurrentDesktop = NULL;
	if (!isDesktopAttached){
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"SetThreadDesktop 失败，错误代码: %s" , std::to_wstring(lastError).c_str());
	}
	else{
		m_IsAttach = TRUE;
	}
	return isDesktopAttached;
}

BOOL DXGICapture::InitializeRenderResources(
	float sourceLeft,   // 源纹理的左边界(0-1)
	float sourceTop,    // 源纹理的上边界(0-1)
	float sourceRight,  // 源纹理的右边界(0-1)
	float sourceBottom  // 源纹理的下边界(0-1))
)
{
	HRESULT hr;

	// 顶点着色器，用于将顶点转换并传递纹理坐标
	const char* vertexShaderCode = R"(
        struct VS_INPUT {
            float4 Pos : POSITION;
            float2 Tex : TEXCOORD;
        };
        
        struct PS_INPUT {
            float4 Pos : SV_POSITION;
            float2 Tex : TEXCOORD;
        };
        
        PS_INPUT main(VS_INPUT input) {
            PS_INPUT output;
            output.Pos = input.Pos;
            output.Tex = input.Tex;
            return output;
        }
    )";

	//像素着色器,用于纹理进行采样
	const char* pixelShaderCode = R"(
		Texture2D tex : register(t0);
		SamplerState samplerState : register(s0);
		
		float4 main(float4 pos : SV_POSITION, float2 texCoord : TEXCOORD) : SV_Target {
		    return tex.Sample(samplerState, texCoord);
		}
	)";

	//编译顶点着色器
	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3DCompile(
		vertexShaderCode, strlen(vertexShaderCode),
		"VS", nullptr, nullptr, "main", "vs_4_0",
		0, 0, &vsBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"顶点着色器编译错误: %S",
				static_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		return FALSE;
	}

	//创建顶点着色器
	hr = m_pDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(hr)) {
		vsBlob->Release();
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建顶点着色器失败");
		return FALSE;
	}

	//创建输入布局
	D3D11_INPUT_ELEMENT_DESC layout[] = {
	  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = m_pDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);
	vsBlob->Release();
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建输入布局失败");
		return FALSE;
	}

	//编译像素着色器
	ID3DBlob* psBlob = nullptr;
	hr = D3DCompile(
		pixelShaderCode, strlen(pixelShaderCode),
		"PS", nullptr, nullptr, "main", "ps_4_0",
		0, 0, &psBlob, &errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) {
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"像素着色器编译错误: %S",
				static_cast<char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		return FALSE;
	}

	//创建像素着色器
	hr = m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &m_pixelShader);
	psBlob->Release();
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建像素着色器失败");
		return FALSE;
	}

	//创建采样器状态
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 线性过滤，平滑缩放
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_pDevice->CreateSamplerState(&sampDesc, &m_samplerState);
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建采样器状态失败");
		return FALSE;
	}

	// 使用传入的参数创建顶点缓冲区，指定选择区域的纹理坐标
	ScreenVertex vertices[] = {
		{ -1.0f, -1.0f, 0.0f, sourceLeft,  sourceBottom }, // 左下
		{ -1.0f,  1.0f, 0.0f, sourceLeft,  sourceTop },    // 左上
		{  1.0f, -1.0f, 0.0f, sourceRight, sourceBottom }, // 右下
		{  1.0f,  1.0f, 0.0f, sourceRight, sourceTop }     // 右上
	};
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(ScreenVertex) * 4;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	hr = m_pDevice->CreateBuffer(&bufferDesc, &initData, &m_vertexBuffer);
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建顶点缓冲区失败");
		return FALSE;
	}

	DEBUG_CONSOLE_STR(ConsoleHandle, L"渲染资源初始化成功");
	return TRUE;
}


BOOL DXGICapture::UpdateCaptureRegion(
	int captureX,      // 捕获区域左上角X坐标(像素)
	int captureY,      // 捕获区域左上角Y坐标(像素)
	int captureWidth,  // 捕获区域宽度(像素)
	int captureHeight) // 捕获区域高度(像素)
{
	// 获取互斥锁，确保与捕获线程同步
	std::lock_guard<std::mutex> lock(m_CaptureMutex);

	// 获取屏幕总分辨率(从已存储的输出描述中)
	int screenWidth = m_Outputdesc.DesktopCoordinates.right - m_Outputdesc.DesktopCoordinates.left;
	int screenHeight = m_Outputdesc.DesktopCoordinates.bottom - m_Outputdesc.DesktopCoordinates.top;

	// 确保参数有效
	if (screenWidth <= 0 || screenHeight <= 0) 
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"错误：屏幕分辨率信息无效");
		return FALSE;
	}

	// 计算捕获区域的右下角坐标
	int captureRight = captureX + captureWidth;
	int captureBottom = captureY + captureHeight;

	// 进行内部归一化转换
	float normalizedLeft = static_cast<float>(captureX) / screenWidth;
	float normalizedTop = static_cast<float>(captureY) / screenHeight;
	float normalizedRight = static_cast<float>(captureRight) / screenWidth;
	float normalizedBottom = static_cast<float>(captureBottom) / screenHeight;

	// 限制归一化坐标范围在0-1之间
	normalizedLeft = max(0.0f, min(normalizedLeft, 1.0f));
	normalizedTop = max(0.0f, min(normalizedTop, 1.0f));
	normalizedRight = max(0.0f, min(normalizedRight, 1.0f));
	normalizedBottom = max(0.0f, min(normalizedBottom, 1.0f));

	// 更新存储的捕获区域参数
	m_captureLeft = normalizedLeft;
	m_captureTop = normalizedTop;
	m_captureRight = normalizedRight;
	m_captureBottom = normalizedBottom;

	DEBUG_CONSOLE_FMT(ConsoleHandle, L"更新捕获区域：像素(%d,%d,%d,%d) -> 归一化(%.3f,%.3f,%.3f,%.3f)",
		captureX, captureY, captureWidth, captureHeight,
		normalizedLeft, normalizedTop, normalizedRight, normalizedBottom);

	// 如果渲染资源尚未初始化，则无需更新
	if (!m_isInitSuccees || !m_vertexBuffer) {
		return TRUE;  // 下次初始化时会使用新参数
	}

	// 更新顶点缓冲区中的纹理坐标
	ScreenVertex vertices[] = {
		{ -1.0f, -1.0f, 0.0f, normalizedLeft,  normalizedBottom }, // 左下
		{ -1.0f,  1.0f, 0.0f, normalizedLeft,  normalizedTop },    // 左上
		{  1.0f, -1.0f, 0.0f, normalizedRight, normalizedBottom }, // 右下
		{  1.0f,  1.0f, 0.0f, normalizedRight, normalizedTop }     // 右上
	};

	// 更新顶点缓冲区
	if (m_deviceContext && m_vertexBuffer) {
		// 先创建临时缓冲区
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(ScreenVertex) * 4;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = vertices;

		ID3D11Buffer* newVertexBuffer = nullptr;
		HRESULT hr = m_pDevice->CreateBuffer(&bufferDesc, &initData, &newVertexBuffer);
		if (FAILED(hr)) {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"更新顶点缓冲区失败");
			return FALSE;
		}

		// 替换旧的顶点缓冲区
		if (m_vertexBuffer) {
			m_vertexBuffer->Release();
		}
		m_vertexBuffer = newVertexBuffer;

		DEBUG_CONSOLE_STR(ConsoleHandle, L"捕获区域顶点缓冲区更新成功");
	}

	return TRUE;
}

void DXGICapture::SetCaptureCursor(bool capture)
{
	 m_captureCursor = capture; 
}

BOOL DXGICapture::QueryFrame()
{
	//首先判断是否初始化过并绑定过线程
	if (!m_isInitSuccees || !m_IsAttach) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"QueryFrame调用方式错误");
		return FALSE;
	}

	//获取一帧数据
	DXGI_OUTDUPL_FRAME_INFO frameInfo;
	IDXGIResource* desktopResourse = nullptr;
	HRESULT hr = m_Outputdup->AcquireNextFrame(500, &frameInfo, &desktopResourse);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
		return TRUE;
	}
	if (!desktopResourse)
	{
		return FALSE;
	}
	else if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"获取一帧数据失败");
		return FALSE;
	}

	// 处理光标信息
	if (m_captureCursor) {
		ProcessCursorInfo(frameInfo);
	}

	//获取ID3D11Texture2D接口
	ID3D11Texture2D* textureDesktop = nullptr;
	hr = desktopResourse->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&textureDesktop));
	if (FAILED(hr) || !textureDesktop) {
		desktopResourse->Release();
		m_Outputdup->ReleaseFrame();
		DEBUG_CONSOLE_STR(ConsoleHandle, L"获取ID3D11Texture2D接口失败");
		return FALSE;
	}

	//检查是否需要缩放
	int sourceWidth = m_Outputdesc.DesktopCoordinates.right - m_Outputdesc.DesktopCoordinates.left;
	int sourceHeight = m_Outputdesc.DesktopCoordinates.bottom - m_Outputdesc.DesktopCoordinates.top;
	bool needsScaling = (sourceWidth != m_maxWidth) || (sourceHeight != m_maxHeight);

	////GPU渲染准备工作
	if (needsScaling) {
		// 复制桌面帧到源纹理
		m_deviceContext->CopyResource(m_sourceTexture, textureDesktop);

		// 设置渲染管线状态
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

		// 设置视口
		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(m_maxWidth);
		viewport.Height = static_cast<float>(m_maxHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_deviceContext->RSSetViewports(1, &viewport);

		// 设置着色器及相关资源
		m_deviceContext->IASetInputLayout(m_inputLayout);
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		UINT stride = sizeof(ScreenVertex);
		UINT offset = 0;
		m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
		m_deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
		m_deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
		m_deviceContext->PSSetShaderResources(0, 1, &m_shaderResourceView);
		m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);

		// GPU渲染全屏四边形
		m_deviceContext->Draw(4, 0);

		// 解绑渲染目标和着色器资源
		ID3D11RenderTargetView* nullRTV = nullptr;
		m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
		ID3D11ShaderResourceView* nullSRV = nullptr;
		m_deviceContext->PSSetShaderResources(0, 1, &nullSRV);

		// 复制渲染结果到CPU可访问的暂存纹理
		m_deviceContext->CopyResource(m_stageTexture, m_targetTexture);
	}
	else {
		m_deviceContext->CopyResource(m_stageTexture, textureDesktop);
	}
	//释放桌面纹理
	textureDesktop->Release();

	//读取像素数据
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = m_deviceContext->Map(m_stageTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (FAILED(hr)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"读取像素数据失败");
		return FALSE;
	}

	//计算图像大小
	m_CaptureSize = m_maxWidth * m_maxHeight * 4;

	// 拷贝数据到缓冲区，考虑可能的行间距
	const BYTE* srcData = static_cast<const BYTE*>(mappedResource.pData);
	for (int y = 0; y < m_maxHeight; ++y) {
		memcpy(
			m_Capturebuffer + y * m_maxWidth * 4,
			srcData + y * mappedResource.RowPitch,
			m_maxWidth * 4
		);
	}
	m_deviceContext->Unmap(m_stageTexture, 0);

	// 在帧上绘制光标
	if (m_captureCursor) {
		DrawCursorOnFrame(m_Capturebuffer, m_maxWidth, m_maxHeight);
	}

	m_Outputdup->ReleaseFrame();
	return TRUE;
}

BOOL DXGICapture::StartCaptureThread()
{
	if (m_ThreadRunning) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"捕获线程正在进行，StartCaptureThread调用失败");
		return false;
	}
	m_ThreadRunning = true;
	m_CaptureThread = std::thread([this]()
		{
			//绑定桌面
			if (!this->AttachToThread()) {
				DEBUG_CONSOLE_STR(ConsoleHandle, L"桌面绑定失败");
			}
			this->initDup();
			while (m_ThreadRunning) {
				if (this->QueryFrame()) {
					//成功获取一帧
					std::lock_guard<std::mutex> lock(m_CaptureMutex);
					m_newFrameAvailable = true;
					m_CaptureCV.notify_one();
				}
				//根据帧率，限制捕获频率
				std::this_thread::sleep_for(std::chrono::milliseconds(m_frameDurationMs));
			}
		});
	return TRUE;
}

void DXGICapture::StopCaptureThread()
{
	if (!m_ThreadRunning) {
		return;
	}
	m_ThreadRunning = false;
	if (m_CaptureThread.joinable()) {
		m_CaptureThread.join();
	}
	this->DelInitDup();
	
	DEBUG_CONSOLE_STR(ConsoleHandle, L"屏幕捕获线程结束");
}
#endif