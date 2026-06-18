#include "GradientMapNode.h"
#include "algorithms/ImageAlgorithm.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QComboBox>
#include <QLabel>

// ====================================================================
// GradientBar — interactive gradient editor widget
// ====================================================================
class GradientBar : public QWidget {
    Q_OBJECT
public:
    GradientBar(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(200, 40);
        setMouseTracking(true);
        // Default: black → white
        m_stops = { {0.0, QColor(0,0,0)}, {1.0, QColor(255,255,255)} };
    }

    void setStops(const QVector<QPair<double, QColor>> &stops) {
        m_stops = stops;
        if (m_stops.size() < 2) {
            m_stops = { {0.0, QColor(0,0,0)}, {1.0, QColor(255,255,255)} };
        }
        m_dragIdx = -1;
        update();
    }

    QVector<QPair<double, QColor>> stops() const { return m_stops; }

signals:
    void stopsChanged();

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        int w = width(), h = height();
        if (w < 10) return;

        // Draw gradient
        for (int x = 0; x < w; ++x) {
            double t = x / (double)(w - 1);
            QColor c = colorAt(t);
            p.setPen(c);
            p.drawLine(x, 4, x, h - 16);
        }

        // Border
        p.setPen(QColor(100, 100, 100));
        p.drawRect(0, 4, w - 1, h - 20);

        // Draw stops as triangles
        for (int i = 0; i < m_stops.size(); ++i) {
            int sx = (int)(m_stops[i].first * (w - 1));
            bool active = (i == m_dragIdx || i == m_highlightIdx);
            QColor fill = active ? QColor(255, 200, 100) : QColor(200, 200, 200);
            QColor border = active ? QColor(255, 170, 50) : QColor(120, 120, 120);

            // Triangle pointer
            QPainterPath tri;
            tri.moveTo(sx, h - 14);
            tri.lineTo(sx - 5, h - 3);
            tri.lineTo(sx + 5, h - 3);
            tri.closeSubpath();
            p.setPen(QPen(border, 1));
            p.setBrush(fill);
            p.drawPath(tri);

            // Color dot on gradient
            p.setPen(Qt::NoPen);
            p.setBrush(m_stops[i].second);
            p.drawEllipse(QPointF(sx, h - 20), 5, 5);
        }

        // Position label on hover
        if (m_highlightIdx >= 0 && m_highlightIdx < m_stops.size()) {
            QString label = QString("(%1, %2)")
                .arg(m_stops[m_highlightIdx].second.red())
                .arg(m_stops[m_highlightIdx].second.green())
                .arg(m_stops[m_highlightIdx].second.blue());
            p.setPen(QColor(200, 200, 200));
            p.drawText(rect().adjusted(4, 0, -4, -2), Qt::AlignBottom | Qt::AlignRight,
                       QString("R%1 G%2 B%3")
                           .arg(m_stops[m_highlightIdx].second.red(), 3)
                           .arg(m_stops[m_highlightIdx].second.green(), 3)
                           .arg(m_stops[m_highlightIdx].second.blue(), 3));
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            int idx = hitTest(event->pos());
            if (idx >= 0) {
                m_dragIdx = idx;
                m_dragOffset = event->pos().x() - posToWidget(m_stops[idx].first);
            } else {
                // Add new stop
                double pos = widgetToPos(event->pos().x());
                if (pos >= 0.01 && pos <= 0.99) {
                    QColor col = colorAt(pos);
                    m_stops.append({pos, col});
                    sortStops();
                    m_dragIdx = -1;
                    for (int i = 0; i < m_stops.size(); ++i) {
                        if (qAbs(m_stops[i].first - pos) < 0.001) {
                            m_dragIdx = i;
                            break;
                        }
                    }
                    emit stopsChanged();
                    update();
                }
            }
        } else if (event->button() == Qt::RightButton) {
            int idx = hitTest(event->pos());
            if (idx >= 0 && m_stops.size() > 2) {
                m_stops.removeAt(idx);
                m_dragIdx = -1;
                m_highlightIdx = -1;
                emit stopsChanged();
                update();
            }
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (m_dragIdx >= 0) {
            double pos = widgetToPos(event->pos().x() - m_dragOffset);
            pos = qBound(0.0, pos, 1.0);
            // Constrain between neighbors
            if (m_dragIdx > 0)
                pos = qMax(pos, m_stops[m_dragIdx - 1].first + 0.005);
            if (m_dragIdx < m_stops.size() - 1)
                pos = qMin(pos, m_stops[m_dragIdx + 1].first - 0.005);
            m_stops[m_dragIdx].first = pos;
            update();
        } else {
            int idx = hitTest(event->pos());
            if (idx != m_highlightIdx) {
                m_highlightIdx = idx;
                update();
            }
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && m_dragIdx >= 0) {
            m_dragIdx = -1;
            emit stopsChanged();
            update();
        }
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            int idx = hitTest(event->pos());
            if (idx >= 0) {
                QColor cur = m_stops[idx].second;
                QColor c = QColorDialog::getColor(cur, this, "选择颜色");
                if (c.isValid()) {
                    m_stops[idx].second = c;
                    emit stopsChanged();
                    update();
                }
            }
        }
    }

