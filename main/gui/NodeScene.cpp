#include "NodeScene.h"
#include "NodeGraphicsItem.h"
#include "PortGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
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
    QGraphicsScene::keyPressEvent(event);
}

void NodeScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if we're starting a drag from an output port
        QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
        if (auto *portItem = qgraphicsitem_cast<PortGraphicsItem*>(item)) {
            emit nodeSelected(portItem->nodeItem()->node());
            if (portItem->port().direction == PortDirection::Output) {
                startConnection(portItem);
                event->accept();
                return;
            }
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
