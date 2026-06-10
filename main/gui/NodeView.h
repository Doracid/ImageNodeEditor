#pragma once

#include <QGraphicsView>

class NodeScene;

class NodeView : public QGraphicsView {
    Q_OBJECT
public:
    explicit NodeView(NodeScene *scene, QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_panning = false;
    QPoint m_lastMousePos;
};
