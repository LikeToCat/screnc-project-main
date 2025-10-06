#pragma once

// 滚动区域控件裁剪管理类
class CScrollClipManager
{
private:
    CRect m_clipRect;       // 裁剪区域
    std::vector<std::pair<CWnd*, CRgn*>> m_clippedControls; // 被裁剪的控件和对应的区域

public:
    CScrollClipManager() {}

    ~CScrollClipManager()
    {
        Reset();
    }

    // 设置裁剪区域
    void SetClipRect(const CRect& rect)
    {
        m_clipRect = rect;
    }

    // 清除所有裁剪
    void Reset()
    {
        // 还原所有控件的区域
        for (auto& pair : m_clippedControls)
        {
            if (pair.first && pair.first->GetSafeHwnd())
            {
                pair.first->SetWindowRgn(NULL, FALSE);
            }
            delete pair.second;
        }
        m_clippedControls.clear();
    }

    // 裁剪控件，保证它不会超出裁剪区域
    void ClipControl(CWnd* pControl, const CRect& controlRect)
    {
        if (!pControl || !pControl->GetSafeHwnd())
            return;

        // 创建控件的完整区域
        CRgn* pRgn = new CRgn();
        pRgn->CreateRectRgnIndirect(&controlRect);

        // 创建裁剪区域
        CRgn clipRgn;
        clipRgn.CreateRectRgnIndirect(&m_clipRect);

        // 计算两个区域的交集
        CRgn resultRgn;
        resultRgn.CreateRectRgn(0, 0, 0, 0);
        resultRgn.CombineRgn(pRgn, &clipRgn, RGN_AND);

        // 应用裁剪区域到控件
        pControl->SetWindowRgn((HRGN)resultRgn.Detach(), FALSE);

        // 保存控件和区域以便后续清理
        m_clippedControls.push_back(std::make_pair(pControl, pRgn));
    }
};