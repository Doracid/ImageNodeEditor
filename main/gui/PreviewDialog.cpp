#include "PreviewDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>

PreviewDialog::PreviewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("预览");
    setMinimumSize(300, 300);
    resize(800, 600);

    auto *layout = new QVBoxLayout(this);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_imageLabel, 1);

    auto *btnLayout = new QHBoxLayout();
    auto *closeBtn = new QPushButton("关闭");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
}

void PreviewDialog::setImage(const QImage &img)
{
    m_originalImage = img;
    if (img.isNull()) {
        m_imageLabel->setText("无图像数据");
        return;
    }
    updatePixmap();
}

void PreviewDialog::setTitle(const QString &title)
{
    setWindowTitle("预览 - " + title);
}

void PreviewDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (!m_originalImage.isNull())
        updatePixmap();
}

void PreviewDialog::updatePixmap()
{
    if (m_originalImage.isNull()) return;

    // Available area = label size inside the layout
    QSize avail = m_imageLabel->size();
    if (avail.width() < 10 || avail.height() < 10) return;

    QPixmap pm = QPixmap::fromImage(m_originalImage);
    QPixmap scaled = pm.scaled(avail, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_imageLabel->setPixmap(scaled);
}
