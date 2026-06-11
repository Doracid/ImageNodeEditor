#pragma once

#include <QMainWindow>
#include <QPointer>
#include <QTabWidget>
#include <QUuid>

class NodeScene;
class NodeView;
class NodeListPanel;
class LogPanel;
class PropertyPanel;
class PreviewDialog;
class WorkflowEngine;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    NodeScene *scene() const { return m_scene; }

public slots:
    void executeWorkflow();

private slots:
    void onNodeSelected(class Node *node);
    void onNodeDeselected();
    void onShowPreview(class Node *node);
    void onAddNode(const QString &typeName);
    void onSaveWorkflow();
    void onLoadWorkflow();
    void onClearAll();
    void onShowAbout();
    void onStartReplace(const QUuid &nodeId);
    void onFinishReplace(const QString &typeName);
    void onCancelReplace();

private:
    void setupMenuBar();
    void setupToolBar();
    void setupCentralWidget();

    NodeScene      *m_scene;
    NodeView       *m_view;
    NodeListPanel  *m_nodeList;
    LogPanel       *m_logPanel;
    PropertyPanel  *m_propertyPanel;
    QTabWidget     *m_rightTabs;
    PreviewDialog  *m_previewDialog = nullptr;
    QUuid           m_replaceNodeId;
};
