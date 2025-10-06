#pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>
// 自定义居中文本的ListBox
class CLarListBoxCtrl : public CListBox
{
public:
    CLarListBoxCtrl();
    virtual ~CLarListBoxCtrl();
protected:
    DECLARE_MESSAGE_MAP()
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg void OnNcPaint();
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
public:
    //Get func
    inline int GetItemHeight() { return m_nItemHeight; }
    int GetMaxStrWidth(const CStringArray& ary);   //返回下拉框中最宽的文本宽度

    //下拉框相关
    void SetMaxDisplayItems(int8_t maxItems);
    void SetItemHeight(int nHeight);
    void SetTextColor(COLORREF color) { m_TextColor = color; }
    void SetBackColor(COLORREF color) { m_BackColor = color; }
    void SetSelectionColor(COLORREF color) { m_SelectionColor = color; }
    void SetHoverColor(COLORREF color) { m_HoverColor = color; }
    void SetBorderColor(COLORREF color) { m_BorderColor = color; }
    void SetFont(CFont* pFont, BOOL bRedraw = TRUE);
    void SetTransferMessageDstCwnd(CWnd* parent) { m_Cwnd_MessageTransferDst = parent; }
    void SetListBoxName(std::wstring strBoxName) { m_strBoxName = strBoxName; }
    void SetFontSize(int nPointSize);

    //下拉框滚动条相关
    void SetScrollbarColors(COLORREF bgColor, COLORREF thumbColor, COLORREF arrowColor);
    void SetScrollbarWidth(int width);
    void EnableCustomScrollbar(bool enable = true);

    //下拉框行为预设
    void SetLeaveAutoHide(bool isAutoLeaveHide);
private:
    void DrawCustomScrollbar(CDC* pDC);
    void UpdateScrollBar();
    float GetTextWidth(const CString& text);
private:
    std::wstring m_strBoxName;//下拉框标识
    CWnd* m_Cwnd_MessageTransferDst = nullptr;
    float m_Scale;
    float m_MaxStringWidth = 0;

    //下拉框相关
    int m_nItemHeight;
    COLORREF m_TextColor;
    COLORREF m_BackColor;
    COLORREF m_SelectionColor;
    COLORREF m_HoverColor;
    COLORREF m_BorderColor;
    CFont m_Font;
    bool m_bHasCustomFont;
    int m_nHoverIndex;          // 当前鼠标悬停的项目索引
    bool m_IsNullParent = false;
    CStringArray m_Array_ListboxItems;
    bool m_bool_IsVScroll = false;
    int8_t m_int8_MaxDisplayItem = -1;//默认
    int m_textSize;
    Gdiplus::Font* m_GdiFont;

    //下拉框滚动条相关
    COLORREF m_ScrollbarBgColor = RGB(36, 37, 40);      // 滚动条背景颜色
    COLORREF m_ScrollbarThumbColor = RGB(88, 89, 91);   // 滚动条滑块颜色
    COLORREF m_ScrollbarArrowColor = RGB(88, 89, 91);   // 滚动条箭头颜色
    int m_nScrollbarWidth = 16;            // 滚动条宽度
    bool m_bCustomScrollbar = true;        // 是否使用自定义滚动条样式
    int m_nScrollThumbMinHeight = 30;      // 滑块最小高度

    //下拉框鼠标离开时隐藏相关
    bool m_bTracking;                      // 鼠标是否正在跟踪
    bool m_bMouseEntered;                  // 鼠标是否首次进入列表
    bool m_IsNeedAutoLeaveHide;            // 是否在鼠标离开下拉框时隐藏窗口
};

class CLarListBoxCWnd : public CWnd
{
public:
    CLarListBoxCtrl m_Ctrl_listbox;

public:
    CLarListBoxCWnd();
    ~CLarListBoxCWnd();
    BOOL Create(const int Width, const int ItemHeight, CWnd* pParentWnd, CStringArray& items, const std::wstring string,
    int maxWidth = -1);
    inline const CStringArray& getItems() { if (m_IsCreated)return m_items; }
protected:

    DECLARE_MESSAGE_MAP()
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
private:
    bool m_IsCreated = false;
    CStringArray m_items;
};

class CLarPopupListBoxs
{
public:
    CLarPopupListBoxs();
    ~CLarPopupListBoxs();
    //下拉框行为预设
    void SetListBoxHideWhenMouseLeave(bool isLeaveAutoHide);

    //下拉框操作
    bool addListBox(const int Width, const int ItemHeight, CWnd* pParentWnd, CStringArray& items, const std::wstring string);
    bool ShowListBox(const std::wstring boxName, const CRect& DisplayRect = CRect(0, 0, 0, 0));
    bool HideListBox();
    bool IsListBoxVisiable(const std::wstring boxName);
    void getDisplayRect(const std::wstring boxName, _Out_ CRect* DisplayGlobalRect);
    void setDisplayRect(const std::wstring boxName, _In_ CRect* DisplayGlobalRect);
    void DeleteAllRow(std::wstring);
    void UpdateDroplistXY(const wchar_t* listBoxName, int X, int Y);
    void DeleteListBox(std::wstring name);

    //下拉框样式设置
    void SetTextSize(const int size, const std::wstring boxName);
    void SetItemHeight(int nHeight);
    void SetTextColor(COLORREF color);
    void SetBackColor(COLORREF color);
    void SetSelectionColor(COLORREF color);
    void SetHoverColor(COLORREF color);
    void SetBorderColor(COLORREF color);
    void SetFont(CFont* pFont, BOOL bRedraw = TRUE);
    void SetSingleMode(bool signleMode = true) { m_SignleMode = signleMode; }
    void SetMaxDisplayItems(int8_t maxItems, std::wstring listBoxName);
    void SetListBoxAdaptWithText(const std::wstring boxName);
    void SetMaxWidth(int maxWidth);

    //下拉框滚动条相关
    void SetScrollbarColors(COLORREF bgColor, COLORREF thumbColor, COLORREF arrowColor);
    void SetScrollbarWidth(int width);
    void EnableCustomScrollbar(bool enable);
private://辅助函数
    void UpdateAdaptTextMaxWidth(CLarListBoxCWnd* cWnd);
private:
    std::map<std::wstring, CLarListBoxCWnd*> m_Map_ListBox;//所有创建的下拉框控件
    bool m_SignleMode = true;
    int m_maxWidth = -1;
};
