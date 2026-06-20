#pragma once

#include "engine/WorkflowEngine.h"
#include "core/Node.h"
#include <QGraphicsScene>
#include <QMap>
#include <QUuid>
#include <QJsonObject>
#include <QVector>

class NodeGraphicsItem;
class PortGraphicsItem;
class ConnectionGraphicsItem;

class NodeScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit NodeScene(QObject *parent = nullptr);
    ~NodeScene() override;

    // Workflow engine access
    WorkflowEngine *engine() { return &m_engine; }
    const WorkflowEngine *engine() const { return &m_engine; }

    // Node graphics management
    NodeGraphicsItem *addNodeToScene(Node *node, const QPointF &pos = {});
    void removeNodeFromScene(const QUuid &nodeId, bool autoBypass = false);
    NodeGraphicsItem *nodeGraphics(const QUuid &nodeId) const;
    QList<NodeGraphicsItem*> allNodeGraphics() const { return m_nodeItems.values(); }

    // Connection management
    ConnectionGraphicsItem *addConnectionToScene(const QUuid &src, int srcPort,
                                                  const QUuid &tgt, int tgtPort);
    void removeConnectionFromScene(ConnectionGraphicsItem *conn);

    // Drag-connection state
    void startConnection(PortGraphicsItem *source);
    void updateConnectionDrag(const QPointF &pos);
    void endConnection(PortGraphicsItem *target);
    bool isDraggingConnection() const { return m_dragging; }

    // Notifications from items
    void nodeMoved(NodeGraphicsItem *item);
    void nodeMoveFinished(NodeGraphicsItem *item);
    void nodeDoubleClicked(NodeGraphicsItem *item);

    // Delete a specific node by id
    void deleteNode(const QUuid &nodeId) { removeNodeFromScene(nodeId, true); }

    // Clear all
    void clearAll();

    // Delete currently selected items
    void deleteSelected();

    // Auto-connect all nodes left-to-right
    void autoConnect();

    // Replace node (keep connections if port types are compatible)
    bool replaceNode(const QUuid &oldNodeId, const QString &newTypeName, QString &errorMsg);

signals:
    void nodeSelected(Node *node);
    void nodeDeselected();
    void connectionAdded();
    void connectionRemoved();
    void workflowModified();
    void showPreview(Node *node);
    void connectionError(const QString &msg);
    void replaceNodeRequested(QUuid nodeId);

public slots:
    void startNodeReplace(const QUuid &nodeId);
    void undo();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    WorkflowEngine m_engine;
    QMap<QUuid, NodeGraphicsItem*> m_nodeItems;
    QVector<ConnectionGraphicsItem*> m_connectionItems;

    // Drag-connection state
    bool m_dragging = false;
    PortGraphicsItem *m_dragSource = nullptr;
    QGraphicsLineItem *m_dragLine = nullptr;

    // Undo stack
    QVector<QJsonObject> m_undoStack;
    static constexpr int kMaxUndo = 50;
    void saveSnapshot();
    bool m_suppressSnapshot = false;
};
