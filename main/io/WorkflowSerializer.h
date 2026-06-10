#pragma once

#include "engine/WorkflowEngine.h"
#include "core/NodeRegistry.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class WorkflowSerializer {
public:
    // Save workflow to JSON byte array
    static QJsonObject save(const WorkflowEngine &engine);
    static bool saveToFile(const WorkflowEngine &engine, const QString &filePath, QString &errorMsg);

    // Load workflow from JSON
    static bool load(QJsonObject obj, WorkflowEngine &engine, QString &errorMsg);
    static bool loadFromFile(const QString &filePath, WorkflowEngine &engine, QString &errorMsg);
};
