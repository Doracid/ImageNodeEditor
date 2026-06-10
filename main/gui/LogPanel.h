#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QPlainTextEdit>

class LogPanel : public QWidget {
    Q_OBJECT
public:
    explicit LogPanel(QWidget *parent = nullptr);

public slots:
    void log(const QString &msg);
    void clear();

private:
    QPlainTextEdit *m_log;
};
