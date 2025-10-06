#include "stdafx.h"
#include "theApp.h"
#include "Ui_RaiSDL.h"
static int init_rowIndex = 0;

extern HANDLE ConsoleHandle;
static SDL_HitTestResult SDLCALL Ui_RaiSDL_HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data);

Ui_RaiSDL::Ui_RaiSDL()
:LLarSDL()
{
    m_te_raiGou = nullptr;       
    m_te_raiNGou = nullptr;      
    m_te_raiCrown = nullptr;   
    m_te_raiLine = nullptr;
    m_te_titletxt = nullptr;
    m_btn_close = nullptr;
    m_Font_Title = nullptr;
    m_Font_CenterTitle = nullptr;   
    m_Font_Center = nullptr;      
    m_btn_continue = nullptr;
    m_btn_openvip = nullptr;
    m_te_openvipLogo = nullptr;
    m_te_centerPullDown = nullptr;
    m_te_centerPullDowntxt = nullptr;
    m_te_TitleLogo = nullptr;
    m_iShadowSize = 6 * m_Scale;
    m_color_shadowRoot = SDL_Color{ 19,21,21,255 };
    m_isRenderScrollTips = true;
    int DescendValue = m_color_shadowRoot.a / m_iShadowSize;
    m_showMode = ShowMode::RecordOrOpenVip;
    for (size_t i = 0; i < m_iShadowSize; i++)
    {
        Uint8 alpha = m_color_shadowRoot.a - DescendValue * i;
        SDL_Color c{ m_color_shadowRoot.r,m_color_shadowRoot.g,m_color_shadowRoot.b,alpha };
        m_vec_vcShadowC.push_back(c);
    }
    m_startliney = 98 * m_Scale;
    ret = 0;
}

void Ui_RaiSDL::SetShowMode(ShowMode showMode)
{
    m_showMode = showMode;
}

void Ui_RaiSDL::_ProcessEvents()
{
    SDL_Event e;

    //更新物理算法模型
    m_Scroller->UpdateScrollPhysics();

    // 轮询所有待处理事件
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            // 用户点击「关闭窗口」按钮
            // TODO:若要关闭窗口，请发送消息让其他线程调用ReleaseInstance
            break;

        case SDL_WINDOWEVENT:
            // 窗口相关事件（最小化、恢复、移动、大小改变等）
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                s_isRunning = false;
                break;
            case SDL_WINDOWEVENT_RESIZED:
                // 窗口大小改变
                // e.window.data1 = 新宽度； e.window.data2 = 新高度
                // TODO: 更新渲染目标、重置视口等
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                // 窗口获得焦点
                // TODO: 处理焦点获得
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                // 窗口失去焦点
                // TODO: 处理焦点丢失
                break;
            default:
                // 其他窗口事件
                break;
            }
            break;

        case SDL_KEYDOWN:
            // 键盘按下
            // e.key.keysym.sym 包含键值
            // TODO: 处理按键按下
            break;

        case SDL_KEYUP:
            // 键盘释放
            // TODO: 处理按键释放
            break;

        case SDL_MOUSEMOTION:
        {
            // 鼠标移动
            // e.motion.x, e.motion.y：鼠标位置
            // e.motion.xrel, e.motion.yrel：相对移动
            if (m_showMode == ShowMode::RecordOrOpenVip)
                m_BtnManager_RecordOrOpenVip.UpdateBtnsState(e.motion.x, e.motion.y, false);
            else if(m_showMode == ShowMode::ReturnToOriPage)
                m_BtnManager_ReturnToOriPage.UpdateBtnsState(e.motion.x, e.motion.y, false);
        }
            break;

        case SDL_MOUSEBUTTONDOWN:
        {
            // 鼠标按键按下
            // e.button.button：哪一个按钮（左、中、右）
            // e.button.x, e.button.y：点击位置
            if (e.button.button == SDL_BUTTON_LEFT) 
            {
                // 鼠标左键按下时更新按钮状态（点击效果）
                if (m_showMode == ShowMode::RecordOrOpenVip)
                    m_BtnManager_RecordOrOpenVip.UpdateBtnsState(e.motion.x, e.motion.y, false);
                else if (m_showMode == ShowMode::ReturnToOriPage)
                    m_BtnManager_ReturnToOriPage.UpdateBtnsState(e.motion.x, e.motion.y, false);
            }
        }
            break;

        case SDL_MOUSEBUTTONUP:
        {
            // 鼠标按键释放
            if (e.button.button == SDL_BUTTON_LEFT) 
            {
                // 获取当前悬停或点击的按钮
                if (m_showMode == ShowMode::RecordOrOpenVip)
                {
                    Ui_SDLButton* activeBtn = m_BtnManager_RecordOrOpenVip.GetInHoverOrClickedBtn();
                    if (activeBtn)
                        activeBtn->Click();// 执行按钮点击回调
                    m_BtnManager_RecordOrOpenVip.UpdateBtnsState(e.button.x, e.button.y, false);// 重置按钮点击状态
                }
                else if (m_showMode == ShowMode::ReturnToOriPage)
                {
                    Ui_SDLButton* activeBtn = m_BtnManager_ReturnToOriPage.GetInHoverOrClickedBtn();
                    if (activeBtn)
                        activeBtn->Click();// 执行按钮点击回调
                    m_BtnManager_ReturnToOriPage.UpdateBtnsState(e.button.x, e.button.y, false);// 重置按钮点击状态
                }
            }

        }
            break;

        case SDL_MOUSEWHEEL:
        {
            if (e.wheel.y < 0 && m_isRenderScrollTips)
            {
                m_isRenderScrollTips = false;
                m_cba.bIsShowShadow = false;
            }
            m_Scroller->UpdateMouseWheelEvent(&e);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[权益窗口]:鼠标滚轮滚动: %d", e.wheel.y);
        }
            break;

        case SDL_TEXTINPUT:
            // 文本输入事件（用于输入法/Unicode 字符）
            // e.text.text：输入的文本
            // TODO: 处理文本输入
            break;

        default:
            // 其他事件（手柄、触摸、拖拽等）
            break;
        }
    }
}

