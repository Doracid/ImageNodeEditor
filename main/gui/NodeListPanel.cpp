#include "NodeListPanel.h"
#include "core/NodeRegistry.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>

NodeListPanel::NodeListPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(4, 4, 4, 4);

    auto *title = new QLabel("节点类型");
    title->setStyleSheet("font-size: 13px; font-weight: bold; padding: 4px;");
    outerLayout->addWidget(title);

    m_debugLabel = new QLabel("Loading...");
    m_debugLabel->setStyleSheet("color: #888; padding: 2px 4px;");
    outerLayout->addWidget(m_debugLabel);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);

    m_container = new QWidget();
    m_layout = new QVBoxLayout(m_container);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(2);

    m_scroll->setWidget(m_container);
    outerLayout->addWidget(m_scroll, 1);

    populateButtons();
}

void NodeListPanel::populateButtons()
{
    auto categories = NodeRegistry::instance().categorizedEntries();

    int totalTypes = 0;
    for (const auto &pair : categories) {
        totalTypes += pair.second.size();
    }
    m_debugLabel->setText(QString("已注册 %1 种节点类型").arg(totalTypes));

    for (auto it = categories.constBegin(); it != categories.constEnd(); ++it) {
        const QString &catName = it->first;
        const auto &entries = it->second;
        auto *catLabel = new QLabel(
            QString("<b>%1</b> (%2)").arg(catName).arg(entries.size()));
        catLabel->setStyleSheet("padding: 4px 2px 0 2px; border-bottom: 1px solid #ccc;");
        catLabel->setTextFormat(Qt::RichText);
        m_layout->addWidget(catLabel);

        for (const auto &entry : entries) {
            auto *btn = new QPushButton(entry.displayName);
            btn->setToolTip(entry.description);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setMinimumHeight(34);
            btn->setStyleSheet(
                "QPushButton { text-align: left; padding: 6px 10px; border: 1px solid #aaa; "
                "border-radius: 4px; background: #f0f0f0; font-size: 10pt; }"
                "QPushButton:hover { background: #d0e0ff; border-color: #3366cc; }");

            QString typeName = entry.typeName;
            connect(btn, &QPushButton::clicked, this, [this, typeName]() {
                m_debugLabel->setText("已点击: " + typeName);
                if (m_callback) {
                    m_callback(typeName);
                }
            });

            m_layout->addWidget(btn);
        }
    }

    m_layout->addStretch();
}
