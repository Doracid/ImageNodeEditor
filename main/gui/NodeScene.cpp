#include "NodeScene.h"
#include "NodeGraphicsItem.h"
#include "PortGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "core/NodeRegistry.h"
#include "io/WorkflowSerializer.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QGraphicsLineItem>
#include <cmath>

NodeScene::NodeScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-5000, -5000, 10000, 10000);
}

NodeScene::~NodeScene()
{
    // Items are owned by QGraphicsScene, m_engine owns nodes
}

// ---------------------------------------------------------------------------
// Node management
// ---------------------------------------------------------------------------
NodeGraphicsItem *NodeScene::addNodeToScene(Node *node, const QPointF &pos)
{
    saveSnapshot();
    m_engine.addNode(node);
    auto *item = new NodeGraphicsItem(node);
    addItem(item);
    item->setPos(pos.isNull() ? QPointF(0, 0) : pos);
    // Persist position onto the engine node for snapshot capture
    node->setParam("__posX", item->pos().x());
    node->setParam("__posY", item->pos().y());
    m_nodeItems[node->id()] = item;
    emit workflowModified();
    return item;
}

void NodeScene::removeNodeFromScene(const QUuid &nodeId, bool autoBypass)
{
    auto *item = m_nodeItems.value(nodeId);
    if (!item) return;

    // Record bypass connections before deletion
    struct Bypass {
        QUuid srcNodeId; int srcPort;
        QUuid tgtNodeId; int tgtPort;
    };
    QVector<Bypass> bypasses;

    if (autoBypass) {
        Node *node = item->node();
        // Skip multi-port nodes (ChannelSplit / ChannelMerge)
        if (node->category() != QStringLiteral("MultiPort")) {
            // Collect all incoming connections (source → this node)
            // and all outgoing connections (this node → target)
            QVector<Connection> conns = m_engine.connectionsForNode(nodeId);
            QVector<Connection> inConns, outConns;
            for (const auto &c : conns) {
                if (c.targetNodeId == nodeId) inConns.append(c);
                if (c.sourceNodeId == nodeId) outConns.append(c);
            }
            // Pair each input to an output by matching port index,
            // or pair first input to first output as fallback
            int nPair = qMin(inConns.size(), outConns.size());
            for (int i = 0; i < nPair; ++i) {
                // Find input/output with matching port index if possible
                const Connection *inC = &inConns[i];
                const Connection *outC = &outConns[i];
                for (int j = 0; j < outConns.size(); ++j) {
                    if (outConns[j].sourcePort == inConns[i].targetPort) {
                        outC = &outConns[j];
                        break;
                    }
                }
                if (inC->sourceNodeId == outC->targetNodeId) continue; // self-loop
                bypasses.append({ inC->sourceNodeId, inC->sourcePort,
                                  outC->targetNodeId, outC->targetPort });
            }
        }
    }

    saveSnapshot();

    // Remove connections
    QVector<ConnectionGraphicsItem*> toRemove;
    for (auto *conn : m_connectionItems) {
        if (conn->sourceNodeId() == nodeId || conn->targetNodeId() == nodeId)
            toRemove.append(conn);
    }
    for (auto *conn : toRemove)
        removeConnectionFromScene(conn);

    // Remove from engine (also deletes the node)
    m_engine.removeNode(nodeId);

    // Remove graphics
    m_nodeItems.remove(nodeId);
    removeItem(item);
    delete item;

    // Auto-bypass: connect upstream node directly to downstream node
    for (const auto &b : bypasses) {
        QString err;
        if (m_engine.canConnect(b.srcNodeId, b.srcPort,
                                b.tgtNodeId, b.tgtPort, err)) {
            addConnectionToScene(b.srcNodeId, b.srcPort,
                                 b.tgtNodeId, b.tgtPort);
        }
    }

    emit workflowModified();
}

NodeGraphicsItem *NodeScene::nodeGraphics(const QUuid &nodeId) const
{
    return m_nodeItems.value(nodeId, nullptr);
}