void Ui_RaiSDL::_Render()
{
    //渲染背景
    SDL_Color bkcolor{ 31,36,37,255 };
    SDL_SetRenderDrawColor(m_SDL_Renderer, bkcolor.r, bkcolor.g, bkcolor.b, bkcolor.a);
    SDL_RenderFillRect(m_SDL_Renderer, &m_CtRect_WindowRect);

    if (m_showMode == ShowMode::RecordOrOpenVip)
    {
        //渲染函数
        _RenderCenter();                                //渲染中间滚动区域
        _RenderTitle();                                 //渲染标题栏区域
        m_BtnManager_RecordOrOpenVip.RenderBtns();      //渲染按钮
        _RenderBotShadow();                             //渲染中间区域底部的阴影效果

        //渲染滚动提示字体和logo图
        if (m_isRenderScrollTips)
        {
            SDL_RenderCopy(m_SDL_Renderer, m_te_centerPullDown, NULL, &m_Rect_centerPullDown);
            SDL_RenderCopy(m_SDL_Renderer, m_te_centerPullDowntxt, NULL, &m_Rect_centerPullDowntxt);
        }
    }
    else if(m_showMode == ShowMode::ReturnToOriPage)
    {
        //渲染函数
        _RenderCenter();                                //渲染中间滚动区域
        _RenderTitle();                                 //渲染标题栏区域
        m_BtnManager_ReturnToOriPage.RenderBtns();      //渲染按钮
        _RenderBotShadow();                             //渲染中间区域底部的阴影效果

        //渲染滚动提示字体和logo图
        if (m_isRenderScrollTips)
        {
            SDL_RenderCopy(m_SDL_Renderer, m_te_centerPullDown, NULL, &m_Rect_centerPullDown);
            SDL_RenderCopy(m_SDL_Renderer, m_te_centerPullDowntxt, NULL, &m_Rect_centerPullDowntxt);
        }
    }


    SDL_RenderPresent(m_SDL_Renderer);
    return;
}

void Ui_RaiSDL::_RenderCenter()
{
    //渲染显示权益的中间区域背景
    SDL_Color raiBkColor{ 27,30,30 ,255 };
    SDL_Color raiTBkColor{ 0,0,0,255 };
    SDL_Color raiCBkColor{ 19,21,21,255 };
    SDL_SetRenderDrawColor(m_SDL_Renderer, raiBkColor.r, raiBkColor.g, raiBkColor.b, raiBkColor.a);
    SDL_RenderFillRect(m_SDL_Renderer, &m_Rect_Center);
    SDL_SetRenderDrawColor(m_SDL_Renderer, raiTBkColor.r, raiTBkColor.g, raiTBkColor.b, raiTBkColor.a);
    SDL_RenderFillRect(m_SDL_Renderer, &m_Rect_CenterTitle);
    SDL_SetRenderDrawColor(m_SDL_Renderer, raiCBkColor.r, raiCBkColor.g, raiCBkColor.b, raiCBkColor.a);
    SDL_RenderFillRect(m_SDL_Renderer, &m_Rect_VCenterItems);

    //中间竖向区域的阴影
    for (size_t i = 0; i < m_iShadowSize; i++)
    {
        SDL_Color c = m_vec_vcShadowC[i];
        SDL_SetRenderDrawColor(m_SDL_Renderer, c.r, c.g, c.b, c.a);
        SDL_RenderDrawRect(m_SDL_Renderer, &m_vec_vcShadowR[i]);
    }

    //渲染中间区域标题item
    SDL_RenderCopy(m_SDL_Renderer, m_ri_title.displayT1, NULL, &m_ri_title.rDisplayT1);
    SDL_RenderCopy(m_SDL_Renderer, m_ri_title.displayT2, NULL, &m_ri_title.rDisplayT2);
    SDL_RenderCopy(m_SDL_Renderer, m_ri_title.displayT3, NULL, &m_ri_title.rDisplayT3);
    SDL_RenderCopy(m_SDL_Renderer, m_te_raiCrown, NULL, &m_Rect_raiCrown);//渲染王冠

    //渲染滚动区域项
    m_Scroller->EnterRender(m_SDL_Renderer);
    m_Scroller->GetVisiableIndex(&m_ItemFirstVisiable, &m_ItemLastVisiable);
    for (size_t i = m_ItemFirstVisiable; i < m_ItemLastVisiable; ++i)
    {
        rai_item ri = m_vec_ri[i];
        int dy = m_Scroller->GetExactY(i);
        dy -= 15 * m_Scale;
        int dx = 297 * m_Scale;
        int x = 503 * m_Scale;
        int dx2 = m_Rect_Center.x + m_Rect_Center.w;
        int txty = dy + 20 * m_Scale;
        ri.rDisplayT1.y = txty;
        ri.rDisplayT2.y = txty;
        ri.rDisplayT3.y = txty;
        SDL_RenderCopy(m_SDL_Renderer, ri.displayT1, NULL, &ri.rDisplayT1);
        SDL_RenderCopy(m_SDL_Renderer, ri.displayT2, NULL, &ri.rDisplayT2);
        SDL_RenderCopy(m_SDL_Renderer, ri.displayT3, NULL, &ri.rDisplayT3);
        int cpy = dy + 30 * m_Scale;
        ri.r_cp1.y = cpy;
        ri.r_cp2.y = cpy;
        ri.r_cp3.y = cpy;
        SDL_BlendMode currentMode;
        SDL_GetRenderDrawBlendMode(m_SDL_Renderer, &currentMode);
        if(ri.b_IsDraw1)
            filledCircleRGBA(m_SDL_Renderer,
                ri.r_cp1.x, ri.r_cp1.y, ri.r_cpr1, ri.r_cpc1.r, ri.r_cpc1.g, ri.r_cpc1.b, ri.r_cpc1.a);
        if(ri.b_IsDraw2)
            filledCircleRGBA(m_SDL_Renderer,
                ri.r_cp2.x, ri.r_cp2.y, ri.r_cpr2, ri.r_cpc2.r, ri.r_cpc2.g, ri.r_cpc2.b, ri.r_cpc2.a);
        if(ri.b_IsDraw3)
            filledCircleRGBA(m_SDL_Renderer,
                ri.r_cp3.x, ri.r_cp3.y, ri.r_cpr3, ri.r_cpc3.r, ri.r_cpc3.g, ri.r_cpc3.b, ri.r_cpc3.a);
        SDL_SetRenderDrawBlendMode(m_SDL_Renderer, currentMode);
        SDL_Color lineC{ 24,24,24,255 };
        SDL_SetRenderDrawColor(m_SDL_Renderer, lineC.r, lineC.g, lineC.b, lineC.a);
        for (size_t i = 0; i < 2; i++)
        {
            SDL_RenderDrawLine(m_SDL_Renderer, m_Rect_Center.x, dy - i, dx, dy - i);
            SDL_RenderDrawLine(m_SDL_Renderer, x, dy - i, dx2, dy - i);
        }
    }
    m_Scroller->ExitRender(m_SDL_Renderer); //退出滚动区域渲染
}

