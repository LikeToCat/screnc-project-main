#pragma once
#include <afxwin.h>
#include <gdiplus.h>
#include <memory>
#include <vector>
#include <map>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using std::make_unique;
class CLarBtn : public CButton
{
public:
	CLarBtn();
	virtual ~CLarBtn();
	//枚举值接口
public:
	//新增：用于导入的不同的按钮位图尺寸，以适配用户DPI的枚举值
	enum class BitmapSize {
		SIZE_32x32,//适用于96DPI，用于指定不同的大小的导入的图片
		SIZE_40x40,//适用于120DPI，用于指定不同的大小的导入的图片
		SIZE_48x48,//适用于144DPI，用于指定不同的大小的导入的图片
		SIZE_56x56,//适用于168DPI，用于指定不同的大小的导入的图片
		SIZE_64x64 //适用于192DPI，用于指定不同的大小的导入的图片
	};
	//新增：用于指定的按钮图片将以何种方式显示
	enum class BitmapShowMode
	{
		SHOW_ORIGINAL,//显示原生按钮的图片所有像素
		SHOW_TRANSPARENT_BYERASEBLACK,//显示去除掉黑色像素的按钮的图片
		SHOW_TRANSPARENT_BYERASEWHITE//显示去除掉白色像素的按钮的图片
	};
	//指定缩略图在左边还是在右边
	enum class NailImageLayout
	{
		Left,
		Right
	};
	// 接口：设置当前父窗口对应位置的背景(实现全透明),该函数只能被调用一次，调用一次以上将再调用将不会有任何执行
	void LarSetParentBk();
	/* 接口：将图片进行全透明处理后设置为按钮图片。
	首先判断当前对象是否有设置背景位图，如果是，则剔除传入位图黑色像素或白色像素并与当前按钮背景进行合体，并设置合体后的新的位图。
	如果否，则直到对象被设置背景位图前，都不会执行是的逻辑。(不建议使用此函数，使用LarSetAdaptDpiBitmaps)*/
	void LarSetButtonTransparentBitmapFromResource(UINT nResourceID, int mode);
	// 新增：接口：设置适应于各种不同DPI的按钮显示图片，调用后，对象会自动根据系统DPI显示合适图片，并指定是否全透明显示
	void LarSetAdaptDpiBitmaps(const std::vector<UINT> resourceIDs, CLarBtn::BitmapShowMode mode);
	// 接口：设置悬停时绘画画刷
	void LarSetHoverFillBrush(const SolidBrush& brush);
	// 接口：设置正常时的背景画刷
	void LarSetNormalFiilBrush(const SolidBrush& brush);
	// 接口：设置按下时绘画画刷
	void LarSetClickedFillBrush(const SolidBrush& brush);
	// 接口：设置按钮为静态文本框控件
	void LarSetBtnIsStatic(bool IsStatic);
	// 接口：设置按钮圆角
	void LaSetAngleRounded(double radius);
	// 接口：设置字体家族，例如 "微软雅黑", "Arial" 等
	void LarSetTextFamily(const CString& fontFamily);
	// 接口：设置字体大小，例如 22
	void LarSetTextSize(int fontSize);
	// 接口：设置字体样式，例如加粗、下划线、斜体
	void LarSetTextStyle(bool bold, bool underline, bool italic);
	// 接口：设置字体颜色
	void LaSetTextColor(const Gdiplus::Color& color);
	// 接口：设置字体悬停时颜色
	void LaSetTextHoverColor(const Gdiplus::Color& color);
	// 接口：设置字体按下时颜色
	void LaSetTextClickedColor(const Gdiplus::Color& color);
	// 接口：设置按钮禁用或启用按钮
	void LarSetBtnEnable(bool isEnable);
	// 接口：设置按钮边框
	void LarSetBorderColor(const Gdiplus::Color& color);
	// 接口：是否移除背景
	void LarSetEraseBkEnable(bool isEnable);
	// 接口：设置是否取消文本居中
	void LarSetTextCenter(bool isCenter = true);
	// 即可：获得再次调用LarSetParentBk一次次数
	void LarSetParentBkCalled();
	// 接口：设置无互动效果，互动效果变成鼠标移入客户区改变鼠标形状
	void LarSetButtonNoInteraction(bool enable);
	// 接口：设置按钮边框大小 
	void LarsetBorderSize(int size);
	// 接口：设置按钮缩略图
	void LarSetBtnNailImage(
		UINT nResourceID, NailImageLayout nailLayout,
		int imageWidth, int imageHeight, int diffx = 0, int diffy = 0,
		float hoverBrightNess = 1.2f, float clickBrightNess = 1.3f
	);
	// 接口：设置按钮缩略图（悬停状态）
	void LarSetBtnHoverNailImage(
		UINT nResourceID, NailImageLayout nailLayout,
		int imageWidth, int imageHeight, int diffx = 0, int diffy = 0,
		float hoverBrightNess = 1.2f, float clickBrightNess = 1.3f
	);
	//接口：设置文字显示位置偏移量
	void LarAdjustTextDisplayPos(int diffx, int diffy);
	//接口：设置按钮渐变色
	void LarSetGradualColor(Gdiplus::Color c1, Gdiplus::Color c2, Gdiplus::LinearGradientMode gcMode);
	//接口：设置按钮渐变色
	void LarSetHoverGradualColor(Gdiplus::Color c1, Gdiplus::Color c2, Gdiplus::LinearGradientMode gcMode);
	//接口：设置按钮渐变色
	void LarSetClickGradualColor(Gdiplus::Color c1, Gdiplus::Color c2, Gdiplus::LinearGradientMode gcMode);
	//接口：设置按钮文本是否自动换行
	void LarSetBtnTextMultLine(bool isMultline = true);
	//接口：设置最大文本宽度
	void LarSetTextMaxWidth(int width);
	//接口：设置是否绘画按钮边框
	void LarSetBtnBorderEnable(bool isEnable);
	//接口：设置按钮整体不透明度（0-255，0为完全透明，255为不透明）
	void LarSetOpacity(BYTE alpha);
protected:
	//消息处理
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
	//重写虚函数
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnEraseBkgnd(CDC* pDC);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
private:
	//辅助函数
	void FillWithTextureBrush(Gdiplus::Graphics& graphics, Gdiplus::TextureBrush& textureBrush,
		const CRect& rect, float opacity);
	void CreateRoundedRectanglePath(GraphicsPath& path, const CRect& rect, int radius);
	//初始化（更新）字体样式
	void Init_Update_Font();
	//新增：用于位图于背景位图的合并
	void EraseAndFitBitmapWithBkBitmap(Gdiplus::Bitmap* FittedBitmap, CLarBtn::BitmapShowMode mode);
	//在LarSetParentBk中调用的进行位图与背景位图合成的辅助函数
	void EraseAndFitBitmapWithBkBitmapCallInLarSetParentBk(Gdiplus::Bitmap* FittedBitmap);
	//从资源视图中加载位图
	Bitmap* LoadBitmapFromResource(UINT nResourceID);
	Color ApplyOpacity(const Color& c) const;

