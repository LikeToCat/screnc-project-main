#include "stdafx.h"
#include "ComponentAPI.h"
#include "CDebug.h"
#include "GlobalFunc.h"
extern HANDLE ConsoleHandle;

bool COMAPI::SDL::InitWindowOpacity(
    _In_ SDL_Renderer* render,
    _In_ SDL_Window* window,
    _Out_ SDL_Texture** argbTexture,
    _In_ int textureWidth,
    _In_ int textureHeight,
    _Out_ std::vector<Uint32>* pixBuffer,
    _Out_ HWND* hWnd
)
{
    *argbTexture = SDL_CreateTexture(
        render,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        textureWidth, textureHeight
    );
    if (!(*argbTexture))
    {
        DB(ConsoleHandle, L"SDL窗口透明组件:创建argb纹理失败！");
        return false;
    }

    //设置为窗口（顶层，分层模式）
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo))
    {
        HWND hwnd = wmInfo.info.win.window;
        *hWnd = hwnd;
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_TOPMOST | WS_EX_LAYERED;
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);
    }
    else
    {
        DB(ConsoleHandle, L"SDL窗口透明组件:获取窗口Windows信息失败！");
    }

    (*pixBuffer).resize(textureWidth * textureHeight);
	return true;
}

bool COMAPI::SDL::UpdateArgbFrameToWindow(
    _In_ SDL_Window* window,
    _In_ SDL_Renderer* render,
    _In_ SDL_Texture* argbTexture,
    _In_ std::vector<Uint32>* pixBuffer,
    _In_ HWND* hWnd,
    _In_ int Opciality
 )
{
    int w = 0, h = 0;
    if (SDL_QueryTexture(argbTexture, nullptr, nullptr, &w, &h) != 0)
    {
        DB(ConsoleHandle, L"SDL窗口透明组件:查询 argbTexture 大小失败");
        return false;
    }

    int left, top;
    SDL_GetWindowPosition(window, &left, &top);
    POINT ptDst{ left,top };

    // 直接从 target 读回整帧像素
    SDL_RenderReadPixels(
        render,
        nullptr,                                // 从整个 target
        SDL_PIXELFORMAT_ARGB8888,
        (*pixBuffer).data(),
        w * sizeof(Uint32)
    );

    // 用 DIB+UpdateLayeredWindow 输出到桌面
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;  // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hBmp = CreateDIBSection(
        hdcMem, &bmi,
        DIB_RGB_COLORS,
        &bits, nullptr, 0
    );
    memcpy(bits, (*pixBuffer).data(), w * h * sizeof(Uint32));
    SelectObject(hdcMem, hBmp);

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, Opciality, AC_SRC_ALPHA };
    SIZE  sz = { w, h };
    POINT ptSrc = { 0, 0 };

    UpdateLayeredWindow(
        *hWnd, nullptr,
        &ptDst, &sz,
        hdcMem, &ptSrc,
        0, &bf, ULW_ALPHA
    );

    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

bool COMAPI::SDL::ImageLoad(_In_ SDL_Renderer* renderer, _In_ CString ImagePath, _Out_ SDL_Texture** Texture)
{
    ImagePath = GlobalFunc::GetExecutablePathFolder() + ImagePath;
    std::string ImagePathUtf8 = GlobalFunc::ConvertPathToUtf8(ImagePath);
    int imgFlags = IMG_INIT_PNG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL_image初始化失败");
        return false;
    }
    else
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL_image初始化成功");
        SDL_Surface* ImageSurface = IMG_Load(ImagePathUtf8.c_str());
        if (ImageSurface)
        {
            (*Texture) = SDL_CreateTextureFromSurface(renderer, ImageSurface);
            if (!(*Texture))
            {
                DB(ConsoleHandle, L"按钮图片资源纹理加载失败!");
                return false;
            }
        }
        else
        {
            DB(ConsoleHandle, L"按钮图片资源加载失败!");
            return false;
        }
    }
    return true;
}