void Ui_RaiSDL::_RenderTitle()
{
    //渲染标题栏区域背景
    SDL_Color color{ 16,23,24,255 };
    SDL_SetRenderDrawColor(m_SDL_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_SDL_Renderer, &m_Rect_Title);                      //渲染标题背景
    SDL_RenderCopy(m_SDL_Renderer, m_te_titletxt, NULL, &m_Rect_TitleTxt);  //渲染标题文本
    SDL_RenderCopy(m_SDL_Renderer, m_te_TitleLogo, NULL, &m_Rect_TitleLogo);    //渲染标题logo
}

void Ui_RaiSDL::_RenderBotShadow()
{
    if (!m_cba.bIsShowShadow)
        return;
    size_t size = m_cba.vec_lp.size();
    for (size_t i = 0; i < size; i++)
    {
        SDL_Color c = m_cba.vec_color[i];
        line_point lp = m_cba.vec_lp[i];
        SDL_Point p1 = lp.p1;
        SDL_Point p2 = lp.p2;
        SDL_SetRenderDrawColor(m_SDL_Renderer, c.r, c.g, c.b, c.a);
        SDL_RenderDrawLine(m_SDL_Renderer, p1.x, p1.y, p2.x, p2.y);
    }
}

bool Ui_RaiSDL::_Cus_Init()
{
    _initArea();
    if (!_initFont())
    {
        DB(ConsoleHandle, L"Ui_RaiSDL::_initFont初始化字体失败");
        return false;
    }
    if (!_initImage())
    {
        DB(ConsoleHandle, L"Ui_RaiSDL::_initImage初始化图片资源失败");
        return false;
    }
    if (!_initText())
    {
        DB(ConsoleHandle, L"Ui_RaiSDL::_initText初始化文本失败");
        return false;
    }
    if (!_initRecordOrOpenVip())
    {
        DB(ConsoleHandle, L"Ui_RaiSDL::_initRecordOrOpenVip初始化控件失败");
        return false;
    }
    if (!_InitReturnToOriPage())
    {
        DB(ConsoleHandle, L"Ui_RaiSDL::_InitReturnToOriPage初始化控件失败");
        return false;
    }
    if (!_initBotShadow())
    {
        DB(ConsoleHandle, L"Ui_RaiSDL::_initBotShadow初始化控件失败");
        return false;
    }
    if (!_initCom())
    {
        DB(ConsoleHandle, L"Ui_RaiSDL::_initCom初始化控件失败");
        return false;
    }
    return true;
}

bool Ui_RaiSDL::_RegSDLHitTestFunc()
{
    SDL_SetWindowHitTest(m_SDL_Window, Ui_RaiSDL_HitTestCallback, NULL);
    return true;
}

