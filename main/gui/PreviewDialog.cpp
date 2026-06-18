#include "PreviewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

PreviewDialog::PreviewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("预览");
    setMinimumSize(400, 300);
    resize(900, 600);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    // Compare toggle
    auto *topBar = new QHBoxLayout();
    m_compareCheck = new QCheckBox("显示原图对比");
    m_compareCheck->setChecked(true);
    topBar->addWidget(m_compareCheck);
    topBar->addStretch();
    layout->addLayout(topBar);

    // Splitter with two image panels
    m_splitter = new QSplitter(Qt::Horizontal, this);

    m_originalPanel = new ImagePanel();
    m_originalPanel->setLabel("原图");
    m_splitter->addWidget(m_originalPanel);

    m_resultPanel = new ImagePanel();
    m_resultPanel->setLabel("效果图");
    m_splitter->addWidget(m_resultPanel);

    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
    layout->addWidget(m_splitter, 1);

    // Close button
    auto *btnLayout = new QHBoxLayout();
    auto *closeBtn = new QPushButton("关闭");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(m_compareCheck, &QCheckBox::toggled, this, &PreviewDialog::onToggleCompare);
}

void PreviewDialog::setResultImage(const QImage &img)
{
    m_resultPanel->setImage(img);
}

void PreviewDialog::setOriginalImage(const QImage &img)
{
    m_originalPanel->setImage(img);
    m_hasOriginal = !img.isNull();
    m_originalPanel->setVisible(m_compareCheck->isChecked() && m_hasOriginal);
}

void PreviewDialog::setTitle(const QString &title)
{
    setWindowTitle("预览 - " + title);
}

void PreviewDialog::onToggleCompare(bool checked)
{
    m_originalPanel->setVisible(checked && m_hasOriginal);
}