// ---------------------------------------------------------------------------
// Connections
// ---------------------------------------------------------------------------
ConnectionGraphicsItem *NodeScene::addConnectionToScene(const QUuid &src, int srcPort,
                                                         const QUuid &tgt, int tgtPort)
{
    saveSnapshot();
    auto *srcItem = nodeGraphics(src);
    auto *tgtItem = nodeGraphics(tgt);
    if (!srcItem || !tgtItem) return nullptr;

    auto *srcPortItem = srcItem->portItem(srcPort, PortDirection::Output);
    auto *tgtPortItem = tgtItem->portItem(tgtPort, PortDirection::Input);
    if (!srcPortItem || !tgtPortItem) return nullptr;

    QString err;
    if (!m_engine.addConnection(src, srcPort, tgt, tgtPort, err)) {
        emit connectionError(err);
        return nullptr;
    }

    auto *conn = new ConnectionGraphicsItem(srcPortItem, tgtPortItem);
    addItem(conn);
    m_connectionItems.append(conn);
    emit connectionAdded();
    emit workflowModified();
    return conn;
}

void NodeScene::removeConnectionFromScene(ConnectionGraphicsItem *conn)
{
    if (!conn) return;
    saveSnapshot();
    m_engine.removeConnection(conn->sourceNodeId(), conn->sourcePortIndex(),
                              conn->targetNodeId(), conn->targetPortIndex());
    m_connectionItems.removeOne(conn);
    removeItem(conn);
    delete conn;
    emit connectionRemoved();
    emit workflowModified();
}

// ---------------------------------------------------------------------------
// Drag-connection
// ---------------------------------------------------------------------------
void NodeScene::startConnection(PortGraphicsItem *source)
{
    if (!source || source->port().direction != PortDirection::Output) return;
    m_dragging = true;
    m_dragSource = source;
    m_dragLine = new QGraphicsLineItem();
    m_dragLine->setPen(QPen(QColor(100, 100, 100), 2, Qt::DashLine));
    m_dragLine->setLine(QLineF(source->centerScene(), source->centerScene()));
    m_dragLine->setZValue(10);
    addItem(m_dragLine);
}

void NodeScene::updateConnectionDrag(const QPointF &pos)
{
    if (!m_dragging || !m_dragLine || !m_dragSource) return;
    m_dragLine->setLine(QLineF(m_dragSource->centerScene(), pos));
}

void NodeScene::endConnection(PortGraphicsItem *target)
{
    if (!m_dragging) return;

    if (target && target != m_dragSource && target->port().direction == PortDirection::Input) {
        QString err;
        auto *srcItem = m_dragSource->nodeItem();
        auto *tgtItem = target->nodeItem();
        if (m_engine.canConnect(srcItem->nodeId(), m_dragSource->port().index,
                                tgtItem->nodeId(), target->port().index, err))
        {
            addConnectionToScene(srcItem->nodeId(), m_dragSource->port().index,
                                 tgtItem->nodeId(), target->port().index);
        } else {
            emit connectionError(err);
        }
    }

    // Clean up drag line
    if (m_dragLine) {
        removeItem(m_dragLine);
        delete m_dragLine;
        m_dragLine = nullptr;
    }
    m_dragging = false;
    m_dragSource = nullptr;
}

// ---------------------------------------------------------------------------
// Notifications
// ---------------------------------------------------------------------------
void NodeScene::nodeMoved(NodeGraphicsItem *item)
{
    // Update all connections attached to this node (called on every pixel drag)
    for (auto *conn : m_connectionItems) {
        if (conn->sourceNodeId() == item->nodeId() ||
            conn->targetNodeId() == item->nodeId())
            conn->updatePath();
    }
}

void NodeScene::nodeMoveFinished(NodeGraphicsItem *item)
{
    // Snapshot the state BEFORE the move so Ctrl+Z can restore the old position.
    saveSnapshot();
    // Persist final position after the snapshot was taken.
    if (Node *n = m_engine.node(item->nodeId())) {
        n->setParam("__posX", item->pos().x());
        n->setParam("__posY", item->pos().y());
    }
}

void NodeScene::nodeDoubleClicked(NodeGraphicsItem *item)
{
    emit showPreview(item->node());
}

// ---------------------------------------------------------------------------
// Clear
// ---------------------------------------------------------------------------
void NodeScene::clearAll()
{
    saveSnapshot();
    m_engine.clear(); // deletes all nodes
    for (auto *conn : m_connectionItems) {
        removeItem(conn);
        delete conn;
    }
    m_connectionItems.clear();
    for (auto *item : m_nodeItems) {
        removeItem(item);
        delete item; // node already deleted by engine.clear()
    }
    m_nodeItems.clear();
    emit workflowModified();
}

