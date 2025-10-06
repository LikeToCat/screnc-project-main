#include "stdafx.h"
#include "LarStringConversion.h"
#include "CDebug.h" 
#include "CompatibleManager.h"

// 单例实例获取
CompatibleManager& CompatibleManager::GetInstance()
{
    static CompatibleManager instance;
    return instance;
}
CompatibleManager::CompatibleManager()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"CompatibleManager 初始化");
}
CompatibleManager::~CompatibleManager()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"CompatibleManager 销毁");
}

// 获取编码器类型
EncoderType CompatibleManager::GetEncoderType(const AVCodec* codec)
{
    if (!codec) {
        return EncoderType::UNKNOWN;
    }

    const char* name = codec->name;
    if (strstr(name, "nvenc")) {
        return EncoderType::NVENC;
    }
    else if (strstr(name, "amf")) {
        return EncoderType::AMF;
    }
    else if (strstr(name, "qsv")) {
        return EncoderType::QSV;
    }
    else if (strstr(name, "264") || strstr(name, "libx264")) {
        return EncoderType::X264;
    }

    return EncoderType::UNKNOWN;
}

// 测试编码器基本可用性
bool CompatibleManager::TestEncoderAvailability(const char* encoderName)
{
    const AVCodec* codec = avcodec_find_encoder_by_name(encoderName);
    return codec != nullptr;
}

// 测试特定编码器参数集是否兼容
bool CompatibleManager::TestEncoderParamSet(const AVCodec* codec, const EncoderParamSet& paramSet, int width, int height)
{
    if (!codec) {
        return false;
    }

    // 创建临时编码器上下文进行测试
    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        return false;
    }

    // 设置基本参数
    ctx->width = width;
    ctx->height = height;
    ctx->time_base = { 1, 30 };
    ctx->framerate = { 30, 1 };
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    ctx->bit_rate = 1000000;

    // 创建选项字典
    AVDictionary* options = nullptr;
    for (const auto& param : paramSet.params) {
        av_dict_set(&options, param.first.c_str(), param.second.c_str(), 0);
    }

    // 尝试打开编码器
    int ret = avcodec_open2(ctx, codec, &options);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码器参数集 '%s' 打开失败: %s\n",
            LARSC::c2w(paramSet.name.c_str()), LARSC::c2w(errBuf));
        avcodec_free_context(&ctx);
        av_dict_free(&options);
        return false;
    }

    // 创建并设置一个测试帧
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码器参数集 '%s' 测试失败: 无法分配帧\n",
            LARSC::c2w(paramSet.name.c_str()));
        avcodec_free_context(&ctx);
        av_dict_free(&options);
        return false;
    }

    frame->format = ctx->pix_fmt;
    frame->width = ctx->width;
    frame->height = ctx->height;
    frame->pts = 0;

    // 分配帧缓冲区
    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码器参数集 '%s' 测试失败: 无法分配帧缓冲区\n",
            LARSC::c2w(paramSet.name.c_str()));
        av_frame_free(&frame);
        avcodec_free_context(&ctx);
        av_dict_free(&options);
        return false;
    }

    // 确保帧缓冲区可写
    ret = av_frame_make_writable(frame);
    if (ret < 0) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码器参数集 '%s' 测试失败: 帧缓冲区不可写\n",
            LARSC::c2w(paramSet.name.c_str()));
        av_frame_free(&frame);
        avcodec_free_context(&ctx);
        av_dict_free(&options);
        return false;
    }

    // 填充帧数据 (简单的灰度图)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            frame->data[0][y * frame->linesize[0] + x] = x + y + 128 & 255; // Y平面
        }
    }
    // 填充U/V平面为中性值
    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width / 2; x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128; // U平面
            frame->data[2][y * frame->linesize[2] + x] = 128; // V平面
        }
    }

    // 创建数据包
    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码器参数集 '%s' 测试失败: 无法分配数据包\n",
            LARSC::c2w(paramSet.name.c_str()));
        av_frame_free(&frame);
        avcodec_free_context(&ctx);
        av_dict_free(&options);
        return false;
    }

    // 尝试发送帧进行编码
    ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码器参数集 '%s' 发送帧失败: %s\n",
            LARSC::c2w(paramSet.name.c_str()), LARSC::c2w(errBuf));
        av_packet_free(&pkt);
        av_frame_free(&frame);
        avcodec_free_context(&ctx);
        av_dict_free(&options);
        return false;
    }

    // 尝试接收编码后的数据包
    ret = avcodec_receive_packet(ctx, pkt);
    bool success = (ret >= 0);

    if (!success) {
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码器参数集 '%s' 接收数据包失败: %s\n",
            LARSC::c2w(paramSet.name.c_str()), LARSC::c2w(errBuf));
    }
    else {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码器参数集 '%s' 兼容性测试通过 - 成功编码测试帧\n",
            LARSC::c2w(paramSet.name.c_str()));
    }

    // 释放资源
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&ctx);
    av_dict_free(&options);

    return success;
}

