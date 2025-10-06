#include "stdafx.h"
#include "Ui_VideoListDlg.h"
#include "resource.h"
#include "CDebug.h"
#include "Ui_MainDlg.h"
#include "GlobalFunc.h"
#include "LarStringConversion.h"
#include "theApp.h"
#include "Ui_UserProfileDlg.h"
#include "ModalDialogFunc.h"
#include <algorithm>

extern HANDLE ConsoleHandle;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_EDIT_RENAME 2001 //编辑框控件

IMPLEMENT_DYNAMIC(Ui_VideoListDlg, CDialogEx)

Ui_VideoListDlg::Ui_VideoListDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(Ui_VideoListDlg::IDD, pParent)
    , m_Scale(1.0f)
    , m_ScrollPos(0)
    , m_ScrollRange(0)
    , m_ScrollPageSize(0)
    , m_IsScrolling(false)
    , m_RowHeight(60)
    , m_HoverButtonIndex(-1)
    , m_isEditing(false)
    , m_editingVideoIndex(-1)
    , m_selectionCounter(0)
    , m_IconPlay(nullptr)
    , m_IconOpen(nullptr)
    , m_IconDelete(nullptr)
    , m_IsScrollbarHovered(false)
    , m_IsScrollbarDragging(false)
    , m_ScrollbarDragStartY(0)
    , m_ScrollbarDragStartPos(0)
    , m_IconToRec(nullptr)
{
    // 设置颜色
    m_BgColor = Gdiplus::Color(26, 31, 37);
    m_HeaderBgColor = Gdiplus::Color(36, 41, 47);
    m_RowBgColor1 = Gdiplus::Color(37, 39, 46);
    m_RowBgColor2 = Gdiplus::Color(26, 31, 37);
    m_TextColor = Gdiplus::Color(153, 153, 153);
    m_BorderColor = Gdiplus::Color(60, 65, 71);

    m_WindowWidth = 0;
    m_WindowHeight = 0;

    m_Bool_IsShadowInit = false;
}

Ui_VideoListDlg::~Ui_VideoListDlg()
{
    CleanupResources();
}

void Ui_VideoListDlg::Ui_SetWindowRect(const CRect& rect)
{
    m_WindowWidth = rect.Width();
    m_WindowHeight = rect.Height();
}

void Ui_VideoListDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    // 标题栏区域控件
    DDX_Control(pDX, VIDEOLISTDLG_BTN_DELETESELECT, m_Btn_DeleteSelect);
    DDX_Control(pDX, VIDEOLISTDLG_BTN_ADDVIDEO, m_Btn_AddVideo);
    DDX_Control(pDX, VIDEOLISTDLG_BTN_OPENFOLDE, m_Btn_OpenFolder);
    DDX_Control(pDX, VIDEOLISTDLG_BTN_GORECORD, m_Btn_GoRecord);
}

BEGIN_MESSAGE_MAP(Ui_VideoListDlg, CDialogEx)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_LBUTTONDOWN()
    ON_WM_SETCURSOR()
    ON_WM_NCHITTEST()
    ON_WM_MOUSEWHEEL()
    ON_EN_KILLFOCUS(IDC_EDIT_RENAME, &Ui_VideoListDlg::OnEnKillFocusEditRename)
    ON_EN_UPDATE(IDC_EDIT_RENAME, &Ui_VideoListDlg::OnEnUpdateEditRename)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_BN_CLICKED(VIDEOLISTDLG_BTN_RETURN, &Ui_VideoListDlg::OnBnClickedBtnReturn)
    ON_BN_CLICKED(VIDEOLISTDLG_BTN_CLOSE, &Ui_VideoListDlg::OnBnClickedBtnClose)
    ON_WM_NCACTIVATE()
    ON_BN_CLICKED(VIDEOLISTDLG_BTN_OPENFOLDE, &Ui_VideoListDlg::OnBnClickedBtnOpenfolde)
    ON_BN_CLICKED(VIDEOLISTDLG_BTN_ADDVIDEO, &Ui_VideoListDlg::OnBnClickedBtnAddvideo)
    ON_BN_CLICKED(VIDEOLISTDLG_BTN_DELETESELECT, &Ui_VideoListDlg::OnBnClickedBtnDeleteselect)
    ON_BN_CLICKED(VIDEOLSITDLG_BTN_MINIMAL, &Ui_VideoListDlg::OnBnClickedBtnMinimal)
    ON_WM_TIMER()
    ON_WM_MOVE()
    ON_WM_MOUSELEAVE()
    ON_BN_CLICKED(VIDEOLISTDLG_BTN_GORECORD, &Ui_VideoListDlg::OnBnClickedBtnGorecord)
END_MESSAGE_MAP()

BOOL Ui_VideoListDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    // 获取用户DPI并更新缩放
    GetUserDPI();
    UpdateScale();
    LoadVideoList();  // 加载视频列表
    InitializeUI();   // 初始化界面
    LoadResources();  // 加载资源

    //设置窗口双缓冲
    ModifyStyleEx(0, WS_EX_COMPOSITED);
    LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
    SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);

    // 启用鼠标离开通知
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = this->GetSafeHwnd();
    tme.dwHoverTime = HOVER_DEFAULT;
    ::TrackMouseEvent(&tme);

    //提示工具初始化
    m_ToolTip.LarCreate();
    return TRUE;
}

void Ui_VideoListDlg::OnOK()
{
    
}

void Ui_VideoListDlg::OnCancel()
{
    CleanupResources();
    CDialogEx::OnCancel();
}

BOOL Ui_VideoListDlg::OnEraseBkgnd(CDC* pDC)
{
    // 返回TRUE表示背景已擦除，避免闪烁
    return TRUE;
}

void Ui_VideoListDlg::OnPaint()
{
    CPaintDC dc(this);
    m_Shadow.Show(m_hWnd);

    // 获取需要repaint的区域
    CRect clipRect;
    dc.GetClipBox(clipRect);

    if (clipRect.IsRectEmpty() ||
        (clipRect.Width() >= m_WindowWidth && clipRect.Height() >= m_WindowHeight))
    {
        // 全窗repaint——双缓冲
        Gdiplus::Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
        Gdiplus::Graphics memGraphics(&memBitmap);

        // —— 最高质量设置 —— 
        memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        memGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        memGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        memGraphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
        memGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

        // 绘制背景
        Gdiplus::SolidBrush clientBrush(m_BgColor);
        Gdiplus::SolidBrush bottomBrush(Gdiplus::Color(30, 35, 41));

        memGraphics.FillRectangle(&clientBrush, m_ClientArea);
        memGraphics.FillRectangle(&bottomBrush, m_BottomBtnArea);

        // 分隔线
        Gdiplus::Pen separatorPen(Gdiplus::Color(50, 55, 61), 1);
        memGraphics.DrawLine(&separatorPen, 0, m_BottomBtnArea.Y, m_WindowWidth, m_BottomBtnArea.Y);

        // 视频列表 / 空列表提示
        if (m_VideoList.empty())
        {
            DrawEmptyListMessage(&memGraphics);
        }
        else
        {
            DrawVideoList(&memGraphics);
            DrawScrollbar(&memGraphics);
        }
        
        //顶部与主窗口的分界线绘制
        CRect winRect;
        GetWindowRect(winRect);
        SolidBrush tbBrush(Color(0, 0, 0));
        Pen tbpen(&tbBrush, 2);
        Point p1, p2;
        p1.X = 0;
        p1.Y = 0;
        p2.X = winRect.Width();
        p2.Y = 0;
        memGraphics.DrawLine(&tbpen, p1, p2);

        // 刷屏
        Gdiplus::Graphics screen(dc.GetSafeHdc());
        screen.DrawImage(&memBitmap, 0, 0, m_WindowWidth, m_WindowHeight);
    }
    else
    {
        // 局部repaint
        Gdiplus::Bitmap memBitmap(clipRect.Width(), clipRect.Height());
        Gdiplus::Graphics memGraphics(&memBitmap);

        //最高质量设置
        memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        memGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        memGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        memGraphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
        memGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
        memGraphics.TranslateTransform(-clipRect.left, -clipRect.top);

        bool clientAreaNeedsRedraw = clipRect.top < m_BottomBtnArea.Y
            && clipRect.bottom > 0;
        bool titleBarNeedsRedraw = clipRect.top < 0;
        bool bottomAreaNeedsRedraw = clipRect.bottom > m_BottomBtnArea.Y;

        // 标题栏
        if (titleBarNeedsRedraw)
        {
            Gdiplus::SolidBrush titleBarBrush(Gdiplus::Color(26, 27, 32));
            Gdiplus::Rect titleClip(clipRect.left, clipRect.top,
                clipRect.Width(),
                min(0 - clipRect.top, clipRect.Height()));
            memGraphics.FillRectangle(&titleBarBrush, titleClip);
        }

        // 客户区与列表
        if (clientAreaNeedsRedraw)
        {
            Gdiplus::SolidBrush clientBrush(m_BgColor);
            int clientTop = max(clipRect.top, 0);
            int clientBottom = min(clipRect.bottom, m_BottomBtnArea.Y);
            Gdiplus::Rect clientClip(clipRect.left, clientTop,
                clipRect.Width(), clientBottom - clientTop);
            memGraphics.FillRectangle(&clientBrush, clientClip);

            if (!m_VideoList.empty())
            {
                Gdiplus::Region orig;
                memGraphics.GetClip(&orig);
                memGraphics.SetClip(clientClip);

                DrawVideoList(&memGraphics);
                DrawScrollbar(&memGraphics);

                memGraphics.SetClip(&orig);
            }
            else if (clipRect.Height() >= m_ClientArea.Height / 2)
            {
                DrawEmptyListMessage(&memGraphics);
            }
        }

        // 底部按钮区
        if (bottomAreaNeedsRedraw)
        {
            Gdiplus::SolidBrush bottomBrush(Gdiplus::Color(30, 35, 41));
            int bottomTop = max(clipRect.top, m_BottomBtnArea.Y);
            Gdiplus::Rect bottomClip(clipRect.left, bottomTop,
                clipRect.Width(), clipRect.bottom - bottomTop);
            memGraphics.FillRectangle(&bottomBrush, bottomClip);

            if (bottomTop == m_BottomBtnArea.Y || clipRect.top < m_BottomBtnArea.Y)
            {
                Gdiplus::Pen separatorPen(Gdiplus::Color(50, 55, 61), 1);
                memGraphics.DrawLine(&separatorPen,
                    clipRect.left, m_BottomBtnArea.Y,
                    clipRect.right, m_BottomBtnArea.Y);
            }
        }

        // 窗口边框
        CRect winRect;
        GetWindowRect(winRect);
        Gdiplus::SolidBrush borderBrush(Gdiplus::Color(0, 0, 0));
        Gdiplus::Pen borderPen(&borderBrush, 3);
        memGraphics.DrawRectangle(&borderPen,
            0, 0, winRect.Width(), winRect.Height());

        //顶部与主窗口的分界线绘制
        SolidBrush tbBrush(Color(0, 0, 0));
        Pen tbpen(&tbBrush, 2);
        Point p1, p2;
        p1.X = 0;
        p1.Y = 0;
        p2.X = winRect.Width();
        p2.Y = 0;
        memGraphics.DrawLine(&tbpen, p1, p2);

        // 刷局部
        Gdiplus::Graphics screen(dc.GetSafeHdc());
        screen.DrawImage(&memBitmap,
            clipRect.left, clipRect.top,
            clipRect.Width(), clipRect.Height());
    }

    DB(ConsoleHandle, L"Ui_VideoListDlg:repaint..");
}

void Ui_VideoListDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    // 更新窗口尺寸
    m_WindowWidth = cx;
    m_WindowHeight = cy;

    // 更新滚动参数 - 使用客户区域高度（不包括底部按钮区域）
    m_ScrollPageSize = m_ClientArea.Height;
    if (m_VideoList.size() > 0) {
        m_ScrollRange = m_VideoList.size() * m_RowHeight;
    }
    else {
        m_ScrollRange = 0;
    }

    // repaint窗口
    Invalidate();
}

