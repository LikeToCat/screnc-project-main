#include "stdafx.h"
#include "GameFocusDetector.h"
#include <psapi.h>
#include <tlhelp32.h>
#include <algorithm>
#include <shlwapi.h>  // 添加路径处理支持

#pragma comment(lib, "shlwapi.lib")  // 链接Shlwapi库

// 初始化静态成员
GameFocusDetector* GameFocusDetector::s_instance = nullptr;

// 获取单例实例
GameFocusDetector& GameFocusDetector::GetInstance()
{
    if (s_instance == nullptr)
    {
        s_instance = new GameFocusDetector();
    }
    return *s_instance;
}

// 构造函数
GameFocusDetector::GameFocusDetector()
    : m_eventHook(NULL), m_lastFocusedWindow(NULL), m_isMonitoring(false)
{
    DB(ConsoleHandle, L"GameFocusDetector 实例已创建");
}

// 析构函数
GameFocusDetector::~GameFocusDetector()
{
    StopMonitoring();
    DB(ConsoleHandle, L"GameFocusDetector 实例已销毁");
}

// 获取当前焦点窗口句柄
HWND GameFocusDetector::GetFocusedWindow()
{
    return GetForegroundWindow();
}

// 判断当前焦点窗口是否是游戏窗口
bool GameFocusDetector::IsFocusedWindowGame()
{
    HWND focusedWindow = GetFocusedWindow();
    if (!focusedWindow)
    {
        DB(ConsoleHandle, L"无法获取当前焦点窗口");
        return false;
    }

    return IsGameWindow(focusedWindow);
}

// 判断指定窗口是否是游戏窗口
bool GameFocusDetector::IsGameWindow(HWND hwnd)
{
    if (!hwnd || !IsWindow(hwnd))
    {
        DB(ConsoleHandle, L"无效的窗口句柄");
        return false;
    }

    // 获取窗口标题
    wchar_t windowTitle[256] = { 0 };
    GetWindowTextW(hwnd, windowTitle, 256);
    std::wstring windowTitleStr = windowTitle;

    // 获取窗口进程ID
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);

    DBFMT(ConsoleHandle, L"检测窗口: HWND=%p, 标题=\"%s\", PID=%u",
        (void*)hwnd, windowTitleStr.c_str(), processId);

    // 排除指定进程ID
    if (m_excludedProcessId != 0 && processId == m_excludedProcessId)
    {
        DBFMT(ConsoleHandle, L"排除当前程序窗口 PID=%u", processId);
        return false;
    }

    // 获取进程路径
    std::wstring processPath;
    if (!GetProcessPath(processId, processPath))
    {
        DBFMT(ConsoleHandle, L"无法获取进程路径: PID=%u", processId);
    }
    else
    {
        DBFMT(ConsoleHandle, L"进程路径: %s", processPath.c_str());
    }

    // 判断进程路径是否是游戏路径
    if (!processPath.empty() && IsGamePath(processPath))
    {
        DBFMT(ConsoleHandle, L"通过路径匹配确认为游戏窗口: \"%s\"", processPath.c_str());
        return true;
    }

    // 第一步：判断窗口标题是否是游戏标题
    if (IsGameWindowTitle(windowTitleStr, processId))
    {
        DBFMT(ConsoleHandle, L"通过标题匹配确认为游戏窗口: \"%s\"", windowTitleStr.c_str());
        return true;
    }

    // 第二步：判断窗口依赖是否与游戏相关
    //if (HasGameRelatedDependencies(processId))
    //{
    //    DBFMT(ConsoleHandle, L"通过依赖项分析确认为游戏窗口: \"%s\"", windowTitleStr.c_str());
    //    return true;
    //}

    DBFMT(ConsoleHandle, L"窗口不是游戏: \"%s\"", windowTitleStr.c_str());
    return false;
}