// ---------------------------------------------------------------------------
// Delete selected
// ---------------------------------------------------------------------------
void NodeScene::deleteSelected()
{
    if (selectedItems().isEmpty()) return;
    saveSnapshot();
    QList<QGraphicsItem*> selected = selectedItems();
    for (auto *item : selected) {
        if (auto *conn = qgraphicsitem_cast<ConnectionGraphicsItem*>(item))
            removeConnectionFromScene(conn);
        else if (auto *nodeItem = qgraphicsitem_cast<NodeGraphicsItem*>(item))
            removeNodeFromScene(nodeItem->nodeId(), true);
    }
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------
void NodeScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        deleteSelected();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape && m_dragging) {
        // Cancel connection drag
        if (m_dragLine) {
            removeItem(m_dragLine);
            delete m_dragLine;
            m_dragLine = nullptr;
        }
        m_dragging = false;
        m_dragSource = nullptr;
        event->accept();
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}

void NodeScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Safety: clear any stuck connection drag first
        if (m_dragging) {
            endConnection(nullptr);
        }

        // Check if we're starting a drag from an output port
        QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
        if (auto *portItem = qgraphicsitem_cast<PortGraphicsItem*>(item)) {
            // Defer node selection to avoid nested event processing from
            // property panel widget creation (can cause stuck drag state)
            Node *node = portItem->nodeItem()->node();
            if (portItem->port().direction == PortDirection::Output) {
                startConnection(portItem);
                QTimer::singleShot(0, this, [this, node]() { emit nodeSelected(node); });
                event->accept();
                return;
            }
            emit nodeSelected(node);
        } else if (auto *nodeItem = qgraphicsitem_cast<NodeGraphicsItem*>(item)) {
            emit nodeSelected(nodeItem->node());
        } else if (!item) {
            emit nodeDeselected();
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void NodeScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragging) {
        updateConnectionDrag(event->scenePos());
        event->accept();
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void NodeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragging) {
        // Search for target port within a 20px radius of the release point,
        // so small aiming errors still connect successfully.
        const qreal kSnapRadius = 20.0;
        QRectF searchRect(event->scenePos().x() - kSnapRadius,
                          event->scenePos().y() - kSnapRadius,
                          kSnapRadius * 2, kSnapRadius * 2);
        QList<QGraphicsItem*> items = this->items(searchRect);
        PortGraphicsItem *target = nullptr;
        qreal bestDist = kSnapRadius;
        for (auto *i : items) {
            if (auto *p = qgraphicsitem_cast<PortGraphicsItem*>(i)) {
                if (p->port().direction == PortDirection::Input) {
                    qreal d = QLineF(p->centerScene(), event->scenePos()).length();
                    if (d < bestDist) {
                        bestDist = d;
                        target = p;
                    }
                }
            }
        }
        endConnection(target);
        event->accept();
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

// ---------------------------------------------------------------------------
// Auto-connect
// ---------------------------------------------------------------------------
void NodeScene::autoConnect()
{
    if (m_nodeItems.size() < 2) return;

    // Collect and sort nodes by X position (left to right)
    QList<NodeGraphicsItem*> items = m_nodeItems.values();
    std::sort(items.begin(), items.end(),
              [](NodeGraphicsItem *a, NodeGraphicsItem *b) {
                  return a->pos().x() < b->pos().x();
              });

    struct AvailOut {
        QUuid nodeId;
        int   portIndex;
        DataType dataType;
    };
    QVector<AvailOut> available;

    auto removeFromAvailable = [&](const QUuid &nodeId, int portIndex) {
        for (int i = 0; i < available.size(); ++i) {
            if (available[i].nodeId == nodeId && available[i].portIndex == portIndex) {
                available.removeAt(i);
                return;
            }
        }
    };

    // --- Phase 1: Pre-populate pool with outputs from nodes already fully connected ---
    // Nodes with no inputs (source) or all inputs already satisfied by existing
    // connections contribute their outputs to the pool upfront.
    // This ensures that newly added nodes (e.g. ImageOutput at position 0,0)
    // can find their source even when sorted before the source in X order.
    for (auto *item : items) {
        QUuid nodeId = item->nodeId();
        Node *node = m_engine.node(nodeId);
        if (!node) continue;

        if (node->inputPorts().isEmpty()) {
            for (int pi = 0; pi < node->outputPorts().size(); ++pi)
                available.append({ nodeId, pi, node->outputPorts()[pi].dataType });
        } else {
            bool allFed = true;
            for (int pi = 0; pi < node->inputPorts().size(); ++pi) {
                if (m_engine.inputConnectionFor(nodeId, pi).sourceNodeId.isNull()) {
                    allFed = false;
                    break;
                }
            }
            if (allFed) {
                for (int pi = 0; pi < node->outputPorts().size(); ++pi)
                    available.append({ nodeId, pi, node->outputPorts()[pi].dataType });
            }
        }
    }

    // --- Phase 2: Iterative left-to-right pass ---
    // Multiple passes ensure that nodes whose source appears later in X order
    // will still get connected on a subsequent iteration.
    int connected = 0;
    bool changed;
    do {
        changed = false;
        for (auto *item : items) {
            const QUuid nodeId = item->nodeId();
            Node *node = m_engine.node(nodeId);
            if (!node) continue;

            // Check if this node has any unconnected input ports still
            bool hasUnconnected = false;
            for (int pi = 0; pi < node->inputPorts().size(); ++pi) {
                if (m_engine.inputConnectionFor(nodeId, pi).sourceNodeId.isNull()) {
                    hasUnconnected = true;
                    break;
                }
            }
            if (!hasUnconnected) continue;

            // Try to satisfy each unconnected input port from the available pool
            int newlyFed = 0;
            for (int pi = 0; pi < node->inputPorts().size(); ++pi) {
                if (!m_engine.inputConnectionFor(nodeId, pi).sourceNodeId.isNull())
                    continue;

                const Port &inPort = node->inputPorts()[pi];
                // Try each compatible candidate in the pool until canConnect
                // succeeds. The first candidate may fail because its source port
                // is already connected (single-connection restriction), so fall
                // through to the next candidate.
                for (int ai = 0; ai < available.size(); ++ai) {
                    if (!isTypeCompatible(available[ai].dataType, inPort.dataType))
                        continue;

                    const auto &avail = available[ai];
                    QString err;
                    if (m_engine.canConnect(avail.nodeId, avail.portIndex,
                                             nodeId, pi, err)) {
                        addConnectionToScene(avail.nodeId, avail.portIndex,
                                             nodeId, pi);
                        connected++;
                        newlyFed++;
                        removeFromAvailable(avail.nodeId, avail.portIndex);
                        break;
                    }
                }
            }

            if (newlyFed > 0) {
                changed = true;
                // Add this node's outputs to the pool for downstream nodes
                for (int pi = 0; pi < node->outputPorts().size(); ++pi)
                    available.append({ nodeId, pi, node->outputPorts()[pi].dataType });
            }
        }
    } while (changed);

    if (connected > 0) {
        emit workflowModified();
    }
}

// ---------------------------------------------------------------------------
// Replace node
// ---------------------------------------------------------------------------
void NodeScene::startNodeReplace(const QUuid &nodeId)
{
    emit replaceNodeRequested(nodeId);
}

bool NodeScene::replaceNode(const QUuid &oldNodeId, const QString &newTypeName, QString &errorMsg)
{
    auto *oldGraphics = nodeGraphics(oldNodeId);
    if (!oldGraphics) { errorMsg = "找不到原节点。"; return false; }

    QPointF oldPos = oldGraphics->pos();
    QVector<Connection> oldConns = m_engine.connectionsForNode(oldNodeId);

    saveSnapshot();

    // Quick port-count pre-check
    Node *tempCheck = NodeRegistry::instance().create(newTypeName);
    if (!tempCheck) { errorMsg = "未知节点类型: " + newTypeName; return false; }

    int maxIn = -1, maxOut = -1;
    for (const auto &c : oldConns) {
        if (c.targetNodeId == oldNodeId) maxIn = qMax(maxIn, c.targetPort);
        if (c.sourceNodeId == oldNodeId) maxOut = qMax(maxOut, c.sourcePort);
    }

    bool portErr = false;
    if (maxIn >= 0 && maxIn >= tempCheck->inputPorts().size()) portErr = true;
    if (maxOut >= 0 && maxOut >= tempCheck->outputPorts().size()) portErr = true;
    delete tempCheck;
    if (portErr) {
        errorMsg = "新节点的端口数不足以保留原有连线。操作已取消。";
        return false;
    }

    // Clone old node for undo
    Node *oldNode = oldGraphics->node();
    Node *oldClone = oldNode->clone();
    oldClone->setId(oldNode->id());
    for (auto it = oldNode->allParams().constBegin(); it != oldNode->allParams().constEnd(); ++it)
        oldClone->setParam(it.key(), it.value());

    // Remove old node (engine deletes it, scene removes graphics + connections)
    removeNodeFromScene(oldNodeId);

    // Create and add new node at same position
    Node *newNode = NodeRegistry::instance().create(newTypeName);
    if (!newNode) {
        delete oldClone;
        errorMsg = "创建新节点失败。";
        return false;
    }
    addNodeToScene(newNode, oldPos);
    QUuid newId = newNode->id();

    // Try reconnecting each saved connection
    QVector<Connection> restored;
    bool allOk = true;
    for (const auto &c : oldConns) {
        Connection nc = c;
        if (nc.sourceNodeId == oldNodeId) nc.sourceNodeId = newId;
        if (nc.targetNodeId == oldNodeId) nc.targetNodeId = newId;

        QString connErr;
        if (m_engine.canConnect(nc.sourceNodeId, nc.sourcePort,
                                nc.targetNodeId, nc.targetPort, connErr))
        {
            auto *connItem = addConnectionToScene(nc.sourceNodeId, nc.sourcePort,
                                                  nc.targetNodeId, nc.targetPort);
            if (connItem) {
                restored.append(nc);
                continue;
            }
        }
        // Connection failed — rollback
        allOk = false;
        break;
    }

    if (!allOk) {
        // Remove new node and any connections already added
        removeNodeFromScene(newId);

        // Restore old node from clone
        m_engine.addNode(oldClone);
        auto *newGraphics = new NodeGraphicsItem(oldClone);
        addItem(newGraphics);
        newGraphics->setPos(oldPos);
        m_nodeItems[oldClone->id()] = newGraphics;

        // Restore old connections
        for (const auto &c : oldConns) {
            addConnectionToScene(c.sourceNodeId, c.sourcePort,
                                 c.targetNodeId, c.targetPort);
        }

        errorMsg = "端口类型不兼容，无法保留全部连线。已撤销替换操作。";
        emit workflowModified();
        return false;
    }

    // Success: new node is in place, old clone is not needed
    delete oldClone;
    emit workflowModified();
    return true;
}

// ---------------------------------------------------------------------------
// Undo support
// ---------------------------------------------------------------------------
void NodeScene::saveSnapshot()
{
    if (m_suppressSnapshot) return;
    QJsonObject state = WorkflowSerializer::save(m_engine);
    // Remove positions beyond undo limit
    while (m_undoStack.size() >= kMaxUndo)
        m_undoStack.removeFirst();
    m_undoStack.append(state);
}

void NodeScene::undo()
{
    if (m_undoStack.isEmpty()) return;
    QJsonObject state = m_undoStack.takeLast();

    // Remember selection
    QList<QUuid> selectedIds;
    for (auto *item : selectedItems()) {
        if (auto *n = qgraphicsitem_cast<NodeGraphicsItem*>(item))
            selectedIds.append(n->nodeId());
    }

    // Suppress snapshots during restore so addNodeToScene/addConnectionToScene
    // don't push new states onto the undo stack (which would create an infinite loop).
    m_suppressSnapshot = true;

    // Clear current scene
    for (auto *conn : m_connectionItems) {
        removeItem(conn);
        delete conn;
    }
    m_connectionItems.clear();
    for (auto *item : m_nodeItems) {
        removeItem(item);
        delete item;
    }
    m_nodeItems.clear();
    m_engine.clear();

    // Restore from snapshot
    QString err;
    WorkflowEngine tempEngine;
    if (!WorkflowSerializer::load(state, tempEngine, err)) {
        m_suppressSnapshot = false;
        return;
    }

    // Rebuild graphics items
    for (auto *node : tempEngine.allNodes()) {
        QPointF pos;
        double x = node->param("__posX", -10000).toDouble();
        double y = node->param("__posY", -10000).toDouble();
        if (x > -9999) pos.setX(x);
        if (y > -9999) pos.setY(y);

        QUuid oldId = node->id();
        Node *clone = node->clone();
        clone->setId(oldId);
        for (auto it = node->allParams().constBegin(); it != node->allParams().constEnd(); ++it)
            clone->setParam(it.key(), it.value());

        auto *gi = addNodeToScene(clone, pos);
        if (selectedIds.contains(oldId) && gi)
            gi->setSelected(true);
    }

    for (const auto &c : tempEngine.allConnections())
        addConnectionToScene(c.sourceNodeId, c.sourcePort, c.targetNodeId, c.targetPort);

    m_suppressSnapshot = false;
    emit workflowModified();
}

void NodeScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Grid
    painter->setPen(QPen(QColor(200, 200, 200), 0.5));
    int gridSize = 20;
    double left   = std::floor(rect.left() / gridSize) * gridSize;
    double top    = std::floor(rect.top() / gridSize) * gridSize;
    for (double x = left; x < rect.right(); x += gridSize)
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    for (double y = top; y < rect.bottom(); y += gridSize)
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
}