void Ui_VideoListDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // 检查是否点击了滚动条
    const int scrollbarWidth = 8;
    CRect scrollbarRect(
        m_WindowWidth - scrollbarWidth - 4,
        0,
        m_WindowWidth,
        m_BottomBtnArea.Y
    );

    if (scrollbarRect.PtInRect(point)) {
        // 计算滑块位置
        float thumbRatio = (float)m_ScrollPageSize / m_ScrollRange;
        thumbRatio = min(thumbRatio, 1.0f);

        int thumbHeight = (int)(m_ClientArea.Height * thumbRatio);
        thumbHeight = max(thumbHeight, 30);

        float scrollRatio = (float)m_ScrollPos / (m_ScrollRange - m_ScrollPageSize);
        if (m_ScrollRange <= m_ScrollPageSize) scrollRatio = 0;

        int thumbY = (int)((m_ClientArea.Height - thumbHeight) * scrollRatio);

        CRect thumbRect(
            scrollbarRect.left,
            thumbY,
            scrollbarRect.right,
            thumbY + thumbHeight
        );

        if (thumbRect.PtInRect(point)) {
            // 开始拖动滑块
            m_IsScrollbarDragging = true;
            m_ScrollbarDragStartY = point.y;
            m_ScrollbarDragStartPos = m_ScrollPos;
            SetCapture(); // 捕获鼠标
        }
        else {
            // 点击滑块外部的滚动条区域，直接跳转到相应位置
            float clickRatio = (float)(point.y) / m_ClientArea.Height;
            int newScrollPos = (int)(clickRatio * m_ScrollRange);
            newScrollPos = max(0, min(newScrollPos, m_ScrollRange - m_ScrollPageSize));

            if (newScrollPos != m_ScrollPos) {
                m_ScrollPos = newScrollPos;
                InvalidateRect(CRect(0, 0, m_WindowWidth, m_BottomBtnArea.Y), FALSE);
            }
        }

        return;
    }

    // 检查是否点击了按钮区域
    for (const auto& btn : m_ButtonAreas) {
        if (point.x >= btn.rect.X &&
            point.x <= btn.rect.X + btn.rect.Width &&
            point.y >= btn.rect.Y &&
            point.y <= btn.rect.Y + btn.rect.Height) {
            HandleButtonClick(btn.videoIndex, btn.buttonType);
            return;
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

BOOL Ui_VideoListDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    // 获取当前鼠标位置
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);

    // 检查是否悬停在按钮上
    bool onButton = false;
    for (const auto& btn : m_ButtonAreas) {
        if (point.x >= btn.rect.X &&
            point.x <= btn.rect.X + btn.rect.Width &&
            point.y >= btn.rect.Y &&
            point.y <= btn.rect.Y + btn.rect.Height) {
            // 设置手型光标
            ::SetCursor(::LoadCursor(NULL, IDC_HAND));
            onButton = true;
            break;
        }
    }

    if (onButton) {
        return TRUE;
    }

    return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT Ui_VideoListDlg::OnNcHitTest(CPoint point)
{
    return CDialogEx::OnNcHitTest(point);
}

void Ui_VideoListDlg::CleanUpGdiPngRes()
{

}

std::vector<VideoInfo> Ui_VideoListDlg::GetUserAddedVideos() const
{
    std::vector<VideoInfo> userVideos;

    // 遍历所有视频，提取用户添加的视频
    for (const auto& video : m_VideoList)
    {
        if (video.isUserAdded)
        {
            userVideos.push_back(video);
        }
    }

    return userVideos;
}

BOOL Ui_VideoListDlg::AddVideoToList(const CString& videoFilePath, BOOL showMessage)
{
    //防止重复添加相同的视频文件
    if (m_LastVideoFilePath == videoFilePath)
        return FALSE;

    // 检查文件是否存在
    if (!PathFileExists(videoFilePath))
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, _T("添加视频失败，文件不存在: %s"), (LPCTSTR)videoFilePath);
        return FALSE;
    }

    // 获取文件名
    CString fileName = PathFindFileName(videoFilePath);

    // 检查文件格式
    CString fileExt = videoFilePath.Mid(videoFilePath.ReverseFind('.')).MakeLower();
    if (fileExt != _T(".mp4") && fileExt != _T(".avi") && fileExt != _T(".flv"))
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, _T("添加视频失败，不支持的文件格式: %s"), (LPCTSTR)fileExt);
        return FALSE;
    }

    // 创建新的视频信息
    VideoInfo newVideo;
    newVideo.fileName = fileName.GetString();
    newVideo.filePath = videoFilePath.GetString();
    newVideo.isUserAdded = true;  // 标记为用户添加
    newVideo.selected = false;
    newVideo.selectOrder = 0;

    // 获取文件大小
    HANDLE hFile = CreateFile(videoFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(hFile, &fileSize))
        {
            double sizeMB = static_cast<double>(fileSize.QuadPart) / (1024 * 1024);
            wchar_t buffer[50];
            if (sizeMB >= 1000) {
                swprintf_s(buffer, L"%.2f GB", sizeMB / 1000.0);
            }
            else {
                swprintf_s(buffer, L"%.2f MB", sizeMB);
            }
            newVideo.fileSize = buffer;
        }
        CloseHandle(hFile);
    }
    else
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, _T("获取文件大小失败: %s, 错误码: %d"),
            (LPCTSTR)videoFilePath, GetLastError());
    }

    // 获取文件日期
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(videoFilePath, &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        SYSTEMTIME stUTC, stLocal;
        FileTimeToSystemTime(&findData.ftCreationTime, &stUTC);
        SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

        wchar_t dateBuffer[30];
        swprintf_s(dateBuffer, L"%04d-%02d-%02d",
            stLocal.wYear, stLocal.wMonth, stLocal.wDay);
        newVideo.fileDate = dateBuffer;

        wchar_t recordDateBuffer[50];
        swprintf_s(recordDateBuffer, L"%04d-%02d-%02d %02d:%02d",
            stLocal.wYear, stLocal.wMonth, stLocal.wDay,
            stLocal.wHour, stLocal.wMinute);
        newVideo.recordDate = recordDateBuffer;

        FindClose(hFind);
    }

    // 获取视频时长
    newVideo.duration = L"00:00:00";  // 默认值
    GetVideoInfoUsingFFmpeg(videoFilePath, newVideo);

    // 添加到列表顶部
    m_VideoList.insert(m_VideoList.begin(), newVideo);

    // 更新滚动范围
    m_ScrollRange = m_VideoList.size() * m_RowHeight;

    // 记录添加信息
    DEBUG_CONSOLE_FMT(ConsoleHandle, _T("成功添加视频: %s, 完整路径: %s"),
        (LPCTSTR)fileName, (LPCTSTR)videoFilePath);

    // 更新视图
    Invalidate();

    // 如果需要显示提示消息
    if (showMessage)
    {
        MessageBox(_T("视频已添加到列表！"), _T("添加成功"), MB_ICONINFORMATION);
    }

    m_LastVideoFilePath = videoFilePath;
    return TRUE;
}

