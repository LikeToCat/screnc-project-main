#pragma once
//OpenGL
#include "glad.h"

//SDL
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h> 

//C++
#include <functional>
#include <vector>
#include <map>
#include <unordered_map>

namespace COMCLASS
{
	namespace SDL
	{
		/// <summary>
		/// 区域滚动组件器
		/// </summary>
		class WindowScroller
		{
		public:
			WindowScroller(SDL_Rect scrollArea,int itemSize,int itemHeight);
			~WindowScroller();
			WindowScroller(const WindowScroller&) = delete;
			WindowScroller& operator=(WindowScroller&) = delete;

			//render call
			void EnterRender(SDL_Renderer* renderer);		//开始渲染滚动区域前调用
			void ExitRender(SDL_Renderer* renderer);		//渲染滚动区域完毕后调用

			//event call
			void UpdateScrollPhysics();						//在进入SDL事件处理前调用，更新滚动模型
			void UpdateMouseWheelEvent(SDL_Event* e);		//处理SDL鼠标滚轮事件调用

			//set and get
			void SetScrollSensitivity(float sensitivity);	//设置滚动灵敏度
			inline void GetVisiableIndex(int* firstVisiable, int* lastVisiable)
			{
				// 计算可见项范围
				int firstVisible = (int)(m_Float_CurrentScroll / m_itemHeight);
				*firstVisiable = max(0, firstVisible);

				// 计算可见项数量（多渲染一个确保平滑过渡）
				int visibleCount = (m_ScrollArea.h / m_itemHeight) + 2;
				*lastVisiable = min(firstVisible + visibleCount, m_itemSize);
			}//获取滚动区域渲染item的最小下标和最大下标
			inline int GetExactY(int index)
			{ 
				return m_ScrollArea.y + ((index) * (m_itemHeight)) - (int)m_Float_CurrentScroll;
			}//根据传入的Index,计算得出当前item的实际的Y坐标
		private:
			Uint32 m_Uint32_LastFrameTime;
			float m_Float_CurrentScroll;
			float m_Float_TargetScroll;
			float m_Float_ScrollVelocity;
			float m_Float_ScrollDamping;
			float m_Float_ScrollSensitivity;
			int m_itemSize;
			int m_itemHeight;
			SDL_Rect m_ScrollArea;
		};

		/// <summary>
		/// SDL窗口拖拽器
		/// </summary>
		class WindowDrag
		{
		public:
			WindowDrag();
			~WindowDrag();
			bool MoveActive(_In_ SDL_Window* Window);	//进入拖动状态
			bool MoveTo(_In_ SDL_Window* Window, _Out_ CRect* MovedArea = nullptr); //再拖动状态下调用，会移动窗口
			bool MoveInActive();	//退出拖动状态
		private:
			int m_DragOffsetX = -1;
			int m_DragOffsetY = -1;
			bool m_Bool_IsDragging = false;
		};

		/// <summary>
		/// SDL窗口管理器
		/// </summary>
		class WindowManager
		{
		public:
			static WindowManager* QueryComponent();
			static void ReleaseComponent();
			static void GlobalInit();
			bool AddWindow(_In_ void* windowIns, _In_ int uid);
			bool findWindow(_In_ int uid,_Out_ void** windowIns);
			bool ShowWindow(_In_ int uid);
			bool HideWindow(_In_ int uid);
			bool DestoryWindow(_In_ int uid);
			bool GetWindowCount(_Out_ int* count);
			bool GetWindowsMap(_Out_ std::map<int, void*>** map);
			bool findShowingWindow(_Out_ int* uid, _Out_ void** windowIns);
		private:
			std::map<int, void*> m_Map_WindowIns;
		private:
			static WindowManager* s_ins;
		};

		class GLTexManager;
		class VBOVAOShader;

		/// <summary>
		/// SDL窗口的OpenGL渲染技术
		/// </summary>
		class WindowOpenGL
		{
		public:
			//OpenGL上下文能力
			struct glCtxAbality
			{
				int glMajor = 2;                // OpenGL 上下文主版本号（Major Version）
				int glMinor = 1;                // OpenGL 上下文次版本号（Minor Version）
				int msaaSamples = 8;            // 多重采样（MSAA）样本数，0 表示关闭 MSAA
				int depthBits = 24;             // 深度缓冲区位深（Depth Buffer Bits）
				int stencilBits = 8;            // 模板缓冲区位深（Stencil Buffer Bits）
				bool sRgbFramebuffer = false;   // 是否启用 sRGB 帧缓冲，以获得正确的线性颜色空间
				bool doubleBuffer = true;		// 是否使用双缓冲（Double Buffering），提高渲染平滑度
			};

