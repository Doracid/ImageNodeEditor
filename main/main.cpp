// ============================================================
// Image Node Editor — Node-based Image Processing Workflow Tool
// ============================================================

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "core/NodeRegistry.h"
#include "gui/MainWindow.h"
#include "cli/CommandLineRunner.h"

// Forward declarations for registration helpers
// (defined in registration.cpp or at the bottom)
void registerAllNodeTypes();

int main(int argc, char *argv[])
{
    // Register all node types before anything
    registerAllNodeTypes();

    // Single QApplication for both CLI and GUI modes (Qt5 compatible)
    QApplication app(argc, argv);
    app.setApplicationName("ImageNodeEditor");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("基于节点的图像处理工作流工具");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption workflowOpt("workflow", "工作流 JSON 文件路径。", "file");
    QCommandLineOption noGuiOpt("no-gui", "无 GUI 模式运行（需配合 --workflow）。");
    QCommandLineOption imageOpt("image", "输入图像路径（覆盖工作流中的 ImageInputNode）。", "file");
    QCommandLineOption outputOpt("output", "输出图像路径（覆盖工作流中的 ImageOutputNode）。", "file");
    parser.addOption(workflowOpt);
    parser.addOption(noGuiOpt);
    parser.addOption(imageOpt);
    parser.addOption(outputOpt);
    parser.process(app);

    QString workflowPath = parser.value(workflowOpt);
    bool noGui = parser.isSet(noGuiOpt);
    QString imagePath = parser.value(imageOpt);
    QString outputPath = parser.value(outputOpt);

    if (noGui) {
        if (workflowPath.isEmpty()) {
            printf("错误: --no-gui 模式必须指定 --workflow。\n");
            parser.showHelp(1);
            return 1;
        }

        QString errorMsg;
        int result = CommandLineRunner::run(workflowPath, imagePath, outputPath, &errorMsg);
        if (result != 0) {
            fprintf(stderr, "错误: %s\n", qPrintable(errorMsg));
        }
        return result;
    }

    // ---- GUI mode ----
    MainWindow w;
    if (!workflowPath.isEmpty()) {
        QMetaObject::invokeMethod(&w, [&w, &workflowPath]() {
            // TODO: load workflow directly
        }, Qt::QueuedConnection);
    }

    w.show();

    return app.exec();
}

// ============================================================
// Node type registration
// ============================================================
#include "nodes/ImageInputNode.h"
#include "nodes/ImageOutputNode.h"
#include "nodes/ImageViewerNode.h"
#include "nodes/CropNode.h"
#include "nodes/ResizeNode.h"
#include "nodes/BrightnessContrastNode.h"
#include "nodes/GaussianBlurNode.h"
#include "nodes/GrayscaleNode.h"
#include "nodes/EdgeDetectionNode.h"
#include "nodes/ChannelSplitNode.h"
#include "nodes/ChannelMergeNode.h"
#include "nodes/WatermarkNode.h"
#include "nodes/RotateNode.h"
#include "nodes/InvertNode.h"
#include "nodes/SaturationNode.h"
#include "nodes/HueShiftNode.h"
#include "nodes/GammaNode.h"
#include "nodes/ThresholdNode.h"
#include "nodes/SepiaNode.h"
#include "nodes/SharpenNode.h"
#include "nodes/ColorTemperatureNode.h"
#include "nodes/FadeNode.h"
#include "nodes/PixelateNode.h"
#include "nodes/VignetteNode.h"
#include "nodes/PencilSketchNode.h"
#include "nodes/CartoonNode.h"
#include "nodes/ExposureNode.h"
#include "nodes/VibranceNode.h"
#include "nodes/WhiteBalanceNode.h"
#include "nodes/HighlightsShadowsNode.h"
#include "nodes/WhitesBlacksNode.h"
#include "nodes/ClarityNode.h"
#include "nodes/ToneCurveNode.h"
#include "nodes/AutoEnhanceNode.h"
#include "nodes/ComicStyleNode.h"
#include "nodes/GradientMapNode.h"
#include "nodes/OilPaintingNode.h"
#include "nodes/PolarCoordsNode.h"
#include "nodes/LensFlareNode.h"
#include "nodes/MetalStyleNode.h"

