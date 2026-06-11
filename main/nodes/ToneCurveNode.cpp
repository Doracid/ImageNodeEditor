#include "ToneCurveNode.h"
#include "algorithms/ImageAlgorithm.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFont>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QTimer>
#include <algorithm>

// ====================================================================
// Smooth Catmull-Rom curve evaluation
// ====================================================================
static double evalSmooth(double x, const QVector<QPointF> &pts)
{
    int n = pts.size();
    if (n < 2) return x;
    if (x <= pts[0].x()) return pts[0].y();
    if (x >= pts[n - 1].x()) return pts[n - 1].y();

    int seg = 0;
    for (int i = 0; i < n - 1; ++i) {
        if (x >= pts[i].x() && x <= pts[i + 1].x()) { seg = i; break; }
    }

    QPointF p0 = pts[qMax(0, seg - 1)];
    QPointF p1 = pts[seg];
    QPointF p2 = pts[seg + 1];
    QPointF p3 = pts[qMin(n - 1, seg + 2)];

    double t = (x - p1.x()) / (p2.x() - p1.x());
    double t2 = t * t, t3 = t2 * t;

    return 0.5 * ((2.0 * p1.y()) + (-p0.y() + p2.y()) * t
        + (2.0 * p0.y() - 5.0 * p1.y() + 4.0 * p2.y() - p3.y()) * t2
        + (-p0.y() + 3.0 * p1.y() - 3.0 * p2.y() + p3.y()) * t3);
}

// ====================================================================
// LUT builder (256-level, smooth Catmull-Rom interpolation)
// ====================================================================
static QVector<int> buildLUT(const QVector<QPointF> &points)
{
    QVector<int> lut(256);
    if (points.size() < 2) {
        for (int i = 0; i < 256; ++i) lut[i] = i;
        return lut;
    }

    QVector<QPointF> pts = points;
    std::sort(pts.begin(), pts.end(),
              [](const QPointF &a, const QPointF &b) { return a.x() < b.x(); });
    if (pts.first().x() > 0.0) pts.prepend(QPointF(0.0, 0.0));
    if (pts.last().x() < 1.0) pts.append(QPointF(1.0, 1.0));

    for (int i = 0; i < 256; ++i) {
        double x = i / 255.0;
        lut[i] = qBound(0, (int)std::round(evalSmooth(x, pts) * 255.0), 255);
    }
    return lut;
}

// Combine two LUTs: out[i] = overlay[base[i]]
static QVector<int> combineLUTs(const QVector<int> &base, const QVector<int> &overlay)
{
    QVector<int> result(256);
    for (int i = 0; i < 256; ++i)
        result[i] = overlay[base[i]];
    return result;
}

// ============================================================
// CurveEditor
// ============================================================
CurveEditor::CurveEditor(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(180, 150);
    // Default: straight diagonal line (no curve adjustment)
    m_points = { QPointF(0.0, 0.0), QPointF(1.0, 1.0) };
}

void CurveEditor::setPoints(const QVector<QPointF> &pts)
{
    m_points = pts;
    sortPoints();
    update();
}

// -------------------------------------------------------------------
// Paint
// -------------------------------------------------------------------
void CurveEditor::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const double w = width();
    const double h = height();
    const double drawW = w - 2 * kMargin;
    const double drawH = h - 2 * kMargin;

    if (drawW <= 0 || drawH <= 0) return;

    // Background
    p.fillRect(rect(), QColor(40, 40, 42));
    QRectF gridRect(kMargin, kMargin, drawW, drawH);
    p.fillRect(gridRect, QColor(52, 52, 55));

    // Grid (4x4)
    p.setPen(QPen(QColor(68, 68, 72), 0.5));
    for (int i = 0; i <= 4; ++i) {
        double t = kMargin + i * drawW / 4.0;
        p.drawLine(QPointF(t, kMargin), QPointF(t, kMargin + drawH));
        t = kMargin + i * drawH / 4.0;
        p.drawLine(QPointF(kMargin, t), QPointF(kMargin + drawW, t));
    }

    // Diagonal reference
    p.setPen(QPen(QColor(90, 90, 95), 1.0));
    p.drawLine(normToWidget(QPointF(0, 0)), normToWidget(QPointF(1, 1)));

    // Curve (smooth Catmull-Rom style via cubic bezier)
    if (m_points.size() >= 2) {
        QPainterPath curvePath;
        const int n = m_points.size();
        if (n == 2) {
            // Straight line for exactly 2 points
            curvePath.moveTo(normToWidget(m_points[0]));
            curvePath.lineTo(normToWidget(m_points[1]));
        } else {
            curvePath.moveTo(normToWidget(m_points[0]));
            for (int i = 0; i < n - 1; ++i) {
                QPointF p0 = m_points[qMax(0, i - 1)];
                QPointF p1 = m_points[i];
                QPointF p2 = m_points[i + 1];
                QPointF p3 = m_points[qMin(n - 1, i + 2)];

                QPointF cp1 = p1 + (p2 - p0) / 6.0;
                QPointF cp2 = p2 - (p3 - p1) / 6.0;

                curvePath.cubicTo(normToWidget(cp1), normToWidget(cp2), normToWidget(p2));
            }
        }

        p.setPen(QPen(QColor(255, 170, 50), 2.5));
        p.setBrush(Qt::NoBrush);
        p.drawPath(curvePath);
    }

    // Control points
    for (int i = 0; i < m_points.size(); ++i) {
        QPointF pt = normToWidget(m_points[i]);
        bool active = (i == m_dragIndex);

        if (active) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 170, 50, 50));
            p.drawEllipse(pt, kDotRad + 5, kDotRad + 5);
        }

        p.setPen(QPen(active ? QColor(255, 200, 100) : QColor(200, 200, 200), 1.5));
        p.setBrush(active ? QColor(255, 170, 50) : QColor(130, 130, 138));
        p.drawEllipse(pt, kDotRad, kDotRad);
    }
}

