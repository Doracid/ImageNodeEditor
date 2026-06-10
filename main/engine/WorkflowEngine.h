#pragma once

#include "core/Node.h"
#include "DataPacket.h"
#include <QMap>
#include <QSet>
#include <QVector>
#include <QPair>

struct Connection {
    QUuid sourceNodeId;
    int   sourcePort;
    QUuid targetNodeId;
    int   targetPort;
};

class WorkflowEngine {
public:
    WorkflowEngine() = default;
    ~WorkflowEngine(); // deletes all owned nodes

    // Node management
    void addNode(Node *node);
    void removeNode(const QUuid &nodeId);
    Node *node(const QUuid &nodeId) const;
    QList<Node*> allNodes() const { return m_nodes.values(); }

    // Connection management
    bool canConnect(const QUuid &sourceNode, int sourcePort,
                    const QUuid &targetNode, int targetPort,
                    QString &errorMsg) const;
    bool addConnection(const QUuid &sourceNode, int sourcePort,
                       const QUuid &targetNode, int targetPort,
                       QString &errorMsg);
    void removeConnection(const QUuid &sourceNode, int sourcePort,
                          const QUuid &targetNode, int targetPort);
    QVector<Connection> allConnections() const { return m_connections; }
    QVector<Connection> connectionsForNode(const QUuid &nodeId) const;

    // DAG check & topological sort using Kahn's algorithm
    bool validateDAG(QString &errorMsg) const;
    QVector<QUuid> topologicalSort(QString &errorMsg) const;

    // Find input connections for a specific port
    Connection inputConnectionFor(const QUuid &nodeId, int portIndex) const;

    // Execution
    using ResultMap = QMap<QUuid, QVector<DataPacket>>; // nodeId -> outputs
    bool execute(ResultMap &results, QString &errorMsg);

    // Clear all
    void clear();

private:
    QMap<QUuid, Node*> m_nodes;          // owns the nodes
    QVector<Connection> m_connections;
};
