// CLarEdit.h
#pragma once

#include <afxwin.h>
#include <memory>
#include <gdiplus.h>
#pragma comment ( lib, "Gdiplus.lib")
// 用来表示字体样式的位掩码
enum FontStyle {
	FS_NORMAL = 0x0,
	FS_BOLD = 0x1,
	FS_UNDERLINE = 0x2,
	FS_ITALIC = 0x4
};

class CLarEdit : public CEdit
{
public:
	CLarEdit();
	//接口：设置文本字体接口（包括字体家族、大小、样式和字体颜色）
	void LarSetTextFont(
		const CString fontFamily = _T("微软雅黑"),
		int fontSize = 22,
		int fontStyle = FS_NORMAL);
	//接口：设置边框颜色，需要手动在OnPaint函数中调用LarDrawRectBorder才会有绘画出的效果
	void LarSetBorderColor(Gdiplus::Color color);
	//接口：设置边框厚度, 需要手动在OnPaint函数中调用LarDrawRectBorder才会有绘画出的效果
	void LarSetBorderThickness(double thickness);
	//接口：设置占位文本, 需要手动在OnPaint函数中调用LarDrawRectBorder才会有绘画出的效果
	void LarSetPlaceHolderText(const CString placeHolderText, Gdiplus::Color color,bool SurePlace);
	//接口：设置边框扩大系数，用于调整边框于文字的等比例间距,需要手动在OnPaint函数中调用LarDrawRectBorder才会有绘画出的效果
	void LarSetBorderExpansion(double BorderExpansion);
	//接口(在OnPaint函数中调用)：绘画编辑框的边框在OnPaint函数中调用，启用Lar接口设置的控件效果
	void LarDrawRectBorder();
	//接口：设置控件只读
	void LarSetReadOnly(BOOL isReadOnly);
	//杰克：设置文本颜色
	void SetTextColor(COLORREF color) { m_textColor = color; Invalidate(); }
protected:////辅助函数
	void UpdateControlFont(const CString& fontFamily, int fontSize, int fontStyle);	//更新字体
	void DrawPlaceHolderText(Gdiplus::Rect gdipRect, Gdiplus::Graphics* graphics);	//绘画占位文本
protected://消息处理于虚函数
	afx_msg void OnPaint();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow(CREATESTRUCT& cs);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
private://成员变量
	CFont m_font;						//字体样式
	Gdiplus::Color m_borderColor;		//用于指定边框颜色
	double m_borderThickness;			//边框厚度
	CString m_placeHolderText;			//占位文本
	bool m_IsEditing;					//正在被编辑
	int m_fontSize;						//文本大小
	CString m_fontFamily;				//文本字体家族
	Gdiplus::Color m_PlaceTextColor;	//设置占位文本颜色
	double m_BorderExpansion;			//设置边框增大系数
	double m_Scale;						//缩放系数
	bool m_SurePlace;					//设置的占位文本可以直接开始编辑 
	bool m_IsReadOnly;					//是否为只读文本

	//编辑字体相关
	COLORREF m_textColor;
	CBrush m_brush;
public:

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};