// -------------------------------------------------------------------
// Mouse
// -------------------------------------------------------------------
void CurveEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        int idx = hitTest(QPointF(event->pos()));
        if (idx >= 0 && m_points.size() > 2) {
            removePoint(idx);
            emit pointsChanged();
            update();
        }
        return;
    }

    if (event->button() == Qt::LeftButton) {
        int idx = hitTest(QPointF(event->pos()));
        if (idx >= 0) {
            m_dragIndex = idx;
            QPointF wp = normToWidget(m_points[idx]);
            m_dragOffset = QPointF(event->pos().x() - wp.x(),
                                   event->pos().y() - wp.y());
        } else {
            QPointF np = widgetToNorm(QPointF(event->pos()));
            if (np.x() >= 0.0 && np.x() <= 1.0 &&
                np.y() >= 0.0 && np.y() <= 1.0) {
                addPoint(np);
                for (int i = 0; i < m_points.size(); ++i) {
                    if (qAbs(m_points[i].x() - np.x()) < 0.001 &&
                        qAbs(m_points[i].y() - np.y()) < 0.001) {
                        m_dragIndex = i;
                        break;
                    }
                }
                QPointF wp = normToWidget(m_points[m_dragIndex]);
                m_dragOffset = QPointF(event->pos().x() - wp.x(),
                                       event->pos().y() - wp.y());
                emit pointsChanged(); // only on add, not during drag
            }
        }
        update();
    }
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragIndex >= 0) {
        QPointF np = widgetToNorm(QPointF(event->pos()) - m_dragOffset);
        m_points[m_dragIndex].setY(qBound(0.0, np.y(), 1.0));

        double minX = (m_dragIndex > 0)
            ? m_points[m_dragIndex - 1].x() + 0.002 : 0.0;
        double maxX = (m_dragIndex < m_points.size() - 1)
            ? m_points[m_dragIndex + 1].x() - 0.002 : 1.0;
        m_points[m_dragIndex].setX(qBound(minX, np.x(), maxX));

        update(); // visual update only, no signal during drag
    }
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragIndex >= 0) {
        m_dragIndex = -1;
        emit pointsChanged(); // signal only on release
        update();
    }
}

void CurveEditor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

int CurveEditor::heightForWidth(int w) const
{
    // 宽:高 = 2:3 → height = w * 3/2
    return qMax(120, (int)(w * 1.5));
}

// -------------------------------------------------------------------
// Coordinate helpers
// -------------------------------------------------------------------
QPointF CurveEditor::widgetToNorm(const QPointF &wp) const
{
    double w = width();
    double h = height();
    double drawW = w - 2 * kMargin;
    double drawH = h - 2 * kMargin;
    if (drawW <= 0 || drawH <= 0) return QPointF(0, 0);

    return QPointF((wp.x() - kMargin) / drawW,
                   1.0 - (wp.y() - kMargin) / drawH);
}

QPointF CurveEditor::normToWidget(const QPointF &np) const
{
    double w = width();
    double h = height();
    double drawW = w - 2 * kMargin;
    double drawH = h - 2 * kMargin;

    return QPointF(kMargin + np.x() * drawW,
                   (h - kMargin) - np.y() * drawH);
}

int CurveEditor::hitTest(const QPointF &wp) const
{
    for (int i = 0; i < m_points.size(); ++i) {
        QPointF pt = normToWidget(m_points[i]);
        double dx = wp.x() - pt.x();
        double dy = wp.y() - pt.y();
        if (dx * dx + dy * dy <= kHitRad * kHitRad)
            return i;
    }
    return -1;
}

void CurveEditor::addPoint(const QPointF &np)
{
    m_points.append(np);
    sortPoints();
}

void CurveEditor::removePoint(int index)
{
    if (index >= 0 && index < m_points.size())
        m_points.removeAt(index);
}

void CurveEditor::sortPoints()
{
    std::sort(m_points.begin(), m_points.end(),
              [](const QPointF &a, const QPointF &b) {
                  return a.x() < b.x();
              });
}

// ============================================================
// ToneCurveNode
// ============================================================
static const char *kChannelKeys[] = { "curve_W", "curve_R", "curve_G", "curve_B" };
static const char *kChannelNames[] = { "白 (RGB)", "红 (R)", "绿 (G)", "蓝 (B)" };

