#include "PropertyPanel.h"
#include "core/Node.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QGroupBox>
#include <QDir>
#include <QFileInfo>

// ---------------------------------------------------------------------------
// Chinese display names for param keys
// ---------------------------------------------------------------------------
static QString paramLabel(const QString &key)
{
    static const QMap<QString, QString> m = {
        {"filePath",     "文件路径"},
        {"format",       "格式"},
        {"x",            "X 坐标"},
        {"y",            "Y 坐标"},
        {"width",        "宽度"},
        {"height",       "高度"},
        {"keepRatio",    "保持比例"},
        {"radius",       "模糊半径"},
        {"brightness",   "亮度"},
        {"contrast",     "对比度"},
        {"angle",        "旋转角度"},
        {"bgRed",        "背景色 R"},
        {"bgGreen",      "背景色 G"},
        {"bgBlue",       "背景色 B"},
        {"text",         "水印文字"},
        {"fontSize",     "字体大小"},
        {"red",          "R"},
        {"green",        "G"},
        {"blue",         "B"},
        {"opacity",      "透明度"},
        {"posX",         "位置 X"},
        {"posY",         "位置 Y"},
        {"lowThreshold", "低阈值"},
        {"highThreshold","高阈值"},
        {"saturation",   "饱和度"},
        {"gamma",        "伽马值"},
        {"level",        "阈值等级"},
        {"intensity",    "强度"},
        {"strength",     "锐化强度"},
        {"rotation",     "旋转角度"},
        {"temperature",  "色温"},
        {"fade",         "褪色程度"},
        {"blockSize",    "像素块大小"},
        {"levels",       "颜色层级"},
        {"detailBoost",  "细节增强"},
        {"strength",     "美化强度"},
    };
    return m.value(key, key);
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    m_titleLabel = new QLabel("未选中节点", this);
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    mainLayout->addWidget(m_titleLabel);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);

    m_formContainer = new QWidget();
    m_formLayout = new QFormLayout(m_formContainer);
    m_formLayout->setSpacing(6);
    m_scroll->setWidget(m_formContainer);

    mainLayout->addWidget(m_scroll, 1);
}

void PropertyPanel::showNode(Node *node)
{
    m_currentNode = node;
    m_titleLabel->setText(node->title());
    rebuildUI();
}

void PropertyPanel::clearNode()
{
    m_currentNode = nullptr;
    m_titleLabel->setText("未选中节点");
    rebuildUI();
}