void Ui_RaiSDL::_initArea()
{
    //中间可滚动区域
    m_Rect_Center.x = 16 * m_Scale;
    m_Rect_Center.y = 55 * m_Scale;
    m_Rect_Center.w = 768 * m_Scale;
    m_Rect_Center.h = 471 * m_Scale;

    //中间滚动区域的上方标题区域
    m_Rect_CenterTitle.w = m_Rect_Center.w;
    m_Rect_CenterTitle.h = 47 * m_Scale;
    m_Rect_CenterTitle.x = m_Rect_Center.x;
    m_Rect_CenterTitle.y = m_Rect_Center.y;

    //中间竖向区域的背景
    m_Rect_VCenterItems.w = 202 * m_Scale;
    m_Rect_VCenterItems.h = 462 * m_Scale;
    m_Rect_VCenterItems.x = 301 * m_Scale;
    m_Rect_VCenterItems.y = m_Rect_Center.y + 3 * m_Scale;

    //中间竖向区域的背景矩形集合初始化
    for (size_t i = 1; i <= m_iShadowSize; i++)
    {
        SDL_Rect r
        {
            m_Rect_VCenterItems.x - i,
            m_Rect_VCenterItems.y - i,
            m_Rect_VCenterItems.w + i * 2,
            m_Rect_VCenterItems.h + i * 2
        };
        m_vec_vcShadowR.push_back(r);
    }

    //王冠纹理区域
    m_Rect_raiCrown.w = 26 * m_Scale;
    m_Rect_raiCrown.h = 24 * m_Scale;
    m_Rect_raiCrown.x = m_Rect_Center.x + 330 * m_Scale;
    m_Rect_raiCrown.y = m_Rect_Center.y + 8 * m_Scale;

    //标题区域
    m_Rect_Title.w = m_Rect_WindowRect.w;
    m_Rect_Title.h = 40 * m_Scale;
    m_Rect_Title.x = 0;
    m_Rect_Title.y = 0;

    //开通会员金色钻石logo区域
    m_Rect_openvipLogo.w = 25 * m_Scale;
    m_Rect_openvipLogo.h = 20 * m_Scale;
    m_Rect_openvipLogo.x = 456 * m_Scale;
    m_Rect_openvipLogo.y = 574 * m_Scale;

    //中间区域下拉logo区域
    m_Rect_centerPullDown.w = 20 * m_Scale;
    m_Rect_centerPullDown.h = 18 * m_Scale;
    m_Rect_centerPullDown.x = m_Rect_Center.x + (m_Rect_Center.w - m_Rect_centerPullDown.w) / 2;
    m_Rect_centerPullDown.y = m_Rect_Center.y + m_Rect_Center.h - m_Rect_centerPullDown.h;

    //标题区域的标题logo
    m_Rect_TitleLogo.w = 24 * m_Scale;
    m_Rect_TitleLogo.h = 24 * m_Scale;
    m_Rect_TitleLogo.x = 15 * m_Scale;
    m_Rect_TitleLogo.y = (m_Rect_Title.h - m_Rect_TitleLogo.h) / 2;
}

bool Ui_RaiSDL::_initText()
{
    init_rowIndex = 0;

    //初始化标题区域文本
    if (!COMAPI::SDL::TextLoad(
        m_SDL_Renderer, m_Font_Title,
        L"VIP尊享权益", SDL_Color{ 255,255,255 },
        &m_te_titletxt, &m_Rect_TitleTxt.w, & m_Rect_TitleTxt.h
    ))
    {
        DB(ConsoleHandle, L"初始化标题区域文本纹理失败");
    }
    m_Rect_TitleTxt.x = m_Rect_TitleLogo.x + m_Rect_TitleLogo.w + 5 * m_Scale;
    m_Rect_TitleTxt.y = 9 * m_Scale;

    //初始化中间区域下方的提示文本
    if (!COMAPI::SDL::TextLoad(
        m_SDL_Renderer, m_Font_Center,
        L"鼠标下滑展示更多信息", SDL_Color{ 255,255,255 },
        &m_te_centerPullDowntxt, &m_Rect_centerPullDowntxt.w, & m_Rect_centerPullDowntxt.h
    ))
    {
        DB(ConsoleHandle, L"初始化中间区域下方的提示文本纹理失败");
    }
    m_Rect_centerPullDowntxt.x = m_Rect_Center.x + (m_Rect_Center.w - m_Rect_centerPullDowntxt.w) / 2;
    m_Rect_centerPullDowntxt.y = m_Rect_Center.y + m_Rect_Center.h - m_Rect_centerPullDowntxt.h - 20 * m_Scale;

    //初始化中间区域文本
    SDL_Color rai_title_c{ 179,179 ,179 ,255 };
    SDL_Color rai_normal_c{ 179,179 ,179 ,255 };
    SDL_Color rai_center_C{ 255,255 ,255 ,255 };
    if (!_AddRaiItem(
        { L"功能特色",L"会员权益",L"非会员权益" },
        { rai_title_c ,rai_title_c ,rai_title_c }
    ))//第一个参数列表中的参数如果传空，代表显示对勾纹理即可
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"单次录制时长",L"无限时长",L"1分钟" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0,0,0,
        false,false, false,
        SDL_Color{75,75,75,255}, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true,false,false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"去除品牌水印",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 0,
        false, false, true,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"视频帧率",L"最高支持180fps",L"最高支持24fps" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 0, 0,
        false, false, false,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"多种录制模式",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 1,
        false, false, false,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"多电脑同时登陆",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 0,
        false, false, true,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"摄像头录制",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 1,
        false, false, false,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"应用窗口录制",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 1,
        false, false, false,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"游戏模式",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 1,
        false, false, false,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"自定义区域录制",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 1,
        false, false, false,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"缩放录制",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 1,
        false, false, false,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"只录声音",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 1,
        false, false, false,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"视频去除水印",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 0,
        false, false, true,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"尊贵身份标识",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 0,
        false, false, true,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"专属技术支持",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 0,
        false, false, true,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    if (!_AddRaiItem(
        { L"软件免费升级",L"",L"" },
        { rai_normal_c ,rai_center_C ,rai_normal_c },
        0, 2, 0,
        false, false, true,
        SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 }, SDL_Color{ 75,75,75,255 },
        true, false, false
    ))
    {
        return false;
    }
    return true;
}

