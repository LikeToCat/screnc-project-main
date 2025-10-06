#pragma once
#include <afxwin.h>
#include <vector>
#include <algorithm>

// 窗口管理器类 - 用于管理和广播消息给多个对话框
class DialogManager
{
public:
    static DialogManager* GetInstance();
    static void ReleaseInstance();
    void RegisterDialog(CWnd* pDialog);
    void UnregisterDialog(CWnd* pDialog);
    void BroadcastMessage(UINT message, WPARAM wParam, LPARAM lParam);

private:
    DialogManager();
    ~DialogManager();
    DialogManager(const DialogManager&) = delete;
    DialogManager& operator=(const DialogManager&) = delete;
    std::vector<CWnd*> m_dialogs;
    static DialogManager* instance;
};