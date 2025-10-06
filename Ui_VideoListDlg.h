#pragma once
#include "CLarPngBtn.h"
#include "CLarBtn.h"
#include "CLazerStaticText.h"
#include "resource.h"
#include "WndShadow.h"
#include "CLarToolsTips.h"
#include <vector>
#include <string>
#include <fstream>
#include <locale>
#include <codecvt>

// FFmpeg相关头文件
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
// 视频信息结构体
struct VideoInfo
{
    std::wstring filePath;      // 文件路径
    std::wstring fileName;      // 文件名
    std::wstring fileDate;      // 文件日期
    std::wstring duration;      // 视频时长
    std::wstring fileSize;      // 文件大小
    std::wstring recordDate;    // 录制日期
    bool selected;              // 是否被选中
    int selectOrder;            // 选择顺序号
    bool isUserAdded;           // 是否是用户手动添加的视频
};

// 按钮区域结构体
struct ButtonArea
{
    Gdiplus::RectF rect;        // 按钮区域
    int videoIndex;             // 对应的视频索引
    int buttonType;             // 按钮类型: 0=多选框, 1=重命名, 2=播放, 3=打开, 4=删除
};

// 按钮类型常量
#define BTN_TYPE_CHECKBOX   0
#define BTN_TYPE_RENAME     1
#define BTN_TYPE_PLAY       2
#define BTN_TYPE_OPEN       3
#define BTN_TYPE_DELETE     4

class Ui_VideoListDlg : public CDialogEx
{
    DECLARE_DYNAMIC(Ui_VideoListDlg)

public:
    enum {
        TIMER_DELAYED_REDRAW = 1001  // 延迟repaint定时器ID
    };
    // 对话框数据
    enum { IDD = IDD_DIALOG_VIDEOLISTDLG };

    Ui_VideoListDlg(CWnd* pParent = nullptr);
    virtual ~Ui_VideoListDlg();

    void Ui_SetWindowRect(const CRect& rect);
    void Ui_UpdateWindowPos(const CRect& rect);
    void CleanUpGdiPngRes();//清理GDI资源（在GDI关闭之前调用）
    Gdiplus::Rect GetBottomBtnAreaRect() { return m_BottomBtnArea; }
    std::vector<VideoInfo> GetUserAddedVideos() const;// 获取用户添加的视频列表
    inline void SetReturnDlg(const CWnd* pCWnd) { m_pCWnd_ReturnDlg = pCWnd; }
    BOOL AddVideoToList(const CString& videoFilePath, BOOL showMessage = FALSE);
    void EnableCWndShadow();
    void WriteToConfigFile();
    bool IsListEmpty();
    void EmptyMsgUIShow();
    void HideToolWindow();
    void HideShadow();
    void RestoreShadow();
protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    DECLARE_MESSAGE_MAP()
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnMove(int x, int y);
    afx_msg void OnMouseLeave();
    afx_msg BOOL OnNcActivate(BOOL bActive);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

    //按钮响应
    afx_msg void OnEnKillFocusEditRename();
    afx_msg void OnEnUpdateEditRename();
    afx_msg void OnBnClickedBtnReturn();
    afx_msg void OnBnClickedBtnClose();
    afx_msg void OnBnClickedBtnOpenfolde();
    afx_msg void OnBnClickedBtnAddvideo();
    afx_msg void OnBnClickedBtnDeleteselect();
    afx_msg void OnBnClickedBtnMinimal();
    afx_msg void OnBnClickedBtnGorecord();
private:
    void BeginRenameEdit(int videoIndex);        // 开始重命名编辑
    void EndRenameEdit(bool save);               // 结束重命名编辑
    void ClearAllSelections();                   // 清楚所有的多选框选择
    void InitializeUI();                         // 初始化界面
    void LoadVideoList();                        // 加载视频列表
    void DrawVideoList(Gdiplus::Graphics* graphics);         // 绘制视频列表
    void DrawEmptyListMessage(Gdiplus::Graphics* graphics);  // 绘制空列表消息
    void HandleButtonClick(int videoIndex, int buttonType);  // 处理按钮点击
    void LoadResources();                   // 加载资源
    void CleanupResources();                // 释放资源
    void GetUserDPI();                      // 获取用户DPI
    void UpdateScale();                     // 更新缩放
    void DrawScrollbar(Gdiplus::Graphics* graphics);//绘画滚动条
    void EnableShadow();
    bool GetVideoInfoUsingFFmpeg(const CString& filePath, VideoInfo& video); // 获取视频时长