private:
    double widgetToPos(int x) const {
        return qBound(0.0, x / (double)(width() - 1), 1.0);
    }

    int posToWidget(double pos) const {
        return (int)(pos * (width() - 1));
    }

    int hitTest(const QPoint &wp) const {
        int w = width();
        for (int i = 0; i < m_stops.size(); ++i) {
            int sx = (int)(m_stops[i].first * (w - 1));
            if (qAbs(wp.x() - sx) < 8 && wp.y() > 4 && wp.y() < height() - 3)
                return i;
        }
        return -1;
    }

    QColor colorAt(double t) const {
        if (m_stops.isEmpty()) return Qt::black;
        if (t <= m_stops[0].first) return m_stops[0].second;
        if (t >= m_stops.last().first) return m_stops.last().second;
        for (int i = 0; i < m_stops.size() - 1; ++i) {
            if (t >= m_stops[i].first && t <= m_stops[i + 1].first) {
                double local = (t - m_stops[i].first)
                             / (m_stops[i + 1].first - m_stops[i].first);
                const QColor &a = m_stops[i].second;
                const QColor &b = m_stops[i + 1].second;
                return QColor(
                    (int)(a.red()   * (1 - local) + b.red()   * local),
                    (int)(a.green() * (1 - local) + b.green() * local),
                    (int)(a.blue()  * (1 - local) + b.blue()  * local));
            }
        }
        return m_stops.last().second;
    }

    void sortStops() {
        std::sort(m_stops.begin(), m_stops.end(),
                  [](const QPair<double, QColor> &a, const QPair<double, QColor> &b) {
                      return a.first < b.first;
                  });
    }

    QVector<QPair<double, QColor>> m_stops;
    int m_dragIdx = -1;
    int m_highlightIdx = -1;
    int m_dragOffset = 0;
};

// ====================================================================
// GradientMapNode
// ====================================================================
static const char *kDefaultStops = "0,0,0,0;1,255,255,255";

QString GradientMapNode::stopsToString(const QVector<QPair<double, QColor>> &stops)
{
    QStringList parts;
    for (const auto &s : stops) {
        parts.append(QString("%1,%2,%3,%4")
            .arg(s.first, 0, 'f', 4)
            .arg(s.second.red())
            .arg(s.second.green())
            .arg(s.second.blue()));
    }
    return parts.join(';');
}

QVector<QPair<double, QColor>> GradientMapNode::stringToStops(const QString &str)
{
    QVector<QPair<double, QColor>> stops;
    QStringList parts = str.split(';', Qt::SkipEmptyParts);
    for (const auto &part : parts) {
        QStringList vals = part.split(',');
        if (vals.size() == 4) {
            bool ok1, ok2, ok3, ok4;
            double pos = vals[0].toDouble(&ok1);
            int r = vals[1].toInt(&ok2);
            int g = vals[2].toInt(&ok3);
            int b = vals[3].toInt(&ok4);
            if (ok1 && ok2 && ok3 && ok4)
                stops.append({qBound(0.0, pos, 1.0),
                             QColor(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255))});
        }
    }
    if (stops.size() < 2)
        stops = { {0.0, QColor(0,0,0)}, {1.0, QColor(255,255,255)} };
    return stops;
}

