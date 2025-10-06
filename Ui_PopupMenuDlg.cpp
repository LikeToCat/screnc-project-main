// Ui_PopupMenuDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_PopupMenuDlg.h"
#include "afxdialogex.h"
#include "PngLoader.h"
#include "CDebug.h"
// Ui_PopupMenuDlg 对话框
extern HANDLE ConsoleHandle;
static bool s_bTrackingMouse = false;
IMPLEMENT_DYNAMIC(Ui_PopupMenuDlg, CDialogEx)

Ui_PopupMenuDlg::Ui_PopupMenuDlg(
	int itemHeight, int MenuWidth, 
	Color BkBrush, Color BorderBrush,
	int uIdSelectRes,
	CWnd* pParent)
	: CDialogEx(IDD_DIALOG_MENU, pParent)
{
	m_Scale = GetUserDpi();
	m_MenuWidth = MenuWidth;
	m_ItemHeight = itemHeight;
	m_Color_BkBrush = BkBrush;
	m_Color_BorderBrush = BorderBrush;
	m_MenuItemCount = 0;
	m_MenuHeight = 0;
	m_StartIndex = 100;
	if (uIdSelectRes != -1)
	{
		m_Bitmap_SelectBitmap = LARPNG::LoadPngFromResource(
			AfxGetInstanceHandle(),
			MAKEINTRESOURCE(uIdSelectRes),
			L"PNG");
	}
	else
	{
		m_Bitmap_SelectBitmap = nullptr;
	}
}

Ui_PopupMenuDlg::~Ui_PopupMenuDlg()
{
}

void Ui_PopupMenuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void Ui_PopupMenuDlg::AddBtn(CString BtnText,
	Color TextColor, Color TextHoverColor, Color TextClickColor,
	Color bkColor, Color bkHoverColor, Color bkClickColor,
	std::function<void()> func, bool IsDrawSelectImage
)
{
	int BtnCount = m_Vec_MenuBtns.size();
	CRect BtnRect;
	BtnRect.left = 0;
	BtnRect.top = BtnCount * m_ItemHeight;
	BtnRect.right = BtnRect.left + m_MenuWidth;
	BtnRect.bottom = BtnRect.top + m_ItemHeight;
	BtnRect.DeflateRect(10 * m_Scale, 2 * m_Scale);

	CLarBtn* pBtn = new CLarBtn();
	MenuBtn menuBtn;
	pBtn->Create(
		BtnText,
		WS_CHILD | WS_VISIBLE,
		BtnRect,
		this,
		m_StartIndex
	);
	pBtn->LaSetTextColor(TextColor);
	pBtn->LaSetTextClickedColor(TextClickColor);
	pBtn->LaSetTextHoverColor(TextHoverColor);
	pBtn->LarSetNormalFiilBrush(bkColor);
	pBtn->LarSetClickedFillBrush(bkClickColor);
	pBtn->LarSetHoverFillBrush(bkHoverColor);
	pBtn->LarSetBorderColor(bkColor);
	pBtn->LarSetTextSize(22);
	menuBtn.Btn = pBtn;
	menuBtn.m_IsDrawSelectImage = IsDrawSelectImage;
	menuBtn.m_MenuBtn_Func = func;
	menuBtn.uId = m_StartIndex;

	m_Vec_MenuBtns.push_back(menuBtn);
	m_MenuItemCount++;
	m_StartIndex++;
	m_MenuHeight += m_ItemHeight;
}

void Ui_PopupMenuDlg::ShowMenu(int x, int y)
{
	::MoveWindow(this->GetSafeHwnd(), x, y, m_MenuWidth, m_MenuHeight, TRUE);
	this->ShowWindow(SW_SHOW);
}

float Ui_PopupMenuDlg::GetUserDpi()
{
	// 获取系统 DPI
	HDC screen = ::GetDC(NULL);
	if (screen == NULL) 
	{
		AfxMessageBox(L"无法获取屏幕 DC。");
		return -1;
	}
	int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
	int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
	::ReleaseDC(NULL, screen);

	// 计算缩放因子（基准 DPI 为 96）
	double scaleX = static_cast<double>(dpiX) / 96.0;
	double scaleY = static_cast<double>(dpiY) / 96.0;
	return scaleY;
}

int Ui_PopupMenuDlg::GetMenuItemIndex(const CPoint& point) const
{
	if (point.x < 0 || point.x >= m_MenuWidth ||
		point.y < 0 || point.y >= m_MenuHeight)
	{
		return -1;
	}
	int idx = point.y / m_ItemHeight;
	return (idx >= 0 && idx < m_MenuItemCount) ? idx : -1;
}

BEGIN_MESSAGE_MAP(Ui_PopupMenuDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
ON_WM_LBUTTONDOWN()
ON_WM_MOUSELEAVE()
ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// Ui_PopupMenuDlg 消息处理程序

BOOL Ui_PopupMenuDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//设置窗口双缓冲
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);
	return TRUE;  
}

BOOL Ui_PopupMenuDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void Ui_PopupMenuDlg::OnPaint()
{
	CPaintDC dc(this);
	//预缓冲Gdiplus对象
	using namespace Gdiplus;
	Bitmap memBitmap(m_MenuWidth, m_MenuHeight);
	Graphics memGraphics(&memBitmap);

	//绘画背景
	SolidBrush bkBrush(m_Color_BkBrush);
	memGraphics.FillRectangle(&bkBrush, 0,0,m_MenuWidth, m_MenuHeight);

	//绘画边框
	SolidBrush broderBrush(m_Color_BorderBrush);
	Pen borderPen(&broderBrush, 3);
	memGraphics.DrawRectangle(&borderPen, 0, 0, m_MenuWidth, m_MenuHeight);

	//一次性绘画到窗口上
	Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&memBitmap, 0, 0, m_MenuWidth, m_MenuHeight);

	DB(ConsoleHandle, L"Ui_PopupMenuDlg:repaint..");
}

void Ui_PopupMenuDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (GetMenuItemIndex(point) != -1)
	{
		DB(ConsoleHandle, L"点击了菜单项");
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_PopupMenuDlg::OnMouseLeave()
{
	POINT pt;
	::GetCursorPos(&pt);
	CRect wndRect;
	GetWindowRect(&wndRect);
	if (wndRect.PtInRect(pt))
	{//如果鼠标在控件区，则不隐藏窗口，否则隐藏
		s_bTrackingMouse = false;
		return;
	}
	ShowWindow(SW_HIDE);
	s_bTrackingMouse = false;
	CDialogEx::OnMouseLeave();
}

void Ui_PopupMenuDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!s_bTrackingMouse)
	{
		TRACKMOUSEEVENT tme = {};
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
		s_bTrackingMouse = true;
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

BOOL Ui_PopupMenuDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT notification = HIWORD(wParam);
	UINT ctrlId = LOWORD(wParam);

	if (notification == BN_CLICKED)
	{
		for (auto& mb : m_Vec_MenuBtns)
		{
			if (mb.uId == (int)ctrlId)
			{
				if (mb.m_MenuBtn_Func)
					mb.m_MenuBtn_Func();
				return TRUE;
			}
		}
	}

	return CDialogEx::OnCommand(wParam, lParam);
}
