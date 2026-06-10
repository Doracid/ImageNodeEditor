#include "LogPanel.h"
#include <QVBoxLayout>
#include <QDateTime>

LogPanel::LogPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_log = new QPlainTextEdit(this);
    m_log->setReadOnly(true);
    m_log->setMaximumBlockCount(500);
    m_log->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_log->setStyleSheet("QPlainTextEdit { background: #1e1e1e; color: #d4d4d4; font-family: Consolas; font-size: 10pt; }");
    layout->addWidget(m_log);

    log("日志初始化完成");
}

void LogPanel::log(const QString &msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_log->appendPlainText(QString("[%1] %2").arg(timestamp, msg));
}

void LogPanel::clear()
{
    m_log->clear();
}
