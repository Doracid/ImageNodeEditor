#pragma once

#include "Node.h"
#include <QMap>
#include <functional>

// Singleton factory — register a creator function for each node type name.
class NodeRegistry {
public:
    using Creator = std::function<Node*()>;

    static NodeRegistry &instance();

    void registerType(const QString &typeName, const QString &category,
                      const QString &displayName, const QString &description,
                      Creator creator);

    Node *create(const QString &typeName) const;
    QStringList allTypeNames() const;

    // Returns category -> list of (typeName, displayName, description)
    struct Entry {
        QString typeName;
        QString displayName;
        QString description;
    };
    // Ordered by priority (important categories first)
    QVector<QPair<QString, QVector<Entry>>> categorizedEntries() const;

private:
    NodeRegistry() = default;

    struct Record {
        QString category;
        QString displayName;
        QString description;
        Creator creator;
    };
    QMap<QString, Record> m_records;
};
