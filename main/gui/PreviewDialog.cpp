#include "PreviewDialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QScreen>

PreviewDialog::PreviewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("预览");
    setMinimumSize(300, 300);
    resize(800, 600);

    auto *layout = new QVBoxLayout(this);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setAlignment(Qt::AlignCenter);

    m_imageLabel = new QLabel();
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_scroll->setWidget(m_imageLabel);

    layout->addWidget(m_scroll, 1);

    auto *btnLayout = new QHBoxLayout();
    auto *closeBtn = new QPushButton("关闭");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
}

void PreviewDialog::setImage(const QImage &img)
{
    if (img.isNull()) {
        m_imageLabel->setText("无图像数据");
        return;
    }
    m_imageLabel->setPixmap(QPixmap::fromImage(img));
    m_imageLabel->resize(img.size());
}

void PreviewDialog::setTitle(const QString &title)
{
    setWindowTitle("预览 - " + title);
}