bool COMAPI::SDL::TextLoad(
    _In_ SDL_Renderer* renderer,
    _In_ TTF_Font* ttfFont,
    _In_ std::wstring fontText,
    _In_ SDL_Color textColor, 
    _Out_ SDL_Texture** textTexture,
    _Out_ int* TextOriWidth,
    _Out_ int* TextOriHeight
)
{
    // 转换为UTF-8编码，因为SDL_ttf使用UTF-8
    if (*textTexture)
        SDL_DestroyTexture(*textTexture);
    std::string utf8Text = GlobalFunc::ConvertToUtf8(fontText);

    // 创建文字表面
    SDL_Surface* textSurface = nullptr;
    textSurface = TTF_RenderUTF8_Blended(
        ttfFont,
        utf8Text.c_str(),
        textColor
    );
    if (textSurface)
    {// 创建文字纹理
        *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (TextOriWidth)
            *TextOriWidth = textSurface->w;
        if (TextOriHeight)
            *TextOriHeight = textSurface->h;
        SDL_FreeSurface(textSurface);
    }
    else
    {
        if (TextOriWidth)
            *TextOriWidth = -1;
        if (TextOriHeight)
            *TextOriHeight = -1;
        DB(ConsoleHandle, L"创建文字纹理失败");
        return false;
    }
    return true;
}

bool COMAPI::SDL::TTFFontLoad(
    TTF_Font** font, 
    wchar_t* fontfamily, 
    int fontsize
)
{
    // 初始化SDL_ttf库
    if (TTF_WasInit() == 0)
    {
        if (TTF_Init() == -1)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L" TTF初始化失败");
            return false;
        }
        DEBUG_CONSOLE_STR(ConsoleHandle, L" SDL_ttf库还未初始化，当前窗口初始化SDL_ttf库成功");
    }

    // 获取系统字体路径
    wchar_t windowsDir[MAX_PATH];
    GetWindowsDirectoryW(windowsDir, MAX_PATH);
    std::wstring fontPath = std::wstring(windowsDir) + L"\\Fonts\\" + fontfamily;

    // 转换为UTF-8编码
    std::string utf8Path;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0)
    {
        utf8Path.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 转换字体路径为UTF-8编码成功");

    // 加载字体，字号16
    TTF_Font* pfont = TTF_OpenFont(utf8Path.c_str(), fontsize);
    if (!pfont)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"字体加载失败");
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 字体加载成功");

    if (pfont)
    {
        TTF_SetFontStyle(pfont, TTF_STYLE_NORMAL);
        TTF_SetFontOutline(pfont, 0); // 设置为0表示无轮廓
        TTF_SetFontKerning(pfont, 1); // 启用字距调整
        TTF_SetFontHinting(pfont, TTF_HINTING_LIGHT); // 使用轻度微调
    }
    (*font) = pfont;
    return true;
}

bool COMAPI::SDL::CreateBackgroundFromWindow(
    SDL_Renderer* renderer, 
    SDL_Texture** bkTexture, 
    HWND hwnd, 
    float brightnessFactor
)
{
    if (!renderer)
    {
        DB(ConsoleHandle, L"非法渲染器指针,从HWND句柄，创建背景位图失败");
        return false;
    }

    // 从HWND创建原始纹理
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    if (width <= 0 || height <= 0)
    {
        DB(ConsoleHandle, L"窗口尺寸无效");
        return nullptr;
    }

    // 获取窗口DC
    HDC windowDC = GetDC(hwnd);
    if (!windowDC)
    {
        DB(ConsoleHandle, L"获取窗口DC失败");
        return nullptr;
    }

    // 创建兼容DC和位图
    HDC memDC = CreateCompatibleDC(windowDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(windowDC, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

    // 复制窗口内容到位图
    BitBlt(memDC, 0, 0, width, height, windowDC, 0, 0, SRCCOPY);

    // 获取位图数据
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // 负数表示从上到下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    std::vector<BYTE> pixelData(width * height * 4);
    GetDIBits(memDC, hBitmap, 0, height, pixelData.data(), &bmi, DIB_RGB_COLORS);

    // 创建SDL表面
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        pixelData.data(), width, height, 32, width * 4,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000
    );

    SDL_Texture* texture = nullptr;
    if (surface)
    {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    // 清理资源
    SelectObject(memDC, oldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(hwnd, windowDC);

    // 创建降低亮度的纹理
    if (!texture || brightnessFactor < 0.0f || brightnessFactor > 1.0f)
    {
        return nullptr;
    }
    // 获取原始纹理信息
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);

    // 创建目标纹理
    SDL_Texture* targetTexture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (!targetTexture)
    {
        return nullptr;
    }

    // 设置渲染目标为新纹理
    SDL_Texture* originalTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, targetTexture);

    // 设置纹理的色彩调制（降低亮度）
    Uint8 brightness = static_cast<Uint8>(255 * brightnessFactor);
    SDL_SetTextureColorMod(texture, brightness, brightness, brightness);
    SDL_RenderCopy(renderer, texture, NULL, NULL);		// 渲染原始纹理到目标纹理
    SDL_SetTextureColorMod(texture, 255, 255, 255);	// 恢复原始纹理的色彩调制
    SDL_SetRenderTarget(renderer, originalTarget);		// 恢复原始渲染目标


    *bkTexture = targetTexture;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"背景纹理创建成功");
    return true;
}

