#include "stdafx.h"
#include "DialogManager.h"

DialogManager* DialogManager::instance = nullptr;

DialogManager* DialogManager::GetInstance()
{
    if (!instance)
    {
        instance = new DialogManager();
    }
    return instance;
}

void DialogManager::ReleaseInstance()
{
    if (instance)
    {
        delete instance;
        instance = nullptr;
    }
}

DialogManager::DialogManager()
{

}

DialogManager::~DialogManager()
{
    m_dialogs.clear();
}

void DialogManager::RegisterDialog(CWnd* pDialog)
{
    if (pDialog && ::IsWindow(pDialog->GetSafeHwnd()))
    {
        // 检查是否已经注册过，避免重复
        auto it = std::find(m_dialogs.begin(), m_dialogs.end(), pDialog);
        if (it == m_dialogs.end())
        {
            m_dialogs.push_back(pDialog);
        }
    }
}

void DialogManager::UnregisterDialog(CWnd* pDialog)
{
    auto it = std::find(m_dialogs.begin(), m_dialogs.end(), pDialog);
    if (it != m_dialogs.end())
    {
        m_dialogs.erase(it);
    }
}

void DialogManager::BroadcastMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    std::vector<CWnd*> dialogs = m_dialogs;
    for (auto& dialog : dialogs)
    {
        if (dialog && ::IsWindow(dialog->GetSafeHwnd()))
        {
            dialog->PostMessage(message, wParam, lParam);
        }
    }
}