void Ui_VideoListDlg::EnableCWndShadow()
{
    if (m_Bool_IsShadowInit)
    {
        return;
    }
    //设置窗口阴影效果
    LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
    LONG newStyle = nStyle & (~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
    SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
    
    m_Shadow.Create(m_hWnd);
    m_Shadow.SetHideSingleShadow(CWndShadow::ShadowExMode::exTop);
    m_Bool_IsShadowInit = true;
}

void Ui_VideoListDlg::Ui_UpdateWindowPos(const CRect& rect)
{
    m_WindowX = rect.left;
    m_WindowY = rect.top;
    m_WindowWidth = rect.right - rect.left;
    m_WindowHeight = rect.bottom - rect.top;
    MoveWindow(m_WindowX, m_WindowY, m_WindowWidth, m_WindowHeight);
}

void Ui_VideoListDlg::GetUserDPI()
{
    // 获取显示器DPI
    HDC hdc = ::GetDC(NULL);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    ::ReleaseDC(NULL, hdc);

    // 计算缩放系数（相对于96DPI）
    m_Scale = dpiX / 96.0f;
}

void Ui_VideoListDlg::UpdateScale()
{
    //1180 470
    m_WindowWidth = 784.6 * m_Scale;
    m_WindowHeight = 240 * m_Scale;

    // 底部按钮区域
    int bottomAreaHeight = 46 * m_Scale; // 调整按钮区域高度
    m_BottomBtnArea = Gdiplus::Rect(0, m_WindowHeight - bottomAreaHeight, m_WindowWidth, bottomAreaHeight);

    // 客户区域 
    m_ClientArea = Gdiplus::Rect(0, 0, m_WindowWidth,
        m_WindowHeight - m_BottomBtnArea.Height);

    // 设置行高
    m_RowHeight = 50 * m_Scale;

    //设置批量删除按钮
    float DeleteSelectWidth = 115 * m_Scale;
    float DeleteSelectHeight = 30 * m_Scale;
    float DeleteSelectX = 0.005 * m_BottomBtnArea.Width;
    float DeleteSelectY = m_BottomBtnArea.Y + (m_BottomBtnArea.Height - DeleteSelectHeight) / 2 + 1 * m_Scale;
    m_Btn_DeleteSelect.MoveWindow(DeleteSelectX, DeleteSelectY, DeleteSelectWidth, DeleteSelectHeight);

    //导入视频按钮
    float AddVideoBtnWidth = 98 * m_Scale;
    float AddVideoBtnHeight = 31 * m_Scale;
    float AddVideoBtnX = 0.74 * m_BottomBtnArea.Width;
    float AddVideoBtnY = DeleteSelectY + (DeleteSelectHeight - AddVideoBtnHeight) / 2 - 2 * m_Scale;
    m_Btn_AddVideo.MoveWindow(AddVideoBtnX, AddVideoBtnY, AddVideoBtnWidth, AddVideoBtnHeight);

    //打开文件夹按钮
    float OpenFolderWidth = 107 * m_Scale;
    float OpenFolderHeight = 31 * m_Scale;
    float OpenFolderX = AddVideoBtnX + AddVideoBtnWidth;
    float OpenFolderY = AddVideoBtnY;
    m_Btn_OpenFolder.MoveWindow(OpenFolderX, OpenFolderY, OpenFolderWidth, OpenFolderHeight);

    //去录制按钮
    float goRW = 120 * m_Scale;
    float goRH = 30 * m_Scale;
    float goRx = (m_WindowWidth - goRW) / 2;
    float goRy = (m_WindowHeight - goRH) / 2 + 30 * m_Scale;
    m_Btn_GoRecord.MoveWindow(goRx, goRy, goRW, goRH);

    //去录制图片logo区域
    m_Rect_IconToRec.Width = 50 * m_Scale;
    m_Rect_IconToRec.Height = 50 * m_Scale;
    m_Rect_IconToRec.X = (m_WindowWidth - m_Rect_IconToRec.Width) / 2;
    m_Rect_IconToRec.Y = (m_WindowHeight - m_Rect_IconToRec.Height) / 2 - 40 * m_Scale;

    m_Rect_IconToRecText.Width = 300 * m_Scale;
    m_Rect_IconToRecText.Height = 25 * m_Scale;
    m_Rect_IconToRecText.X = (m_WindowWidth - m_Rect_IconToRecText.Width) / 2;
    m_Rect_IconToRecText.Y = (m_WindowHeight - m_Rect_IconToRecText.Height) / 2;
}

void Ui_VideoListDlg::InitializeUI()
{
    //批量删除
    Gdiplus::SolidBrush BkBrush(Gdiplus::Color(255, 30, 35, 41));
    m_Btn_DeleteSelect.LarSetTextSize(20);
    m_Btn_DeleteSelect.LarSetTextStyle(false, false, false);
    m_Btn_DeleteSelect.LaSetTextColor(Color(125, 255, 255, 255));
    m_Btn_DeleteSelect.LarSetBorderColor(Color(125, 30, 35, 41));
    m_Btn_DeleteSelect.LarSetEraseBkEnable(false);
    m_Btn_DeleteSelect.LarSetButtonNoInteraction(true);
    m_Btn_DeleteSelect.LaSetTextHoverColor(Color(255, 255, 255, 255));
    m_Btn_DeleteSelect.LaSetTextClickedColor(Color(255, 255, 255, 255));
    m_Btn_DeleteSelect.LarSetNormalFiilBrush(BkBrush);
    m_Btn_DeleteSelect.LarSetBtnNailImage(
        VIDEOLISTDLG_PNG_DELETENUMS, CLarBtn::NailImageLayout::Left,
        16 * m_Scale, 16 * m_Scale, 8 * m_Scale, -1 * m_Scale
    );
    m_Btn_DeleteSelect.LarAdjustTextDisplayPos(-1 * m_Scale, 0);

    //导入视频
    m_Btn_AddVideo.LarSetTextSize(20);
    m_Btn_AddVideo.LarSetTextStyle(false, false, false);
    m_Btn_AddVideo.LaSetTextColor(Color(125, 255, 255, 255));
    m_Btn_AddVideo.LarSetBorderColor(Color(125, 30, 35, 41));
    m_Btn_AddVideo.LarSetEraseBkEnable(false);
    m_Btn_AddVideo.LarSetButtonNoInteraction(true);
    m_Btn_AddVideo.LaSetTextHoverColor(Color(255, 255, 255, 255));
    m_Btn_AddVideo.LaSetTextClickedColor(Color(255, 255, 255, 255));
    m_Btn_AddVideo.LarSetNormalFiilBrush(BkBrush);
    m_Btn_AddVideo.LarSetBtnNailImage(
        VIDEOLISTDLG_PNG_ADDVIDEO, CLarBtn::NailImageLayout::Left,
        16 * m_Scale, 16 * m_Scale, 0, -1 * m_Scale
    );
    m_Btn_AddVideo.LarAdjustTextDisplayPos(1 * m_Scale, 0);

    //打开文件夹
    m_Btn_OpenFolder.LarSetTextSize(20);
    m_Btn_OpenFolder.LarSetTextStyle(false, false, false);
    m_Btn_OpenFolder.LaSetTextColor(Color(125, 255, 255, 255));
    m_Btn_OpenFolder.LarSetBorderColor(Color(125, 30, 35, 41));
    m_Btn_OpenFolder.LarSetEraseBkEnable(false);
    m_Btn_OpenFolder.LarSetButtonNoInteraction(true);
    m_Btn_OpenFolder.LaSetTextHoverColor(Color(255, 255, 255, 255));
    m_Btn_OpenFolder.LaSetTextClickedColor(Color(255, 255, 255, 255));
    m_Btn_OpenFolder.LarSetNormalFiilBrush(BkBrush);
    m_Btn_OpenFolder.LarSetBtnNailImage(
        VIDEOLISTDLG_PNG_OPENFOLDE, CLarBtn::NailImageLayout::Left,
        16 * m_Scale, 16 * m_Scale, 0, -1 * m_Scale
    );
    m_Btn_OpenFolder.LarAdjustTextDisplayPos(4 * m_Scale, 0);

    //去录制
    m_Btn_GoRecord.LarSetTextSize(18);
    m_Btn_GoRecord.LarSetGradualColor(
        Color(18, 169, 180), Color(1, 190, 131), 
        Gdiplus::LinearGradientMode::LinearGradientModeHorizontal
    );
    m_Btn_GoRecord.LarSetHoverGradualColor(
        Color(28, 179, 190), Color(10, 200, 141),
        Gdiplus::LinearGradientMode::LinearGradientModeHorizontal
    );
    m_Btn_GoRecord.LarSetClickGradualColor(
        Color(38, 189, 200), Color(20, 210, 151),
        Gdiplus::LinearGradientMode::LinearGradientModeHorizontal
    );
    m_Btn_GoRecord.LaSetTextColor(Color(230, 230, 230));
    m_Btn_GoRecord.LaSetTextHoverColor(Color(245, 245, 245));
    m_Btn_GoRecord.LaSetTextClickedColor(Color(255, 255, 255));
    m_Btn_GoRecord.ShowWindow(SW_HIDE);
}

void Ui_VideoListDlg::LoadResources()
{
    // 清理旧资源
    CleanupResources();

    m_IconPlay = nullptr;
    m_IconOpen = nullptr;
    m_IconDelete = nullptr;

    // 加载Play图标
    HINSTANCE hInstance = AfxGetInstanceHandle();
    HRSRC hResInfo = FindResource(hInstance, MAKEINTRESOURCE(VIDEOLISTDLG_PNG_PLAY), L"PNG");
    if (hResInfo)
    {
        HGLOBAL hResData = LoadResource(hInstance, hResInfo);
        if (hResData)
        {
            DWORD dwSize = SizeofResource(hInstance, hResInfo);
            LPVOID pResData = LockResource(hResData);

            if (pResData && dwSize > 0)
            {
                IStream* pStream = NULL;
                if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pStream)))
                {
                    pStream->Write(pResData, dwSize, NULL);
                    m_IconPlay = Gdiplus::Bitmap::FromStream(pStream);
                    pStream->Release();

                    if (m_IconPlay)
                    {
                        DEBUG_CONSOLE_FMT(ConsoleHandle, L"Play icon loaded successfully using stream. Size: %dx%d\n",
                            m_IconPlay->GetWidth(), m_IconPlay->GetHeight());
                    }
                }
            }
            UnlockResource(hResData);
        }
        FreeResource(hResData);
    }

    // 加载Open图标
    HRSRC hResInfo1 = FindResource(hInstance, MAKEINTRESOURCE(VIDEOLISTDLG_BTN_OPENLOCALFILE), L"PNG");
    if (hResInfo1)
    {
        HGLOBAL hResData = LoadResource(hInstance, hResInfo1);
        if (hResData)
        {
            DWORD dwSize = SizeofResource(hInstance, hResInfo1);
            LPVOID pResData = LockResource(hResData);

            if (pResData && dwSize > 0)
            {
                IStream* pStream = NULL;
                if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pStream)))
                {
                    pStream->Write(pResData, dwSize, NULL);
                    m_IconOpen = Gdiplus::Bitmap::FromStream(pStream);
                    pStream->Release();

                    if (m_IconOpen)
                    {
                        DEBUG_CONSOLE_FMT(ConsoleHandle, L"Play icon loaded successfully using stream. Size: %dx%d\n",
                            m_IconOpen->GetWidth(), m_IconOpen->GetHeight());
                    }
                }
            }
            UnlockResource(hResData);
        }
        FreeResource(hResData);
    }

    // 加载Delete图标
    HRSRC hResInfo2 = FindResource(hInstance, MAKEINTRESOURCE(VIDEOLISTDLG_BTN_DELETE), L"PNG");
    if (hResInfo2)
    {
        HGLOBAL hResData = LoadResource(hInstance, hResInfo2);
        if (hResData)
        {
            DWORD dwSize = SizeofResource(hInstance, hResInfo2);
            LPVOID pResData = LockResource(hResData);

            if (pResData && dwSize > 0)
            {
                IStream* pStream = NULL;
                if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pStream)))
                {
                    pStream->Write(pResData, dwSize, NULL);
                    m_IconDelete = Gdiplus::Bitmap::FromStream(pStream);
                    pStream->Release();

                    if (m_IconDelete)
                    {
                        DEBUG_CONSOLE_FMT(ConsoleHandle, L"Play icon loaded successfully using stream. Size: %dx%d\n",
                            m_IconDelete->GetWidth(), m_IconDelete->GetHeight());
                    }
                }
            }
            UnlockResource(hResData);
        }
        FreeResource(hResData);
    }

    m_IconToRec = LARPNG::LoadPngFromResource(hInstance, MAKEINTRESOURCE(VIDELISTDLG_PNG_RECORDFILELOGO), L"PNG");
    if (!m_IconToRec)
    {
        DB(ConsoleHandle, L"m_IconToRec加载失败！");
    }
}

void Ui_VideoListDlg::CleanupResources()
{
    // 释放图标资源
    delete m_IconPlay;
    delete m_IconOpen;
    delete m_IconDelete;

    m_IconPlay = nullptr;
    m_IconOpen = nullptr;
    m_IconDelete = nullptr;
}

void Ui_VideoListDlg::ClearAllSelections()
{
    for (auto& video : m_VideoList) {
        video.selected = false;
        video.selectOrder = 0;
    }
    m_selectionCounter = 0;

    // repaint列表区域
    CRect invalidRect(
        0,                       // 左边界
        0,       // 上边界（从标题栏下方开始）
        m_WindowWidth,           // 右边界
        m_WindowHeight           // 下边界
    );
    InvalidateRect(invalidRect, FALSE);
}

void Ui_VideoListDlg::LoadVideoList()
{
    // 清空现有列表
    m_VideoList.clear();

    // 先从配置文件加载用户添加的视频
    LoadUserAddedVideosFromConfig();

    // 记录已经从配置文件加载的视频数量
    size_t userAddedCount = m_VideoList.size();
    DEBUG_CONSOLE_FMT(ConsoleHandle, _T("从配置文件加载了 %d 个用户添加的视频\n"), userAddedCount);

    // 从配置对话框获取视频保存路径
    CString videoPath;
    auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
    pDlg->m_Dlg_Config->m_Btn_Path.GetWindowTextW(videoPath);

    // 如果路径为空，使用默认路径
    if (videoPath.IsEmpty()) {
        videoPath = GlobalFunc::GetDefaultVideoSavePath();
    }

    // 记录到日志
    DEBUG_CONSOLE_FMT(ConsoleHandle, _T("正在扫描视频目录: %s\n"), (LPCTSTR)videoPath);

    // 对整个列表排序
    std::sort(m_VideoList.begin(), m_VideoList.end(),
        [](const VideoInfo& a, const VideoInfo& b) {
            return a.recordDate > b.recordDate;
        });
    
    // 更新滚动范围
    m_ScrollRange = m_VideoList.size() * m_RowHeight;

    // 记录加载完成的总数
    DEBUG_CONSOLE_FMT(ConsoleHandle, _T("视频加载完成，共 %d 个视频文件 (用户添加: %d, 目录扫描: %d)\n"),
        m_VideoList.size(), userAddedCount, m_VideoList.size() - userAddedCount);

    // 更新视图
    Invalidate();
}

