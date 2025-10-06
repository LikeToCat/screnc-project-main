#include "stdafx.h"
#include "theApp.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/buffer.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}
#include "LarStringConversion.h"
#include "CDebug.h"
#include "FFmpegTest.h"
void FFmpegTest::test()
{
    //test1();
    Muxing();
    return;
}

void FFmpegTest::test1()
{
    //void* demuxers = NULL;
//void* muxers = NULL;
//void* inputProtocol = NULL;
//void* outputProtocol = NULL;
//av_demuxer_iterate(&demuxers);
//av_muxer_iterate(&muxers);
//avio_enum_protocols(&outputProtocol, 0);
//avio_enum_protocols(&inputProtocol, 1);
    using namespace LARSC;
    // // 初始化网络（如果需要访问网络协议），并注册 demuxer/muxer（在新版 FFmpeg 中通常不再需要 av_register_all）
    //avformat_network_init();
    //
    //// 分配一个 AVFormatContext，用作演示“通用（与格式无关）的 libavformat Options 在 AVFormatContext 上可见”
    //AVFormatContext* fmt = avformat_alloc_context();
    //if (!fmt) 
    //{
    //    DB(ConsoleHandle, L"avformat_alloc_context() failed");
    //    return;
    //}
    //
    //DB(ConsoleHandle, L"--- 通过 av_opt_next 遍历 AVFormatContext 的 AVOptions（通用 libavformat options）---");
    //
    //// 使用 av_opt_next 枚举（第一个参数是对象指针，第二个参数是上一个返回的 AVOption 指针，传 NULL 开始）
    //const AVOption* opt = NULL;
    //while ((opt = av_opt_next(fmt, opt)) != NULL) {
    //    // 打印基础信息
    //    std::wstring name = c2w(opt->name);
    //    std::wstring help = c2w(opt->help ? opt->help : "(no help)");
    //    
    //    // 为了更健壮，我们打印 type 的整数值而不是尝试格式化所有类型的 default_val
    //    DBFMT(ConsoleHandle, L"option: %ls  type=%d  default_val_int=%lld  help=%ls",
    //        name.c_str(),
    //        opt->type,
    //        (long long)opt->default_val.i64,
    //        help.c_str());
    //}
    //
    //DB(ConsoleHandle, L"--- 通过 avformat_get_class 查看 AVClass，并使用 av_opt_find 查找某个 option ---");
    //
    //// 通过 avformat_get_class() 获取该对象对应的 AVClass，然后可以查询类信息
    //const AVClass* cls = avformat_get_class();
    //if (cls) {
    //    std::wstring clsname = c2w(cls->class_name);
    //    DBFMT(ConsoleHandle, L"AVClass name: %ls", clsname.c_str());
    //}
    //else {
    //    DB(ConsoleHandle, L"avformat_get_class() returned NULL");
    //}
    //
    //// 示例：尝试查找名为 "probesize" 的 option（许多情况下该 option 在 AVFormatContext 或其私有数据中可用）
    //const AVOption* found = av_opt_find(fmt, "probesize", NULL, 0, AV_OPT_SEARCH_FAKE_OBJ);
    //if (found) {
    //    DBFMT(ConsoleHandle, L"Found option 'probesize'  name=%ls  type=%d", c2w(found->name), found->type);
    //}
    //else {
    //    DB(ConsoleHandle, L"Option 'probesize' not found directly on AVFormatContext");
    //}
    //
    //// 额外演示：你也可以查询 av_opt_find 在 avformat_get_class() 上的行为（按类查找）
    //if (cls) {
    //    const AVOption* found2 = av_opt_find((void*)cls, "probesize", NULL, 0, 0);
    //    if (found2) {
    //        DBFMT(ConsoleHandle, L"Found on class: %ls", c2w(found2->name));
    //    }
    //    else {
    //        DB(ConsoleHandle, L"Not found on class");
    //    }
    //}

    //// 清理
    //avformat_free_context(fmt);
    //avformat_network_deinit();

    //DB(ConsoleHandle, L"--- 完成 ---");
}

void FFmpegTest::Muxing()
{
    
    return;
}