// 获取进程路径
bool GameFocusDetector::GetProcessPath(DWORD processId, std::wstring& processPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess)
    {
        DBFMT(ConsoleHandle, L"无法打开进程 PID=%u", processId);
        return false;
    }

    wchar_t pathBuffer[MAX_PATH] = { 0 };
    DWORD pathLength = MAX_PATH;
    BOOL result = QueryFullProcessImageNameW(hProcess, 0, pathBuffer, &pathLength);

    CloseHandle(hProcess);

    if (!result)
    {
        return false;
    }

    processPath = pathBuffer;
    return true;
}

// 判断路径是否是游戏相关路径
bool GameFocusDetector::IsGamePath(const std::wstring& path)
{
    // 将路径转换为小写以便不区分大小写比较
    std::wstring pathLower = path;
    std::transform(pathLower.begin(), pathLower.end(), pathLower.begin(), ::towlower);

    // 1. 常见游戏平台安装目录
    std::vector<std::wstring> gamePlatformPaths = {
        L"\\steam\\steamapps\\common\\",
        L"\\steamlibrary\\steamapps\\common\\",
        L"\\epic games\\",
        L"\\origin games\\",
        L"\\ea games\\",
        L"\\ea\\ea games\\",
        L"\\ubisoft\\",
        L"\\battle.net\\",
        L"\\blizzard\\",
        L"\\riot games\\",
        L"\\gog games\\",
        L"\\bethesda.net\\",
        L"\\xbox games\\",
        L"\\games\\"
    };

    for (const auto& platformPath : gamePlatformPaths)
    {
        if (pathLower.find(platformPath) != std::wstring::npos)
        {
            DBFMT(ConsoleHandle, L"检测到游戏平台路径: %s", platformPath.c_str());
            return true;
        }
    }

    // 3. 检查程序名是否与已知游戏可执行文件名匹配
    std::wstring fileName = PathFindFileNameW(pathLower.c_str());

    std::vector<std::wstring> gameExecutables = {
        L"league of legends.exe",
        L"valorant.exe",
        L"valorant-win64-shipping.exe",
        L"cs2.exe",
        L"csgo.exe",
        L"dota2.exe",
        L"fortniteclient-win64-shipping.exe",
        L"gta5.exe",
        L"gtav.exe",
        L"rdr2.exe",
        L"overwatch.exe",
        L"wow.exe",
        L"wowclassic.exe",
        L"yuanshen.exe",     // 原神
        L"genshinimpact.exe",
        L"minecraft.exe",
        L"javaw.exe",        // Minecraft Java版
        L"starfield.exe",
        L"eldenring.exe",
        L"cyberpunk2077.exe",
        L"witcher3.exe",
        L"apexlegends.exe",
        L"destiny2.exe",
        L"halo.exe",
        L"haloinfinity.exe",
        L"dnf.exe",          // 地下城与勇士
        L"pubg.exe",
        L"rainbowsix.exe",
        L"r6.exe",
        L"finalfantasyxiv.exe",
        L"ffxiv.exe"
    };

    for (const auto& exe : gameExecutables)
    {
        if (fileName == exe)
        {
            DBFMT(ConsoleHandle, L"检测到游戏可执行文件: %s", exe.c_str());
            return true;
        }
    }

    // 4. 检查目录中是否包含常见游戏关键词
    std::vector<std::wstring> gameKeywords = {
        L"\\game\\",
        L"\\games\\",
        L"\\gaming\\"
    };

    for (const auto& keyword : gameKeywords)
    {
        if (pathLower.find(keyword) != std::wstring::npos)
        {
            DBFMT(ConsoleHandle, L"检测到游戏关键词: %s", keyword.c_str());
            return true;
        }
    }

    return false;
}

