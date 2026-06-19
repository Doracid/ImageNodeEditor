#pragma once

#include "core/Node.h"
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QVector>
#include <QMap>
#include <QGraphicsSceneContextMenuEvent>

class PortGraphicsItem;

class NodeGraphicsItem : public QGraphicsItem {
public:
    explicit NodeGraphicsItem(Node *node, QGraphicsItem *parent = nullptr);

    Node *node() const { return m_node; }
    QUuid nodeId() const { return m_node->id(); }

    // Port items
    PortGraphicsItem *portItem(int portIndex, PortDirection dir) const;
    QVector<PortGraphicsItem*> inputPortItems() const { return m_inputPorts; }
    QVector<PortGraphicsItem*> outputPortItems() const { return m_outputPorts; }

    // Dimensions
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void updateLayout();

    enum { Type = QGraphicsItem::UserType + 1 };
    int type() const override { return Type; }

    QPointF portScenePos(int portIndex, PortDirection dir) const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Node *m_node;
    QVector<PortGraphicsItem*> m_inputPorts;
    QVector<PortGraphicsItem*> m_outputPorts;
    QRectF m_rect;
    QPointF m_dragStartPos;

    static constexpr double kWidth  = 160;
    static constexpr double kTitleH = 28;
    static constexpr double kPortH  = 22;
    static constexpr double kMargin = 8;
    static constexpr double kRadius = 8;
};
