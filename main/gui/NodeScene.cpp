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
    m_nodeItems[node->id()] = item;
    emit workflowModified();
    return item;
}

void NodeScene::removeNodeFromScene(const QUuid &nodeId)
{
    saveSnapshot();
    auto *item = m_nodeItems.value(nodeId);
    if (!item) return;

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
    // Update all connections attached to this node
    for (auto *conn : m_connectionItems) {
        if (conn->sourceNodeId() == item->nodeId() ||
            conn->targetNodeId() == item->nodeId())
            conn->updatePath();
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
            removeNodeFromScene(nodeItem->nodeId());
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
        QList<QGraphicsItem*> items = this->items(event->scenePos());
        PortGraphicsItem *target = nullptr;
        for (auto *i : items) {
            if (auto *p = qgraphicsitem_cast<PortGraphicsItem*>(i)) {
                if (p->port().direction == PortDirection::Input) {
                    target = p;
                    break;
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
    saveSnapshot();

    // Sort nodes by X position (left to right)
    QList<NodeGraphicsItem*> items = m_nodeItems.values();
    std::sort(items.begin(), items.end(),
              [](NodeGraphicsItem *a, NodeGraphicsItem *b) {
                  return a->pos().x() < b->pos().x();
              });

    int connected = 0;
    for (int i = 0; i < items.size() - 1; ++i) {
        NodeGraphicsItem *left = items[i];
        NodeGraphicsItem *right = items[i + 1];

        // Try every output port on left with every input port on right
        bool found = false;
        for (auto *outPort : left->outputPortItems()) {
            if (found) break;
            for (auto *inPort : right->inputPortItems()) {
                if (found) break;
                // Skip if already connected
                if (!m_engine.inputConnectionFor(right->nodeId(), inPort->port().index).sourceNodeId.isNull())
                    continue;

                QString err;
                if (m_engine.canConnect(left->nodeId(), outPort->port().index,
                                         right->nodeId(), inPort->port().index, err)) {
                    addConnectionToScene(left->nodeId(), outPort->port().index,
                                          right->nodeId(), inPort->port().index);
                    connected++;
                    found = true;
                }
            }
        }
    }

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

    // Clear current scene (remove all items first, then clear engine)
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
        // If restore fails, undo stack is already popped — nothing more we can do
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