// 判断窗口标题是否匹配已知游戏标题
bool GameFocusDetector::IsGameWindowTitle(const std::wstring& windowTitle, DWORD processId)
{
    // 转换为小写进行不区分大小写的比较
    std::wstring titleLower = windowTitle;
    std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::towlower);

    // 如果标题为空，不进行判断
    if (titleLower.empty())
    {
        return false;
    }

    // 首先确定进程不是会改变标题的应用（浏览器、音乐播放器等）
    std::wstring exeName;
    bool isTitleChangingApp = IsTitleChangingApp(processId, exeName);

    if (isTitleChangingApp)
    {
        DBFMT(ConsoleHandle, L"检测到会改变标题的应用: %s, 跳过标题判断", exeName.c_str());
        return false;
    }

    // 已知游戏标题关键词（全部小写）
    std::vector<std::wstring> gameKeywords = {
        // 游戏名称关键词
        L"league of legends", L"valorant", L"counter-strike", L"dota",
        L"apex legends", L"fortnite", L"minecraft", L"grand theft auto",
        L"gta", L"overwatch", L"world of warcraft", L"wow", L"call of duty",
        L"pubg", L"battlegrounds", L"genshin impact", L"原神", L"elden ring",
        L"cyberpunk", L"rainbow six", L"siege", L"rocket league",
        L"hearthstone", L"destiny", L"final fantasy", L"lost ark",
        L"battlefield", L"halo", L"assassin's creed", L"witcher",
        L"fallout", L"red dead redemption", L"star wars", L"fifa",
        L"nba", L"dark souls", L"sekiro", L"monster hunter", L"path of exile",
        L"diablo", L"rust", L"ark", L"英雄联盟", L"无畏契约", L"反恐精英",
        L"刀塔", L"堡垒之夜", L"我的世界", L"绝地求生", L"守望先锋",
        L"魔兽世界", L"使命召唤", L"地下城与勇士", L"王者荣耀", L"和平精英",
        L"永劫无间", L"战神", L"只狼", L"怪物猎人", L"最终幻想", L"失落的方舟",

        // 游戏类型/模式关键词
        L"battle royale", L"deathmatch", L"team deathmatch", L"capture the flag",
        L"game lobby", L"ranked", L"competitive", L"casual", L"campaign",
        L"multiplayer", L"single player", L"co-op", L"cooperative", L"pvp", L"pve",
        L"mmorpg", L"rpg", L"fps", L"moba", L"sandbox", L"survival", L"open world"
    };

    // 部分匹配窗口标题（检查是否包含游戏关键词）
    for (const auto& keyword : gameKeywords)
    {
        if (titleLower.find(keyword) != std::wstring::npos)
        {
            DBFMT(ConsoleHandle, L"标题包含游戏关键词: \"%s\"", keyword.c_str());
            return true;
        }
    }

    // 完整游戏标题匹配（不区分大小写）
    std::vector<std::wstring> fullGameTitles = {
        // 精选热门游戏标题 - 只列出一些不包含上面关键词的游戏完整名称
        L"among us", L"roblox", L"osu!", L"teamfight tactics", L"tft",
        L"legends of runeterra", L"valheim", L"terraria", L"stardew valley",
        L"sea of thieves", L"factorio", L"rimworld", L"hogwarts legacy",
        L"starfield", L"animal crossing", L"ghost of tsushima", L"god of war",
        L"half-life", L"portal", L"left 4 dead", L"back 4 blood",
        // 中文游戏名
        L"剑网3", L"天刀", L"天涯明月刀", L"逆水寒", L"梦幻西游", L"大话西游",
        L"新天龙八部", L"天龙八部", L"征途", L"剑侠情缘", L"流放之路", L"幻塔",
        L"崩坏", L"明日方舟", L"阴阳师"
    };

    // 严格匹配窗口标题
    for (const auto& gameTitle : fullGameTitles)
    {
        if (titleLower == gameTitle)
        {
            DBFMT(ConsoleHandle, L"标题完全匹配已知游戏: \"%s\"", gameTitle.c_str());
            return true;
        }
    }

    DB(ConsoleHandle, L"标题未匹配任何游戏关键词或完整游戏名");
    return false;
}