    //配置文件相关
    void LoadUserAddedVideosFromConfig();            // 从配置文件加载用户添加的视频
    std::wstring TrimString(const std::wstring& str);// 修剪字符串两端空白
    void ProcessConfigFileContent(const wchar_t* content, HANDLE ConsoleHandle);// 配置文件内容处理函数

private:
    // 窗口基本参数
    float m_Scale;                // DPI缩放系数
    CRect m_WindowRect;           // 窗口区域
    int m_WindowWidth;            // 窗口宽度
    int m_WindowHeight;           // 窗口高度
    int m_WindowX;                // 窗口左上角X坐标
    int m_WindowY;                // 窗口左上角Y坐标
    Gdiplus::Rect m_ClientArea;   // 客户区域
    Gdiplus::Rect m_BottomBtnArea;// 底部按钮区域 
    const CWnd* m_pCWnd_ReturnDlg = nullptr;// 点击返回后的返回的窗口(默认为主对话框)

    // 滚动相关参数
    int m_ScrollPos;            // 滚动位置
    int m_ScrollRange;          // 滚动范围
    int m_ScrollPageSize;       // 页面大小
    bool m_IsScrolling;         // 是否正在滚动
    CPoint m_LastMousePos;      // 上次鼠标位置

    // 视频列表相关参数
    std::vector<VideoInfo> m_VideoList;     // 视频列表
    std::vector<ButtonArea> m_ButtonAreas;  // 按钮区域列表
    int m_RowHeight;                        // 行高
    int m_HoverButtonIndex;                 // 当前悬停的按钮索引
    int m_HoverButtonType = -1;
    int m_selectionCounter;                 // 选择计数器，用于跟踪选择顺序

    // 图标资源
    Gdiplus::Bitmap* m_IconPlay;            // 播放按钮图标
    Gdiplus::Bitmap* m_IconOpen;            // 打开文件图标
    Gdiplus::Bitmap* m_IconDelete;          // 删除按钮图标
    Gdiplus::Bitmap* m_IconToRec;           // 去录制吧 logo

    //区域
    Gdiplus::Rect m_Rect_IconToRec;         //去录制吧 logo区域
    Gdiplus::RectF m_Rect_IconToRecText;         //去录制吧 文本区域

    // 界面设置
    Gdiplus::Color m_BgColor;               // 背景颜色
    Gdiplus::Color m_HeaderBgColor;         // 表头背景色
    Gdiplus::Color m_RowBgColor1;           // 奇数行背景色
    Gdiplus::Color m_RowBgColor2;           // 偶数行背景色
    Gdiplus::Color m_TextColor;             // 文本颜色
    Gdiplus::Color m_BorderColor;           // 边框颜色

    //
    CEdit m_editRename;                      // 重命名编辑框
    int m_editingVideoIndex;                 // 正在编辑的视频索引
    bool m_isEditing;                        // 是否正在编辑

    // 窗口底部控件
    CLarBtn m_Btn_DeleteSelect; // 批量删除
    CLarBtn m_Btn_OpenFolder;// 打开文件夹
    CLarBtn m_Btn_AddVideo;  // 导入视频
    CLarBtn m_Btn_GoRecord;     // 去录制

    // 滚动条相关
    bool m_IsScrollbarHovered;    // 鼠标是否悬停在滚动条上
    bool m_IsScrollbarDragging;   // 是否正在拖动滚动条
    int m_ScrollbarDragStartY;    // 拖动开始的Y坐标
    int m_ScrollbarDragStartPos;  // 拖动开始时的滚动位置

    
    CString m_LastVideoFilePath;   // 最近添加的视频文件(防止重复添加)

    CWndShadow m_Shadow;
    bool m_Bool_IsShadowInit = false;
    int m_redrawTimerCount = 0;

    CLarToolsTips m_ToolTip;       // 按钮提示工具

};