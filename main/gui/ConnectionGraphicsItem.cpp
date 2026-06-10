#include "ConnectionGraphicsItem.h"
#include "PortGraphicsItem.h"
#include "NodeGraphicsItem.h"
#include "NodeScene.h"
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QObject>
#include <QGraphicsSceneContextMenuEvent>
#include <cmath>

ConnectionGraphicsItem::ConnectionGraphicsItem(PortGraphicsItem *source,
                                               PortGraphicsItem *target,
                                               QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
    , m_source(source)
    , m_target(target)
{
    setPen(QPen(Qt::black, 2.0));
    setBrush(Qt::NoBrush);
    setFlag(ItemIsSelectable);
    setZValue(0);
    updatePath();
}

QUuid ConnectionGraphicsItem::sourceNodeId() const
{
    return m_source->nodeItem()->nodeId();
}

QUuid ConnectionGraphicsItem::targetNodeId() const
{
    return m_target->nodeItem()->nodeId();
}

int ConnectionGraphicsItem::sourcePortIndex() const
{
    return m_source->port().index;
}

int ConnectionGraphicsItem::targetPortIndex() const
{
    return m_target->port().index;
}

void ConnectionGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *deleteAction = menu.addAction("删除连线");
    QObject::connect(deleteAction, &QAction::triggered, [this]() {
        NodeScene *s = qobject_cast<NodeScene*>(scene());
        if (s) s->removeConnectionFromScene(this);
    });
    menu.exec(event->screenPos());
}

void ConnectionGraphicsItem::updatePath()
{
    QPointF p1 = m_source->centerScene();
    QPointF p2 = m_target->centerScene();
    double dx = std::abs(p2.x() - p1.x()) * 0.5;
    if (dx < 50) dx = 50;

    QPainterPath p;
    p.moveTo(p1);
    p.cubicTo(p1 + QPointF(dx, 0), p2 - QPointF(dx, 0), p2);
    setPath(p);
}
