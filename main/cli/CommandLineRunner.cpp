#include "CommandLineRunner.h"
#include "io/WorkflowSerializer.h"
#include "engine/WorkflowEngine.h"
#include <cstring>

static int loadAndExecute(const QString &workflowPath,
                          const QString &imagePath,
                          const QString &outputPath,
                          QString *errorMsg)
{
    auto err = [&](const QString &m) { if (errorMsg) *errorMsg = m; };

    WorkflowEngine engine;
    QString loadErr;
    if (!WorkflowSerializer::loadFromFile(workflowPath, engine, loadErr)) {
        err("加载工作流失败: " + loadErr);
        return 1;
    }

    if (engine.allNodes().isEmpty()) {
        err("工作流为空。");
        return 1;
    }

    // Override ImageInputNode filePath if --image is given
    if (!imagePath.isEmpty()) {
        for (auto *n : engine.allNodes()) {
            if (std::strcmp(n->metaObject()->className(), "ImageInputNode") == 0) {
                n->setParam("filePath", imagePath);
                printf("  输入: %s\n", qPrintable(imagePath));
                break;
            }
        }
    }

    // Override ImageOutputNode filePath if --output is given
    if (!outputPath.isEmpty()) {
        for (auto *n : engine.allNodes()) {
            if (std::strcmp(n->metaObject()->className(), "ImageOutputNode") == 0) {
                n->setParam("filePath", outputPath);
                break;
            }
        }
    }

    // Execute
    WorkflowEngine::ResultMap results;
    QString execErr;
    if (!engine.execute(results, execErr)) {
        err("执行失败: " + execErr);
        return 1;
    }

    // Print output summary
    for (auto *node : engine.allNodes()) {
        auto it = results.constFind(node->id());
        if (it != results.constEnd()) {
            for (int i = 0; i < it->size(); ++i) {
                const DataPacket &dp = (*it)[i];
                if (dp.isValid()) {
                    if (dp.type() == DataPacket::Image)
                        printf("  [%s] 输出[%d]: %dx%d 图像\n",
                               qPrintable(node->title()), i,
                               dp.image().width(), dp.image().height());
                    else
                        printf("  [%s] 输出[%d]: %zu 张图像\n",
                               qPrintable(node->title()), i,
                               (size_t)dp.imageList().size());
                }
            }
        }
    }

    printf("工作流执行成功。\n");
    if (!outputPath.isEmpty())
        printf("  输出: %s\n", qPrintable(outputPath));
    return 0;
}

int CommandLineRunner::run(const QString &workflowPath,
                           const QString &imagePath,
                           const QString &outputPath,
                           QString *errorMsg)
{
    return loadAndExecute(workflowPath, imagePath, outputPath, errorMsg);
}