void Ui_VideoListDlg::DrawVideoList(Gdiplus::Graphics* graphics)
{
    DB(ConsoleHandle, L"开始绘画视频列表");
    // 清空按钮区域
    m_ButtonAreas.clear();

    // 设置视频列表区域的剪裁区域，确保不会绘制到底部按钮区域
    Gdiplus::Region originalRegion;
    graphics->GetClip(&originalRegion);
    Gdiplus::Rect clipRect(0, 0, m_WindowWidth, m_ClientArea.Height);
    graphics->SetClip(clipRect);

    // 创建画笔和画刷
    Gdiplus::SolidBrush textBrush(Color(235, 235, 235));
    Gdiplus::Pen borderPen(m_BorderColor, 1);
    Gdiplus::SolidBrush rowBg1(m_RowBgColor1);
    Gdiplus::SolidBrush rowBg2(m_RowBgColor2);

    // 创建字体
    float fontSize = 1;
    if (m_Scale == 1)
    {
        fontSize = 10 * m_Scale;
    }
    else if (m_Scale == 1.25)
    {
        fontSize = 9 * m_Scale;
    }
    else if (m_Scale == 2.25)
    {
        fontSize = 5 * m_Scale;
    }
    else if (m_Scale == 2)
    {
        fontSize = 6 * m_Scale;
    }
    else if (m_Scale == 1.75)
    {
        fontSize = 7 * m_Scale;
    }
    else if (m_Scale == 1.5)
    {
        fontSize = 8 * m_Scale;
    }
    else//缩放大于225%
    {
        fontSize = 4 * m_Scale;
    }
    Gdiplus::Font contentFont(L"微软雅黑", fontSize);

    // 定义列宽
    float checkboxWidth = 40 * m_Scale;
    float fileNameWidth = 180 * m_Scale;
    float durationWidth = 100 * m_Scale;
    float sizeWidth = 110 * m_Scale;
    // 增加日期宽度以确保完整显示
    float dateWidth = 170 * m_Scale;
    float buttonWidth = 30 * m_Scale;
    float spacing = 10 * m_Scale;

    // 设置文本格式
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

    // 设置左对齐格式
    Gdiplus::StringFormat leftAlign;
    leftAlign.SetAlignment(Gdiplus::StringAlignmentNear);
    leftAlign.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    leftAlign.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    leftAlign.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

    // 定义视频列表的起始Y坐标
    float startY = 0;

    // 绘制行
    for (size_t i = 0; i < m_VideoList.size(); i++)
    {
        const VideoInfo& video = m_VideoList[i];

        // 计算行位置（考虑滚动位置）
        float rowY = startY + i * m_RowHeight - m_ScrollPos;

        // 如果行不在可见区域内，跳过
        if (rowY + m_RowHeight < 0 || rowY > m_WindowHeight) {
            continue;
        }

        // 绘制行背景
        Gdiplus::RectF rowRect(0, rowY, (float)m_WindowWidth, (float)m_RowHeight);
        if (i % 2 == 0) {
            graphics->FillRectangle(&rowBg1, rowRect);
        }
        else {
            graphics->FillRectangle(&rowBg2, rowRect);
        }

        // 绘制行分隔线
        graphics->DrawLine(&borderPen, 0.0f, (float)rowRect.GetBottom(), (float)m_WindowWidth, (float)rowRect.GetBottom());

        //多选框区域
        Gdiplus::RectF checkboxRect(
            spacing + 0.5f,
            rowY + (m_RowHeight - 20.0f * m_Scale) / 2.0f,
            18.0f * m_Scale,
            18.0f * m_Scale
        );
        Gdiplus::Pen checkboxPen(m_TextColor, 1.0f);
        if (video.selected)
        {
            // 填充圆形背景
            Gdiplus::SolidBrush checkboxFillBrush(Gdiplus::Color(0, 139, 255));
            graphics->FillEllipse(&checkboxFillBrush, checkboxRect);

            //// 构造像素单位字体
            //float fontSizePx = (video.selectOrder >= 10 ? 8.0f : 12.0f) * m_Scale;
            //Gdiplus::Font numberFont(
            //    L"Arial",
            //    fontSizePx,
            //    Gdiplus::FontStyleRegular,
            //    Gdiplus::UnitPixel    
            //);
            //
            //// 准备要绘制的字符串
            //wchar_t orderText[8];
            //swprintf_s(orderText, L"%d", video.selectOrder);
            //
            //// 用 MeasureString 得到真实文字尺寸
            //Gdiplus::StringFormat fmt;
            //// 先用 Near，MeasureString 时不做居中
            //fmt.SetAlignment(Gdiplus::StringAlignmentNear);
            //fmt.SetLineAlignment(Gdiplus::StringAlignmentNear);
            //fmt.SetFormatFlags(Gdiplus::StringFormatFlagsMeasureTrailingSpaces);
            //
            //Gdiplus::RectF measured;
            //graphics->MeasureString(
            //    orderText, -1,
            //    &numberFont,
            //    checkboxRect,
            //    &fmt,
            //    &measured
            //);
            //
            //// 计算绘制起点，使文字在圆形区域居中
            //float x = checkboxRect.X + (checkboxRect.Width - measured.Width) * 0.5f + 0.75f * m_Scale;
            //float y = checkboxRect.Y + (checkboxRect.Height - measured.Height) * 0.5f;
            //
            //// 保存 Graphics 状态并开启抗锯齿
            //Gdiplus::GraphicsState gstate = graphics->Save();
            //graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
            //
            //// 绘制文字
            //Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255));
            //graphics->DrawString(
            //    orderText,
            //    -1,
            //    &numberFont,
            //    Gdiplus::PointF(x, y),
            //    &textBrush
            //);

            // 恢复状态，不影响后续绘制
            //graphics->Restore(gstate);
        }
        else
        {
            // 未选中时仅绘制空心圆
            graphics->DrawEllipse(&checkboxPen, checkboxRect);
        }

        // 添加多选框按钮区域
        m_ButtonAreas.push_back({
            checkboxRect,
            (int)i,
            BTN_TYPE_CHECKBOX
            });

        // 绘制文件名
        float currentX = checkboxWidth;
        Gdiplus::RectF fileNameRect(currentX, rowY, fileNameWidth, rowRect.Height);
        graphics->DrawString(video.fileName.c_str(), -1, &contentFont,
            fileNameRect, &leftAlign, &textBrush);

        // 绘制时长
        currentX += fileNameWidth + spacing *2;
        Gdiplus::RectF durationRect(currentX, rowY, durationWidth, rowRect.Height);
        graphics->DrawString(video.duration.c_str(), -1, &contentFont,
            durationRect, &format, &textBrush);

        // 绘制文件大小
        currentX += durationWidth + spacing *2;
        Gdiplus::RectF sizeRect(currentX, rowY, sizeWidth, rowRect.Height);
        graphics->DrawString(video.fileSize.c_str(), -1, &contentFont,
            sizeRect, &format, &textBrush);

        // 绘制录制日期
        currentX += sizeWidth + spacing;
        Gdiplus::RectF dateRect(currentX, rowY, dateWidth, rowRect.Height);
        graphics->DrawString(video.recordDate.c_str(), -1, &contentFont,
            dateRect, &format, &textBrush);

        // 计算按钮位置，放在行的最右侧
        float ButtonHeight = 18 * m_Scale;
        float ButtonWidth = 18 * m_Scale;
        float buttonsStartX = m_WindowWidth - (buttonWidth * 3 + spacing * 4);
        float buttonsY = rowY + (m_RowHeight - ButtonHeight) / 2;
        // 绘制播放按钮
        Gdiplus::RectF playRect(
            buttonsStartX,
            buttonsY,
            ButtonWidth,
            ButtonHeight
        );
        Gdiplus::RectF PlayHoverRect;
        PlayHoverRect.X = playRect.X - 5 * m_Scale;
        PlayHoverRect.Y = playRect.Y - 5 * m_Scale;
        PlayHoverRect.Width = playRect.Width + 10 * m_Scale;
        PlayHoverRect.Height = playRect.Height + 10 * m_Scale;
        if (m_IconPlay) 
        {
            // 获取图标原始尺寸
            UINT width = m_IconPlay->GetWidth();
            UINT height = m_IconPlay->GetHeight();

            // 计算保持宽高比的缩放尺寸
            float scale = min(playRect.Width / width, playRect.Height / height);
            float newWidth = width * scale;
            float newHeight = height * scale;

            // 居中显示图标
            float x = playRect.X + (playRect.Width - newWidth) / 2;
            float y = playRect.Y + (playRect.Height - newHeight) / 2;

            // 绘制图标
            graphics->DrawImage(m_IconPlay,
                Gdiplus::RectF(x, y, ButtonWidth, ButtonHeight),
                0, 0, width, height,
                Gdiplus::UnitPixel);
        }
        else 
        {
            // 如果图标加载失败，使用绘制的三角形作为备用
            Gdiplus::SolidBrush playBrush(m_TextColor);
            Gdiplus::Point playPoints[3] = 
            {
                Gdiplus::Point((int)(playRect.X + 5), (int)(playRect.Y + 5)),
                Gdiplus::Point((int)(playRect.X + 5), (int)(playRect.Y + 15)),
                Gdiplus::Point((int)(playRect.X + 15), (int)(playRect.Y + 10))
            };
            graphics->FillPolygon(&playBrush, playPoints, 3);
        }
        bool isPlayButtonHovered = (m_HoverButtonIndex == (int)i && m_HoverButtonType == BTN_TYPE_PLAY);
        if (isPlayButtonHovered) 
        { // 绘制悬停背景为矩形
            Gdiplus::SolidBrush hoverBrush(Gdiplus::Color(20, 255, 255, 255)); // 略微加深颜色
            graphics->FillRectangle(&hoverBrush, PlayHoverRect);
        }
        // 添加播放按钮区域
        m_ButtonAreas.push_back({
            PlayHoverRect,
            (int)i,
            BTN_TYPE_PLAY
            });

        // 绘制打开文件按钮
        buttonsStartX += buttonWidth + spacing;
        Gdiplus::RectF openRect(
            buttonsStartX,
            buttonsY,
            ButtonWidth,
            ButtonHeight
        );
        Gdiplus::RectF OpenHoverRect;
        OpenHoverRect.X = openRect.X - 3 * m_Scale;
        OpenHoverRect.Y = openRect.Y - 3 * m_Scale;
        OpenHoverRect.Width = openRect.Width + 8 * m_Scale;
        OpenHoverRect.Height = openRect.Height + 8 * m_Scale;
        if (m_IconOpen) {
            // 使用加载的PNG图标
            graphics->DrawImage(m_IconOpen, openRect);
        }
        else {
            // 如果图标加载失败，使用绘制的文件夹图标作为备用
            Gdiplus::SolidBrush openBrush(m_TextColor);
            graphics->FillRectangle(&openBrush,
                openRect.X + 5.0f, openRect.Y + 8.0f,
                10.0f, 8.0f);
            graphics->FillRectangle(&openBrush,
                openRect.X + 3.0f, openRect.Y + 5.0f,
                14.0f, 3.0f);
        }
        bool isOpenButtonHovered = (m_HoverButtonIndex == (int)i && m_HoverButtonType == BTN_TYPE_OPEN);
        if (isOpenButtonHovered) 
        {
            // 绘制悬停背景为矩形
            Gdiplus::SolidBrush hoverBrush(Gdiplus::Color(20, 255, 255, 255));
            graphics->FillRectangle(&hoverBrush, OpenHoverRect);
        }
        // 添加打开文件按钮区域
        m_ButtonAreas.push_back({
            OpenHoverRect,
            (int)i,
            BTN_TYPE_OPEN
            });

        // 绘制删除按钮
        buttonsStartX += buttonWidth + spacing;
        Gdiplus::RectF deleteRect(
            buttonsStartX,
            buttonsY,
            ButtonWidth,
            ButtonHeight
        );
        Gdiplus::RectF DeleteHoverRect;
        DeleteHoverRect.X = deleteRect.X - 5 * m_Scale;
        DeleteHoverRect.Y = deleteRect.Y - 5 * m_Scale;
        DeleteHoverRect.Width = deleteRect.Width + 10 * m_Scale;
        DeleteHoverRect.Height = deleteRect.Height + 10 * m_Scale;
        if (m_IconDelete) 
        {
            // 使用加载的PNG图标
            graphics->DrawImage(m_IconDelete, deleteRect);
        }
        else {
            // 如果图标加载失败，使用绘制的X图标作为备用
            Gdiplus::Pen deletePen(m_TextColor, 1.5f);
            graphics->DrawLine(&deletePen,
                deleteRect.X + 5.0f, deleteRect.Y + 5.0f,
                deleteRect.X + 15.0f, deleteRect.Y + 15.0f);
            graphics->DrawLine(&deletePen,
                deleteRect.X + 15.0f, deleteRect.Y + 5.0f,
                deleteRect.X + 5.0f, deleteRect.Y + 15.0f);
        }
        bool isDeleteButtonHovered = (m_HoverButtonIndex == (int)i && m_HoverButtonType == BTN_TYPE_DELETE);
        if (isDeleteButtonHovered) 
        {
            // 绘制悬停背景为矩形
            Gdiplus::SolidBrush hoverBrush(Gdiplus::Color(20, 255, 0, 0));

            graphics->FillRectangle(&hoverBrush, DeleteHoverRect);
        }
        // 添加删除按钮区域
        m_ButtonAreas.push_back({
            DeleteHoverRect,
            (int)i,
            BTN_TYPE_DELETE
            });
    }

    // 绘制完成后恢复原始剪裁区域
    graphics->SetClip(&originalRegion);
}

void Ui_VideoListDlg::DrawEmptyListMessage(Gdiplus::Graphics* graphics)
{
    // 创建字体和画刷
    int size;
    if (m_Scale == 1.0)
    {
        size = 9 * m_Scale;
    }
    else if (m_Scale == 1.25)
    {
        size = 8 * m_Scale;
    }
    else if (m_Scale == 1.5)
    {
        size = 7 * m_Scale;
    }
    else if (m_Scale == 1.75)
    {
        size = 6 * m_Scale;
    }
    else if (m_Scale == 2.0)
    {
        size = 5 * m_Scale;
    }
    else if (m_Scale == 2.25)
    {
        size = 4 * m_Scale;
    }
    Gdiplus::Font messageFont(L"微软雅黑", size);
    Gdiplus::SolidBrush textBrush(m_TextColor);

    // 设置文本格式
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    // 绘制提示信息 - 注意使用m_ClientArea而不是整个窗口
    graphics->DrawString(
        L"视频列表中没有文件，快去录制吧",
        -1,
        &messageFont,
        m_Rect_IconToRecText,
        &format,
        &textBrush
    );

    //绘画中间的图片logo
    if (m_IconToRec)
    {
        graphics->DrawImage(m_IconToRec, m_Rect_IconToRec);
    }
}

