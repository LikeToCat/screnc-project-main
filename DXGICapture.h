#pragma once
#ifdef TARGET_WIN10
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <d3dcompiler.h>
//渲染所需的顶点参数结构体
struct ScreenVertex {
	float posX, posY, posZ;    // 顶点位置
	float texU, texV;          // 纹理坐标
};

// 鼠标光标相关结构体
struct CursorInfo {
	bool visible;                // 光标是否可见
	POINT position;              // 光标在屏幕上的位置
	HCURSOR hCursor;             // 光标句柄
	CURSORINFO cursorInfo;       // 系统光标信息
	ICONINFO iconInfo;           // 光标图标信息
	int width;                   // 光标宽度
	int height;                  // 光标高度
	BYTE* shapeBuffer;           // 光标形状缓冲区
	DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo;  // 光标形状信息
};

class DXGICapture
{
public:
	DXGICapture(
		int AreaWidth,
		int AreaHeight,
		int frameRate,
		float captureLeft = 0.0f,    // 捕获区域左边界(归一化坐标)
		float captureTop = 0.0f,     // 捕获区域上边界(归一化坐标)
		float captureRight = 1.0f,   // 捕获区域右边界(归一化坐标)
		float captureBottom = 1.0f,  // 捕获区域下边界(归一化坐标)
		bool captureCursor = false    // 是否捕获鼠标
	); 
	~DXGICapture();
	//主要接口
	virtual BOOL CaptureImage(char*& pImageData, int* nLen);//获取一帧图像
	BOOL StartCaptureThread();//启动捕获专用线程
	void StopCaptureThread();//关闭捕获专用线程
	bool IsCaptureThreadRunning()const { return m_ThreadRunning; }
	BOOL UpdateCaptureRegion(
		int captureLeft,    // 新的捕获区域左边界(归一化坐标)
		int captureTop,     // 新的捕获区域上边界(归一化坐标)
		int captureWidth,   // 新的捕获区域右边界(归一化坐标)
		int captureHeight); //更新捕获区域

	void SetCaptureCursor(bool capture);	// 设置是否捕获鼠标
	bool IsCapturingCursor() const { return m_captureCursor; }	        // 获取是否捕获鼠标
private:
	void initDup();//初始化桌面复制接口
	void DelInitDup();//释放所有资源
	BOOL QueryFrame();//获取一帧数据并处理
	BOOL AttachToThread();//绑定每个桌面的捕获线程 
	BOOL InitializeRenderResources(
		float sourceLeft = 0.0f,   // 源纹理的左边界(0-1)
		float sourceTop = 0.0f,    // 源纹理的上边界(0-1)
		float sourceRight = 1.0f,  // 源纹理的右边界(0-1)
		float sourceBottom = 1.0f  // 源纹理的下边界(0-1))
	);//渲染资源的初始化

	// 鼠标光标捕获相关函数
	BOOL ProcessCursorInfo(DXGI_OUTDUPL_FRAME_INFO& frameInfo);   // 处理光标信息
	BOOL DrawCursorOnFrame(char* buffer, int width, int height);  // 在帧上绘制光标
private:
	bool m_isInitSuccees;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_deviceContext;
	D3D11_TEXTURE2D_DESC m_frameTextureDesc;
	ID3D11Texture2D* m_stageTexture;
	IDXGIOutputDuplication* m_Outputdup;
	DXGI_OUTPUT_DESC m_Outputdesc;
	IDXGISurface* m_dxgiSurface;
	bool m_IsAttach;
	unsigned char* m_pImageData;
	int m_maxWidth;
	int m_maxHeight;
	int m_frameDurationMs;//捕获每一帧后等待的时间

	//捕获区域
	float m_captureLeft;
	float m_captureTop;
	float m_captureRight;
	float m_captureBottom;

	//线程控制
	std::thread m_CaptureThread;
	std::atomic<bool> m_ThreadRunning;
	std::mutex m_CaptureMutex;
	std::condition_variable m_CaptureCV;

	//捕获缓冲区
	char* m_Capturebuffer;
	int m_CaptureSize;
	bool m_newFrameAvailable;

	//渲染资源
	ID3D11Texture2D* m_sourceTexture;//全分辨率源纹理
	ID3D11RenderTargetView* m_renderTargetView;//GPU渲染目标内存视图
	ID3D11ShaderResourceView* m_shaderResourceView;//GPU着色器参考的数据内存视图
	ID3D11SamplerState* m_samplerState;//GPU着色器参考数据时的采样指南
	ID3D11VertexShader* m_vertexShader;//顶点着色器
	ID3D11PixelShader* m_pixelShader;//像素着色器
	ID3D11Buffer* m_vertexBuffer;//顶点数据缓冲区
	ID3D11InputLayout* m_inputLayout;//解释顶点缓冲区中顶点的数据的解释器
	ID3D11Texture2D* m_targetTexture;  // 渲染目标纹理

	// 鼠标光标相关成员
	bool m_captureCursor;          // 是否捕获鼠标
	CursorInfo m_cursor;           // 光标信息
	BYTE* m_cursorShapeBuffer;     // 光标形状缓冲区
	UINT m_cursorShapeBufferSize;  // 光标形状缓冲区大小
	bool m_cursorShapeChanged;     // 光标形状是否改变
};
#endif // TARGET_WIN10