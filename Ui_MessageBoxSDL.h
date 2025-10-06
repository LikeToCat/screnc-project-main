#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <string>
#include <vector>
#include <sstream>
#include <functional>  // 用于回调函数

class Ui_MessageBoxSDL {
public:
    // 构造函数
    Ui_MessageBoxSDL(
        SDL_Renderer* renderer,
        const std::string& title, 
        const std::string& message,
        const std::string& type, 
        const char* iconPath, 
        float Scale,
        int cornerRadius = 0
    );
    ~Ui_MessageBoxSDL();
public:
    void AddButton(const std::string& text, int returnValue);   // 添加按钮
    int DoModal();   // 模态显示 - 阻塞直到用户响应
    void Show(std::function<void(int)> callback = nullptr);  // 非模态显示 - 立即返回
    bool HandleEvent(const SDL_Event& event); // 处理事件 - 在主循环中调用
    void Render(); // 渲染对话框 - 在主渲染循环中调用
    bool IsShowing() const { return m_isShown; }    // 检查对话框是否正在显示
    int GetResult() const { return m_result; }// 获取对话框结果
    void Close(int result = -1); // 关闭对话框
private:
    // 按钮结构
    struct Button 
    {
        std::string text;
        int returnValue;
        SDL_Rect rect;
        bool hovered = false;
    };
    void CheckButtonClick(int mouseX, int mouseY); // 检查按钮点击
    void DrawMessageBox();    // 绘制对话框
    void Initialize(); // 初始化对话框
    void Cleanup(); // 清理资源

    //圆角窗口模式相关
    void DrawRoundedRect(SDL_Renderer* renderer, const SDL_Rect& rect, int cornerRadius, SDL_Color color);
    void DrawRoundedRectFilled(SDL_Renderer* renderer, const SDL_Rect& rect, int cornerRadius, SDL_Color color);
public:
    int m_cornerRadius = 0;
    TTF_Font* m_font;
    TTF_Font* m_BoldFont;
    int m_TestSize;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_Icon;
    float m_Scale;
    int m_IconWidth;
    int m_IconHeight; 
    std::string m_title;
    std::string m_message;
    std::string m_MessageType;
    std::string m_IconPath;
    int m_x, m_y, m_width, m_height;
    int m_textHeight;
    bool m_isShown;
    int m_result;
    std::vector<Button> m_buttons;

    SDL_Cursor* m_ClickCursor = nullptr;
    SDL_Cursor* m_NormalCursor = nullptr;

    // 半透明背景纹理
    SDL_Texture* m_dimTexture;

    // 回调函数，当对话框关闭时调用
    std::function<void(int)> m_callback;
};