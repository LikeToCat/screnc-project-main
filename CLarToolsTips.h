#pragma once
#include <afxwin.h>
#include <gdiplus.h>
#include <chrono>
#include "WndShadow.h"
#pragma comment(lib, "gdiplus.lib")
class CLarToolsTips : public CWnd
{
    DECLARE_DYNAMIC(CLarToolsTips)

public:
    CLarToolsTips();
    virtual ~CLarToolsTips();

    //初始化相关
    BOOL LarCreate(CRect gWindowRect = CRect(0, 0, 100, 100), CWnd* pParentWnd = NULL);//创建窗口 
    
    //窗口样式设置
    void LarSetBorderColor(Gdiplus::Color borderColor);     //设置窗口边框颜色
    void LarSetBorderSize(int size);                        //设置边框大小
    void LarSetTextColor(Gdiplus::Color textColor);         //设置文本颜色
    void LarSetBkColor(Gdiplus::Color bkColor);             //设置背景颜色
    void LarSetTextSize(int fontSize);                      //设置文本大小
    void LarSetShadowEnable(bool IsEnableShadow);           //设置启用窗口阴影

    //窗口基本属性设置
    void LarSetTipText(const CString& text);                //设置提示文本
    void LarSetToolTipsDisplayTime(int millSeconds);        //设置提示文本持续时间（窗口显示时间,默认一直显示）

    //窗口控制
    void ShowToolTipsWindow(CRect gWindowRect);                                     //按传递的矩形参数来显示窗口  
    void ShowToolTipsWindow(int x, int y, int diffwidth = 0, int diffheight = 0);   //设定窗口显示位置，宽高调整系数（窗口大小自适应调整）
private:
    BOOL RegisterWindowClass(); //注册窗口（Create时调用）
    float GetUserDPI();         //获取用户DPI（构造函数初始化时调用）
    void InitToolWindow(bool IsMinimalWithMain);    //设置窗口样式为工具窗口
    //辅助绘画函数
    void DrawMyText(Gdiplus::Graphics* memGraphics);
protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
private:
    //窗口基本属性
    float m_Scale;      //窗口DPI系数
    CString m_Str_Tips; //窗口显示的提示文本
    std::chrono::system_clock::time_point m_TimePoint_Display;  //窗口显示时间
    bool m_bool_IsAlawysDisplay;        //是否一直显示
    CRect m_CRect_Window;               //窗口显示的矩形区域
    int m_WindowWidth;                  //窗口宽
    int m_WindowHeight;                 //窗口高
    int m_fontSize;                     //窗口显示提示文本的文本字体大小

    //窗口样式属性
    Gdiplus::Color m_Color_border;      //边框颜色
    Gdiplus::Color m_Color_textColor;   //文本颜色
    Gdiplus::Color m_Color_BkColor;     //窗口背景颜色
    int m_int_borderSize;               //边框大小
    bool m_IsDisplayShadow;             //是否显示窗口阴影

    //定时器
    UINT m_Uint_Timer;      //窗口消失定时器

    CWndShadow m_Shadow;    //窗口阴影框架
private:
    static const LPCTSTR m_lpszClassName;   //窗口类名
};