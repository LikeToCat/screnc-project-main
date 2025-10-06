// LazerStaticText.h
#pragma once
#include<afxwin.h>
// CLazerStaticText 类继承自 CStatic，用于自定义绘制静态文本
class CLazerStaticText : public CStatic
{
public:
	enum DrawMode
	{
		GdiplusHightQualityMode,
		NormalMode
	};
	CLazerStaticText();
	virtual ~CLazerStaticText();

	// **公共接口**
	// 设置字体家族，例如 "微软雅黑", "Arial" 等
	void LarSetTextFamily(LPCTSTR fontFamily);

	// 设置字体大小，例如 22
	void LarSetTextSize(int fontSize);

	// 设置字体样式，例如加粗、下划线、斜体
	// style 可以是组合的，如 FW_BOLD | TRUE (下划线)
	void LarSetTextStyle(bool bold, bool underline, bool italic);

	void LarSetBreakLine(bool isEnable);
	inline CString LarGetText() { return m_CustomText; }
	//设置字体颜色
	void LarSetTextColor(COLORREF colorRef);
	//设置文字内容
	void LarSetText(CString Text);
	void LarSetIsEraseBk(bool enable);
	void LarSetEraseColor(COLORREF colorRef);
	void LarSetStrikeOut(bool isStrikeOut);
	//设置文本布局模式 
	void LarSetTextCenter();
	void LarSetTextLeft();
	void LarSetTextRight();
	//设置绘画模式
	void LarSetDrawMode(DrawMode drawMode);
	//设置监听父窗口的拖动状态
	void LarSetMonitorWindowMove(bool* Dirtymark);
	void LarSetTextAlpha(int alpha);
protected:
	CFont m_Font;           // 自定义字体对象
	CString m_FontFamily;   // 当前字体家族
	int m_FontSize;         // 当前字体大小
	int m_TextAlpha;
	bool m_Isleft;			// 是否左对齐
	bool m_IsRight;			// 是否右对齐
	bool m_IsCenter;		// 是否居中
	bool m_IsBold;          // 是否加粗
	bool m_IsUnderline;     // 是否有下划线
	bool m_IsItalic;        // 是否斜体
	bool m_IsBreakLine;		// 是否换行
	bool m_IsStrikeOut;		// 是否绘画删除线
	COLORREF m_TextColor;	// 字体颜色
	double m_DpiScale;		// 用户当前DPI
	CString m_CustomText;	// 用户当前文字
	bool m_IsEraseBk;		// 是否擦除背景
	COLORREF m_EraseColor;	// 擦除背景时所用颜色
	DrawMode m_DrawMode;	// 绘画mode
	bool* m_isInSizeMove;	// 父窗口是否正在被拖动(外部管理变量) 
	bool m_isMonitorWindowMove;	//是否监听父窗口的拖动状态

	// 重写绘制函数
	afx_msg void OnPaint();
	virtual BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
private:
	// 初始化字体，根据当前的字体设置创建 CFont 对象
	void InitFont();
	//辅助函数
	void DrawFullJustifiedText(CDC* pDC, const CString& text, const CRect& rect);
};