#pragma once
#include <afxext.h>
#include <atlimage.h>
#include <shellapi.h>
class Ui_ScreenCaptureDlg : public CDialogEx
{
    DECLARE_DYNAMIC(Ui_ScreenCaptureDlg)

public:
    Ui_ScreenCaptureDlg(HBITMAP hBmp, CWnd* pParent = nullptr);
    virtual ~Ui_ScreenCaptureDlg();

    enum { IDD = IDD_DIALOG_SCREENCAPTURE };

protected:
    virtual BOOL  OnInitDialog() override;
    virtual void  PostNcDestroy() override;
    DECLARE_MESSAGE_MAP()
    afx_msg void  OnPaint();
    afx_msg void  OnTimer(UINT_PTR nIDEvent);
    afx_msg void  OnClose();
private:
    void          SaveCroppedImage();
    void          ShowTrayTip(LPCTSTR szInfo,
        LPCTSTR szTitle = _T("½ØÍ¼Íê³É"));
private:

    HBITMAP       m_hScreenBmp;      // ÆÁÄ»Î»Í¼¾ä±ú
    int           m_borderThickness; // ±ß¿òºñ¶È
    CString       m_saveFolder;      // ±£´æÄ¿Â¼
};