	//成员变量
	bool m_IsNoInteraction;				// 设置按钮无互动效果,效果变成鼠标移入客户区改变鼠标形状
	Gdiplus::Bitmap* m_BkBitmap;		// 背景位图
	RECT bitmapRect;					// 背景位图的rect参数
	bool m_bIsHovered;					// 鼠标是否悬停
	bool m_bIsPressed;					// 鼠标是否按下
	bool m_IsStatic;					// 按钮是否为不会有任何响应的静态按钮
	double m_DpiScale;					// DPI系数
	SolidBrush* m_HoverSolidBrush;		// 处理悬停时的绘画画刷
	SolidBrush* m_ClickedSolidBrush;	// 处理按下时的绘画画刷
	SolidBrush* m_NormalSolidBrush;		// 处理正常时的背景画刷
	CString m_FontFamily;				// 当前字体家族
	int m_FontSize;						// 当前字体大小
	bool m_IsBold;						// 是否加粗
	bool m_IsUnderline;					// 是否有下划线
	bool m_IsItalic;					// 是否斜体
	CString m_bitmapPath;				// 按钮显示位图的资源路径
	int m_mode;							// 传入给按钮用做显示位图时，指定剔除黑色像素还是白色像素的变量
	CFont m_Font;						// 字体对象
	Gdiplus::Color m_TextColor;			// 文本颜色
	bool m_IsAngleRounded;				// 设置按钮是否为圆角按钮
	double m_AngleRoundedRadius;		// 设置绘制圆角参数(用于绘制圆角)
	GraphicsPath m_AngleRoundedpath;	// 绘制圆角时所用路径参数
	Gdiplus::Color m_Textcolor_Hover;	// 悬停时字体颜色
	Gdiplus::Color m_Textcolor_Click;	// 按下时字体颜色
	std::map<BitmapSize, std::unique_ptr<Gdiplus::Bitmap>> m_Bitmaps;//存储适应各种不同DPI的图片
	bool m_IsBitmapsEmpty;				// 判断std::map<BitmapSize, std::unique_ptr<Gdiplus::Bitmap>> m_Bitmaps容器是否为空
	BitmapShowMode m_showMode;				// 指定外部设置的按钮位图的显示模式
	bool m_IsSetParentBkCalled;				// 是否调用过SetParentBk
	bool m_IsBtnEnable;						// 是否启用了按钮
	Color m_TextColor_BeforeBanned;			// 禁用按钮之前文本的显示颜色
	Color m_BorderColor;					// 边框颜色
	bool m_IsEraseBkEnable;					// 是否移除背景
	bool m_IsTextCenter;					// 文本是否居中
	UINT m_nResourceID = 0;					// 保存资源ID（选用）
	int m_BorderSize;
	Gdiplus::Bitmap* m_Bitmap_NailImage;	// 缩略图资源
	Gdiplus::Rect m_Rect_NailImage;			// 缩略图显示位置
	Gdiplus::Bitmap* m_Bitmap_HoverNailImage;		// 缩略图资源
	Gdiplus::Rect m_Rect_HoverNailImage;			// 缩略图显示位置
	int m_Int_TextDiffX;					// 按钮文字x偏移
	int m_Int_TextDiffY;					// 按钮文字y偏移
	float m_Int_NailImageHoverBrn;		    // 缩略图悬停颜色
	float m_Int_NailImageClickBrn;		    // 缩略图点击颜色
	bool m_bool_IsGradualBtn = false;				// 按钮背景是否为渐变色
	bool m_bool_IsMultline = false;					// 按钮文本是否自动换行
	Gdiplus::Color m_Color_gc1;						// 正常：渐变底色1
	Gdiplus::Color m_Color_gc2;						// 正常：渐变底色2
	Gdiplus::LinearGradientMode m_enum_gcMode;		// 正常：渐变模式
	Gdiplus::Color m_Color_hoverGc1;				// 悬停：渐变底色1
	Gdiplus::Color m_Color_hoverGc2;				// 悬停：渐变底色2
	Gdiplus::LinearGradientMode m_enum_hoverGcMode;	// 悬停：渐变模式
	Gdiplus::Color m_Color_clickGc1;				// 点击：渐变底色1
	Gdiplus::Color m_Color_clickGc2;				// 点击：渐变底色2
	Gdiplus::LinearGradientMode m_enum_clickGcMode;	// 点击：渐变模式
	int m_int_textMaxWidth;					// 文本最大宽度
	bool m_bool_EnableBorderDraw;			// 是否绘画按钮边框
	BYTE m_OpacityAlpha = 255;
};