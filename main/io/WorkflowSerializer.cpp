#include "WorkflowSerializer.h"
#include <QFile>
#include <QJsonDocument>

QJsonObject WorkflowSerializer::save(const WorkflowEngine &engine)
{
    QJsonObject root;
    root["version"] = "1.0";

    QJsonArray nodesArr;
    for (const auto *n : engine.allNodes()) {
        QJsonObject obj;
        obj["id"]    = n->id().toString(QUuid::WithoutBraces);
        obj["title"] = n->title();
        obj["type"]  = n->metaObject()->className(); // e.g. "ImageInputNode"

        // Save position if known — stored as node property
        QJsonObject pos;
        QVariant px = n->param("__posX");
        QVariant py = n->param("__posY");
        if (px.isValid()) pos["x"] = px.toDouble();
        if (py.isValid()) pos["y"] = py.toDouble();
        obj["position"] = pos;

        // Save user-facing params
        QJsonObject paramsObj;
        for (auto it = n->allParams().constBegin(); it != n->allParams().constEnd(); ++it) {
            if (it.key().startsWith("__")) continue; // internal
            paramsObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        obj["params"] = paramsObj;

        nodesArr.append(obj);
    }
    root["nodes"] = nodesArr;

    QJsonArray connsArr;
    for (const auto &c : engine.allConnections()) {
        QJsonObject obj;
        obj["sourceNodeId"] = c.sourceNodeId.toString(QUuid::WithoutBraces);
        obj["sourcePort"]   = c.sourcePort;
        obj["targetNodeId"] = c.targetNodeId.toString(QUuid::WithoutBraces);
        obj["targetPort"]   = c.targetPort;
        connsArr.append(obj);
    }
    root["connections"] = connsArr;

    return root;
}

bool WorkflowSerializer::saveToFile(const WorkflowEngine &engine,
                                    const QString &filePath, QString &errorMsg)
{
    QJsonObject root = save(engine);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        errorMsg = QString("Cannot write: %1").arg(file.errorString());
        return false;
    }
    QJsonDocument doc(root);
    if (file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
        errorMsg = QString("Write error: %1").arg(file.errorString());
        return false;
    }
    return true;
}

bool WorkflowSerializer::load(QJsonObject root, WorkflowEngine &engine, QString &errorMsg)
{
    engine.clear();

    // Read nodes
    QJsonArray nodesArr = root["nodes"].toArray();
    QMap<QString, QUuid> idMap; // string id -> QUuid

    for (const auto &val : nodesArr) {
        QJsonObject obj = val.toObject();
        QString typeName = obj["type"].toString();
        QString idStr    = obj["id"].toString();

        // Determine the type name for registry lookup
        // Strip "class " prefix if present
        QString lookupName = typeName;
        if (lookupName.startsWith("class "))
            lookupName = lookupName.mid(6);

        // Map class names to registry names (the registry uses simple names)
        // The registry uses simple type names, but metaObject()->className() returns
        // the fully-qualified name. We strip "class " and the namespace.
        // Since we registered with simple names like "ImageInputNode", just use that.
        if (lookupName.contains("::"))
            lookupName = lookupName.mid(lookupName.lastIndexOf("::") + 2);

        Node *n = NodeRegistry::instance().create(lookupName);
        if (!n) {
            // Try registered type name directly
            n = NodeRegistry::instance().create(typeName);
        }
        if (!n) {
            errorMsg = QString("Unknown node type: %1").arg(typeName);
            return false;
        }

        // Restore id
        QUuid uuid(idStr);
        if (!uuid.isNull())
            n->setId(uuid);

        // Restore title
        if (obj.contains("title"))
            n->setTitle(obj["title"].toString());

        // Restore position
        QJsonObject pos = obj["position"].toObject();
        if (pos.contains("x")) n->setParam("__posX", pos["x"].toDouble());
        if (pos.contains("y")) n->setParam("__posY", pos["y"].toDouble());

        // Restore params
        QJsonObject paramsObj = obj["params"].toObject();
        for (auto it = paramsObj.constBegin(); it != paramsObj.constEnd(); ++it)
            n->setParam(it.key(), it.value().toVariant());

        idMap[idStr] = n->id();
        engine.addNode(n);
    }

    // Read connections
    QJsonArray connsArr = root["connections"].toArray();
    for (const auto &val : connsArr) {
        QJsonObject obj = val.toObject();
        QString srcId = obj["sourceNodeId"].toString();
        int srcPort   = obj["sourcePort"].toInt();
        QString tgtId = obj["targetNodeId"].toString();
        int tgtPort   = obj["targetPort"].toInt();

        QUuid srcUuid = idMap.value(srcId);
        QUuid tgtUuid = idMap.value(tgtId);
        if (srcUuid.isNull() || tgtUuid.isNull()) {
            errorMsg = QString("Connection references unknown node: %1 -> %2")
                           .arg(srcId, tgtId);
            return false;
        }

        QString connErr;
        if (!engine.addConnection(srcUuid, srcPort, tgtUuid, tgtPort, connErr)) {
            errorMsg = QString("Cannot restore connection: %1").arg(connErr);
            return false;
        }
    }

    return true;
}

bool WorkflowSerializer::loadFromFile(const QString &filePath,
                                      WorkflowEngine &engine, QString &errorMsg)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMsg = QString("Cannot open: %1").arg(file.errorString());
        return false;
    }

    QJsonParseError parseErr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError) {
        errorMsg = QString("JSON parse error: %1").arg(parseErr.errorString());
        return false;
    }

    return load(doc.object(), engine, errorMsg);
}