bool Ui_RaiSDL::_initImage()
{
    CString p1 = L"\\SDLAssets\\rai_gou.png";
    CString p2 = L"\\SDLAssets\\rai_normalgou.png";
    CString p3 = L"\\SDLAssets\\rai_crown.png";
    CString p4 = L"\\SDLAssets\\rai_line.png";
    CString p5 = L"\\SDLAssets\\btn_openviplogo.png";
    CString p6 = L"\\SDLAssets\\center_pulldown.png";
    CString p7 = L"\\SDLAssets\\logo.png";
    if (!COMAPI::SDL::ImageLoad(m_SDL_Renderer, p1, &m_te_raiGou))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::ImageLoad图片资源加载失败");
        return false;
    }
    if (!COMAPI::SDL::ImageLoad(m_SDL_Renderer, p2, &m_te_raiNGou))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::ImageLoad图片资源加载失败");
        return false;
    }
    if (!COMAPI::SDL::ImageLoad(m_SDL_Renderer, p3, &m_te_raiCrown))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::ImageLoad图片资源加载失败");
        return false;
    }
    if (!COMAPI::SDL::ImageLoad(m_SDL_Renderer, p4, &m_te_raiLine))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::ImageLoad图片资源加载失败");
        return false;
    }
    if (!COMAPI::SDL::ImageLoad(m_SDL_Renderer, p5, &m_te_openvipLogo))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::ImageLoad图片资源加载失败");
        return false;
    }
    if (!COMAPI::SDL::ImageLoad(m_SDL_Renderer, p6, &m_te_centerPullDown))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::ImageLoad图片资源加载失败");
        return false;
    }
    if (!COMAPI::SDL::ImageLoad(m_SDL_Renderer, p7, &m_te_TitleLogo))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::ImageLoad图片资源加载失败");
        return false;
    }
    return true;
}

bool Ui_RaiSDL::_initRecordOrOpenVip()
{
    // 创建关闭按钮
    CString closeImagePath = L"\\SDLAssets\\Close.png";  // 使用已加载的关闭按钮图片
    SDL_Rect closeRect =
    {
        static_cast<int>(m_CtRect_WindowRect.w - 40 * m_Scale),  // 右上角位置
        static_cast<int>(5 * m_Scale),
        static_cast<int>(30 * m_Scale),                          // 宽度
        static_cast<int>(30 * m_Scale)                           // 高度
    };
    std::function<void()> closeCallback = [this]()// 处理关闭按钮点击事件
    {
        SDL_Event event;
        event.type = SDL_WINDOWEVENT;
        event.window.event = SDL_WINDOWEVENT_CLOSE;
        event.window.windowID = SDL_GetWindowID(m_SDL_Window);
        SDL_PushEvent(&event);
    };
    m_BtnManager_RecordOrOpenVip.CreateBtn(
        closeRect,                  // 按钮区域
        L"",                        // 按钮文本 (如果使用图片可以为空字符串)
        { 255, 255, 255, 255 },     // 文本颜色
        { 255, 255, 0, 255 },       // 悬停文本颜色
        { 255, 0, 0, 255 },         // 点击文本颜色
        { 16, 23, 24, 255 },        // 边框颜色
        { 16, 23, 24, 255 },        // 背景颜色
        { 50, 50, 50, 255 },        // 悬停背景颜色
        { 30, 30, 30, 255 },        // 点击背景颜色
        closeImagePath,             // 图片路径
        m_SDL_Renderer,             // 渲染器
        m_TTF_Font,                 // 字体
        closeCallback,              // 回调函数
        1001                        // 唯一ID
    );
    m_btn_close = m_BtnManager_RecordOrOpenVip.FindBtnByUid(1001); // 获取创建的按钮
    m_btn_close->SetImageStretchParam(0.6f);

    //创建继续试用按钮
    SDL_Rect continueRect =
    {
        static_cast<int>(231 * m_Scale),  
        static_cast<int>(563 * m_Scale),
        static_cast<int>(145 * m_Scale),                         
        static_cast<int>(40 * m_Scale)                          
    };
    std::function<void()> continueCallback = [this]()// 处理关闭按钮点击事件
    {
        SDL_Event event;
        event.type = SDL_WINDOWEVENT;
        event.window.event = SDL_WINDOWEVENT_CLOSE;
        event.window.windowID = SDL_GetWindowID(m_SDL_Window);
        SDL_PushEvent(&event);
        ::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_UIRAI_CONTINUE, NULL, NULL);
    };
    m_BtnManager_RecordOrOpenVip.CreateBtn(
        continueRect,               // 按钮区域
         L"继续试用",               // 按钮文本 (如果使用图片可以为空字符串)
        { 255, 255, 255, 255 },     // 文本颜色
        { 0, 0, 0, 255 },           // 悬停文本颜色
        { 0, 0, 0, 255 },           // 点击文本颜色
        { 255, 255, 255, 255 },     // 边框颜色
        { 31, 36, 37, 255 },        // 背景颜色
        { 216, 216, 216, 255 },     // 悬停背景颜色
        { 216, 216, 216, 255 },     // 点击背景颜色
        NULL,                       // 图片路径
        m_SDL_Renderer,             // 渲染器
        m_TTF_Font,                 // 字体
        continueCallback,           // 回调函数
        1002                        // 唯一ID
    );
    m_btn_continue = m_BtnManager_RecordOrOpenVip.FindBtnByUid(1002); // 获取创建的按钮

    //创建开通会员按钮
    SDL_Rect openvipRect =
    {
        static_cast<int>(439 * m_Scale),
        static_cast<int>(563 * m_Scale),
        static_cast<int>(145 * m_Scale),
        static_cast<int>(40 * m_Scale)
    };
    std::function<void()> openvipCallback = [this]()// 处理关闭按钮点击事件
    {
        SDL_Event event;
        event.type = SDL_WINDOWEVENT;
        event.window.event = SDL_WINDOWEVENT_CLOSE;
        event.window.windowID = SDL_GetWindowID(m_SDL_Window);
        ret = MSG_UIRAI_OPENVIP;
        ::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_UIRAI_OPENVIP, NULL, NULL);
        SDL_PushEvent(&event);
    };
    m_BtnManager_RecordOrOpenVip.CreateBtn(
        openvipRect,                // 按钮区域
        L"开通会员",                // 按钮文本 (如果使用图片可以为空字符串)
        { 63, 34, 1, 255 },         // 文本颜色
        { 63, 34, 1, 255 },         // 悬停文本颜色
        { 63, 34, 1, 255 },         // 点击文本颜色
        { 26, 27, 32, 255 },        // 边框颜色
        { 145, 145, 145, 255 },     // 背景颜色
        { 216, 216, 216, 255 },     // 悬停背景颜色
        { 216, 216, 216, 255 },     // 点击背景颜色
        NULL,                       // 图片路径
        m_SDL_Renderer,             // 渲染器
        m_TTF_Font,                 // 字体
        openvipCallback,            // 回调函数
        1003                        // 唯一ID
    );
    m_btn_openvip = m_BtnManager_RecordOrOpenVip.FindBtnByUid(1003); // 获取创建的按钮
    m_btn_openvip->SetGdColor(SDL_Color{ 255,232,212,255 }, SDL_Color{ 224,184,140,255 });
    m_btn_openvip->SetHGdColor(SDL_Color{ 224,184,140,255 }, SDL_Color{ 255,232,212,255 });
    m_btn_openvip->AdjustTextRect(10 * m_Scale, 0);
    m_btn_openvip->SetExtraRender([this]()
        {
            SDL_RenderCopy(m_SDL_Renderer, m_te_openvipLogo, NULL, &m_Rect_openvipLogo);
        });
    return (m_btn_close != nullptr) && (m_btn_continue != nullptr) && (m_btn_openvip != nullptr);
}

