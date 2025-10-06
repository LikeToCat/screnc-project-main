#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h> 
#include <vector>

// 渐变纹理缓存结构
typedef struct {
	SDL_Texture* texture;
	int width;
	SDL_Color startColor;
	SDL_Color endColor;
	Uint32 lastUpdateTick;
} GradientTexture;

// 全局渐变纹理缓存
static GradientTexture gradientCache = { NULL, 0, {0,0,0,0}, {0,0,0,0}, 0 };


namespace COMAPI
{
	namespace SDL
	{
		/// <summary>
		/// 初始化分层透明窗口所需资源。
		/// 原理：
		/// 1. 创建一个 ARGB8888 格式的可作为渲染目标的 SDL_Texture，用于离屏渲染带透明通道的帧内容；
		/// 2. 通过 SDL_GetWindowWMInfo 获取底层 Win32 HWND，将窗口样式设置为 WS_EX_LAYERED | WS_EX_TOPMOST；
		/// 3. 调整窗口始终置顶并允许分层；
		/// 4. 为像素缓冲区 pixBuffer 分配 textureWidth * textureHeight 大小，用于后续 ReadPixels 操作。
		/// 调用规范：
		/// - 必须在创建 SDL_Window 和 SDL_Renderer 之后、开始帧渲染前调用此函数；
		/// - render 与 window 必须对应同一个 SDL_Window；
		/// - argbTexture 和 pixBuffer 在返回后即可用于后续每帧渲染与拷贝；
		/// - hWnd 返回的 HWND 同样作为后续 UpdateArgbFrameToWindow 的目标窗口句柄。
		/// </summary>
		bool InitWindowOpacity(
			_In_ SDL_Renderer* render,
			_In_ SDL_Window* window,
			_Out_ SDL_Texture** argbTexture,
			_In_ int textureWidth,
			_In_ int textureHeight,
			_Out_ std::vector<Uint32>* pixBuffer,
			_Out_ HWND* hWnd
		);

		/// <summary>
		/// 1. 调用 SDL_RenderReadPixels 从 argbTexture 中读回当前帧 ARGB 像素到 pixBuffer；
		/// 2. 使用 Win32 GDI 创建 DIBSection，并 memcpy 将像素数据写入；
		/// 3. 根据 SDL_Window* window 获取窗口在屏幕上的左上角坐标 ptDst，
		///    并调用 UpdateLayeredWindow 以 ptDst 和 opacity 全局透明度更新分层窗口。
		/// 调用规范：
		/// - 必须先调用 InitWindowOpacity 完成资源初始化；
		/// - 每帧渲染完毕（SDL_SetRenderTarget／SDL_RenderCopy）后使用此接口；
		/// - window、render、argbTexture、pixBuffer 均需与 Init 时保持一致；
		/// - opacity 范围 [0,255]，0 完全透明，255 完全不透明；
		/// - 不需调用 SDL_RenderPresent，分层更新由 UpdateLayeredWindow 直接输出到桌面。
		/// </summary>
		bool UpdateArgbFrameToWindow(
			_In_ SDL_Window* window,
			_In_ SDL_Renderer* render,
			_In_ SDL_Texture* argbTexture,
			_In_ std::vector<Uint32>* pixBuffer,
			_In_ HWND* hWnd,
			_In_ int Opciality
		);

		/// <summary>
		/// 加载图片资源，传入图片路径，然后将传入的纹理加载上图片
		/// </summary>
		bool ImageLoad(_In_ SDL_Renderer* renderer, _In_ CString ImagePath, _Out_ SDL_Texture** Texture);

		/// <summary>
		/// 加载文字纹理
		/// </summary>
		bool TextLoad(
			_In_ SDL_Renderer* renderer,
			_In_ TTF_Font* ttfFont,
			_In_ std::wstring fontText,
			_In_ SDL_Color textColor,
			_Out_ SDL_Texture** textTexture,
			_Out_ int* TextOriWidth,
			_Out_ int* TextOriHeight
		);

		/// <summary>
		/// 加载字体
		/// </summary>
		 bool TTFFontLoad(_Out_ TTF_Font** font,_In_ wchar_t* fontfamily, _In_ int fontsize);

		 /// <summary>
		/// 从HWND句柄，创建背景位图(可以指定亮度)
		/// </summary>
		 bool CreateBackgroundFromWindow(
			 SDL_Renderer* renderer,
			 SDL_Texture** bkTexture,
			 HWND hwnd,
			 float brightnessFactor
		 );
	}
	namespace SDL_RenderAPI
	{
		void renderHorizontalGradient(
			SDL_Renderer* renderer, SDL_Rect rect,
			SDL_Color startColor, SDL_Color endColor,
			int roundedRadius = 0
		);
	}
	namespace MFC
	{
		/// <summary>
		/// 绘画边框的选中效果，具体为绘画一个对勾
		/// </summary>
		bool DrawSelectGouByBorderRect(_In_ Gdiplus::Graphics* memGraphics,
			_In_ int w, _In_ int h,
			_In_ int x, _In_ int y,
			_In_ float scale
		);
		
		/// <summary>
		/// 暂时性的显示窗口置顶
		/// </summary>
		bool SetWindowShowOnTop(_In_ HWND Hwnd);
	}
}

