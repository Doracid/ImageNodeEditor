#include "NodeRegistry.h"
#include <QSet>

NodeRegistry &NodeRegistry::instance()
{
    static NodeRegistry reg;
    return reg;
}

void NodeRegistry::registerType(const QString &typeName, const QString &category,
                                const QString &displayName, const QString &description,
                                Creator creator)
{
    m_records[typeName] = { category, displayName, description, std::move(creator) };
}

Node *NodeRegistry::create(const QString &typeName) const
{
    auto it = m_records.constFind(typeName);
    if (it == m_records.constEnd()) return nullptr;
    return (it->creator)();
}

QStringList NodeRegistry::allTypeNames() const
{
    return QStringList(m_records.keys());
}

QVector<QPair<QString, QVector<NodeRegistry::Entry>>> NodeRegistry::categorizedEntries() const
{
    // Desired display order (most important first)
    static const QStringList order = {
        "输入", "输出", "滤波", "风格化", "色彩调整", "几何变换", "转换", "多端口"
    };

    // Group by category
    QMap<QString, QVector<Entry>> grouped;
    for (auto it = m_records.constBegin(); it != m_records.constEnd(); ++it)
        grouped[it->category].append({ it.key(), it->displayName, it->description });

    // Produce ordered result
    QVector<QPair<QString, QVector<Entry>>> result;
    QSet<QString> seen;
    for (const QString &cat : order) {
        auto mapIt = grouped.constFind(cat);
        if (mapIt != grouped.constEnd()) {
            result.append({cat, mapIt.value()});
            seen.insert(cat);
        }
    }
    // Append any categories not in the predefined order
    for (auto it = grouped.constBegin(); it != grouped.constEnd(); ++it) {
        if (!seen.contains(it.key()))
            result.append({it.key(), it.value()});
    }
    return result;
}
