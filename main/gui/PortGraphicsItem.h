#pragma once

#include "core/Port.h"
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QBrush>

class NodeGraphicsItem;

class PortGraphicsItem : public QGraphicsEllipseItem {
public:
    PortGraphicsItem(const Port &port, NodeGraphicsItem *parent);

    const Port &port() const { return m_port; }
    NodeGraphicsItem *nodeItem() const { return m_nodeItem; }

    // Scene-coordinate center of this port (for connections)
    QPointF centerScene() const;

    static QColor colorForType(DataType type);

    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Port m_port;
    NodeGraphicsItem *m_nodeItem;
};
