#pragma once
#include "LLarSDL.h"
#include "Ui_SDLButton.h"
#include <vector>
class Ui_RaiSDL : public LLarSDL
{
public:
    enum class ShowMode
    {
        ReturnToOriPage,
        RecordOrOpenVip
    };

    struct rai_item
    {
        bool b_IsDraw1, b_IsDraw2, b_IsDraw3;           //是否在左边渲染小圆点
        std::wstring s1,s2,s3;                          //显示文本内容，从左到右
        SDL_Color c1, c2, c3;                           //显示文本颜色，从左到右
        SDL_Texture* t1, * t2, * t3;                    //显示的文本纹理，从左到右
        int w1, w2, w3, h1, h2, h3;                     //显示文本的宽高
        SDL_Rect d1, d2, d3;                            //显示文本的区域
        SDL_Texture* g1,*g2,*g3;                        //显示的对勾纹理（如果这个有实际值，则只显示这个对勾纹理）
        SDL_Texture* r_lt1,*r_lt2,*r_lt3;               //显示的横线纹理
        SDL_Rect r_g1,r_g2,r_g3;                        //显示对勾的区域
        SDL_Rect r_l1, r_l2, r_l3;                      //显示横线纹理的区域
        SDL_Color r_cpc1, r_cpc2, r_cpc3;               //显示的小圆点颜色
        SDL_Point r_cp1, r_cp2, r_cp3;                  //显示的小圆点圆心坐标
        int r_cpr1, r_cpr2, r_cpr3;                     //显示的小圆点的半径
        SDL_Texture* displayT1,*displayT2,*displayT3;   //最终决定显示的纹理
        SDL_Rect rDisplayT1, rDisplayT2, rDisplayT3;    //最终决定显示的纹理的区域
    };
    struct line_point
    {
        SDL_Point p1;
        SDL_Point p2;
    };
    struct center_bot_shadow
    {
        int shadowSize;                     //阴影大小
        SDL_Color RootColor;                //阴影根颜色
        std::vector<SDL_Color> vec_color;   //阴影颜色容器
        std::vector<line_point> vec_lp;     //阴影区域容器
        bool bIsShowShadow;                 //是否显示阴影
    };
    
    Ui_RaiSDL();

    void SetShowMode(ShowMode showMode);

    inline int getRet(){ return ret; }
protected:
    //窗口核心运行函数
    virtual void _ProcessEvents()override;      // 处理SDL事件，如鼠标点击和键盘输入
    virtual void _Render()override;             // 渲染窗口内容，包括面板、按钮和录制区域边框
    virtual bool _Cus_Init()override;           // 自定义初始化
    virtual bool _RegSDLHitTestFunc()override;  // 注册SDL窗口的点击测试回调
private:
    //init
    void _initArea();
    bool _initText();
    bool _initImage();
    bool _initRecordOrOpenVip();
    bool _InitReturnToOriPage();
    bool _initFont();
    bool _initBotShadow();
    bool _initCom();
    bool _AddRaiItem(
        std::vector<std::wstring> str_list,
        std::vector<SDL_Color> color_list,
        UINT isg1_y = 0, UINT isg2_y = 0, UINT isg3_y = 0,
        bool isg1_Line = false, bool isg2_Line = false, bool isg3_Line = false,
        SDL_Color cp_c1 = SDL_Color{ 75,75,75,255 }, 
        SDL_Color cp_c2 = SDL_Color{ 75,75,75,255 }, 
        SDL_Color cp_c3 = SDL_Color{ 75,75,75,255 },
        bool IsDrawCp1 = false, bool IsDrawCp2 = false, bool IsDrawCp3 = false
    );

    //render
    void _RenderCenter();
    void _RenderTitle();
    void _RenderBotShadow();

private:
    std::vector<rai_item> m_vec_ri;
    rai_item m_ri_title;            //标题riItem
    int m_startliney;
    TTF_Font* m_Font_Title;         //标题字体
    TTF_Font* m_Font_CenterTitle;   //中间区域标题字体
    TTF_Font* m_Font_Center;        //中间区域字体
    bool m_isRenderScrollTips;      //是否渲染滚动提示

    SDL_Rect m_Rect_Center;         //中间可滚动区域
    SDL_Rect m_Rect_CenterTitle;    //中间滚动区域的上方标题区域
    SDL_Rect m_Rect_VCenterItems;   //中间竖向区域的背景
    SDL_Rect m_Rect_raiCrown;       //王冠纹理区域
    SDL_Rect m_Rect_Title;          //标题区域
    SDL_Rect m_Rect_TitleTxt;       //标题文本纹理的区域
    SDL_Rect m_Rect_openvipLogo;    //开通会员金色钻石logo区域
    SDL_Rect m_Rect_centerPullDown;     //中间区域下拉logo区域
    SDL_Rect m_Rect_centerPullDowntxt;  //中间区域下拉文字提示区域
    SDL_Rect m_Rect_TitleLogo;          //标题logo区域

    SDL_Texture* m_te_raiGou;       //黄色对勾纹理
    SDL_Texture* m_te_raiNGou;      //白色对勾纹理
    SDL_Texture* m_te_raiCrown;     //王冠纹理
    SDL_Texture* m_te_raiLine;      //横线纹理
    SDL_Texture* m_te_titletxt;     //标题文本纹理
    SDL_Texture* m_te_openvipLogo;  //开通会员金色钻石logo
    SDL_Texture* m_te_centerPullDown;   //中间区域下拉logo
    SDL_Texture* m_te_centerPullDowntxt;//中间区域下拉文字提示区域
    SDL_Texture* m_te_TitleLogo;        //标题logo 

    //按钮相关
    Ui_SDLButton* m_btn_close;                          //关闭按钮
    Ui_SDLButton* m_btn_continue;                       //继续试用
    Ui_SDLButton* m_btn_openvip;                        //开通VIP
    Ui_SDLButtonManager m_BtnManager_RecordOrOpenVip;   //按钮管理器(界面模式1)
    Ui_SDLButtonManager m_BtnManager_ReturnToOriPage;   //按钮管理器(界面模式2)

    //中间区域底部阴影相关
    center_bot_shadow m_cba;

    //中间竖向区域的阴影相关
    SDL_Color m_color_shadowRoot;           //中间竖向区域的阴影根颜色
    std::vector<SDL_Color> m_vec_vcShadowC; //中间竖向区域的阴影颜色集合
    std::vector<SDL_Rect> m_vec_vcShadowR;  //中间竖向区域的阴影矩形集合
    int m_iShadowSize;                      //中间竖向区域的阴影大小

    //滚动相关
    COMCLASS::SDL::WindowScroller* m_Scroller;   //窗口区域滚动渲染器
    int m_ItemFirstVisiable;
    int m_ItemLastVisiable;

    int ret;            //窗口结束码
    ShowMode m_showMode;//显示模式
};