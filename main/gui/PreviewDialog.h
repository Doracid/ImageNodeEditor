#pragma once

#include <QDialog>
#include <QLabel>
#include <QImage>

class PreviewDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreviewDialog(QWidget *parent = nullptr);

    void setImage(const QImage &img);
    void setTitle(const QString &title);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updatePixmap();

    QImage m_originalImage;
    QLabel *m_imageLabel;
};
