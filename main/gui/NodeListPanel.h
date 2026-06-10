#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <functional>

class NodeListPanel : public QWidget {
    Q_OBJECT
public:
    explicit NodeListPanel(QWidget *parent = nullptr);

    // Callback invoked when a node type is clicked
    using AddNodeCallback = std::function<void(const QString&)>;
    void setAddNodeCallback(AddNodeCallback cb) { m_callback = cb; }

private:
    void populateButtons();
    AddNodeCallback m_callback;
    QVBoxLayout *m_layout;
    QScrollArea *m_scroll;
    QWidget *m_container;
    QLabel *m_debugLabel;
};