// 根据预设和格式获取最佳编码器及其参数
AVDictionary* CompatibleManager::GetBestEncoder(const AVCodec** codec, EncoderPreset preset, ContainerFormat format)
{
    // 首先尝试硬件编码器，按照优先级顺序
    std::vector<const char*> hwEncoders = { "h264_nvenc", "h264_amf", "h264_qsv" };
    bool foundEncoder = false;

    for (const auto& encoderName : hwEncoders) {
        if (TestEncoderAvailability(encoderName)) {
            *codec = avcodec_find_encoder_by_name(encoderName);
            if (*codec) {
                // 找到硬件编码器，现在测试兼容性
                EncoderParamSet bestParams = FindBestCompatibleParams(*codec, preset, format);

                // 如果找到兼容参数，使用该编码器
                if (!bestParams.name.empty()) {
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"选择硬件编码器: %s 使用参数集: %s\n",
                        LARSC::c2w(encoderName), LARSC::c2w(bestParams.name.c_str()));
                    foundEncoder = true;
                    return CreateOptionDictionary(bestParams);
                }

                DEBUG_CONSOLE_FMT(ConsoleHandle, L"硬件编码器 %s 没有找到兼容的参数集\n",
                    LARSC::c2w(encoderName));
            }
        }
    }

    // 如果没有找到兼容的硬件编码器，使用软件编码器
    if (!foundEncoder) {
        *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (*codec) {
            EncoderParamSet bestParams = FindBestCompatibleParams(*codec, preset, format);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"选择软件编码器: libx264 使用参数集: %s\n",
                LARSC::c2w(bestParams.name.c_str()));
            return CreateOptionDictionary(bestParams);
        }
        else {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"错误: 无法找到任何可用的编码器!");
            return nullptr;
        }
    }
    return nullptr;
}

// 查找最佳兼容的参数集
EncoderParamSet CompatibleManager::FindBestCompatibleParams(const AVCodec* codec, EncoderPreset preset, ContainerFormat format)
{
    EncoderType encoderType = GetEncoderType(codec);

    // 生成缓存键
    std::string cacheKey = std::string(codec->name) + "_" +
        std::to_string(static_cast<int>(preset)) + "_" +
        std::to_string(static_cast<int>(format));

    // 检查缓存
    auto cacheIt = m_compatibilityCache.find(cacheKey);
    if (cacheIt != m_compatibilityCache.end()) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"使用缓存的兼容参数集: %s\n",
            LARSC::c2w(cacheIt->second.name.c_str()));
        return cacheIt->second;
    }

    // 获取该编码器所有可能的参数集
    std::vector<EncoderParamSet> paramSets = GetEncoderParamSets(encoderType, preset, format);

    // 从最优质的开始测试，直到找到一个兼容的参数集
    for (const auto& paramSet : paramSets) {
        if (TestEncoderParamSet(codec, paramSet)) {
            // 找到兼容的参数集，缓存它
            m_compatibilityCache[cacheKey] = paramSet;
            return paramSet;
        }
    }

    // 如果没有找到兼容的参数集，返回一个空的
    return EncoderParamSet();
}

