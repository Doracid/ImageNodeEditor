#include "NodeGraphicsItem.h"
#include "PortGraphicsItem.h"
#include "NodeScene.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QStyleOptionGraphicsItem>
#include <QFontMetrics>
#include <QMenu>
#include <QAction>
#include <QObject>
#include <cmath>

static QColor categoryColor(const QString &cat)
{
    if (cat == "Input")                return QColor(76, 175, 80);   // green
    if (cat == "Output")               return QColor(244, 67, 54);   // red
    if (cat == "Filter" || cat == "滤波")  return QColor(33, 150, 243);  // blue
    if (cat == "Conversion" || cat == "转换") return QColor(255, 152, 0); // orange
    if (cat == "MultiPort")            return QColor(156, 39, 176);  // purple
    if (cat == "Stylize" || cat == "风格化")  return QColor(233, 30, 99);  // pink
    if (cat == "色彩调整")             return QColor(255, 193, 7);   // amber/gold
    if (cat == "几何变换")             return QColor(0, 150, 136);   // teal
    return QColor(158, 158, 158); // gray
}

NodeGraphicsItem::NodeGraphicsItem(Node *node, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_node(node)
{
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setZValue(1);

    // Create port visuals
    int nIn = m_node->inputPorts().size();
    int nOut = m_node->outputPorts().size();

    for (int i = 0; i < nIn; ++i) {
        auto *p = new PortGraphicsItem(m_node->inputPorts()[i], this);
        m_inputPorts.append(p);
    }
    for (int i = 0; i < nOut; ++i) {
        auto *p = new PortGraphicsItem(m_node->outputPorts()[i], this);
        m_outputPorts.append(p);
    }

    updateLayout();
}

void NodeGraphicsItem::updateLayout()
{
    int nPorts = std::max(m_inputPorts.size(), m_outputPorts.size());
    double h = kTitleH + nPorts * kPortH + kMargin;

    // Position body
    m_rect = QRectF(0, 0, kWidth, h);

    // Position input ports (left side)
    double y = kTitleH + kPortH / 2;
    for (auto *p : m_inputPorts) {
        p->setPos(0, y);
        y += kPortH;
    }

    // Position output ports (right side)
    y = kTitleH + kPortH / 2;
    for (auto *p : m_outputPorts) {
        p->setPos(kWidth, y);
        y += kPortH;
    }
}

QRectF NodeGraphicsItem::boundingRect() const
{
    return m_rect.adjusted(-2, -2, 4, 4);
}

void NodeGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // Shadow
    QRectF shadowRect = m_rect.translated(3, 3);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 40));
    painter->drawRoundedRect(shadowRect, kRadius, kRadius);

    // Body
    QColor bg = isSelected() ? QColor(255, 255, 220) : QColor(240, 240, 240);
    painter->setBrush(bg);
    painter->setPen(QPen(Qt::black, 1.5));
    painter->drawRoundedRect(m_rect, kRadius, kRadius);

    // Title bar
    QRectF titleRect = m_rect.adjusted(0, 0, 0, -m_rect.height() + kTitleH);
    QColor catColor = categoryColor(m_node->category());
    painter->setBrush(catColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(titleRect, kRadius, kRadius);
    // Square off bottom corners of title
    painter->drawRect(titleRect.adjusted(0, kRadius * 0.5, 0, 0));

    // Title text
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(10);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(titleRect.adjusted(6, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft,
                      m_node->title());

    // Port labels
    font.setPointSize(8);
    font.setBold(false);
    painter->setFont(font);
    painter->setPen(Qt::black);

    double y = kTitleH;
    for (auto *p : m_inputPorts) {
        QRectF labelRect(16, y, kWidth - 24, kPortH);
        painter->drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft,
                          p->port().name);
        y += kPortH;
    }
    y = kTitleH;
    for (auto *p : m_outputPorts) {
        QRectF labelRect(0, y, kWidth - 16, kPortH);
        painter->drawText(labelRect, Qt::AlignVCenter | Qt::AlignRight,
                          p->port().name);
        y += kPortH;
    }
}

QVariant NodeGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged) {
        // Update connections
        NodeScene *s = qobject_cast<NodeScene*>(scene());
        if (s) s->nodeMoved(this);
    }
    return QGraphicsItem::itemChange(change, value);
}

void NodeGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    NodeScene *s = qobject_cast<NodeScene*>(scene());
    if (s) s->nodeDoubleClicked(this);
}

void NodeGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;
    QAction *replaceAction = menu.addAction("替换节点");
    QObject::connect(replaceAction, &QAction::triggered, [this]() {
        NodeScene *s = qobject_cast<NodeScene*>(scene());
        if (s) s->startNodeReplace(nodeId());
    });
    menu.addSeparator();
    QAction *deleteAction = menu.addAction("删除节点");
    QObject::connect(deleteAction, &QAction::triggered, [this]() {
        NodeScene *s = qobject_cast<NodeScene*>(scene());
        if (s) s->deleteNode(nodeId());
    });
    menu.exec(event->screenPos());
}

QPointF NodeGraphicsItem::portScenePos(int portIndex, PortDirection dir) const
{
    if (dir == PortDirection::Input) {
        if (portIndex >= 0 && portIndex < m_inputPorts.size())
            return m_inputPorts[portIndex]->centerScene();
    } else {
        if (portIndex >= 0 && portIndex < m_outputPorts.size())
            return m_outputPorts[portIndex]->centerScene();
    }
    return scenePos();
}

PortGraphicsItem *NodeGraphicsItem::portItem(int portIndex, PortDirection dir) const
{
    if (dir == PortDirection::Input) {
        return (portIndex >= 0 && portIndex < m_inputPorts.size()) ? m_inputPorts[portIndex] : nullptr;
    }
    return (portIndex >= 0 && portIndex < m_outputPorts.size()) ? m_outputPorts[portIndex] : nullptr;
}