ToneCurveNode::ToneCurveNode()
    : Node("色调曲线")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    // Each channel curve defaults to straight diagonal (identity)
    for (int i = 0; i < 4; ++i)
        m_params[QString(kChannelKeys[i])] = QString("0,0;1,1");
}

QVector<QPointF> ToneCurveNode::parsePoints(const QString &str) const
{
    QStringList parts = str.split(';', Qt::SkipEmptyParts);
    QVector<QPointF> pts;
    for (const auto &part : parts) {
        QStringList xy = part.split(',');
        if (xy.size() == 2) {
            bool xOk, yOk;
            double x = xy[0].toDouble(&xOk);
            double y = xy[1].toDouble(&yOk);
            if (xOk && yOk)
                pts.append(QPointF(qBound(0.0, x, 1.0), qBound(0.0, y, 1.0)));
        }
    }
    return pts;
}

bool ToneCurveNode::process(const QVector<DataPacket> &inputs,
                            QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }

    // Parse all 4 curves
    auto ptsW = parsePoints(m_params["curve_W"].toString());
    auto ptsR = parsePoints(m_params["curve_R"].toString());
    auto ptsG = parsePoints(m_params["curve_G"].toString());
    auto ptsB = parsePoints(m_params["curve_B"].toString());

    // Build LUTs
    auto lutW = buildLUT(ptsW);
    auto lutR = buildLUT(ptsR);
    auto lutG = buildLUT(ptsG);
    auto lutB = buildLUT(ptsB);

    // Combine white + per-channel: final[i] = channelLUT[whiteLUT[i]]
    auto finalR = combineLUTs(lutW, lutR);
    auto finalG = combineLUTs(lutW, lutG);
    auto finalB = combineLUTs(lutW, lutB);

    // Apply to image
    QImage src = inputs[0].image();
    if (src.isNull()) { errorMsg = "输入图像无效。"; return false; }
    QImage dst = src.convertToFormat(QImage::Format_ARGB32);

    for (int y = 0; y < dst.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(dst.scanLine(y));
        for (int x = 0; x < dst.width(); ++x) {
            QRgb px = line[x];
            line[x] = qRgba(finalR[qRed(px)], finalG[qGreen(px)],
                            finalB[qBlue(px)], qAlpha(px));
        }
    }

    outputs[0] = DataPacket(dst);
    return true;
}

QWidget *ToneCurveNode::createParamWidget()
{
    auto *container = new QWidget();
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);

    // Channel selector
    auto *combo = new QComboBox();
    for (int i = 0; i < 4; ++i)
        combo->addItem(kChannelNames[i]);
    combo->setCurrentIndex(m_currentChannel);
    layout->addWidget(combo);

    // Reset button
    auto *resetBtn = new QPushButton("恢复初始");
    resetBtn->setFixedHeight(24);
    resetBtn->setStyleSheet("QPushButton { font-size: 12px; }");
    layout->addWidget(resetBtn);

    // Curve editor
    auto *editor = new CurveEditor();
    editor->setMinimumWidth(180);
    layout->addWidget(editor);

    // Load current channel's curve into editor
    editor->setPoints(parsePoints(m_params[QString(kChannelKeys[m_currentChannel])].toString()));

    // Reset button: restore straight diagonal
    connect(resetBtn, &QPushButton::clicked, this, [this, editor]() {
        editor->setPoints({ QPointF(0.0, 0.0), QPointF(1.0, 1.0) });
        auto pts = editor->points();
        QStringList parts;
        for (const auto &pt : pts)
            parts.append(QString("%1,%2").arg(pt.x(), 0, 'f', 6).arg(pt.y(), 0, 'f', 6));
        m_params[QString(kChannelKeys[m_currentChannel])] = parts.join(';');
        QTimer::singleShot(0, this, [this]() { emit paramsChanged(); });
    });

    // Channel switch: save current, load new
    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, editor](int idx) {
                // Save current curve from editor to params
                auto pts = editor->points();
                QStringList parts;
                for (const auto &pt : pts)
                    parts.append(QString("%1,%2").arg(pt.x(), 0, 'f', 6).arg(pt.y(), 0, 'f', 6));
                m_params[QString(kChannelKeys[m_currentChannel])] = parts.join(';');

                // Switch channel
                m_currentChannel = idx;

                // Load new curve into editor
                editor->setPoints(parsePoints(m_params[QString(kChannelKeys[idx])].toString()));
            });

    // Editor change → save param (deferred to avoid nested event processing)
    connect(editor, &CurveEditor::pointsChanged, this, [this, editor]() {
        auto pts = editor->points();
        QStringList parts;
        for (const auto &pt : pts)
            parts.append(QString("%1,%2").arg(pt.x(), 0, 'f', 6).arg(pt.y(), 0, 'f', 6));
        m_params[QString(kChannelKeys[m_currentChannel])] = parts.join(';');
        // Defer paramChanged signal to avoid layout re-entrancy
        QTimer::singleShot(0, this, [this]() { emit paramsChanged(); });
    });

    return container;
}
