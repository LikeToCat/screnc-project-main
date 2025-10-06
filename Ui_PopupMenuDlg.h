#pragma once


// Ui_PopupMenuDlg 对话框
#include "CLarBtn.h"
#include <vector>
#include <functional>
class Ui_PopupMenuDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_PopupMenuDlg)
	struct MenuBtn
	{
		CLarBtn* Btn = nullptr;
		bool m_IsDrawSelectImage = false;
		int uId;
		std::function<void()> m_MenuBtn_Func;
	};
public:
	Ui_PopupMenuDlg(
		int itemHeight, int MenuWidth,
		Color BkBrush, Color BorderBrush,
		int uIdSelectRes = -1,
		CWnd* pParent = nullptr
	); 
	virtual ~Ui_PopupMenuDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MENU };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
public:
	void AddBtn(
		CString BtnText,
		Color TextColor, Color TextHoverColor, Color TextClickColor,
		Color bkColor, Color bkHoverColor, Color bkClickColor,
		std::function<void()> func, bool IsDrawSelectImage = false
	);
	void ShowMenu(int x, int y);
private://辅助函数
	float GetUserDpi();
	int GetMenuItemIndex(const CPoint& point) const;
private:
	std::vector<MenuBtn> m_Vec_MenuBtns;
	int m_MenuWidth;
	int m_MenuHeight;
	int m_ItemHeight;
	int m_Scale;
	int m_MenuItemCount;
	Color m_Color_BkBrush;
	Color m_Color_BorderBrush;
	Gdiplus::Bitmap* m_Bitmap_SelectBitmap;
	int m_StartIndex;
};