			//画质选项
			struct glCtxOption
			{
				bool msaaActive = true;						// 是否启用MSAA
				bool srgbBuffer = true;						// 是否启用sRGB帧缓冲获取正确的线性颜色空间	
				bool blend = true;							// 是否启用透明混合（Alpha Blending）
				GLenum blendSrc = GL_SRC_ALPHA;				// 混合源因子（Source Factor）
				GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;	// 混合目标因子（Destination Factor）
				bool lineSmooth = true;						// 是否启用线段平滑（GL_LINE_SMOOTH）
				bool pointSmooth = true;					// 是否启用点平滑（GL_POINT_SMOOTH）
				bool alphaToCoverage = false;				// 是否启用样本 alpha 覆盖（Alpha-to-Coverage）
				bool depthTest = false;						// 是否启用深度测试（Depth Test）
				bool stencilTest = false;					// 是否启用模板测试
				bool cullFace = false;						// 是否启用面剔除（Face Culling）
				GLenum cullMode = GL_BACK;					// 剔除哪一面（GL_BACK/GL_FRONT/GL_FRONT_AND_BACK）
			};

			//清屏色
			struct ClearColor
			{
				float r;
				float g;
				float b;
				float a;
			};

			WindowOpenGL();
			~WindowOpenGL();

			//关键接口
			bool CreateOpenGLSDLWindow(
				int windowwidth, int windowheight, 
				const glCtxAbality& qtySet = glCtxAbality(), const glCtxOption& dqtySet = glCtxOption()
			);							//创建OpenGL渲染管线的SDL窗口
			bool ClearFrame();			//清理当前渲染内容		
			void fillFrame();			//渲染一帧
			bool SwapFrameToWindow();	//将渲染内容推上去

			//Set方法
			void setglCtxOption(const glCtxOption& dqtySet);			//设置启用的渲染选项
			void setClearColor(float r, float g, float b, float a);		//设置清屏颜色
			void setViewport(int x, int y, int w, int h);				//设置OpenGL视口(响应窗口size变化时调用)
			void setGLTexManager(GLTexManager* texManager);				//设置GLTexManager资源管理器组件
			void setVBOVAOShader(VBOVAOShader* Shader);					//设置VBO VAO Shader管线渲染功能组件

			//绘制封装
			bool drawTexture(const std::string& key, float x, float y, float w, float h);//绘画图片UI

			//渲染回调设置
			void addOpenGLDrawCallBack(std::string cbkey, std::function<void()> drawCB);	//设置渲染逻辑回调
			void removeOpenGLDrawCallBack(std::string cbkey);			//删除渲染逻辑回调
			void removeAllOpenGLDrawCallBack();							//删除所有渲染回调

		private:
			bool initOpenGL(int windowwidth, int windowheight, 
				const glCtxAbality& qtySet, const glCtxOption& dqtySet
			);	//初始化SDL OpenGL
			bool JudgeCallLegal();	//判断接口调用是否合法

		private://内部所有权持有成员
			SDL_Window* m_Win = nullptr;		//SDL窗口
			SDL_GLContext m_glctx = nullptr;	//SDL OpenGL上下文
			bool m_IsInitialized = false;		//是否已经初始化成功
			glCtxAbality m_qtySet;				//当前渲染画质预设
			glCtxOption m_dqtySet;				//当前渲染动态画质预设
			std::map<std::string, std::function<void()>> m_fDrawCbs;	//绘图回调
			ClearColor m_sClearColor;			//清屏色

		private://外部所有权持有成员
			GLTexManager* m_ptexMgr = nullptr;		//SDL OpenGL的资源管理器组件
			VBOVAOShader* m_pShader = nullptr;		//VBO VAO Shader管线渲染功能组件
		};

		/// <summary>
		/// SDL窗口的OpenGL渲染技术的资源管理器(由WindowOpenGL所有)(GLTexManager的清理需要在WindowOpenGL之前)
		/// </summary>
		class GLTexManager
		{
		public:
			GLTexManager();
			~GLTexManager();

			bool load(const std::string& key, const std::string& path);//加载图片资源，并指定它的key
			bool unload(const std::string& key);	//卸载指定key对应的图片资源
			bool unloadAll();						//卸载所有的图片资源
			GLuint get(const std::string& key)const;//查找并返回key对应的图片tex
		private:
			std::unordered_map<std::string, GLuint> m_map;
		};

		/// <summary>
		/// SDL OpenGL的VBO VAO Shader管线渲染功能组件
		/// </summary>
		class VBOVAOShader
		{
		public:
			VBOVAOShader();
			~VBOVAOShader();

			//关键接口
			bool createShaderProgram(std::string sVSSouce, std::string m_sFSSouce);
			bool createVBOVAO(unsigned int* vbo, unsigned int* vao);

		private://内部调用
			void debugGLErrorMsg(wchar_t info[128], int shaderId);
		private:
			std::string m_sVertexShaderSource;		//顶点着色器glsl代码
			std::string m_sFragmentShaderSource;	//片段着色器glsl代码
		};
	}
}