bool COMAPI::MFC::DrawSelectGouByBorderRect(
    _In_ Gdiplus::Graphics* memGraphics, 
    _In_ int w, _In_ int h,
    _In_ int x, _In_ int y,
    _In_ float scale
)
{
    // 绘制对勾
    Gdiplus::Color checkColor(255, 255, 255, 255);
    Gdiplus::Pen checkPen(checkColor, 2 * scale);

    // 计算对勾的关键点
    float padding = w * 0.2f;
    float midX = x + w / 2.0f;

    // 对勾的三个点：左下，中下，右上
    Gdiplus::PointF pt1(x + padding, y + h / 2.0f);
    Gdiplus::PointF pt2(midX - padding / 2, y + h - padding);
    Gdiplus::PointF pt3(x + h - padding, y + padding);

    // 绘制对勾
    memGraphics->DrawLine(&checkPen, pt1, pt2);
    memGraphics->DrawLine(&checkPen, pt2, pt3);
    return true;
}

bool COMAPI::MFC::SetWindowShowOnTop(_In_ HWND Hwnd)
{
    ::SetWindowPos(Hwnd,
        HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE);
    ::SetWindowPos(Hwnd,
        HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE);
    ::SetForegroundWindow(Hwnd);
    return true;
}

void COMAPI::SDL_RenderAPI::renderHorizontalGradient(
    SDL_Renderer* renderer, SDL_Rect rect,
    SDL_Color startColor, SDL_Color endColor,
    int roundedRadius
)
{
    if (roundedRadius != 0 && roundedRadius > 0)
    {
        // 限制圆角半径
        int maxRadius = (rect.w < rect.h ? rect.w : rect.h) / 2;
        if (roundedRadius > maxRadius) roundedRadius = maxRadius;

        // 圆角渐变渲染 - 按列绘制
        for (int x = 0; x < rect.w; x++)
        {
            // 计算水平渐变比例
            float ratio = (float)x / (rect.w - 1);

            // 计算渐变颜色
            Uint8 r = (Uint8)(startColor.r + (endColor.r - startColor.r) * ratio);
            Uint8 g = (Uint8)(startColor.g + (endColor.g - startColor.g) * ratio);
            Uint8 b = (Uint8)(startColor.b + (endColor.b - startColor.b) * ratio);
            Uint8 a = (Uint8)(startColor.a + (endColor.a - startColor.a) * ratio);

            SDL_SetRenderDrawColor(renderer, r, g, b, a);

            // 计算当前列的有效绘制范围
            int startY = 0;
            int endY = rect.h - 1;

            // 如果在圆角区域，需要计算裁剪范围
            if (x < roundedRadius)
            {
                // 左侧圆角
                int dx = roundedRadius - x;
                int dy = (int)sqrt(roundedRadius * roundedRadius - dx * dx);
                startY = roundedRadius - dy;
                endY = rect.h - roundedRadius + dy - 1;
            }
            else if (x >= rect.w - roundedRadius)
            {
                // 右侧圆角
                int dx = x - (rect.w - roundedRadius);
                int dy = (int)sqrt(roundedRadius * roundedRadius - dx * dx);
                startY = roundedRadius - dy;
                endY = rect.h - roundedRadius + dy - 1;
            }

            // 绘制当前列
            if (startY <= endY)
            {
                SDL_RenderDrawLine(renderer,
                    rect.x + x, rect.y + startY,
                    rect.x + x, rect.y + endY);
            }
        }
    }
    else
    {
        // 水平渐变 - 绘制垂直线条
        for (int x = 0; x < rect.w; x++)
        {
            float ratio = (float)x / (rect.w - 1);

            Uint8 r = (Uint8)(startColor.r + (endColor.r - startColor.r) * ratio);
            Uint8 g = (Uint8)(startColor.g + (endColor.g - startColor.g) * ratio);
            Uint8 b = (Uint8)(startColor.b + (endColor.b - startColor.b) * ratio);
            Uint8 a = (Uint8)(startColor.a + (endColor.a - startColor.a) * ratio);

            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            SDL_RenderDrawLine(renderer, rect.x + x, rect.y, rect.x + x, rect.y + rect.h - 1);
        }
    }
}