// 获取特定编码器类型、预设和格式的所有可能参数集
std::vector<EncoderParamSet> CompatibleManager::GetEncoderParamSets(EncoderType encoderType, EncoderPreset preset, ContainerFormat format)
{
    std::vector<EncoderParamSet> paramSets;

    // 根据编码器类型生成参数集
    switch (encoderType) {
    case EncoderType::NVENC:
        paramSets = GenerateNVENCParamSets(preset);
        break;
    case EncoderType::AMF:
        paramSets = GenerateAMFParamSets(preset);
        break;
    case EncoderType::QSV:
        paramSets = GenerateQSVParamSets(preset);
        break;
    case EncoderType::X264:
    default:
        paramSets = GenerateX264ParamSets(preset);
        break;
    }

    // 为每个参数集添加容器格式特定参数
    for (auto& paramSet : paramSets) {
        AddContainerFormatParams(paramSet, format);
    }

    return paramSets;
}

// 创建编码器参数字典
AVDictionary* CompatibleManager::CreateOptionDictionary(const EncoderParamSet& paramSet)
{
    AVDictionary* options = nullptr;

    // 将参数集的参数添加到字典
    for (const auto& param : paramSet.params) {
        av_dict_set(&options, param.first.c_str(), param.second.c_str(), 0);
    }

    return options;
}

// 将容器格式特定参数添加到参数集
void CompatibleManager::AddContainerFormatParams(EncoderParamSet& paramSet, ContainerFormat format)
{
    switch (format) {
    case ContainerFormat::FLV:
        paramSet.params["profile"] = "baseline";
        break;
    case ContainerFormat::AVI:
        paramSet.params["profile"] = "main";
        break;
    case ContainerFormat::MP4:
        // 对于MP4，如果是高质量预设且支持，则使用high profile
        if (paramSet.name.find("High") != std::string::npos ||
            paramSet.name.find("Slow") != std::string::npos) {
            paramSet.params["profile"] = "high";
        }
        else {
            paramSet.params["profile"] = "main";
        }
        break;
    }
}

