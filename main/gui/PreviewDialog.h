#pragma once

#include <QDialog>
#include <QWidget>
#include <QCheckBox>
#include <QSplitter>
#include <QImage>
#include <QPainter>

// Panel that paints an image scaled to fit while keeping aspect ratio
class ImagePanel : public QWidget {
    Q_OBJECT
public:
    void setImage(const QImage &img) { m_image = img; update(); }
    void setLabel(const QString &label) { m_label = label; update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.fillRect(rect(), QColor(30, 30, 30));
        if (m_image.isNull()) {
            p.setPen(QColor(150, 150, 150));
            p.drawText(rect(), Qt::AlignCenter, "无图像数据");
            return;
        }
        QImage scaled = m_image.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        p.drawImage(x, y, scaled);

        // Overlay label
        if (!m_label.isEmpty()) {
            QFont f = p.font();
            f.setPointSize(11);
            f.setBold(true);
            p.setFont(f);
            int tw = p.fontMetrics().width(m_label) + 16;
            p.fillRect(0, 0, tw, 24, QColor(0, 0, 0, 140));
            p.setPen(Qt::white);
            p.drawText(QRect(6, 2, tw - 8, 20), Qt::AlignLeft | Qt::AlignVCenter, m_label);
        }
    }
private:
    QImage m_image;
    QString m_label;
};

class PreviewDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreviewDialog(QWidget *parent = nullptr);

    void setResultImage(const QImage &img);
    void setOriginalImage(const QImage &img);
    void setTitle(const QString &title);

private slots:
    void onToggleCompare(bool checked);

private:
    bool m_hasOriginal = false;
    QSplitter *m_splitter;
    ImagePanel *m_resultPanel;
    ImagePanel *m_originalPanel;
    QCheckBox *m_compareCheck;
};