void registerAllNodeTypes()
{
    auto &reg = NodeRegistry::instance();

    reg.registerType("ImageInputNode",    "输入",       "图像输入",    "从文件加载图像",           []() -> Node* { return new ImageInputNode(); });
    reg.registerType("ImageOutputNode",   "输出",      "图像输出",   "保存图像到文件",             []() -> Node* { return new ImageOutputNode(); });
    reg.registerType("ImageViewerNode",   "输出",      "图像查看器",   "在窗口中预览图像",        []() -> Node* { return new ImageViewerNode(); });
    reg.registerType("CropNode",          "几何变换",      "裁剪",           "裁剪图像区域",              []() -> Node* { return new CropNode(); });
    reg.registerType("ResizeNode",        "几何变换",      "缩放",         "调整图像尺寸",        []() -> Node* { return new ResizeNode(); });
    reg.registerType("BrightnessContrastNode", "色彩调整", "亮度/对比度","调整亮度与对比度",   []() -> Node* { return new BrightnessContrastNode(); });
    reg.registerType("GaussianBlurNode",  "滤波",      "高斯模糊",  "应用高斯模糊",            []() -> Node* { return new GaussianBlurNode(); });
    reg.registerType("GrayscaleNode",     "转换",  "灰度化",      "转换为灰度图像",           []() -> Node* { return new GrayscaleNode(); });
    reg.registerType("EdgeDetectionNode", "滤波",      "边缘检测", "Sobel 边缘检测",           []() -> Node* { return new EdgeDetectionNode(); });
    reg.registerType("ChannelSplitNode",  "多端口",   "通道分离",  "分离为 R/G/B 通道",      []() -> Node* { return new ChannelSplitNode(); });
    reg.registerType("ChannelMergeNode",  "多端口",   "通道合并",  "合并 R/G/B 为彩色图像",         []() -> Node* { return new ChannelMergeNode(); });
    reg.registerType("WatermarkNode",     "几何变换",      "水印",      "添加文字水印",             []() -> Node* { return new WatermarkNode(); });
    reg.registerType("RotateNode",        "几何变换",      "旋转",         "按角度旋转图像",          []() -> Node* { return new RotateNode(); });
    reg.registerType("InvertNode",        "色彩调整",      "反色",         "反转图像颜色",            []() -> Node* { return new InvertNode(); });
    reg.registerType("SaturationNode",    "色彩调整",      "饱和度",      "调整色彩饱和度",             []() -> Node* { return new SaturationNode(); });
    reg.registerType("HueShiftNode",      "色彩调整",      "色相偏移",     "旋转色相角度",               []() -> Node* { return new HueShiftNode(); });
    reg.registerType("GammaNode",         "色彩调整",      "伽马校正",    "伽马校正",                    []() -> Node* { return new GammaNode(); });
    reg.registerType("ThresholdNode",     "转换",      "二值化",      "全局/大津法/自适应均值/自适应高斯二值化",              []() -> Node* { return new ThresholdNode(); });
    reg.registerType("SepiaNode",         "风格化",    "怀旧",        "怀旧色调效果",               []() -> Node* { return new SepiaNode(); });
    reg.registerType("SharpenNode",       "滤波",      "锐化",        "锐化图像",                   []() -> Node* { return new SharpenNode(); });
    reg.registerType("ColorTemperatureNode", "转换", "冷暖色调",  "调节画面冷暖色温",           []() -> Node* { return new ColorTemperatureNode(); });
    reg.registerType("FadeNode",          "转换",    "褪色",        "褪色至灰度效果",             []() -> Node* { return new FadeNode(); });
    reg.registerType("PixelateNode",      "风格化",    "像素化",      "马赛克像素化效果",            []() -> Node* { return new PixelateNode(); });
    reg.registerType("VignetteNode",      "风格化",    "暗角",        "添加画面边缘暗角",            []() -> Node* { return new VignetteNode(); });
    reg.registerType("PencilSketchNode",  "风格化",    "铅笔画",      "铅笔素描画效果",             []() -> Node* { return new PencilSketchNode(); });
    reg.registerType("CartoonNode",       "风格化",    "卡通风格",    "色彩量化+边缘描边卡通效果",   []() -> Node* { return new CartoonNode(); });
    reg.registerType("ExposureNode",       "色彩调整",  "曝光",        "EV 曝光补偿",                 []() -> Node* { return new ExposureNode(); });
    reg.registerType("VibranceNode",       "色彩调整",  "自然饱和度",  "智能饱和度增强（保护肤色）", []() -> Node* { return new VibranceNode(); });
    reg.registerType("WhiteBalanceNode",   "色彩调整",  "白平衡",      "色温/色调/自动白平衡",       []() -> Node* { return new WhiteBalanceNode(); });
    reg.registerType("HighlightsShadowsNode", "色彩调整", "高光/阴影","独立调整高光和阴影区域",     []() -> Node* { return new HighlightsShadowsNode(); });
    reg.registerType("WhitesBlacksNode",   "色彩调整",  "白色/黑色",   "调整白点和黑点（端点裁剪）", []() -> Node* { return new WhitesBlacksNode(); });
    reg.registerType("ClarityNode",        "滤波",      "清晰度",      "中频对比度增强",              []() -> Node* { return new ClarityNode(); });
    reg.registerType("ToneCurveNode",      "色彩调整",  "色调曲线",    "256 级 LUT 色调曲线",        []() -> Node* { return new ToneCurveNode(); });
    reg.registerType("AutoEnhanceNode",    "色彩调整",  "自动美化",    "自动白平衡+色阶+S曲线+饱和度",[]() -> Node* { return new AutoEnhanceNode(); });
    reg.registerType("ComicStyleNode",     "风格化",    "漫画风",      "黑色轮廓线+白色背景漫画效果", []() -> Node* { return new ComicStyleNode(); });
    reg.registerType("GradientMapNode",    "色彩调整",  "渐变映射",    "灰度映射为彩色渐变色调",    []() -> Node* { return new GradientMapNode(); });
    reg.registerType("OilPaintingNode",    "风格化",    "油画效果",    "模拟油画笔触效果",          []() -> Node* { return new OilPaintingNode(); });
    reg.registerType("PolarCoordsNode",    "风格化",    "极坐标",      "直角坐标↔极坐标转换",        []() -> Node* { return new PolarCoordsNode(); });
    reg.registerType("LensFlareNode",      "风格化",    "镜头光晕",    "模拟镜头逆光光晕效果",      []() -> Node* { return new LensFlareNode(); });
    reg.registerType("MetalStyleNode",     "风格化",    "金属底板",    "金/银/青铜/古铜+凸版/凹版浮雕",[]() -> Node* { return new MetalStyleNode(); });
}