void Ui_VideoListDlg::HandleButtonClick(int videoIndex, int buttonType)
{
    HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (videoIndex < 0 || videoIndex >= (int)m_VideoList.size()) {
        return;
    }

    VideoInfo& video = m_VideoList[videoIndex];

    switch (buttonType) {
    case BTN_TYPE_CHECKBOX:
    {
        bool wasSelected = video.selected;
        int oldOrder = video.selectOrder;
        DB(ConsoleHandle, L"点击了CHECKBOX");
        if (!video.selected) {
            // 选中状态
            m_selectionCounter++;
            video.selected = true;
            video.selectOrder = m_selectionCounter;
        }
        else {
            // 取消选中
            int canceledOrder = video.selectOrder;
            video.selected = false;
            video.selectOrder = 0;

            // 调整序号
            for (auto& v : m_VideoList) {
                if (v.selected && v.selectOrder > canceledOrder) {
                    v.selectOrder--;
                }
            }

            m_selectionCounter--;
        }

        // 计算当前行的位置
        float startY = 0;
        float rowY = startY + videoIndex * m_RowHeight - m_ScrollPos;

        // 只repaint这一行和滚动条区域
        CRect rowRect(
            0,                          // 左边界
            (int)(rowY),                // 上边界
            m_WindowWidth,              // 右边界
            (int)(rowY + m_RowHeight)   // 下边界
        );

        // 如果选择状态变化，也repaint底部删除按钮（因为状态可能变化）
        if (wasSelected != video.selected) {
            // 更新"批量删除"按钮状态
            // 设置批量删除按钮文本
            CString btnText;
            if (m_selectionCounter > 0) {
                btnText.Format(_T("删除(%d)"), m_selectionCounter);
                m_Btn_DeleteSelect.SetWindowText(btnText);
                m_Btn_DeleteSelect.LarSetButtonNoInteraction(false); // 可交互
            }
            else {
                m_Btn_DeleteSelect.SetWindowText(_T("批量删除"));
                m_Btn_DeleteSelect.LarSetButtonNoInteraction(true); // 不可交互
            }
        }

        // 即使选择状态没变化，也需要repaint行（以显示序号变化）
        InvalidateRect(rowRect, FALSE);

        // repaint滚动条
        CRect scrollbarRect(
            m_WindowWidth - 16,     // 左边界
            0,      // 上边界
            m_WindowWidth,          // 右边界
            m_BottomBtnArea.Y       // 下边界
        );
        InvalidateRect(scrollbarRect, FALSE);

        break;
    }
    case BTN_TYPE_RENAME:
        // 开始重命名编辑
        BeginRenameEdit(videoIndex);
        break;

    case BTN_TYPE_PLAY:
        // 播放视频 - 使用系统默认播放器
    {
        if (video.filePath.empty()) {
            MessageBox(L"视频文件路径无效", L"错误", MB_OK | MB_ICONERROR);
            return;
        }

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"正在播放视频: %s", video.filePath.c_str());

        // 使用ShellExecute打开视频文件，使用系统默认播放器播放
        HINSTANCE result = ShellExecute(
            NULL,                       // 父窗口句柄
            L"open",                    // 操作
            video.filePath.c_str(),     // 文件路径
            NULL,                       // 参数
            NULL,                       // 默认目录
            SW_SHOWNORMAL               // 显示命令
        );

        // 检查执行结果
        if ((INT_PTR)result <= 32)
        {
            // 创建完整命令行，打开文件夹并选中文件
            CString command = L"/select,\"" + CString(video.filePath.c_str()) + L"\"";

            // 使用explorer打开文件夹并选中文件
            HINSTANCE result = ShellExecute(
                NULL,                   // 父窗口句柄
                L"open",                // 操作
                L"explorer.exe",        // 资源管理器
                command,                // 参数 - 选中指定文件
                NULL,                   // 默认目录
                SW_SHOWNORMAL           // 显示命令
            );

            // 检查执行结果
            if ((INT_PTR)result <= 32)
            {
                ModalDlg_MFC::ShowModal_OpenVideoFailed();
            }
        }
    }
    break;

    case BTN_TYPE_OPEN:
        // 打开文件所在位置
    {
        HideToolWindow();
        m_Shadow.Show(m_hWnd);
        if (video.filePath.empty()) {
            MessageBox(L"视频文件路径无效", L"错误", MB_OK | MB_ICONERROR);
            return;
        }

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"正在打开文件位置: %s", video.filePath.c_str());

        // 创建完整命令行，打开文件夹并选中文件
        CString command = L"/select,\"" + CString(video.filePath.c_str()) + L"\"";

        // 使用explorer打开文件夹并选中文件
        HINSTANCE result = ShellExecute(
            NULL,                   // 父窗口句柄
            L"open",                // 操作
            L"explorer.exe",        // 资源管理器
            command,                // 参数 - 选中指定文件
            NULL,                   // 默认目录
            SW_SHOWNORMAL           // 显示命令
        );

        // 检查执行结果
        if ((INT_PTR)result <= 32) {
            // ShellExecute失败
            CString errorMsg;
            errorMsg.Format(L"无法打开文件位置，错误码: %d", (INT_PTR)result);
            MessageBox(errorMsg, L"操作失败", MB_OK | MB_ICONERROR);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"打开文件位置失败，错误码: %d", (INT_PTR)result);
        }
        m_Shadow.Show(m_hWnd);
    }
    break;

    case BTN_TYPE_DELETE:
        // 删除视频 - 同时删除文件和列表项
    {
        CString message;
        message.Format(L"确定要删除视频 \"%s\" 吗？\n\n此操作将永久删除该视频文件！",
            video.fileName.c_str());

        if (MessageBox(message, L"确认删除", MB_YESNO | MB_ICONWARNING) == IDYES) {
            bool fileDeleted = false;

            // 尝试删除实际文件
            if (!video.filePath.empty()) {
                // 设置文件属性为普通，移除只读属性
                SetFileAttributes(video.filePath.c_str(), FILE_ATTRIBUTE_NORMAL);

                // 删除文件
                if (DeleteFile(video.filePath.c_str())) {
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"成功删除文件: %s", video.filePath.c_str());
                    fileDeleted = true;
                }
                else {
                    DWORD error = GetLastError();
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"删除文件失败: %s, 错误码: %d",
                        video.filePath.c_str(), error);

                    // 告知用户删除失败但仍从列表移除
                    CString errorMsg;
                    errorMsg.Format(L"无法删除视频文件，错误码: %d\n是否仍要从列表中移除?", error);
                    if (MessageBox(errorMsg, L"删除失败", MB_YESNO | MB_ICONQUESTION) != IDYES) {
                        // 用户选择不移除，则退出
                        return;
                    }
                }
            }

            // 从列表中移除
            m_VideoList.erase(m_VideoList.begin() + videoIndex);

            // 更新滚动范围
            m_ScrollRange = m_VideoList.size() * m_RowHeight;

            // 重新设置滚动信息
            SCROLLINFO si;
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMin = 0;
            si.nMax = m_ScrollRange;
            si.nPage = m_ClientArea.Height;

            // 如果当前滚动位置超出范围，则调整
            if (m_ScrollPos > m_ScrollRange - m_ClientArea.Height && m_ScrollRange > m_ClientArea.Height) {
                m_ScrollPos = m_ScrollRange - m_ClientArea.Height;
            }
            else if (m_ScrollRange <= m_ClientArea.Height) {
                m_ScrollPos = 0;
            }

            si.nPos = m_ScrollPos;
            SetScrollInfo(SB_VERT, &si, TRUE);

            // repaint整个窗口
            Invalidate();

            // 显示成功消息
            if (fileDeleted) {
                CString successMsg;
                successMsg.Format(L"已删除视频: %s", video.fileName.c_str());
                MessageBox(successMsg, L"删除成功", MB_OK | MB_ICONINFORMATION);
            }
        }
    }
    break;
    }
}

BOOL Ui_VideoListDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // 获取鼠标位置
    CPoint point = pt;
    ScreenToClient(&point);

    // 只有当鼠标在客户区域内时才处理滚动
    if (point.y >= 0 && point.y < m_BottomBtnArea.Y) {
        // 计算滚动量
        int scrollAmount = zDelta > 0 ? -30 : 30;

        // 更新滚动位置
        int newScrollPos = m_ScrollPos + scrollAmount;

        // 限制滚动范围
        newScrollPos = max(0, min(newScrollPos, m_ScrollRange - m_ScrollPageSize));

        // 如果滚动位置有变化，repaint
        if (newScrollPos != m_ScrollPos) {
            int oldScrollPos = m_ScrollPos;
            m_ScrollPos = newScrollPos;

            // 只repaint客户区域
            CRect invalidRect(
                0,                 // 左边界
                0, // 上边界
                m_WindowWidth,     // 右边界
                m_BottomBtnArea.Y  // 下边界
            );
            InvalidateRect(invalidRect, FALSE);
        }
    }

    return TRUE;
}

void Ui_VideoListDlg::BeginRenameEdit(int videoIndex)
{
    if (videoIndex < 0 || videoIndex >= (int)m_VideoList.size()) {
        return;
    }

    // 获取需要编辑的视频
    VideoInfo& video = m_VideoList[videoIndex];

    // 查找对应行的文件名区域
    float checkboxWidth = 40 * m_Scale;
    float fileNameWidth = 180 * m_Scale;
    float startY = 0;
    float rowY = startY + videoIndex * m_RowHeight - m_ScrollPos;

    // 创建编辑框（如果还不存在）
    if (!m_editRename.GetSafeHwnd())
    {
        // 创建编辑控件 (IDC_EDIT_RENAME需要在资源中定义)
        m_editRename.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            CRect(0, 0, 0, 0), this, IDC_EDIT_RENAME);

        // 设置字体
        LOGFONT lf = { 0 };
        wcscpy_s(lf.lfFaceName, L"微软雅黑");
        lf.lfHeight = -MulDiv(9 * m_Scale, GetDeviceCaps(::GetDC(NULL), LOGPIXELSY), 72);
        CFont* pFont = new CFont();
        pFont->CreateFontIndirect(&lf);
        m_editRename.SetFont(pFont, TRUE);
        pFont->Detach(); // 控件将接管字体
        delete pFont;
    }

    // 调整编辑框位置和大小
    CRect editRect(
        checkboxWidth,                   // X起始位置
        rowY,                            // Y起始位置
        checkboxWidth + fileNameWidth,   // 右边界
        rowY + m_RowHeight              // 下边界
    );
    m_editRename.MoveWindow(editRect);

    // 设置文本并显示
    m_editRename.SetWindowText(video.fileName.c_str());
    m_editRename.ShowWindow(SW_SHOW);
    m_editRename.SetFocus();
    m_editRename.SetSel(0, -1); // 全选文本

    // 更新状态
    m_isEditing = true;
    m_editingVideoIndex = videoIndex;
}

void Ui_VideoListDlg::EndRenameEdit(bool save)
{
    if (!m_isEditing) {
        return;
    }

    if (save && m_editingVideoIndex >= 0 && m_editingVideoIndex < (int)m_VideoList.size()) {
        // 获取编辑框文本
        CString newName;
        m_editRename.GetWindowText(newName);

        if (!newName.IsEmpty()) {
            // 更新视频文件名
            m_VideoList[m_editingVideoIndex].fileName = newName.GetString();

            // 实际项目中，这里应该实现文件系统上的实际重命名
            // RenameVideoFile(m_VideoList[m_editingVideoIndex].filePath, newName);
        }
    }

    // 隐藏编辑框
    m_editRename.ShowWindow(SW_HIDE);

    // 重置编辑状态
    m_isEditing = false;
    m_editingVideoIndex = -1;

    // repaint列表
    Invalidate(FALSE);
}

