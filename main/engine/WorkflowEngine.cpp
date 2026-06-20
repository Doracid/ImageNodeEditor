#include "WorkflowEngine.h"
#include "core/DataType.h"
#include <QQueue>
#include <QHash>

// ---------------------------------------------------------------------------
// Node management
// ---------------------------------------------------------------------------
WorkflowEngine::~WorkflowEngine()
{
    qDeleteAll(m_nodes);
    m_nodes.clear();
}

void WorkflowEngine::addNode(Node *node)
{
    if (node) m_nodes[node->id()] = node;
}

void WorkflowEngine::removeNode(const QUuid &nodeId)
{
    delete m_nodes.take(nodeId);
    // also remove all connections involving this node
    m_connections.erase(
        std::remove_if(m_connections.begin(), m_connections.end(),
            [&](const Connection &c) {
                return c.sourceNodeId == nodeId || c.targetNodeId == nodeId;
            }),
        m_connections.end());
}

Node *WorkflowEngine::node(const QUuid &nodeId) const
{
    return m_nodes.value(nodeId, nullptr);
}

QVector<Connection> WorkflowEngine::connectionsForNode(const QUuid &nodeId) const
{
    QVector<Connection> result;
    for (const auto &c : m_connections) {
        if (c.sourceNodeId == nodeId || c.targetNodeId == nodeId)
            result.append(c);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Connection management
// ---------------------------------------------------------------------------
bool WorkflowEngine::canConnect(const QUuid &sourceNode, int sourcePort,
                                const QUuid &targetNode, int targetPort,
                                QString &errorMsg) const
{
    auto *src = m_nodes.value(sourceNode, nullptr);
    auto *tgt = m_nodes.value(targetNode, nullptr);
    if (!src || !tgt) {
        errorMsg = QStringLiteral("Node not found.");
        return false;
    }
    if (sourceNode == targetNode) {
        errorMsg = QStringLiteral("Cannot connect a node to itself.");
        return false;
    }
    if (sourcePort < 0 || sourcePort >= src->outputPorts().size()) {
        errorMsg = QStringLiteral("Source port index out of range.");
        return false;
    }
    if (targetPort < 0 || targetPort >= tgt->inputPorts().size()) {
        errorMsg = QStringLiteral("Target port index out of range.");
        return false;
    }

    const Port &srcPort = src->outputPorts()[sourcePort];
    const Port &tgtPort = tgt->inputPorts()[targetPort];

    if (!isTypeCompatible(srcPort.dataType, tgtPort.dataType)) {
        errorMsg = QString("Type mismatch: %1 → %2")
                       .arg(dataTypeName(srcPort.dataType),
                            dataTypeName(tgtPort.dataType));
        return false;
    }

    // Check if target port already has an incoming connection
    for (const auto &c : m_connections) {
        if (c.targetNodeId == targetNode && c.targetPort == targetPort) {
            errorMsg = QStringLiteral("Target port already has a connection.");
            return false;
        }
    }

    // Check if source port already has an outgoing connection
    for (const auto &c : m_connections) {
        if (c.sourceNodeId == sourceNode && c.sourcePort == sourcePort) {
            errorMsg = QStringLiteral("Source port already has a connection.");
            return false;
        }
    }

    return true;
}

bool WorkflowEngine::addConnection(const QUuid &sourceNode, int sourcePort,
                                   const QUuid &targetNode, int targetPort,
                                   QString &errorMsg)
{
    if (!canConnect(sourceNode, sourcePort, targetNode, targetPort, errorMsg))
        return false;

    Connection c;
    c.sourceNodeId = sourceNode;
    c.sourcePort   = sourcePort;
    c.targetNodeId = targetNode;
    c.targetPort   = targetPort;
    m_connections.append(c);
    return true;
}

void WorkflowEngine::removeConnection(const QUuid &sourceNode, int sourcePort,
                                      const QUuid &targetNode, int targetPort)
{
    m_connections.erase(
        std::remove_if(m_connections.begin(), m_connections.end(),
            [&](const Connection &c) {
                return c.sourceNodeId == sourceNode &&
                       c.sourcePort == sourcePort &&
                       c.targetNodeId == targetNode &&
                       c.targetPort == targetPort;
            }),
        m_connections.end());
}

Connection WorkflowEngine::inputConnectionFor(const QUuid &nodeId, int portIndex) const
{
    for (const auto &c : m_connections) {
        if (c.targetNodeId == nodeId && c.targetPort == portIndex)
            return c;
    }
    return {};
}

// ---------------------------------------------------------------------------
// DAG validation & topological sort (Kahn's algorithm)
// ---------------------------------------------------------------------------
bool WorkflowEngine::validateDAG(QString &errorMsg) const
{
    QVector<QUuid> sorted = topologicalSort(errorMsg);
    return sorted.size() == m_nodes.size();
}

QVector<QUuid> WorkflowEngine::topologicalSort(QString &errorMsg) const
{
    // in-degree count
    QHash<QUuid, int> inDegree;
    for (const auto &id : m_nodes.keys())
        inDegree[id] = 0;

    for (const auto &c : m_connections)
        inDegree[c.targetNodeId]++;

    QQueue<QUuid> queue;
    for (auto it = inDegree.constBegin(); it != inDegree.constEnd(); ++it)
        if (it.value() == 0)
            queue.enqueue(it.key());

    QVector<QUuid> result;
    while (!queue.isEmpty()) {
        QUuid id = queue.dequeue();
        result.append(id);
        for (const auto &c : m_connections) {
            if (c.sourceNodeId == id) {
                inDegree[c.targetNodeId]--;
                if (inDegree[c.targetNodeId] == 0)
                    queue.enqueue(c.targetNodeId);
            }
        }
    }

    if (result.size() != m_nodes.size()) {
        QSet<QUuid> missing(m_nodes.keys().begin(), m_nodes.keys().end());
        for (const auto &visited : result)
            missing.remove(visited);
        errorMsg = QString("Cycle detected! %1 node(s) could not be ordered.")
                       .arg(missing.size());
    }

    return result;
}

// ---------------------------------------------------------------------------
// Execution
// ---------------------------------------------------------------------------
bool WorkflowEngine::execute(ResultMap &results, QString &errorMsg)
{
    results.clear();

    if (m_connections.isEmpty()) {
        errorMsg = "工作流中没有连线。";
        return false;
    }

    // 1. Compute indegree (incoming) and outdegree (outgoing) per node
    QHash<QUuid, int> inDeg, outDeg;
    for (const auto &c : m_connections) {
        outDeg[c.sourceNodeId]++;
        inDeg[c.targetNodeId]++;
        // ensure keys exist
        if (!inDeg.contains(c.sourceNodeId))    inDeg[c.sourceNodeId] = 0;
        if (!outDeg.contains(c.targetNodeId))   outDeg[c.targetNodeId] = 0;
    }

    // 2. Find start nodes (no input, has output) and end nodes (no output, has input)
    QSet<QUuid> starts, ends;
    for (auto it = inDeg.constBegin(); it != inDeg.constEnd(); ++it) {
        if (it.value() == 0 && outDeg.value(it.key(), 0) > 0)
            starts.insert(it.key());
    }
    for (auto it = outDeg.constBegin(); it != outDeg.constEnd(); ++it) {
        if (it.value() == 0 && inDeg.value(it.key(), 0) > 0)
            ends.insert(it.key());
    }

    // 3. Build adjacency map: sourceNodeId -> list of targetNodeIds
    QHash<QUuid, QVector<QUuid>> fwdEdges; // forward: source -> targets
    QHash<QUuid, QVector<QUuid>> revEdges; // reverse: target -> sources
    for (const auto &c : m_connections) {
        fwdEdges[c.sourceNodeId].append(c.targetNodeId);
        revEdges[c.targetNodeId].append(c.sourceNodeId);
    }

    // 4. BFS forward from all start nodes
    QSet<QUuid> fwdReachable;
    {
        QQueue<QUuid> q;
        for (const auto &s : starts) { q.enqueue(s); fwdReachable.insert(s); }
        while (!q.isEmpty()) {
            QUuid id = q.dequeue();
            for (const auto &next : fwdEdges.value(id)) {
                if (!fwdReachable.contains(next)) {
                    fwdReachable.insert(next);
                    q.enqueue(next);
                }
            }
        }
    }

    // 5. BFS backward from all end nodes
    QSet<QUuid> bwdReachable;
    {
        QQueue<QUuid> q;
        for (const auto &e : ends) { q.enqueue(e); bwdReachable.insert(e); }
        while (!q.isEmpty()) {
            QUuid id = q.dequeue();
            for (const auto &prev : revEdges.value(id)) {
                if (!bwdReachable.contains(prev)) {
                    bwdReachable.insert(prev);
                    q.enqueue(prev);
                }
            }
        }
    }

    // 6. Active nodes = forward ∩ backward reachable
    QSet<QUuid> active = fwdReachable;
    active.intersect(bwdReachable);

    if (active.isEmpty()) {
        errorMsg = "没有完整的输入→输出链路。";
        return false;
    }

    // 7. Topological sort and execute only active nodes
    QVector<QUuid> order = topologicalSort(errorMsg);
    for (const auto &id : active) {
        if (!order.contains(id)) {
            errorMsg = "工作流中存在环。";
            return false;
        }
    }

    for (const QUuid &nodeId : order) {
        if (!active.contains(nodeId)) continue; // skip nodes not on a full path
        Node *n = m_nodes[nodeId];
        if (!n) continue;

        // Collect inputs from upstream connections
        QVector<DataPacket> inputs(n->inputPorts().size());
        for (int i = 0; i < n->inputPorts().size(); ++i) {
            Connection inConn = inputConnectionFor(nodeId, i);
            if (inConn.sourceNodeId.isNull()) continue;
            const auto &srcOutputs = results[inConn.sourceNodeId];
            if (inConn.sourcePort < srcOutputs.size())
                inputs[i] = srcOutputs[inConn.sourcePort];
        }

        QString valError;
        if (!n->validate(valError)) {
            errorMsg = QString("[%1] Validation failed: %2").arg(n->title(), valError);
            return false;
        }

        QVector<DataPacket> outputs(n->outputPorts().size());
        if (!n->process(inputs, outputs, errorMsg)) {
            errorMsg = QString("[%1] %2").arg(n->title(), errorMsg);
            return false;
        }

        results[nodeId] = outputs;
    }

    return true;
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------
void WorkflowEngine::clear()
{
    qDeleteAll(m_nodes);
    m_nodes.clear();
    m_connections.clear();
}
