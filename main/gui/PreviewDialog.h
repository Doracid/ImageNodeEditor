#pragma once

#include <QDialog>
#include <QLabel>
#include <QImage>
#include <QScrollArea>

class PreviewDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreviewDialog(QWidget *parent = nullptr);

    void setImage(const QImage &img);
    void setTitle(const QString &title);

private:
    QLabel *m_imageLabel;
    QScrollArea *m_scroll;
};