//生成NVENC编码器参数集列表   
std::vector<EncoderParamSet> CompatibleManager::GenerateNVENCParamSets(EncoderPreset preset)
{
    std::vector<EncoderParamSet> paramSets;

    // 基于用户选择的预设生成一系列从标准预设到高兼容性预设的参数集
    switch (preset) {
    case EncoderPreset::Slow: {
        // 最高质量预设 - p7 (理想参数，但可能不兼容)
        EncoderParamSet highQualityP7 = {
            "NVENC_Slow_P7",
            {
                {"preset", "p7"},
                {"rc", "vbr_hq"},
                {"spatial_aq", "1"},
                {"temporal_aq", "1"},
                {"aq-strength", "15"},
                {"rc-lookahead", "32"}
            }
        };
        paramSets.push_back(highQualityP7);

        // p6 预设
        EncoderParamSet qualityP6 = {
            "NVENC_Slow_P6",
            {
                {"preset", "p6"},
                {"rc", "vbr_hq"},
                {"spatial_aq", "1"},
                {"temporal_aq", "1"},
                {"rc-lookahead", "32"}
            }
        };
        paramSets.push_back(qualityP6);

        // p5 预设
        EncoderParamSet qualityP5 = {
            "NVENC_Slow_P5",
            {
                {"preset", "p5"},
                {"rc", "vbr"},
                {"spatial_aq", "1"},
                {"temporal_aq", "1"}
            }
        };
        paramSets.push_back(qualityP5);

        // p4 预设
        EncoderParamSet qualityP4 = {
            "NVENC_Slow_P4",
            {
                {"preset", "p4"},
                {"rc", "vbr"},
                {"spatial_aq", "1"}
            }
        };
        paramSets.push_back(qualityP4);

        // p3 预设
        EncoderParamSet qualityP3 = {
            "NVENC_Slow_P3",
            {
                {"preset", "p3"},
                {"rc", "cbr"},
                {"spatial_aq", "1"}
            }
        };
        paramSets.push_back(qualityP3);

        // 回退兼容性预设 - p2
        EncoderParamSet compatP2 = {
            "NVENC_Slow_P2",
            {
                {"preset", "p2"},
                {"rc", "cbr"},
                {"zerolatency", "1"}
            }
        };
        paramSets.push_back(compatP2);

        // 最低兼容性预设 - p1 + 额外兼容性选项
        EncoderParamSet fallbackP1 = {
            "NVENC_Slow_P1_Fallback",
            {
                {"preset", "p1"},
                {"rc", "cbr"},
                {"zerolatency", "1"},
                {"surfaces", "32"},
                {"delay", "0"},
                {"gpu", "any"}
            }
        };
        paramSets.push_back(fallbackP1);
        break;
    }

    case EncoderPreset::Medium: {
        // 中等质量 - p5 预设
        EncoderParamSet qualityP5 = {
            "NVENC_Medium_P5",
            {
                {"preset", "p5"},
                {"rc", "vbr"},
                {"spatial_aq", "1"}
            }
        };
        paramSets.push_back(qualityP5);

        // 中等质量 - p4 预设
        EncoderParamSet qualityP4 = {
            "NVENC_Medium_P4",
            {
                {"preset", "p4"},
                {"rc", "cbr"},
                {"spatial_aq", "1"}
            }
        };
        paramSets.push_back(qualityP4);

        // 中等质量 - p3 预设
        EncoderParamSet qualityP3 = {
            "NVENC_Medium_P3",
            {
                {"preset", "p3"},
                {"rc", "cbr"},
                {"zerolatency", "1"}
            }
        };
        paramSets.push_back(qualityP3);

        // 回退兼容性预设 - p2
        EncoderParamSet compatP2 = {
            "NVENC_Medium_P2",
            {
                {"preset", "p2"},
                {"rc", "cbr"},
                {"zerolatency", "1"}
            }
        };
        paramSets.push_back(compatP2);

        // 最低兼容性预设 - p1 + 额外兼容性选项
        EncoderParamSet fallbackP1 = {
            "NVENC_Medium_P1_Fallback",
            {
                {"preset", "p1"},
                {"rc", "cbr"},
                {"zerolatency", "1"},
                {"surfaces", "32"},
                {"delay", "0"},
                {"gpu", "any"}
            }
        };
        paramSets.push_back(fallbackP1);
        break;
    }

    case EncoderPreset::Fast:
    default: {
        // 快速预设 - p3
        EncoderParamSet fastP3 = {
            "NVENC_Fast_P3",
            {
                {"preset", "p3"},
                {"rc", "cbr"},
                {"zerolatency", "1"}
            }
        };
        paramSets.push_back(fastP3);

        // 快速预设 - p2
        EncoderParamSet fastP2 = {
            "NVENC_Fast_P2",
            {
                {"preset", "p2"},
                {"rc", "cbr"},
                {"zerolatency", "1"}
            }
        };
        paramSets.push_back(fastP2);

        // 最低兼容性预设 - p1 + 额外兼容性选项
        EncoderParamSet fallbackP1 = {
            "NVENC_Fast_P1_Fallback",
            {
                {"preset", "p1"},
                {"rc", "cbr"},
                {"zerolatency", "1"},
                {"surfaces", "32"},
                {"delay", "0"},
                {"gpu", "any"}
            }
        };
        paramSets.push_back(fallbackP1);
        break;
    }
    }

    return paramSets;
}