// 判断进程是否是会改变标题的应用（如浏览器、音乐播放器）
bool GameFocusDetector::IsTitleChangingApp(DWORD processId, std::wstring& exeName)
{
    // 会改变窗口标题的应用列表（音乐播放器、浏览器等）
    std::vector<std::wstring> titleChangingApps = {
        // 浏览器
        L"chrome.exe", L"firefox.exe", L"msedge.exe", L"opera.exe",
        L"brave.exe", L"vivaldi.exe", L"iexplore.exe", L"safari.exe",
        L"360chrome.exe", L"360se.exe", L"liebao.exe", L"maxthon.exe",
        L"qqbrowser.exe", L"sogouexplorer.exe",

        // 音乐播放器
        L"cloudmusic.exe", L"qqmusic.exe", L"kugou.exe", L"kuwo.exe",
        L"spotify.exe", L"music.ui.exe", L"wmplayer.exe", L"itunes.exe",
        L"foobar2000.exe", L"aimp.exe", L"groove.exe", L"musicbee.exe",
        L"dopamine.exe", L"potplayer.exe",

        // 视频播放器
        L"vlc.exe", L"mpc-hc.exe", L"mpc-hc64.exe", L"mpc-be.exe",
        L"potplayer.exe", L"kmplayer.exe", L"gomplayer.exe",
        L"splayer.exe", L"mpv.exe", L"smplayer.exe",

        // 文档查看器
        L"acrobat.exe", L"acrord32.exe", L"foxitreader.exe", L"SumatraPDF.exe",

        // 即时通讯软件
        L"wechat.exe", L"qq.exe", L"tim.exe", L"dingtalk.exe",
        L"slack.exe", L"discord.exe", L"teams.exe", L"skype.exe",
        L"telegram.exe", L"whatsapp.exe",

        // 开发工具
        L"devenv.exe", L"code.exe", L"pycharm64.exe", L"idea64.exe",
        L"eclipse.exe", L"android studio.exe", L"visual studio code.exe",

        // 其他会改变标题的应用
        L"explorer.exe", L"notepad.exe", L"notepad++.exe", L"wordpad.exe",
        L"winword.exe", L"excel.exe", L"powerpnt.exe", L"outlook.exe",
        L"onenote.exe", L"powershell.exe", L"cmd.exe", L"terminal.exe"
    };

    // 获取进程可执行文件名
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess)
    {
        return false;
    }

    wchar_t processPath[MAX_PATH] = { 0 };
    bool foundExeName = false;

    if (GetModuleFileNameEx(hProcess, NULL, processPath, MAX_PATH))
    {
        std::wstring processName = processPath;
        std::transform(processName.begin(), processName.end(), processName.begin(), ::towlower);

        // 提取可执行文件名
        size_t lastSlash = processName.find_last_of(L'\\');
        if (lastSlash != std::wstring::npos)
        {
            exeName = processName.substr(lastSlash + 1);
            foundExeName = true;

            // 检查是否是会改变标题的应用
            for (const auto& app : titleChangingApps)
            {
                if (exeName == app)
                {
                    CloseHandle(hProcess);
                    return true;
                }
            }
        }
    }

    CloseHandle(hProcess);

    if (!foundExeName)
    {
        exeName = L"[unknown]";
    }

    return false;
}