bool Ui_RaiSDL::_InitReturnToOriPage()
{
    CString closeImagePath = L"\\SDLAssets\\Close.png";
    SDL_Rect closeRect =
    {
        static_cast<int>(m_CtRect_WindowRect.w - 40 * m_Scale),
        static_cast<int>(5 * m_Scale),
        static_cast<int>(30 * m_Scale),
        static_cast<int>(30 * m_Scale)
    };
    std::function<void()> closeCallback = [this]()
        {
            SDL_Event event;
            event.type = SDL_WINDOWEVENT;
            event.window.event = SDL_WINDOWEVENT_CLOSE;
            event.window.windowID = SDL_GetWindowID(m_SDL_Window);
            SDL_PushEvent(&event);
        };
    m_BtnManager_ReturnToOriPage.CreateBtn(
        closeRect,
        L"",
        SDL_Color{ 255,255,255,255 },     // 文本颜色
        SDL_Color{ 255,255,0,255 },       // 悬停文本
        SDL_Color{ 255,0,0,255 },         // 点击文本
        SDL_Color{ 16,23,24,255 },        // 边框
        SDL_Color{ 16,23,24,255 },        // 背景
        SDL_Color{ 50,50,50,255 },        // 悬停背景
        SDL_Color{ 30,30,30,255 },        // 点击背景
        closeImagePath,
        m_SDL_Renderer,
        m_TTF_Font,
        closeCallback,
        2001
    );
    if (Ui_SDLButton* btnClose = m_BtnManager_ReturnToOriPage.FindBtnByUid(2001))
    {
        btnClose->SetImageStretchParam(0.6f);  
    }

    const int radius = static_cast<int>(8 * m_Scale);
    const int btnW = static_cast<int>(180 * m_Scale);
    const int btnH = static_cast<int>(40 * m_Scale);
    const int btnX = (m_CtRect_WindowRect.w - btnW) / 2;
    const int bottomPadding = static_cast<int>(45 * m_Scale); 
    const int btnY = m_CtRect_WindowRect.h - bottomPadding - btnH;

    SDL_Rect returnRect{ btnX, btnY, btnW, btnH };
    std::function<void()> returnCallback = [this]()
        {
            SDL_Event event;
            event.type = SDL_WINDOWEVENT;
            event.window.event = SDL_WINDOWEVENT_CLOSE;
            event.window.windowID = SDL_GetWindowID(m_SDL_Window);
            SDL_PushEvent(&event);
        };

    SDL_Color textWhite{ 255,255,255,255 };
    SDL_Color blueN{ 0,162,255,255 };   
    SDL_Color blueH{ 0,175,255,255 };   
    SDL_Color blueC{ 0,140,230,255 };   

    m_BtnManager_ReturnToOriPage.CreateBtn(
        returnRect,
        L"返回套餐页",                    
        textWhite, textWhite, textWhite,  
        blueN,                            
        blueN,                            
        blueH,                            
        blueC,                            
        L"",                               
        m_SDL_Renderer,
        m_TTF_Font,
        returnCallback,
        2002,                              
        radius
    );

    if (Ui_SDLButton* btnReturn = m_BtnManager_ReturnToOriPage.FindBtnByUid(2002))
    {
        btnReturn->SetGdColor(blueN, blueN);
        btnReturn->SetHGdColor(blueH, blueH);
    }

    return (m_BtnManager_ReturnToOriPage.FindBtnByUid(2001) != nullptr) &&
        (m_BtnManager_ReturnToOriPage.FindBtnByUid(2002) != nullptr);
}

bool Ui_RaiSDL::_initFont()
{
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_Title, L"msyh.ttc", 14 * m_Scale))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::TTFFontLoad字体加载失败");
        return false;
    }
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_CenterTitle, L"msyh.ttc", 16 * m_Scale))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::TTFFontLoad字体加载失败");
        return false;
    }
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_Center, L"msyh.ttc", 14 * m_Scale))
    {
        DB(ConsoleHandle, L"COMAPI::SDL::TTFFontLoad字体加载失败");
        return false;
    }
    return true;
}

bool Ui_RaiSDL::_initBotShadow()
{
    m_cba.RootColor = SDL_Color{ 20,20,20,255 };
    m_cba.shadowSize = 85 * m_Scale;
    m_cba.bIsShowShadow = true;
    int DescendValue = m_cba.RootColor.a / m_cba.shadowSize;
    for (size_t i = 0; i < m_cba.shadowSize; i++)
    {
        Uint8 alpha = m_cba.RootColor.a - DescendValue * i;
        SDL_Color c{ m_cba.RootColor.r,m_cba.RootColor.g,m_cba.RootColor.b,alpha };
        m_cba.vec_color.push_back(c);
    }
    for (size_t i = 1; i <= m_cba.shadowSize; i++)
    {
        line_point lp;
        SDL_Point p1{ m_Rect_Center.x,m_Rect_Center.y + m_Rect_Center.h - i };
        SDL_Point p2{ m_Rect_Center.x + m_Rect_Center.w,m_Rect_Center.y + m_Rect_Center.h - i };
        lp.p1 = p1;
        lp.p2 = p2;
        m_cba.vec_lp.push_back(lp);
    }
    return true;
}

