#pragma once

#include <QGraphicsPathItem>
#include <QUuid>

class PortGraphicsItem;

class ConnectionGraphicsItem : public QGraphicsPathItem {
public:
    ConnectionGraphicsItem(PortGraphicsItem *source, PortGraphicsItem *target,
                           QGraphicsItem *parent = nullptr);

    PortGraphicsItem *sourcePort() const { return m_source; }
    PortGraphicsItem *targetPort() const { return m_target; }

    QUuid sourceNodeId() const;
    QUuid targetNodeId() const;
    int sourcePortIndex() const;
    int targetPortIndex() const;

    void updatePath();

    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    PortGraphicsItem *m_source;
    PortGraphicsItem *m_target;
};