// ---------------------------------------------------------------------------
// UI rebuild
// ---------------------------------------------------------------------------
void PropertyPanel::rebuildUI()
{
    // Immediately remove all old form rows
    while (m_formLayout->rowCount() > 0)
        m_formLayout->removeRow(0);
    m_paramWidgets.clear();

    if (!m_currentNode) return;

    Node *node = m_currentNode;

    // Custom widget provided by node itself?
    QWidget *custom = node->createParamWidget();
    if (custom) {
        m_formLayout->addRow(custom);
        m_paramWidgets.append(custom);
        return;
    }

    bool isOutput = (node->category() == "Output");

    const auto params = node->allParams();
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        const QString &key = it.key();
        if (key.startsWith("__")) continue;

        // Hide auto-detected format (extension-based)
        if (key == "format") continue;

        const QString label = paramLabel(key);
        const QVariant &val = it.value();

        // ------------------------------------------------------------
        // Output node filePath → save dialog
        // ------------------------------------------------------------
        if (isOutput && key == "filePath") {
            auto *w = new QWidget();
            auto *hl = new QHBoxLayout(w);
            hl->setContentsMargins(0,0,0,0);
            auto *le = new QLineEdit(val.toString());
            le->setPlaceholderText("选择保存路径...");
            auto *btn = new QPushButton("浏览...");
            btn->setFixedWidth(60);
            hl->addWidget(le, 1);
            hl->addWidget(btn);

            connect(btn, &QPushButton::clicked, this, [this, le, key]() {
                QString path = QFileDialog::getSaveFileName(
                    this, "保存图像", le->text(), "图片 (*.png *.jpg *.jpeg *.bmp *.tif)");
                if (!path.isEmpty()) {
                    le->setText(path);
                    if (m_currentNode) {
                        m_currentNode->setParam(key, path);
                        emit paramChanged();
                    }
                }
            });
            connect(le, &QLineEdit::editingFinished, this, [this, le, key]() {
                if (m_currentNode) {
                    m_currentNode->setParam(key, le->text());
                    emit paramChanged();
                }
            });

            m_formLayout->addRow(label, w);
            m_paramWidgets.append(w);
            continue;
        }

        // ------------------------------------------------------------
        // File path (input nodes) → line edit + browse button
        // ------------------------------------------------------------
        if (key.contains("path", Qt::CaseInsensitive) && !isOutput) {
            auto *w = new QWidget();
            auto *hl = new QHBoxLayout(w);
            hl->setContentsMargins(0,0,0,0);
            auto *le = new QLineEdit(val.toString());
            auto *btn = new QPushButton("浏览...");
            btn->setFixedWidth(60);
            hl->addWidget(le, 1);
            hl->addWidget(btn);

            connect(btn, &QPushButton::clicked, this, [this, le, key]() {
                QString path = QFileDialog::getOpenFileName(
                    this, "打开文件", le->text(), "图片 (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.gif *.webp *.svg);;所有文件 (*.*)");
                if (!path.isEmpty()) {
                    le->setText(path);
                    if (m_currentNode) {
                        m_currentNode->setParam(key, path);
                        emit paramChanged();
                    }
                }
            });
            connect(le, &QLineEdit::editingFinished, this, [this, le, key]() {
                if (m_currentNode) {
                    m_currentNode->setParam(key, le->text());
                    emit paramChanged();
                }
            });

            m_formLayout->addRow(label, w);
            m_paramWidgets.append(w);
            continue;
        }

        // ------------------------------------------------------------
        // Bool → checkbox
        // ------------------------------------------------------------
        if (val.type() == QVariant::Bool) {
            auto *cb = new QCheckBox();
            cb->setChecked(val.toBool());
            connect(cb, &QCheckBox::toggled, this, [this, key](bool v) {
                if (m_currentNode) { m_currentNode->setParam(key, v); emit paramChanged(); }
            });
            m_formLayout->addRow(label, cb);
            m_paramWidgets.append(cb);
            continue;
        }

        // ------------------------------------------------------------
        // Double → double spin box (with bounds)
        // ------------------------------------------------------------
        if (val.type() == QVariant::Double) {
            auto *sb = new QDoubleSpinBox();
            ParamBound b = node->paramBound(key);
            if (b.min.isValid() && b.max.isValid()) {
                sb->setRange(b.min.toDouble(), b.max.toDouble());
                sb->setSingleStep(b.step);
            } else {
                sb->setRange(-10000, 10000);
                sb->setSingleStep(0.1);
            }
            sb->setValue(val.toDouble());
            connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, key](double v) {
                if (m_currentNode) { m_currentNode->setParam(key, v); emit paramChanged(); }
            });
            m_formLayout->addRow(label, sb);
            m_paramWidgets.append(sb);
            continue;
        }

        // ------------------------------------------------------------
        // Int → spin box (with bounds)
        // ------------------------------------------------------------
        if (val.type() == QVariant::Int) {
            auto *sb = new QSpinBox();
            ParamBound b = node->paramBound(key);
            if (b.min.isValid() && b.max.isValid()) {
                sb->setRange(b.min.toInt(), b.max.toInt());
            } else {
                sb->setRange(-100000, 100000);
            }
            sb->setValue(val.toInt());
            connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, key](int v) {
                if (m_currentNode) { m_currentNode->setParam(key, v); emit paramChanged(); }
            });
            m_formLayout->addRow(label, sb);
            m_paramWidgets.append(sb);
            continue;
        }

        // ------------------------------------------------------------
        // Default: line edit
        // ------------------------------------------------------------
        auto *le = new QLineEdit(val.toString());
        connect(le, &QLineEdit::editingFinished, this, [this, le, key]() {
            if (m_currentNode) { m_currentNode->setParam(key, le->text()); emit paramChanged(); }
        });
        m_formLayout->addRow(label, le);
        m_paramWidgets.append(le);
    }
}