bool Ui_RaiSDL::_initCom()
{
    SDL_Rect scrollArea
    {
        m_Rect_Center.x,
        m_Rect_Center.y + 60 * m_Scale,
        m_Rect_Center.w,
        m_Rect_Center.h - 60 * m_Scale
    };
    m_Scroller = new COMCLASS::SDL::WindowScroller(scrollArea, m_vec_ri.size(), 60 * m_Scale);
    return m_Scroller != nullptr;
}

bool Ui_RaiSDL::_AddRaiItem(
    std::vector<std::wstring> str_list,
    std::vector<SDL_Color> color_list,
    UINT isg1_y, UINT isg2_y, UINT isg3_y,
    bool isg1_Line, bool isg2_Line, bool isg3_Line,
    SDL_Color cp_c1 ,
    SDL_Color cp_c2 ,
    SDL_Color cp_c3 ,
    bool IsDrawCp1, bool IsDrawCp2, bool IsDrawCp3
)
{
    //获取当前添加的是第几个item
    size_t size = m_vec_ri.size();
    // 初始化文本内容
    rai_item ri;
    ri.s1 = str_list[0];
    ri.s2 = str_list[1];
    ri.s3 = str_list[2];

    // 初始化文本颜色
    ri.c1 = color_list[0];
    ri.c2 = color_list[1];
    ri.c3 = color_list[2];

    // 初始化纹理指针为空
    ri.t1 = nullptr;
    ri.t2 = nullptr;
    ri.t3 = nullptr;
   

    //引用字体
    TTF_Font* tf = (init_rowIndex == 0) ? m_Font_CenterTitle : m_Font_Center;

    // 加载第一个文本纹理
    if (!ri.s1.empty())
    {
        if (!COMAPI::SDL::TextLoad(m_SDL_Renderer, tf, ri.s1, ri.c1, &ri.t1, &ri.w1, &ri.h1))
        {
            DB(ConsoleHandle, L"InitRaiItem: 加载纹理失败");
            return false;
        }
    }
    
    // 加载第二个文本纹理
    if (!ri.s2.empty())
    {
        if (!COMAPI::SDL::TextLoad(m_SDL_Renderer, tf, ri.s2, ri.c2, &ri.t2, &ri.w2, &ri.h2))
        {
            DB(ConsoleHandle, L"InitRaiItem: 加载纹理失败");
            // 清理已加载的资源
            if (ri.t1) SDL_DestroyTexture(ri.t1);
            ri.t1 = nullptr;
            return false;
        }
    }
   
    // 加载第三个文本纹理
    if (!ri.s3.empty())
    {
        if (!COMAPI::SDL::TextLoad(m_SDL_Renderer, tf, ri.s3, ri.c3, &ri.t3, &ri.w3, &ri.h3))
        {
            DB(ConsoleHandle, L"InitRaiItem: 加载纹理失败");
            // 清理已加载的资源
            if (ri.t1) SDL_DestroyTexture(ri.t1);
            if (ri.t2) SDL_DestroyTexture(ri.t2);
            ri.t1 = nullptr;
            ri.t2 = nullptr;
            return false;
        }
    }
    

    int x1, x2, x3, dy;
    x1 = 16 * m_Scale + (285 * m_Scale - ri.w1) / 2;
    x2 = 301 * m_Scale + (202 * m_Scale - ri.w2) / 2;
    x3 = 503 * m_Scale + (285 * m_Scale - ri.w3) / 2;
    if (init_rowIndex == 0)
        dy = 65 * m_Scale;
    else
        dy = 119 * m_Scale + (init_rowIndex - 1) * (59.5 * m_Scale);
    ri.d1.w = ri.w1;
    ri.d1.h = ri.h1;
    ri.d1.x = x1;
    ri.d1.y = dy;
    ri.d2.w = ri.w2;
    ri.d2.h = ri.h2;
    if (init_rowIndex == 0)
        ri.d2.x = x2 + 10 * m_Scale;
    else
        ri.d2.x = x2;
    ri.d2.y = dy;
    ri.d3.w = ri.w3;
    ri.d3.h = ri.h3;
    ri.d3.x = x3;
    ri.d3.y = dy;

    ri.b_IsDraw1 = IsDrawCp1;
    ri.r_cp1.x = x1 - 10 * m_Scale;
    ri.r_cp1.y = dy + 10 * m_Scale;
    ri.r_cpr1 = 4 * m_Scale;
    ri.r_cpc1 = cp_c1;

    ri.b_IsDraw2 = IsDrawCp2;
    ri.r_cp2.x = x2 - 10 * m_Scale;
    ri.r_cp2.y = dy + 10 * m_Scale;
    ri.r_cpr2 = 4 * m_Scale;
    ri.r_cpc2 = cp_c2;

    ri.b_IsDraw3 = IsDrawCp3;
    ri.r_cp3.x = x3 - 10 * m_Scale;
    ri.r_cp3.y = dy + 10 * m_Scale;
    ri.r_cpr3 = 4 * m_Scale;
    ri.r_cpc3 = cp_c3;

    //设置对勾纹理即区域
    ri.g1 = (isg1_y == 0) ? nullptr : (isg1_y == 1) ? m_te_raiNGou : m_te_raiGou;
    ri.r_g1.w = 24 * m_Scale;
    ri.r_g1.h = 22 * m_Scale;
    ri.r_g1.x = 15 * m_Scale + (285 * m_Scale - ri.r_g1.w) / 2;
    ri.r_g1.y = 99 * m_Scale + (59.5 * m_Scale - ri.r_g1.h) / 2 + (init_rowIndex - 1) * (59.5 * m_Scale);

    ri.g2 = (isg2_y == 0) ? nullptr : (isg2_y == 1) ? m_te_raiNGou : m_te_raiGou;
    ri.r_g2.w = 24 * m_Scale;
    ri.r_g2.h = 22 * m_Scale;
    ri.r_g2.x = 301 * m_Scale + (202 * m_Scale - ri.r_g2.w) / 2;
    ri.r_g2.y = ri.r_g1.y;

    ri.g3 = (isg3_y == 0) ? nullptr : (isg3_y == 1) ? m_te_raiNGou : m_te_raiGou;
    ri.r_g3.w = 24 * m_Scale;
    ri.r_g3.h = 22 * m_Scale;
    ri.r_g3.x = 503 * m_Scale + (285 * m_Scale - ri.r_g1.w) / 2;
    ri.r_g3.y = ri.r_g1.y;

    //设置横线纹理及区域
    ri.r_lt1 = isg1_Line ? m_te_raiLine : nullptr;
    ri.r_l1.w = 22 * m_Scale;
    ri.r_l1.h = 22 * m_Scale;
    ri.r_l1.x = 15 * m_Scale + (285 * m_Scale - ri.r_l1.w) / 2;
    ri.r_l1.y = 99 * m_Scale + (59.5 * m_Scale - ri.r_l1.h) / 2;

    ri.r_lt2 = isg2_Line ? m_te_raiLine : nullptr;
    ri.r_l2.w = 22 * m_Scale;
    ri.r_l2.h = 22 * m_Scale;
    ri.r_l2.x = 301 * m_Scale + (285 * m_Scale - ri.r_l2.w) / 2;
    ri.r_l2.y = 99 * m_Scale + (59.5 * m_Scale - ri.r_l2.h) / 2;

    ri.r_lt3 = isg3_Line ? m_te_raiLine : nullptr;
    ri.r_l3.w = 22 * m_Scale;
    ri.r_l3.h = 22 * m_Scale;
    ri.r_l3.x = 503 * m_Scale + (285 * m_Scale - ri.r_l3.w) / 2;
    ri.r_l3.y = 99 * m_Scale + (59.5 * m_Scale - ri.r_l3.h) / 2;

    //决定item1最终显示的纹理和区域
    if (ri.g1 != nullptr && ri.r_lt1 != nullptr)
    {
        ri.displayT1 = ri.g1;
        ri.rDisplayT1 = ri.r_g1;
    }
    else
    {
        ri.displayT1 = (ri.g1 != nullptr) ? ri.g1 : (ri.r_lt1 != nullptr) ? ri.r_lt1 : nullptr;
        if (ri.displayT1 == m_te_raiGou || ri.displayT1 == m_te_raiNGou)
        {
            ri.rDisplayT1 = ri.r_g1;
        }
        else if (ri.displayT1 == m_te_raiLine)
        {
            ri.rDisplayT1 = ri.r_l1;
        }
        else if (ri.displayT1 == nullptr)
        {
            ri.displayT1 = ri.t1;
            ri.rDisplayT1 = ri.d1;
        }
    }
    
    //决定item1最终显示的纹理和区域
    if (ri.g2 != nullptr && ri.r_lt2 != nullptr)
    {
        ri.displayT2 = ri.g2;
        ri.rDisplayT2 = ri.r_g2;
    }
    else
    {
        ri.displayT2 = (ri.g2 != nullptr) ? ri.g2 : (ri.r_lt2 != nullptr) ? ri.r_lt2 : nullptr;
        if (ri.displayT2 == m_te_raiGou || ri.displayT2 == m_te_raiNGou)
        {
            ri.rDisplayT2 = ri.r_g2;
        }
        else if (ri.displayT2 == m_te_raiLine)
        {
            ri.rDisplayT2 = ri.r_l2;
        }
        else if (ri.displayT2 == nullptr)
        {
            ri.displayT2 = ri.t2;
            ri.rDisplayT2 = ri.d2;
        }
    }

    //决定item1最终显示的纹理和区域
    if (ri.g3 != nullptr && ri.r_lt3 != nullptr)
    {
        ri.displayT3 = ri.g3;
        ri.rDisplayT3 = ri.r_g3;
    }
    else
    {
        ri.displayT3 = (ri.g3 != nullptr) ? ri.g3 : (ri.r_lt3 != nullptr) ? ri.r_lt3 : nullptr;
        if (ri.displayT3 == m_te_raiGou || ri.displayT3 == m_te_raiNGou)
        {
            ri.rDisplayT3 = ri.r_g3;
        }
        else if (ri.displayT3 == m_te_raiLine)
        {
            ri.rDisplayT3 = ri.r_l3;
        }
        else if (ri.displayT3 == nullptr)
        {
            ri.displayT3 = ri.t3;
            ri.rDisplayT3 = ri.d3;
        }
    }

    if(init_rowIndex != 0)
        m_vec_ri.push_back(ri);
    else
        m_ri_title = ri;
    ++init_rowIndex;
    return true;
}

SDL_HitTestResult SDLCALL Ui_RaiSDL_HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data)
{
    const int TITLEBAR_HEIGHT = 40;
    int w, h;
    SDL_GetWindowSize(win, &w, &h);
    if (area->y <= TITLEBAR_HEIGHT && area->x < (w / 4 * 3))
    {
        return SDL_HITTEST_DRAGGABLE;
    }
    return SDL_HITTEST_NORMAL;
}