bool GameFocusDetector::HasGameRelatedDependencies(DWORD processId)
{
    DBFMT(ConsoleHandle, L"开始分析进程 PID=%u 的依赖项", processId);

    // 打开进程
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess)
    {
        DBFMT(ConsoleHandle, L"无法打开进程 PID=%u", processId);
        return false;
    }

    // 分析进程加载的DLL
    int gameEngineDllScore = 0;    // 游戏引擎DLL
    int graphicsDllScore = 0;      // 图形相关DLL
    int gameSupportDllScore = 0;   // 游戏支持DLL
    int audioPhysicsDllScore = 0;  // 音频和物理引擎DLL

    // 储存已找到的游戏DLL
    std::vector<std::wstring> foundGameDlls;

    // 游戏引擎相关DLL
    std::vector<std::wstring> gameEngineDlls = {
        L"unityplayer.dll", L"unity.dll", L"unrealengine", L"ue4game", L"ue5game",
        L"cryengine", L"godot", L"source", L"valve", L"gamebryo",
        L"renpy", L"siglus", L"siglusengine", L"kirikiri", L"mono.dll",
        L"leagueclient.dll", L"riotclient.dll", L"game.dll", L"riotgames",
        L"cocos2d", L"frostbite", L"id tech", L"rage.dll", L"unity3d",
        L"unreal", L"unreal4", L"unreal5", L"gameengine", L"gameengine64"
    };

    // 图形相关DLL
    std::vector<std::wstring> graphicsDlls = {
        L"d3d9.dll", L"d3d10.dll", L"d3d11.dll", L"d3d12.dll", L"dxgi.dll",
        L"opengl32.dll", L"vulkan-1.dll", L"nvapi", L"d3dcompiler_",
        L"dxva2.dll", L"libgl", L"sdl2", L"sdl", L"glew", L"nvoglv",
        L"amddxx", L"amdocl", L"atigktxx", L"aticfx", L"nvwgf2",
        L"nvopencl", L"nvcuda", L"nvoptix"
    };

    // 游戏支持DLL
    std::vector<std::wstring> gameSupportDlls = {
        L"xinput", L"dinput", L"steam_api", L"easyanticheat",
        L"battleye", L"vanguard", L"discord_game_sdk", L"gameoverlayrenderer",
        L"galaxyext.dll", L"uplay", L"anticheatsys", L"tencent", L"ggprivate",
        L"xboxgame", L"xaudio2_", L"joypad", L"gameinput", L"gamecontroller",
        L"directinput", L"rawInput", L"hidclass", L"hidusage", L"hidparse"
    };

    // 音频和物理引擎DLL
    std::vector<std::wstring> audioPhysicsDlls = {
        L"fmod", L"wwise", L"xaudio2", L"miles", L"openal", L"audio3d",
        L"physx", L"havok", L"bullet", L"ogg.dll", L"vorbis", L"opus.dll",
        L"bink", L"binkw", L"radaudio", L"directsound", L"dsound", L"audioses",
        L"gameaudio", L"soundengine", L"physics", L"physicsengine", L"nvidia_physx"
    };

    // 使用EnumProcessModules
    HMODULE hModules[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded))
    {
        DWORD moduleCount = cbNeeded / sizeof(HMODULE);
        DBFMT(ConsoleHandle, L"进程加载了 %u 个模块", moduleCount);

        for (DWORD i = 0; i < moduleCount; i++)
        {
            wchar_t modulePath[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, hModules[i], modulePath, sizeof(modulePath) / sizeof(wchar_t)))
            {
                std::wstring moduleName = modulePath;
                std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::towlower);

                // 提取DLL文件名
                std::wstring dllFileName;
                size_t lastSlash = moduleName.find_last_of(L'\\');
                if (lastSlash != std::wstring::npos)
                {
                    dllFileName = moduleName.substr(lastSlash + 1);
                }
                else
                {
                    dllFileName = moduleName;
                }

                // 检查游戏引擎DLL
                for (const auto& dll : gameEngineDlls)
                {
                    if (dllFileName.find(dll) != std::wstring::npos)
                    {
                        gameEngineDllScore += 3;
                        foundGameDlls.push_back(dllFileName);
                        DBFMT(ConsoleHandle, L"发现游戏引擎DLL: %s", dllFileName.c_str());
                        break;
                    }
                }

                // 检查图形相关DLL
                for (const auto& dll : graphicsDlls)
                {
                    if (dllFileName.find(dll) != std::wstring::npos)
                    {
                        graphicsDllScore += 1;
                        foundGameDlls.push_back(dllFileName);
                        DBFMT(ConsoleHandle, L"发现图形相关DLL: %s", dllFileName.c_str());
                        break;
                    }
                }

                // 检查游戏支持DLL
                for (const auto& dll : gameSupportDlls)
                {
                    if (dllFileName.find(dll) != std::wstring::npos)
                    {
                        gameSupportDllScore += 2;
                        foundGameDlls.push_back(dllFileName);
                        DBFMT(ConsoleHandle, L"发现游戏支持DLL: %s", dllFileName.c_str());
                        break;
                    }
                }

                // 检查音频和物理引擎DLL
                for (const auto& dll : audioPhysicsDlls)
                {
                    if (dllFileName.find(dll) != std::wstring::npos)
                    {
                        audioPhysicsDllScore += 2;
                        foundGameDlls.push_back(dllFileName);
                        DBFMT(ConsoleHandle, L"发现音频/物理引擎DLL: %s", dllFileName.c_str());
                        break;
                    }
                }
            }
        }
    }

    CloseHandle(hProcess);

    // 计算总分数
    int totalScore = gameEngineDllScore + gameSupportDllScore +
        audioPhysicsDllScore + (graphicsDllScore > 0 ? 1 : 0);

    DBFMT(ConsoleHandle, L"DLL分析结果: 总分=%d, 引擎=%d, 支持=%d, 音频物理=%d, 图形=%d",
        totalScore, gameEngineDllScore, gameSupportDllScore,
        audioPhysicsDllScore, graphicsDllScore);

    // 依据分数判断是否是游戏窗口
    if (gameEngineDllScore > 0)
    {
        DB(ConsoleHandle, L"检测到游戏引擎DLL，确认为游戏窗口");
        return true;
    }
    else if (totalScore >= 4)
    {
        DB(ConsoleHandle, L"DLL特征分数达到阈值，确认为游戏窗口");
        return true;
    }
    else if (gameSupportDllScore > 0 && graphicsDllScore > 0)
    {
        DB(ConsoleHandle, L"同时检测到游戏支持DLL和图形DLL，确认为游戏窗口");
        return true;
    }

    DB(ConsoleHandle, L"依赖项分析未能确认为游戏窗口");
    return false;
}