// 生成AMF编码器的参数集列表
std::vector<EncoderParamSet> CompatibleManager::GenerateAMFParamSets(EncoderPreset preset)
{
    std::vector<EncoderParamSet> paramSets;

    switch (preset) {
    case EncoderPreset::Slow: {
        // 最高质量预设
        EncoderParamSet highQuality = {
            "AMF_Slow_Quality",
            {
                {"usage", "transcoding"},
                {"quality", "quality"},
                {"profile", "high"},
                {"rate_control", "vbr_peak"},
                {"preanalysis", "true"},
                {"vbaq", "true"},
                {"frame_skipping", "false"}
            }
        };
        paramSets.push_back(highQuality);

        // 兼容性预设
        EncoderParamSet compatQuality = {
            "AMF_Slow_Balanced",
            {
                {"usage", "transcoding"},
                {"quality", "balanced"},
                {"profile", "high"},
                {"rate_control", "vbr_latency"},
                {"vbaq", "true"}
            }
        };
        paramSets.push_back(compatQuality);

        // 终极兼容预设
        EncoderParamSet fallbackPreset = {
            "AMF_Slow_Fallback",
            {
                {"usage", "transcoding"},
                {"quality", "speed"},
                {"profile", "main"},
                {"rate_control", "cbr"}
            }
        };
        paramSets.push_back(fallbackPreset);
        break;
    }

    case EncoderPreset::Medium: {
        // 中等质量预设
        EncoderParamSet mediumQuality = {
            "AMF_Medium_Balanced",
            {
                {"usage", "transcoding"},
                {"quality", "balanced"},
                {"profile", "main"},
                {"rate_control", "vbr_latency"},
                {"vbaq", "true"}
            }
        };
        paramSets.push_back(mediumQuality);

        // 兼容性预设
        EncoderParamSet compatPreset = {
            "AMF_Medium_Speed",
            {
                {"usage", "transcoding"},
                {"quality", "speed"},
                {"profile", "main"},
                {"rate_control", "cbr"}
            }
        };
        paramSets.push_back(compatPreset);

        // 终极兼容预设
        EncoderParamSet fallbackPreset = {
            "AMF_Medium_Fallback",
            {
                {"usage", "lowlatency"},
                {"quality", "speed"},
                {"profile", "main"},
                {"rate_control", "cbr"}
            }
        };
        paramSets.push_back(fallbackPreset);
        break;
    }

    case EncoderPreset::Fast:
    default: {
        // 快速预设
        EncoderParamSet fastQuality = {
            "AMF_Fast_Speed",
            {
                {"usage", "lowlatency"},
                {"quality", "speed"},
                {"profile", "baseline"},
                {"rate_control", "cbr"},
                {"frame_skipping", "true"}
            }
        };
        paramSets.push_back(fastQuality);

        // 终极速度预设
        EncoderParamSet ultraFast = {
            "AMF_Fast_UltraLowLatency",
            {
                {"usage", "ultralowlatency"},
                {"quality", "speed"},
                {"profile", "baseline"},
                {"rate_control", "cbr"},
                {"frame_skipping", "true"},
                {"header_insertion_mode", "none"}
            }
        };
        paramSets.push_back(ultraFast);
        break;
    }
    }

    return paramSets;
}

// 生成QSV编码器的参数集列表
std::vector<EncoderParamSet> CompatibleManager::GenerateQSVParamSets(EncoderPreset preset)
{
    std::vector<EncoderParamSet> paramSets;

    switch (preset) {
    case EncoderPreset::Slow: {
        // 最高质量预设
        EncoderParamSet highQuality = {
            "QSV_Slow_Quality",
            {
                {"preset", "veryslow"},
                {"profile", "high"},
                {"rdo", "1"},
                {"pic_timing_sei", "1"},
                {"extbrc", "1"},
                {"vcm", "0"}
            }
        };
        paramSets.push_back(highQuality);

        // 兼容性预设
        EncoderParamSet compatQuality = {
            "QSV_Slow_Balanced",
            {
                {"preset", "slow"},
                {"profile", "high"},
                {"extbrc", "1"}
            }
        };
        paramSets.push_back(compatQuality);

        // 终极兼容预设
        EncoderParamSet fallbackPreset = {
            "QSV_Slow_Fallback",
            {
                {"preset", "medium"},
                {"profile", "main"}
            }
        };
        paramSets.push_back(fallbackPreset);
        break;
    }

    case EncoderPreset::Medium: {
        // 中等质量预设
        EncoderParamSet mediumQuality = {
            "QSV_Medium_Balanced",
            {
                {"preset", "medium"},
                {"profile", "main"}
            }
        };
        paramSets.push_back(mediumQuality);

        // 兼容性预设
        EncoderParamSet compatPreset = {
            "QSV_Medium_Speed",
            {
                {"preset", "fast"},
                {"profile", "main"}
            }
        };
        paramSets.push_back(compatPreset);

        // 终极兼容预设
        EncoderParamSet fallbackPreset = {
            "QSV_Medium_Fallback",
            {
                {"preset", "veryfast"},
                {"profile", "main"},
                {"b_strategy", "0"}
            }
        };
        paramSets.push_back(fallbackPreset);
        break;
    }

    case EncoderPreset::Fast:
    default: {
        // 快速预设
        EncoderParamSet fastQuality = {
            "QSV_Fast_Speed",
            {
                {"preset", "veryfast"},
                {"profile", "baseline"},
                {"low_power", "1"}
            }
        };
        paramSets.push_back(fastQuality);

        // 终极速度预设
        EncoderParamSet ultraFast = {
            "QSV_Fast_UltraSpeed",
            {
                {"preset", "superfast"},
                {"profile", "baseline"},
                {"low_power", "1"},
                {"target_usage", "7"},
                {"b_strategy", "0"},
                {"max_b_frames", "0"}
            }
        };
        paramSets.push_back(ultraFast);
        break;
    }
    }

    return paramSets;
}