void Ui_VideoListDlg::OnEnKillFocusEditRename()
{
    EndRenameEdit(true);
}

void Ui_VideoListDlg::OnEnUpdateEditRename()
{
    // 可以在这里实现实时验证或其他功能
}

void Ui_VideoListDlg::DrawScrollbar(Gdiplus::Graphics* graphics)
{
    // 如果不需要滚动，则不绘制滚动条
    if (m_ScrollRange <= m_ScrollPageSize) {
        return;
    }

    // 滚动条区域
    const int scrollbarWidth = 8;
    Gdiplus::Rect scrollbarRect(
        m_WindowWidth - scrollbarWidth - 4,  // 右侧留出一点边距
        0,
        scrollbarWidth,
        m_ClientArea.Height
    );

    // 绘制滚动条轨道
    Gdiplus::SolidBrush trackBrush(Gdiplus::Color(40, 45, 51));
    graphics->FillRectangle(&trackBrush, scrollbarRect);

    // 计算滑块高度和位置
    float thumbRatio = (float)m_ScrollPageSize / m_ScrollRange;
    thumbRatio = min(thumbRatio, 1.0f); // 确保不超过1.0

    int thumbHeight = (int)(scrollbarRect.Height * thumbRatio);
    thumbHeight = max(thumbHeight, 30); // 最小高度

    float scrollRatio = (float)m_ScrollPos / (m_ScrollRange - m_ScrollPageSize);
    if (m_ScrollRange <= m_ScrollPageSize) {
        scrollRatio = 0;
    }

    int thumbY = scrollbarRect.Y + (int)((scrollbarRect.Height - thumbHeight) * scrollRatio);

    // 绘制滑块
    Gdiplus::Rect thumbRect(
        scrollbarRect.X,
        thumbY,
        scrollbarRect.Width,
        thumbHeight
    );

    Gdiplus::SolidBrush thumbBrush(Gdiplus::Color(100, 100, 100));
    graphics->FillRectangle(&thumbBrush, thumbRect);

    // 如果鼠标悬停在滑块上，绘制高亮效果
    if (m_IsScrollbarHovered) {
        Gdiplus::SolidBrush hoverBrush(Gdiplus::Color(120, 120, 120));
        graphics->FillRectangle(&hoverBrush, thumbRect);
    }
}

BOOL Ui_VideoListDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN) {
        if (pMsg->wParam == VK_RETURN) {
            if (m_isEditing && ::GetFocus() == m_editRename.m_hWnd) {
                EndRenameEdit(true);
                return TRUE;
            }
        }
        else if (pMsg->wParam == VK_ESCAPE) {
            if (m_isEditing) {
                EndRenameEdit(false); // 不保存
                return TRUE;
            }
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}

void Ui_VideoListDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
    pDlg->m_Dlg_Child->HideListBox();
    App.m_Dlg_Main.HideUserProfile();
    // 检查是否在滚动条区域内
    const int scrollbarWidth = 8;
    CRect scrollbarRect(
        m_WindowWidth - scrollbarWidth - 4,
        0,
        m_WindowWidth,
        m_BottomBtnArea.Y
    );

    bool wasHovered = m_IsScrollbarHovered;
    m_IsScrollbarHovered = scrollbarRect.PtInRect(point);

    // 如果悬停状态改变，repaint滚动条
    if (wasHovered != m_IsScrollbarHovered) {
        InvalidateRect(scrollbarRect, FALSE);
    }

    // 处理滚动条拖动
    if (m_IsScrollbarDragging) {
        int deltaY = point.y - m_ScrollbarDragStartY;
        float dragRatio = (float)deltaY / (m_ClientArea.Height - 30); // 30是最小滑块高度

        int newScrollPos = m_ScrollbarDragStartPos + (int)(dragRatio * (m_ScrollRange - m_ScrollPageSize));
        newScrollPos = max(0, min(newScrollPos, m_ScrollRange - m_ScrollPageSize));

        if (newScrollPos != m_ScrollPos) {
            m_ScrollPos = newScrollPos;
            // 只repaint客户区
            InvalidateRect(CRect(0, 0, m_WindowWidth, m_BottomBtnArea.Y), FALSE);
        }
    }

    // 检查是否悬停在按钮上
    int oldHoverIndex = m_HoverButtonIndex;
    int oldHoverType = m_HoverButtonType;

    m_HoverButtonIndex = -1;
    m_HoverButtonType = -1;

    for (const auto& btn : m_ButtonAreas) {
        // 仅检查三种操作按钮（播放、打开、删除），跳过多选框
        if (btn.buttonType != BTN_TYPE_CHECKBOX &&
            point.x >= btn.rect.X &&
            point.x <= btn.rect.X + btn.rect.Width &&
            point.y >= btn.rect.Y &&
            point.y <= btn.rect.Y + btn.rect.Height) {

            m_HoverButtonIndex = btn.videoIndex;
            m_HoverButtonType = btn.buttonType;
            break;
        }
    }

    // 如果悬停状态发生变化，repaint按钮
    if (oldHoverIndex != m_HoverButtonIndex || oldHoverType != m_HoverButtonType) {
        // 如果之前有悬停的按钮，repaint该按钮区域
        if (oldHoverIndex != -1) {
            for (const auto& btn : m_ButtonAreas) {
                if (btn.videoIndex == oldHoverIndex && btn.buttonType == oldHoverType) {
                    CRect invalidRect(btn.rect.X, btn.rect.Y,
                        btn.rect.X + btn.rect.Width,
                        btn.rect.Y + btn.rect.Height);

                    //repaint按钮为悬停状态绘画
                    InvalidateRect(invalidRect, FALSE);
                    break;
                }
            }
        }

        // 如果现在有悬停的按钮，repaint该按钮区域
        if (m_HoverButtonIndex != -1)
        {
            for (const auto& btn : m_ButtonAreas)
            {
                if (btn.videoIndex == m_HoverButtonIndex && btn.buttonType == m_HoverButtonType)
                {
                    CRect invalidRect(btn.rect.X, btn.rect.Y,
                        btn.rect.X + btn.rect.Width,
                        btn.rect.Y + btn.rect.Height);

                    //按钮提示窗口出现
                    if (btn.buttonType == 2)
                        m_ToolTip.LarSetTipText(L"播放");
                    else if (btn.buttonType == 3)
                        m_ToolTip.LarSetTipText(L"打开");
                    else if (btn.buttonType == 4)
                        m_ToolTip.LarSetTipText(L"删除");

                    CRect rcRect;
                    GetWindowRect(rcRect);
                    m_ToolTip.ShowToolTipsWindow(rcRect.left + btn.rect.X - 11 * m_Scale, rcRect.top + btn.rect.Y + btn.rect.Height + 8 * m_Scale);
                    InvalidateRect(invalidRect, FALSE);

                    auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
                    pDlg->m_Dlg_Child->HideListBox();
                    break;
                }
            }
        }
        else
        {
            if(m_ToolTip.IsWindowVisible())
                m_ToolTip.ShowWindow(SW_HIDE);
        }
    }

    CDialogEx::OnMouseMove(nFlags, point);
}

void Ui_VideoListDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_IsScrollbarDragging) {
        m_IsScrollbarDragging = false;
        ReleaseCapture();
    }

    CDialogEx::OnLButtonUp(nFlags, point);
}

void Ui_VideoListDlg::OnBnClickedBtnReturn()
{
    if (m_pCWnd_ReturnDlg)
    {
        auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
        auto preturnDlg = (Ui_GamingDlg*)m_pCWnd_ReturnDlg;
        preturnDlg->ShowWindow(SW_SHOW);
        preturnDlg->SetTimer(Ui_ConfigDlg::TIMER_DELAYED_REDRAW, 50, NULL);
        this->ShowWindow(SW_HIDE);
    }
    else
    {
        GetParent()->ShowWindow(SW_SHOW);
        this->ShowWindow(SW_HIDE);
    }
    m_Shadow.Show(m_hWnd);
}

void Ui_VideoListDlg::OnBnClickedBtnClose()
{
    PostQuitMessage(0);
}

void Ui_VideoListDlg::EnableShadow()
{
    // 启用非客户区渲染
    DWORD dwPolicy = DWMNCRP_ENABLED;
    HRESULT hr = DwmSetWindowAttribute(this->GetSafeHwnd(), DWMWA_NCRENDERING_POLICY, &dwPolicy, sizeof(dwPolicy));
    if (FAILED(hr))
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, _T("DwmSetWindowAttribute 调用失败, HRESULT=0x%x\n"), hr);
    }

    // 扩展窗口客户区以显示阴影，margins 可根据需求调整
    MARGINS margins = { 1, 1, 1, 1 };
    hr = DwmExtendFrameIntoClientArea(this->GetSafeHwnd(), &margins);
    if (FAILED(hr))
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, _T("DwmExtendFrameIntoClientArea 调用失败, HRESULT=0x%x\n"), hr);
    }

}

bool Ui_VideoListDlg::GetVideoInfoUsingFFmpeg(const CString& filePath, VideoInfo& video)
{
    // 保存当前字符转换区域设置
    _locale_t prevLocale = _get_current_locale();

    // 使用UTF-8编码处理路径，确保支持中文
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
    char* utf8Path = new char[utf8Length];
    WideCharToMultiByte(CP_UTF8, 0, filePath, -1, utf8Path, utf8Length, NULL, NULL);

    AVFormatContext* formatContext = nullptr;
    bool success = false;

    try {
        // 防止FFmpeg内部的locale问题
        setlocale(LC_ALL, "C");

        // 打开媒体文件
        if (avformat_open_input(&formatContext, utf8Path, NULL, NULL) != 0) {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"无法打开视频文件: %s\n", LARSC::c2w(utf8Path));
            throw std::runtime_error("无法打开视频文件");
        }

        // 读取流信息
        if (avformat_find_stream_info(formatContext, NULL) < 0) {
            throw std::runtime_error("无法读取流信息");
        }

        // 获取视频时长（以秒为单位）
        int64_t duration = formatContext->duration;
        if (duration != AV_NOPTS_VALUE) {
            // 将时长从AV_TIME_BASE单位转换为秒
            double seconds = duration / (double)AV_TIME_BASE;

            // 转换为时:分:秒格式
            int hours = static_cast<int>(seconds / 3600);
            int minutes = static_cast<int>((seconds - hours * 3600) / 60);
            int secs = static_cast<int>(seconds - hours * 3600 - minutes * 60);

            wchar_t durationBuffer[20];
            swprintf_s(durationBuffer, L"%02d:%02d:%02d", hours, minutes, secs);
            video.duration = durationBuffer;

            success = true;
        }
    }
    catch (const std::exception& e) {
        // 发生错误，设置默认时长
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"FFmpeg error: %s\n", LARSC::c2w(e.what()));
        video.duration = L"00:00:00";
    }

    // 清理资源
    if (formatContext) {
        avformat_close_input(&formatContext);
    }

    delete[] utf8Path;

    // 恢复locale设置
    _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
    setlocale(LC_ALL, "");

    return success;
}

