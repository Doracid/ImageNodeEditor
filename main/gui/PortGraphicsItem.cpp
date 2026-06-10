#include "PortGraphicsItem.h"
#include "NodeGraphicsItem.h"
#include "NodeScene.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QCursor>

PortGraphicsItem::PortGraphicsItem(const Port &port, NodeGraphicsItem *parent)
    : QGraphicsEllipseItem(-6, -6, 12, 12, parent)
    , m_port(port)
    , m_nodeItem(parent)
{
    setBrush(colorForType(port.dataType));
    setPen(QPen(Qt::black, 1.5));
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable);
    setCursor(Qt::CrossCursor);
    setZValue(2);
}

QPointF PortGraphicsItem::centerScene() const
{
    return mapToScene(0, 0);
}

QColor PortGraphicsItem::colorForType(DataType type)
{
    switch (type) {
        case DataType::ColorImage: return QColor(66, 133, 244);   // blue
        case DataType::GrayImage:  return QColor(128, 128, 128);  // gray
        case DataType::Any:        return QColor(255, 255, 255);  // white
    }
    return Qt::white;
}

void PortGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    setScale(1.5);
    update();
}

void PortGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    setScale(1.0);
    update();
}

void PortGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_port.direction == PortDirection::Output) {
        NodeScene *s = qobject_cast<NodeScene*>(scene());
        if (s) s->startConnection(this);
        event->accept();
        return;
    }
    QGraphicsEllipseItem::mousePressEvent(event);
}

// mouseMoveEvent and mouseReleaseEvent handled by NodeScene