// 生成X264编码器的参数集列表
std::vector<EncoderParamSet> CompatibleManager::GenerateX264ParamSets(EncoderPreset preset)
{
    std::vector<EncoderParamSet> paramSets;

    switch (preset) {
    case EncoderPreset::Slow: {
        // 最高质量预设 - veryslow
        EncoderParamSet highQuality = {
            "X264_Slow_VerySlow",
            {
                {"preset", "veryslow"},
                {"tune", "film"},
                {"crf", "18"} // 低数值=高质量
            }
        };
        paramSets.push_back(highQuality);

        // 备选高质量预设 - slower
        EncoderParamSet highQualityAlt = {
            "X264_Slow_Slower",
            {
                {"preset", "slower"},
                {"tune", "film"},
                {"crf", "20"}
            }
        };
        paramSets.push_back(highQualityAlt);

        // 兼容性预设 - slow
        EncoderParamSet compatQuality = {
            "X264_Slow_Slow",
            {
                {"preset", "slow"},
                {"tune", "film"},
                {"crf", "22"}
            }
        };
        paramSets.push_back(compatQuality);

        // 终极兼容预设😍
        EncoderParamSet fallbackPreset = {
            "X264_Slow_Medium",
            {
                {"preset", "medium"},
                {"tune", "film"},
                {"crf", "23"}
            }
        };
        paramSets.push_back(fallbackPreset);
        break;
    }

    case EncoderPreset::Medium: {
        // 中等质量 - medium
        EncoderParamSet mediumQuality = {
            "X264_Medium_Medium",
            {
                {"preset", "medium"},
                {"tune", "film"},
                {"crf", "23"}
            }
        };
        paramSets.push_back(mediumQuality);

        // 备选中等质量 - fast
        EncoderParamSet mediumFast = {
            "X264_Medium_Fast",
            {
                {"preset", "fast"},
                {"tune", "film"},
                {"crf", "25"}
            }
        };
        paramSets.push_back(mediumFast);

        // 兼容性预设
        EncoderParamSet compatPreset = {
            "X264_Medium_Faster",
            {
                {"preset", "faster"},
                {"tune", "zerolatency"},
                {"crf", "27"}
            }
        };
        paramSets.push_back(compatPreset);

        // 终极兼容预设😍
        EncoderParamSet fallbackPreset = {
            "X264_Medium_VeryFast",
            {
                {"preset", "veryfast"},
                {"tune", "zerolatency"},
                {"crf", "28"}
            }
        };
        paramSets.push_back(fallbackPreset);
        break;
    }

    case EncoderPreset::Fast:
    default: {
        // 快速预设 - veryfast
        EncoderParamSet fastQuality = {
            "X264_Fast_VeryFast",
            {
                {"preset", "veryfast"},
                {"tune", "zerolatency"},
                {"crf", "28"}
            }
        };
        paramSets.push_back(fastQuality);

        // 最快预设 - superfast
        EncoderParamSet superFast = {
            "X264_Fast_SuperFast",
            {
                {"preset", "superfast"},
                {"tune", "zerolatency"},
                {"crf", "30"}
            }
        };
        paramSets.push_back(superFast);

        // 终极速度预设😍
        EncoderParamSet ultraFast = {
            "X264_Fast_UltraFast",
            {
                {"preset", "ultrafast"},
                {"tune", "zerolatency"},
                {"crf", "32"},
                {"refs", "1"},
                {"b_strategy", "0"},
                {"subq", "1"},
                {"me_range", "8"}
            }
        };
        paramSets.push_back(ultraFast);
        break;
    }
    }

    return paramSets;
}