void Ui_VideoListDlg::LoadUserAddedVideosFromConfig()
{
    // 获取应用程序路径
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);

    // 去掉文件名，只保留路径
    PathRemoveFileSpec(szPath);

    // 构建配置文件路径
    CString configPath = szPath;
    configPath += _T("\\UserVideos.ini");

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"尝试从配置文件加载: %s", (LPCTSTR)configPath);

    // 检查配置文件是否存在
    if (GetFileAttributes(configPath) == INVALID_FILE_ATTRIBUTES)
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"配置文件不存在: %s", (LPCTSTR)configPath);
        return; // 配置文件不存在，直接返回
    }

    try
    {
        int videoCount = 0;

        // 方法1: 使用Win32 API直接读取Unicode文本文件
        // 这种方法更适合处理多种可能的编码情况
        HANDLE hFile = CreateFile(configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            // 检测文件编码格式
            DWORD bytesRead = 0;
            BYTE bom[4] = { 0 }; // BOM标记
            ReadFile(hFile, bom, 3, &bytesRead, NULL);

            // 检查是否有BOM标记，确定编码方式
            bool isUTF8 = (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF);
            bool isUTF16LE = (bom[0] == 0xFF && bom[1] == 0xFE);
            bool isUTF16BE = (bom[0] == 0xFE && bom[1] == 0xFF);

            // 重置文件指针到开始位置
            SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

            // 获取文件大小
            DWORD fileSize = GetFileSize(hFile, NULL);
            if (fileSize != INVALID_FILE_SIZE && fileSize > 0)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"成功打开配置文件");

                // 根据编码类型决定如何读取
                if (isUTF16LE || isUTF16BE)
                {
                    // 处理UTF-16编码
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到UTF-16编码");

                    // 分配缓冲区，包含BOM
                    DWORD bufferSize = fileSize + 2;
                    wchar_t* buffer = new wchar_t[bufferSize / 2];
                    ZeroMemory(buffer, bufferSize);

                    // 读取文件内容
                    DWORD bytesRead = 0;
                    ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);

                    // 跳过BOM (如果有)
                    wchar_t* content = buffer;
                    if (buffer[0] == 0xFEFF || buffer[0] == 0xFFFE)
                    {
                        content++;
                    }

                    // 处理文件内容
                    ProcessConfigFileContent(content, ConsoleHandle);

                    delete[] buffer;
                }
                else // UTF-8或ANSI
                {
                    // 跳过UTF-8的BOM (如果有)
                    DWORD skipBytes = isUTF8 ? 3 : 0;
                    SetFilePointer(hFile, skipBytes, NULL, FILE_BEGIN);

                    // 调整读取大小
                    DWORD actualSize = fileSize - skipBytes;

                    // 分配缓冲区
                    char* buffer = new char[actualSize + 1];
                    ZeroMemory(buffer, actualSize + 1);

                    // 读取文件内容
                    DWORD bytesRead = 0;
                    ReadFile(hFile, buffer, actualSize, &bytesRead, NULL);

                    // 确保字符串以null结尾
                    buffer[bytesRead] = 0;

                    if (isUTF8)
                    {
                        DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到UTF-8编码");

                        // 将UTF-8转换为Unicode
                        int wideLength = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, NULL, 0);
                        wchar_t* wideBuffer = new wchar_t[wideLength];
                        MultiByteToWideChar(CP_UTF8, 0, buffer, -1, wideBuffer, wideLength);

                        // 处理文件内容
                        ProcessConfigFileContent(wideBuffer, ConsoleHandle);

                        delete[] wideBuffer;
                    }
                    else
                    {
                        DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到ANSI编码");

                        // 将ANSI转换为Unicode (使用当前代码页)
                        int wideLength = MultiByteToWideChar(CP_ACP, 0, buffer, -1, NULL, 0);
                        wchar_t* wideBuffer = new wchar_t[wideLength];
                        MultiByteToWideChar(CP_ACP, 0, buffer, -1, wideBuffer, wideLength);

                        // 处理文件内容
                        ProcessConfigFileContent(wideBuffer, ConsoleHandle);

                        delete[] wideBuffer;
                    }

                    delete[] buffer;
                }
            }

            CloseHandle(hFile);
        }
        else
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"无法打开配置文件，错误码: %d", GetLastError());
        }
    }
    catch (std::exception& e)
    {
        // 文件操作异常处理
        char* pMBBuffer = (char*)e.what();
        WCHAR pWCBuffer[4096];
        MultiByteToWideChar(CP_ACP, 0, pMBBuffer, strlen(pMBBuffer), pWCBuffer, 4096);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"读取配置文件异常: %s", pWCBuffer);
    }
}

void Ui_VideoListDlg::ProcessConfigFileContent(const wchar_t* content, HANDLE ConsoleHandle)
{
    // 解析配置文件内容
    std::wstring fileContent(content);
    std::wistringstream stream(fileContent);
    std::wstring line;

    int videoCount = 0;
    bool inUserVideosSection = false;
    std::map<int, VideoInfo> tempVideos; // 临时存储视频信息

    // 逐行处理
    while (std::getline(stream, line))
    {
        // 移除两端空白
        line = TrimString(line);

        // 跳过空行
        if (line.empty())
            continue;

        // 检查节名
        if (line == L"[UserAddedVideos]")
        {
            inUserVideosSection = true;
            DEBUG_CONSOLE_STR(ConsoleHandle, L"找到用户添加视频节");
            continue;
        }
        else if (line[0] == '[')
        {
            // 进入其他节，结束读取
            inUserVideosSection = false;
            continue;
        }

        // 确保在正确的节中
        if (!inUserVideosSection)
            continue;

        // 提取键值对
        size_t equalPos = line.find('=');
        if (equalPos != std::wstring::npos)
        {
            std::wstring key = TrimString(line.substr(0, equalPos));
            std::wstring value = TrimString(line.substr(equalPos + 1));

            // 获取视频总数
            if (key == L"Count")
            {
                videoCount = _wtoi(value.c_str());
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"配置文件中有 %d 个用户添加的视频", videoCount);
                continue;
            }

            // 解析视频索引和属性
            // 格式: VideoX_PropertyName=Value
            if (key.substr(0, 5) == L"Video")
            {
                size_t underscorePos = key.find('_', 5);
                if (underscorePos != std::wstring::npos)
                {
                    // 提取视频索引
                    std::wstring indexStr = key.substr(5, underscorePos - 5);
                    int index = _wtoi(indexStr.c_str());

                    // 提取属性名
                    std::wstring propName = key.substr(underscorePos + 1);

                    // 确保索引和属性有效
                    if (index > 0 && !propName.empty())
                    {
                        // 根据属性名存储相应的值
                        if (propName == L"Path")
                        {
                            tempVideos[index].filePath = value;
                            DEBUG_CONSOLE_FMT(ConsoleHandle, L"读取视频 %d 路径: %s", index, value.c_str());
                        }
                        else if (propName == L"Name")
                        {
                            tempVideos[index].fileName = value;
                            DEBUG_CONSOLE_FMT(ConsoleHandle, L"读取视频 %d 名称: %s", index, value.c_str());
                        }
                        else if (propName == L"Duration")
                        {
                            tempVideos[index].duration = value;
                        }

                        // 标记为用户添加的视频
                        tempVideos[index].isUserAdded = true;
                        tempVideos[index].selected = false;
                        tempVideos[index].selectOrder = 0;
                    }
                }
            }
        }
    }

    // 添加有效的视频到列表
    int loadedCount = 0;
    for (int i = 1; i <= videoCount; i++)
    {
        auto it = tempVideos.find(i);
        if (it != tempVideos.end())
        {
            VideoInfo& video = it->second;

            // 检查必要的字段是否存在
            if (!video.filePath.empty() && !video.fileName.empty())
            {
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"检查视频文件是否存在: %s", video.filePath.c_str());

                // 检查文件是否存在
                if (GetFileAttributes(video.filePath.c_str()) != INVALID_FILE_ATTRIBUTES)
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"文件存在，添加到列表");

                    // 添加缺失的信息
                    if (video.duration.empty())
                        video.duration = L"00:00:00";

                    // 获取文件大小和日期信息（如果缺少）
                    if (video.fileSize.empty() || video.fileDate.empty() || video.recordDate.empty())
                    {
                        WIN32_FIND_DATA findData;
                        HANDLE hFind = FindFirstFile(video.filePath.c_str(), &findData);
                        if (hFind != INVALID_HANDLE_VALUE)
                        {
                            // 文件大小
                            if (video.fileSize.empty())
                            {
                                ULARGE_INTEGER fileSize;
                                fileSize.LowPart = findData.nFileSizeLow;
                                fileSize.HighPart = findData.nFileSizeHigh;
                                double sizeMB = static_cast<double>(fileSize.QuadPart) / (1024 * 1024);

                                wchar_t buffer[50];
                                if (sizeMB >= 1000) {
                                    swprintf_s(buffer, L"%.2f GB", sizeMB / 1000.0);
                                }
                                else {
                                    swprintf_s(buffer, L"%.2f MB", sizeMB);
                                }
                                video.fileSize = buffer;
                            }

                            // 文件日期
                            if (video.fileDate.empty() || video.recordDate.empty())
                            {
                                SYSTEMTIME stUTC, stLocal;
                                FileTimeToSystemTime(&findData.ftCreationTime, &stUTC);
                                SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

                                if (video.fileDate.empty())
                                {
                                    wchar_t dateBuffer[30];
                                    swprintf_s(dateBuffer, L"%04d-%02d-%02d",
                                        stLocal.wYear, stLocal.wMonth, stLocal.wDay);
                                    video.fileDate = dateBuffer;
                                }

                                if (video.recordDate.empty())
                                {
                                    wchar_t recordDateBuffer[50];
                                    swprintf_s(recordDateBuffer, L"%04d-%02d-%02d %02d:%02d",
                                        stLocal.wYear, stLocal.wMonth, stLocal.wDay,
                                        stLocal.wHour, stLocal.wMinute);
                                    video.recordDate = recordDateBuffer;
                                }
                            }

                            FindClose(hFind);
                        }
                    }

                    // 使用FFmpeg获取视频时长（如果尚未设置或为默认值）
                    if (video.duration == L"00:00:00")
                    {
                        CString filePath(video.filePath.c_str());
                        GetVideoInfoUsingFFmpeg(filePath, video);
                    }

                    // 添加到视频列表
                    m_VideoList.insert(m_VideoList.begin(), video);
                    loadedCount++;
                }
                else
                {
                    // 文件不存在，记录日志
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"配置文件中的视频不存在: %s", video.filePath.c_str());
                }
            }
        }
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"从配置文件成功加载了 %d 个用户添加的视频", loadedCount);
}

void Ui_VideoListDlg::WriteToConfigFile()
{
    // 1. 取可执行文件所在目录
    wchar_t modulePath[MAX_PATH] = { 0 };
    ::GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    std::wstring exeDir(modulePath);
    size_t pos = exeDir.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        exeDir.resize(pos + 1);
    else
        exeDir += L"\\";

    // 2. 拼出 ini 文件的宽字符路径
    std::wstring wIniPath = exeDir + L"UserVideos.ini";

    // 3. 将宽字符串路径转为 UTF-8，以便 std::ofstream 打开
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string iniPathUtf8 = conv.to_bytes(wIniPath);

    // 4. 打开并截断旧文件（若不存在则创建）
    std::ofstream ofs(iniPathUtf8, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!ofs.is_open())
        return;

    // 辅助：写一行宽字符串，末尾加 CRLF
    auto writeLine = [&](const std::wstring& wline) {
        ofs << conv.to_bytes(wline) << "\r\n";
    };

    // 5. 写入节头和 Count
    writeLine(L"[UserAddedVideos]");
    writeLine(L"Count=" + std::to_wstring(m_VideoList.size()));

    // 6. 按格式写每条记录的 Path/Name/Duration
    for (size_t i = 0; i < m_VideoList.size(); ++i)
    {
        const VideoInfo& v = m_VideoList[i];
        std::wstring idx = std::to_wstring(i + 1);

        writeLine(L"Video" + idx + L"_Path=" + v.filePath);
        writeLine(L"Video" + idx + L"_Name=" + v.fileName);
        writeLine(L"Video" + idx + L"_Duration=" + v.duration);
    }

    ofs.close();
}

bool Ui_VideoListDlg::IsListEmpty()
{
    if (m_VideoList.empty())
        return true;
    else
        return false;
}

void Ui_VideoListDlg::EmptyMsgUIShow()
{
    m_Btn_GoRecord.ShowWindow(SW_HIDE);
}

void Ui_VideoListDlg::HideToolWindow()
{
    if (m_ToolTip.IsWindowVisible())
    {
        m_ToolTip.ShowWindow(SW_HIDE);
    }
}

void Ui_VideoListDlg::HideShadow()
{
    m_Shadow.HideShadow();
    Invalidate(false);
}

void Ui_VideoListDlg::RestoreShadow()
{
    m_Shadow.RestoreFromHide();
    Invalidate(false);
}

std::wstring Ui_VideoListDlg::TrimString(const std::wstring& str)
{
    const wchar_t* whitespaces = L" \t\n\r\f\v";
    size_t start = str.find_first_not_of(whitespaces);
    if (start == std::wstring::npos)
        return L"";

    size_t end = str.find_last_not_of(whitespaces);
    return str.substr(start, end - start + 1);
}