GradientMapNode::GradientMapNode()
    : Node("渐变映射")
{
    m_inputPorts  = { Port("图像", DataType::Any, PortDirection::Input, 0) };
    m_outputPorts = { Port("图像", DataType::Any, PortDirection::Output, 0) };
    m_params["stops"] = kDefaultStops;
}

bool GradientMapNode::process(const QVector<DataPacket> &inputs,
                              QVector<DataPacket> &outputs, QString &errorMsg)
{
    if (!inputs[0].isValid()) { errorMsg = "没有输入图像。"; return false; }
    auto stops = stringToStops(m_params["stops"].toString());
    QImage result = ImageAlgorithm::gradientMap(inputs[0].image(), stops);
    if (result.isNull()) { errorMsg = "渐变映射处理失败。"; return false; }
    outputs[0] = DataPacket(result);
    return true;
}

QWidget *GradientMapNode::createParamWidget()
{
    auto *container = new QWidget();
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *label = new QLabel("点击添加色标，双击修改颜色，右键删除");
    label->setStyleSheet("font-size: 11px; color: #aaa;");
    layout->addWidget(label);

    auto *bar = new GradientBar();
    bar->setStops(stringToStops(m_params["stops"].toString()));
    layout->addWidget(bar);

    // Presets
    auto *presetLayout = new QHBoxLayout();
    auto *presetLabel = new QLabel("预设:");
    presetLabel->setStyleSheet("font-size: 12px;");
    presetLayout->addWidget(presetLabel);

    auto *combo = new QComboBox();
    combo->addItem("自定义", QString());
    combo->addItem("黑白", QString("0,0,0,0;1,255,255,255"));
    combo->addItem("暖色调", QString("0,0,0,0;0.5,180,100,40;1,255,230,200"));
    combo->addItem("冷色调", QString("0,0,0,0;0.5,50,100,180;1,200,230,255"));
    combo->addItem("赛博朋克", QString("0,0,0,0;0.3,100,0,150;0.7,0,200,255;1,255,255,100"));
    combo->addItem("复古金", QString("0,30,20,10;0.5,120,80,30;1,220,190,130"));
    combo->addItem("蓝橙对比", QString("0,0,30,80;0.5,100,100,100;1,200,120,50"));
    combo->addItem("霓虹紫", QString("0,150,0,100;1,60,30,170"));
    presetLayout->addWidget(combo, 1);
    layout->addLayout(presetLayout);

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, bar, combo](int idx) {
                QString data = combo->itemData(idx).toString();
                if (data.isEmpty()) return; // "自定义" has no data
                auto stops = stringToStops(data);
                bar->setStops(stops);
                m_params["stops"] = stopsToString(stops);
                emit paramsChanged();
            });

    // Save when stops change (user edited bar)
    connect(bar, &GradientBar::stopsChanged, this, [this, bar, combo]() {
        m_params["stops"] = stopsToString(bar->stops());
        // Reset combo to "自定义" since gradient is no longer a preset
        combo->blockSignals(true);
        combo->setCurrentIndex(0);
        combo->blockSignals(false);
        emit paramsChanged();
    });

    // Init combo: match current stops against presets, or show "自定义"
    combo->blockSignals(true);
    {
        QString currentStops = m_params["stops"].toString();
        bool matched = false;
        for (int i = 1; i < combo->count(); ++i) {
            if (combo->itemData(i).toString() == currentStops) {
                combo->setCurrentIndex(i);
                matched = true;
                break;
            }
        }
        if (!matched)
            combo->setCurrentIndex(0); // "自定义" — keeps loaded stops intact
    }
    combo->blockSignals(false);

    return container;
}

#include "GradientMapNode.moc"
