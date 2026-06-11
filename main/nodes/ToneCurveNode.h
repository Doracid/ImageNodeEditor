#pragma once

#include "core/Node.h"
#include <QWidget>
#include <QPointF>
#include <QVector>

// ============================================================
// CurveEditor — interactive tone curve widget
// ============================================================
class CurveEditor : public QWidget {
    Q_OBJECT
public:
    explicit CurveEditor(QWidget *parent = nullptr);

    void setPoints(const QVector<QPointF> &pts);
    QVector<QPointF> points() const { return m_points; }

signals:
    void pointsChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int w) const override;

private:
    QPointF widgetToNorm(const QPointF &wp) const;
    QPointF normToWidget(const QPointF &np) const;
    int hitTest(const QPointF &wp) const;
    void addPoint(const QPointF &np);
    void removePoint(int index);
    void sortPoints();

    QVector<QPointF> m_points;
    int m_dragIndex = -1;
    QPointF m_dragOffset;

    static constexpr double kMargin  = 20;
    static constexpr double kDotRad  = 6;
    static constexpr double kHitRad  = 10;
};

// ============================================================
// ToneCurveNode — 白/R/G/B 四通道独立曲线
// ============================================================
class ToneCurveNode : public Node {
    Q_OBJECT
public:
    ToneCurveNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "色调曲线（白/R/G/B 四通道独立曲线）。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    QWidget *createParamWidget() override;
    Node *clone() const override { return new ToneCurveNode(); }

private:
    QVector<QPointF> parsePoints(const QString &str) const;
    int m_currentChannel = 0;
};