BOOL Ui_VideoListDlg::OnNcActivate(BOOL bActive)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    //EnableShadow();
    return CDialogEx::OnNcActivate(bActive);
}

void Ui_VideoListDlg::OnBnClickedBtnOpenfolde()
{
    // 获取视频保存路径
    CString videoPath;
    auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
    pDlg->m_Dlg_Config->m_Btn_Path.GetWindowTextW(videoPath);
    pDlg->m_Dlg_Child->HideListBox();

    // 如果路径为空，使用默认路径
    if (videoPath.IsEmpty()) {
        videoPath = GlobalFunc::GetDefaultVideoSavePath();
    }

    // 确保目录存在
    if (GetFileAttributes(videoPath) == INVALID_FILE_ATTRIBUTES) {
        // 目录不存在，尝试创建
        if (!CreateDirectory(videoPath, NULL)) {
            MessageBox(_T("无法打开或创建视频文件夹！"), _T("错误"), MB_ICONERROR);
            return;
        }
    }

    // 使用ShellExecute打开文件夹
    HINSTANCE result = ShellExecute(
        NULL,           // 父窗口句柄
        _T("open"),     // 操作
        videoPath,      // 要打开的文件夹路径
        NULL,           // 参数
        NULL,           // 默认目录
        SW_SHOWNORMAL   // 显示命令
    );

    // 检查是否成功打开
    if ((INT_PTR)result <= 32) {
        // ShellExecute失败
        MessageBox(_T("无法打开视频文件夹！"), _T("错误"), MB_ICONERROR);
    }
}

void Ui_VideoListDlg::OnBnClickedBtnAddvideo()
{
    m_Shadow.SetDarkness(0);
    auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
    pDlg->m_Dlg_Child->HideListBox();
    // 创建打开文件对话框
    CFileDialog fileDlg(
        TRUE,                       // TRUE为打开文件对话框
        NULL,                       // 默认扩展名
        NULL,                       // 默认文件名
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, // 标志
        _T("视频文件(*.mp4;*.avi;*.flv)|*.mp4;*.avi;*.flv|所有文件(*.*)|*.*||"),  // 过滤器
        NULL                        // 父窗口
    );

    // 显示对话框并获取用户选择
    if (fileDlg.DoModal() == IDOK)
    {
        // 获取选中的文件路径
        CString filePath = fileDlg.GetPathName();
        CString fileName = fileDlg.GetFileName();

        // 检查文件格式
        CString fileExt = filePath.Mid(filePath.ReverseFind('.')).MakeLower();
        if (fileExt != _T(".mp4") && fileExt != _T(".avi") && fileExt != _T(".flv"))
        {
            MessageBox(_T("只支持MP4、AVI和FLV格式的视频文件！"), _T("格式错误"), MB_ICONWARNING);
            return;
        }

        // 创建新的视频信息
        VideoInfo newVideo;
        newVideo.fileName = fileName.GetBuffer();
        newVideo.filePath = filePath.GetBuffer();
        newVideo.isUserAdded = true;  // 标记为用户添加
        newVideo.selected = false;
        newVideo.selectOrder = 0;

        // 获取文件大小
        HANDLE hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER fileSize;
            if (GetFileSizeEx(hFile, &fileSize))
            {
                double sizeMB = static_cast<double>(fileSize.QuadPart) / (1024 * 1024);
                wchar_t buffer[50];
                if (sizeMB >= 1000) {
                    swprintf_s(buffer, L"%.2f GB", sizeMB / 1000.0);
                }
                else {
                    swprintf_s(buffer, L"%.2f MB", sizeMB);
                }
                newVideo.fileSize = buffer;
            }
            CloseHandle(hFile);
        }

        // 获取文件日期
        WIN32_FIND_DATA findData;
        HANDLE hFind = FindFirstFile(filePath, &findData);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            SYSTEMTIME stUTC, stLocal;
            FileTimeToSystemTime(&findData.ftCreationTime, &stUTC);
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

            wchar_t dateBuffer[30];
            swprintf_s(dateBuffer, L"%04d-%02d-%02d",
                stLocal.wYear, stLocal.wMonth, stLocal.wDay);
            newVideo.fileDate = dateBuffer;

            wchar_t recordDateBuffer[50];
            swprintf_s(recordDateBuffer, L"%04d-%02d-%02d %02d:%02d",
                stLocal.wYear, stLocal.wMonth, stLocal.wDay,
                stLocal.wHour, stLocal.wMinute);
            newVideo.recordDate = recordDateBuffer;

            FindClose(hFind);
        }

        // 获取视频时长
        newVideo.duration = L"00:00:00";  // 默认值
        GetVideoInfoUsingFFmpeg(filePath, newVideo);

        // 添加到列表顶部
        m_VideoList.insert(m_VideoList.begin(), newVideo);

        // 更新滚动范围
        m_ScrollRange = m_VideoList.size() * m_RowHeight;

        // 记录添加信息
        DEBUG_CONSOLE_FMT(ConsoleHandle, _T("用户添加视频: %s, 完整路径: %s\n"),
            fileName, filePath);

        // 更新视图
        Invalidate();

        // 提示添加成功
        MessageBox(_T("视频已添加到列表！"), _T("添加成功"), MB_ICONINFORMATION);
    }
    m_Shadow.SetDarkness();
}

void Ui_VideoListDlg::OnBnClickedBtnDeleteselect()
{
    DB(ConsoleHandle, L"开始删除选中视频列");
    auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
    pDlg->m_Dlg_Child->HideListBox();
    // 计算选中的视频数量
    int selectedCount = 0;
    for (const auto& video : m_VideoList)
    {
        if (video.selected)
            selectedCount++;
    }

    // 如果没有选中的视频，则提示用户并返回
    if (selectedCount == 0)
    {
        MessageBox(_T("请先选择要删除的视频"), _T("提示"), MB_ICONINFORMATION);
        return;
    }

    // 确认是否要删除
    CString confirmMsg;
    confirmMsg.Format(_T("确定要删除选中的 %d 个视频吗？\n注意：这将会同时删除视频文件！"), selectedCount);
    if (MessageBox(confirmMsg, _T("确认删除"), MB_YESNO | MB_ICONWARNING) != IDYES)
    {
        return;  // 用户取消删除
    }

    // 记录删除前的视频数量
    size_t originalSize = m_VideoList.size();
    int successDelete = 0;   // 成功删除的文件计数
    int failDelete = 0;      // 删除失败的文件计数
    std::vector<std::wstring> failedFiles; // 删除失败的文件名列表

    // 创建临时向量存储要保留的视频
    std::vector<VideoInfo> remainingVideos;

    // 遍历视频列表
    for (const auto& video : m_VideoList)
    {
        if (video.selected)
        {
            // 尝试删除实际文件
            BOOL deleteSuccess = FALSE;

            // 确保路径有效
            if (!video.filePath.empty())
            {
                // 设置文件属性为普通，移除只读等属性
                SetFileAttributes(video.filePath.c_str(), FILE_ATTRIBUTE_NORMAL);

                // 删除文件
                deleteSuccess = DeleteFile(video.filePath.c_str());

                if (deleteSuccess)
                {
                    // 文件删除成功
                    successDelete++;
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"成功删除文件: %s", video.filePath.c_str());
                }
                else
                {
                    // 删除失败，记录错误
                    failDelete++;
                    DWORD error = GetLastError();
                    failedFiles.push_back(video.fileName);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"删除文件失败: %s, 错误码: %d",
                        video.filePath.c_str(), error);
                }
            }
            else
            {
                // 文件路径为空，计为失败
                failDelete++;
                failedFiles.push_back(video.fileName);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"删除失败: 文件路径为空");
            }
        }
        else
        {
            // 未选中的视频保留
            remainingVideos.push_back(video);
        }
    }

    // 使用保留的视频更新列表
    m_VideoList = remainingVideos;
    for (const auto& video : m_VideoList)
    {
        DBFMT(ConsoleHandle, L"更新后的视频列表容器m_VideoList，fileName：%s,selectOrder:%d",
            video.fileName.c_str(), video.selectOrder);
    }
   

    // 计算实际删除的视频数量
    size_t removedCount = originalSize - m_VideoList.size();

    // 输出调试信息
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"已从列表中删除 %d 个选中的视频", removedCount);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"成功删除 %d 个文件, 失败 %d 个", successDelete, failDelete);

    // 重新设置序号和选择状态
    for (size_t i = 0; i < m_VideoList.size(); i++)
    {
        m_VideoList[i].selectOrder = 0;
        DBFMT(ConsoleHandle, L"保留的视频列表项fileName:%s,selectOrder:%d", m_VideoList[i].fileName.c_str(), m_VideoList[i].selectOrder);
    }
    m_selectionCounter = 0;

    // 更新滚动范围
    m_ScrollRange = m_VideoList.size() * m_RowHeight;

    // 重新计算垂直滚动位置
    if (m_ScrollPos > m_ScrollRange - m_ClientArea.Height && m_ScrollRange > m_ClientArea.Height) {
        m_ScrollPos = m_ScrollRange - m_ClientArea.Height;
    }
    else if (m_ScrollRange <= m_ClientArea.Height) {
        m_ScrollPos = 0; // 如果不需要滚动，重置位置到顶部
    }

    // repaint界面以反映变化
    Invalidate();

    m_Btn_DeleteSelect.SetWindowTextW(L"批量删除");

    // 显示删除结果的提示
    if (removedCount > 0)
    {
        CString successMsg;
        if (failDelete == 0)
        {
            // 全部删除成功
            successMsg.Format(_T("已删除 %d 个视频文件"), successDelete);
            MessageBox(successMsg, _T("删除成功"), MB_ICONINFORMATION);
        }
        else
        {
            // 部分删除失败
            successMsg.Format(_T("已删除 %d 个视频文件，%d 个文件删除失败"),
                successDelete, failDelete);

            // 如果失败数量较少，显示失败的文件名
            if (failDelete <= 5)
            {
                successMsg += _T("\n\n删除失败的文件：\n");
                for (const auto& file : failedFiles)
                {
                    successMsg += file.c_str();
                    successMsg += _T("\n");
                }
            }

            MessageBox(successMsg, _T("删除结果"), MB_ICONINFORMATION);
        }
    }
}

void Ui_VideoListDlg::OnBnClickedBtnMinimal()
{
    auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
    pDlg->m_Dlg_Child->HideListBox();
    m_Shadow.Show(m_hWnd);
    this->ShowWindow(SW_MINIMIZE);
}

void Ui_VideoListDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == TIMER_DELAYED_REDRAW)
    {
        if (m_redrawTimerCount <= 10)
            KillTimer(TIMER_DELAYED_REDRAW);
        // 执行repaint
        Invalidate();
        m_redrawTimerCount++;
    }
    CDialogEx::OnTimer(nIDEvent);
}

void Ui_VideoListDlg::OnMove(int x, int y)
{
    CDialogEx::OnMove(x, y);
    //m_Shadow.Show(m_hWnd);
    // TODO: 在此处添加消息处理程序代码
}

void Ui_VideoListDlg::OnMouseLeave()
{
    // 如果当前有按钮处于悬停状态，需要repaint该按钮
    if (m_HoverButtonIndex != -1 && m_HoverButtonType != -1) {
        for (const auto& btn : m_ButtonAreas) {
            if (btn.videoIndex == m_HoverButtonIndex && btn.buttonType == m_HoverButtonType) {
                CRect invalidRect(btn.rect.X, btn.rect.Y,
                    btn.rect.X + btn.rect.Width,
                    btn.rect.Y + btn.rect.Height);
                InvalidateRect(invalidRect, FALSE);
                break;
            }
        }
    }

    // 清除悬停状态
    m_HoverButtonIndex = -1;
    m_HoverButtonType = -1;

    // 如果滚动条处于悬停状态，也需要更新
    if (m_IsScrollbarHovered) {
        m_IsScrollbarHovered = false;

        const int scrollbarWidth = 8;
        CRect scrollbarRect(
            m_WindowWidth - scrollbarWidth - 4,
            0,
            m_WindowWidth,
            m_BottomBtnArea.Y
        );

        InvalidateRect(scrollbarRect, FALSE);
    }

    CDialogEx::OnMouseLeave();
}

void Ui_VideoListDlg::OnBnClickedBtnGorecord()
{
    
}
