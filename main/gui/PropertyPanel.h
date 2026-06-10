#pragma once

#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QScrollArea>
#include <QMap>
#include <QVariant>
#include <QPointer>

class Node;

class PropertyPanel : public QWidget {
    Q_OBJECT
public:
    explicit PropertyPanel(QWidget *parent = nullptr);

    void showNode(Node *node);
    void clearNode();

signals:
    void paramChanged();

private:
    void rebuildUI();

    QPointer<Node> m_currentNode;
    QLabel *m_titleLabel;
    QWidget *m_formContainer;
    QFormLayout *m_formLayout;
    QScrollArea *m_scroll;

    // Keep track of widgets we create
    QVector<QWidget*> m_paramWidgets;
};
