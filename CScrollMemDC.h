#pragma once

// 将类名从CMemDC改为CScrollMemDC避免冲突
class CScrollMemDC
{
private:
    CDC* m_pDC;        // 目标DC
    CDC  m_MemDC;      // 内存DC
    CBitmap m_Bitmap;  // 内存位图
    CBitmap* m_pOldBitmap; // 原位图
    CRect m_Rect;      // 绘制矩形
    bool m_bValid;     // 是否有效

public:
    // 仅针对特定区域创建内存DC
    CScrollMemDC(CDC* pDC, const CRect& rect) : m_pDC(pDC), m_pOldBitmap(NULL), m_bValid(false)
    {
        m_Rect = rect;

        // 创建兼容的内存DC
        if (m_MemDC.CreateCompatibleDC(m_pDC))
        {
            // 创建兼容的位图
            if (m_Bitmap.CreateCompatibleBitmap(m_pDC, m_Rect.Width(), m_Rect.Height()))
            {
                m_pOldBitmap = m_MemDC.SelectObject(&m_Bitmap);

                // 移动内存DC坐标系统以匹配原始DC的裁剪区域
                m_MemDC.SetWindowOrg(m_Rect.left, m_Rect.top);

                // 设置为有效
                m_bValid = true;
            }
        }
    }

    ~CScrollMemDC()
    {
        if (m_bValid)
        {
            // 将内存DC内容复制到原始DC
            m_pDC->BitBlt(m_Rect.left, m_Rect.top, m_Rect.Width(), m_Rect.Height(),
                &m_MemDC, m_Rect.left, m_Rect.top, SRCCOPY);

            // 恢复原始位图
            if (m_pOldBitmap)
                m_MemDC.SelectObject(m_pOldBitmap);
        }
    }

    // 获取内存DC
    CDC* GetDC() { return m_bValid ? &m_MemDC : m_pDC; }

    // 检查是否有效
    bool IsValid() const { return m_bValid; }
};