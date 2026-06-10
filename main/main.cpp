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

void registerAllNodeTypes()
{
    auto &reg = NodeRegistry::instance();

    reg.registerType("ImageInputNode",    "输入",       "图像输入",    "从文件加载图像",           []() -> Node* { return new ImageInputNode(); });
    reg.registerType("ImageOutputNode",   "输出",      "图像输出",   "保存图像到文件",             []() -> Node* { return new ImageOutputNode(); });
    reg.registerType("ImageViewerNode",   "输出",      "图像查看器",   "在窗口中预览图像",        []() -> Node* { return new ImageViewerNode(); });
    reg.registerType("CropNode",          "滤波",      "裁剪",           "裁剪图像区域",              []() -> Node* { return new CropNode(); });
    reg.registerType("ResizeNode",        "滤波",      "缩放",         "调整图像尺寸",        []() -> Node* { return new ResizeNode(); });
    reg.registerType("BrightnessContrastNode", "滤波", "亮度/对比度","调整亮度与对比度",   []() -> Node* { return new BrightnessContrastNode(); });
    reg.registerType("GaussianBlurNode",  "滤波",      "高斯模糊",  "应用高斯模糊",            []() -> Node* { return new GaussianBlurNode(); });
    reg.registerType("GrayscaleNode",     "转换",  "灰度化",      "转换为灰度图像",           []() -> Node* { return new GrayscaleNode(); });
    reg.registerType("EdgeDetectionNode", "滤波",      "边缘检测", "Sobel 边缘检测",           []() -> Node* { return new EdgeDetectionNode(); });
    reg.registerType("ChannelSplitNode",  "多端口",   "通道分离",  "分离为 R/G/B 通道",      []() -> Node* { return new ChannelSplitNode(); });
    reg.registerType("ChannelMergeNode",  "多端口",   "通道合并",  "合并 R/G/B 为彩色图像",         []() -> Node* { return new ChannelMergeNode(); });
    reg.registerType("WatermarkNode",     "滤波",      "水印",      "添加文字水印",             []() -> Node* { return new WatermarkNode(); });
    reg.registerType("RotateNode",        "滤波",      "旋转",         "按角度旋转图像",          []() -> Node* { return new RotateNode(); });
    reg.registerType("InvertNode",        "滤波",      "反色",         "反转图像颜色",            []() -> Node* { return new InvertNode(); });
}