// 启动焦点窗口监控
bool GameFocusDetector::StartMonitoring()
{
    if (m_isMonitoring)
    {
        DB(ConsoleHandle, L"焦点窗口监控已经在运行中");
        return true;
    }

    // 注册一个事件钩子，监听焦点窗口变化事件
    m_eventHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,   // 要监听的事件：前台窗口变化
        EVENT_SYSTEM_FOREGROUND,   // 事件范围结束
        NULL,                      // 事件处理程序的DLL句柄，NULL表示使用当前进程
        WinEventProc,              // 回调函数
        0,                         // 进程ID，0表示所有进程
        0,                         // 线程ID，0表示所有线程
        WINEVENT_OUTOFCONTEXT      // 异步模式
    );

    if (m_eventHook)
    {
        m_isMonitoring = true;
        m_lastFocusedWindow = GetForegroundWindow();
        DB(ConsoleHandle, L"焦点窗口变化监控已启动");
        return true;
    }
    else
    {
        DBFMT(ConsoleHandle, L"焦点窗口变化监控启动失败，错误码: %d", GetLastError());
        return false;
    }
}

// 停止焦点窗口监控
void GameFocusDetector::StopMonitoring()
{
    if (m_isMonitoring && m_eventHook)
    {
        UnhookWinEvent(m_eventHook);
        m_eventHook = NULL;
        m_isMonitoring = false;
        DB(ConsoleHandle, L"焦点窗口变化监控已停止");
    }
}

// 处理焦点窗口变化
void GameFocusDetector::HandleFocusChange(HWND newFocusedWindow)
{
    // 如果新焦点窗口与上一个相同，不处理
    if (newFocusedWindow == m_lastFocusedWindow)
    {
        return;
    }

    DBFMT(ConsoleHandle, L"焦点窗口变化：HWND=%p", (void*)newFocusedWindow);

    // 获取窗口标题
    wchar_t windowTitle[256] = { 0 };
    GetWindowTextW(newFocusedWindow, windowTitle, 256);
    DBFMT(ConsoleHandle, L"新焦点窗口标题: %s", windowTitle);

    // 判断是否是游戏窗口
    bool isGameWindow = IsGameWindow(newFocusedWindow);

    // 更新最后一个焦点窗口
    m_lastFocusedWindow = newFocusedWindow;

    // 调用回调函数
    if (m_focusChangedCallback)
    {
        m_focusChangedCallback(newFocusedWindow, isGameWindow);
    }
}

// 静态回调函数
void CALLBACK GameFocusDetector::WinEventProc(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD idEventThread,
    DWORD dwmsEventTime
)
{
    // 仅处理窗口对象的焦点变化事件
    if (event == EVENT_SYSTEM_FOREGROUND && idObject == OBJID_WINDOW && s_instance)
    {
        s_instance->HandleFocusChange(hwnd);
    }
}