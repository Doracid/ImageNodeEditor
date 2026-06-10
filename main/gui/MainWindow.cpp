#include "MainWindow.h"
#include "NodeScene.h"
#include "NodeView.h"
#include "NodeListPanel.h"
#include "LogPanel.h"
#include "PropertyPanel.h"
#include "PreviewDialog.h"
#include "NodeGraphicsItem.h"
#include "ConnectionGraphicsItem.h"
#include "core/NodeRegistry.h"
#include "io/WorkflowSerializer.h"
#include "engine/WorkflowEngine.h"

#include <QMenuBar>
#include <QToolBar>
#include <QSplitter>
#include <QDockWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QApplication>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("图像节点编辑器");
    resize(1280, 800);

    m_scene = new NodeScene(this);

    setupMenuBar();
    setupToolBar();
    setupCentralWidget();

    // Status bar
    statusBar()->showMessage("就绪");

    // Scene logging
    connect(m_scene, &NodeScene::nodeSelected, this, &MainWindow::onNodeSelected);
    connect(m_scene, &NodeScene::nodeDeselected, this, &MainWindow::onNodeDeselected);
    connect(m_scene, &NodeScene::showPreview, this, &MainWindow::onShowPreview);
    connect(m_scene, &NodeScene::connectionError, this, [this](const QString &msg) {
        m_logPanel->log("连线错误: " + msg);
        statusBar()->showMessage("连线错误: " + msg, 5000);
    });
    connect(m_scene, &NodeScene::connectionAdded, this, [this]() {
        int nConns = m_scene->engine()->allConnections().size();
        m_logPanel->log(QString("添加连线 (共 %1 条)").arg(nConns));
    });
    connect(m_scene, &NodeScene::connectionRemoved, this, [this]() {
        m_logPanel->log("移除连线");
    });
    connect(m_scene, &NodeScene::workflowModified, this, [this]() {
        int nNodes = m_scene->engine()->allNodes().size();
        int nConns = m_scene->engine()->allConnections().size();
        statusBar()->showMessage(QString("节点: %1 | 连线: %2").arg(nNodes).arg(nConns));
    });
    // Direct callback instead of signal/slot (more reliable)
    m_nodeList->setAddNodeCallback([this](const QString &typeName) {
        onAddNode(typeName);
    });

    m_logPanel->log("应用程序就绪");
}

MainWindow::~MainWindow() = default;

// ---------------------------------------------------------------------------
// Menu bar
// ---------------------------------------------------------------------------
void MainWindow::setupMenuBar()
{
    // File menu
    auto *fileMenu = menuBar()->addMenu("&文件");

    fileMenu->addAction("&新建", this, &MainWindow::onClearAll, QKeySequence::New);
    fileMenu->addAction("&打开工作流...", this, &MainWindow::onLoadWorkflow, QKeySequence::Open);
    fileMenu->addAction("&保存工作流...", this, &MainWindow::onSaveWorkflow, QKeySequence::Save);
    fileMenu->addSeparator();
    fileMenu->addAction("退&出", qApp, &QApplication::quit, QKeySequence::Quit);

    // Edit menu
    auto *editMenu = menuBar()->addMenu("&编辑");
    editMenu->addAction("全&选", m_scene, [this]() { m_scene->clearSelection(); for (auto *item : m_scene->items()) { item->setSelected(true); } }, QKeySequence::SelectAll);
    editMenu->addAction("&删除选中", this, [this]() { m_scene->deleteSelected(); }, QKeySequence::Delete);

    // Run menu
    auto *runMenu = menuBar()->addMenu("&运行");
    runMenu->addAction("&执行工作流", this, &MainWindow::executeWorkflow, QKeySequence("Ctrl+R"));

    // Help menu
    auto *helpMenu = menuBar()->addMenu("&帮助");
    helpMenu->addAction("&关于", this, &MainWindow::onShowAbout);
}

// ---------------------------------------------------------------------------
// Tool bar
// ---------------------------------------------------------------------------
void MainWindow::setupToolBar()
{
    auto *toolBar = addToolBar("主工具栏");
    toolBar->setIconSize(QSize(20, 20));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextOnly);

    toolBar->addAction("▶ 执行", this, &MainWindow::executeWorkflow);
    toolBar->addSeparator();
    toolBar->addAction("💾 保存", this, &MainWindow::onSaveWorkflow);
    toolBar->addAction("📂 打开", this, &MainWindow::onLoadWorkflow);
    toolBar->addSeparator();
    toolBar->addAction("🗑 清空", this, &MainWindow::onClearAll);
}

// ---------------------------------------------------------------------------
// Central widget
// ---------------------------------------------------------------------------
void MainWindow::setupCentralWidget()
{
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    // Left: node list
    m_nodeList = new NodeListPanel();
    m_nodeList->setMinimumWidth(180);
    m_nodeList->setMaximumWidth(250);
    splitter->addWidget(m_nodeList);

    // Center: node view
    m_view = new NodeView(m_scene);
    splitter->addWidget(m_view);

    // Right: tabbed panel with Properties + Log
    m_rightTabs = new QTabWidget();
    m_rightTabs->setMinimumWidth(220);
    m_rightTabs->setMaximumWidth(380);

    m_propertyPanel = new PropertyPanel();
    m_rightTabs->addTab(m_propertyPanel, "属性");

    m_logPanel = new LogPanel();
    m_rightTabs->addTab(m_logPanel, "日志");

    splitter->addWidget(m_rightTabs);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 0);

    setCentralWidget(splitter);

    connect(m_propertyPanel, &PropertyPanel::paramChanged, this, [this]() {
        statusBar()->showMessage("参数已更新", 3000);
        m_logPanel->log("参数已更新");
    });
}

// ---------------------------------------------------------------------------
// Execute workflow
// ---------------------------------------------------------------------------
void MainWindow::executeWorkflow()
{
    if (m_scene->engine()->allNodes().isEmpty()) {
        m_logPanel->log("工作流为空，无法执行");
        QMessageBox::information(this, "提示", "工作流为空，请先添加节点。");
        return;
    }

    m_logPanel->log("开始执行工作流...");
    QString errorMsg;
    WorkflowEngine::ResultMap results;
    if (!m_scene->engine()->execute(results, errorMsg)) {
        m_logPanel->log("执行失败: " + errorMsg);
        QMessageBox::critical(this, "执行错误", errorMsg);
        return;
    }

    m_logPanel->log("工作流执行成功！");
    statusBar()->showMessage("工作流执行成功！", 5000);

    // Store outputs for preview
    for (auto *node : m_scene->engine()->allNodes()) {
        auto it = results.constFind(node->id());
        if (it != results.constEnd() && !it->isEmpty()) {
            QImage firstImg = it->first().image();
            if (!firstImg.isNull()) {
                node->setProperty("__lastOutput", QVariant::fromValue(firstImg));
                m_logPanel->log(QString("%1 生成输出 (%2x%3)")
                    .arg(node->title()).arg(firstImg.width()).arg(firstImg.height()));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------
void MainWindow::onNodeSelected(Node *node)
{
    m_logPanel->log(QString("选中: %1 (%2)").arg(node->title(), node->category()));
    m_propertyPanel->showNode(node);
    m_rightTabs->setCurrentWidget(m_propertyPanel);
}

void MainWindow::onNodeDeselected()
{
    m_logPanel->log("取消选中");
    m_propertyPanel->clearNode();
}

void MainWindow::onShowPreview(Node *node)
{
    QVariant var = node->property("__lastOutput");
    QImage img;
    if (var.isValid())
        img = var.value<QImage>();

    if (img.isNull()) {
        QMessageBox::information(this, "预览", "没有可用图像。\n请先执行工作流，或连接图像查看器节点来预览输出。");
        return;
    }

    if (!m_previewDialog) {
        m_previewDialog = new PreviewDialog(this);
    }
    m_previewDialog->setImage(img);
    m_previewDialog->setTitle(node->title());
    m_previewDialog->show();
    m_previewDialog->raise();
    m_previewDialog->activateWindow();
}

void MainWindow::onAddNode(const QString &typeName)
{
    Node *node = NodeRegistry::instance().create(typeName);
    if (!node) {
        m_logPanel->log("错误: 未知节点类型 '" + typeName + "'");
        QMessageBox::warning(this, "错误", QString("未知节点类型: %1").arg(typeName));
        return;
    }

    // Random position near center of view
    QPointF pos = m_view->mapToScene(m_view->viewport()->rect().center());
    pos += QPointF(rand() % 200 - 100, rand() % 200 - 100);

    m_scene->addNodeToScene(node, pos);
    m_logPanel->log(QString("添加节点: %1").arg(node->title()));
    statusBar()->showMessage(QString("已添加: %1").arg(node->title()), 3000);
}

void MainWindow::onSaveWorkflow()
{
    QString path = QFileDialog::getSaveFileName(this, "保存工作流", "", "JSON (*.json)");
    if (path.isEmpty()) return;
    m_logPanel->log("保存工作流到 " + path);
    QString errorMsg;
    if (!WorkflowSerializer::saveToFile(*m_scene->engine(), path, errorMsg)) {
        m_logPanel->log("保存失败: " + errorMsg);
        QMessageBox::critical(this, "保存错误", errorMsg);
    } else {
        m_logPanel->log("工作流保存成功");
        statusBar()->showMessage("工作流已保存！", 3000);
    }
}

void MainWindow::onLoadWorkflow()
{
    QString path = QFileDialog::getOpenFileName(this, "加载工作流", "", "JSON (*.json)");
    if (path.isEmpty()) return;

    m_logPanel->log("加载工作流: " + path);
    m_scene->clearAll();

    QString errorMsg;
    WorkflowEngine tempEngine;
    if (!WorkflowSerializer::loadFromFile(path, tempEngine, errorMsg)) {
        m_logPanel->log("加载失败: " + errorMsg);
        QMessageBox::critical(this, "加载错误", errorMsg);
        return;
    }

    // Transfer nodes to scene (clone to retain separate ownership)
    int nodeCount = 0;
    for (auto *node : tempEngine.allNodes()) {
        QPointF pos;
        double x = node->param("__posX", -10000).toDouble();
        double y = node->param("__posY", -10000).toDouble();
        if (x > -9999) pos.setX(x);
        if (y > -9999) pos.setY(y);

        QUuid oldId = node->id();
        Node *clone = node->clone();
        clone->setId(oldId);
        for (auto it = node->allParams().constBegin(); it != node->allParams().constEnd(); ++it)
            clone->setParam(it.key(), it.value());

        m_scene->addNodeToScene(clone, pos);
        nodeCount++;
    }

    // Transfer connections
    int connCount = 0;
    for (const auto &c : tempEngine.allConnections()) {
        if (m_scene->addConnectionToScene(c.sourceNodeId, c.sourcePort,
                                          c.targetNodeId, c.targetPort))
            connCount++;
    }

    m_logPanel->log(QString("加载了 %1 个节点和 %2 条连线").arg(nodeCount).arg(connCount));
    statusBar()->showMessage("工作流已加载！", 3000);
}

void MainWindow::onClearAll()
{
    if (m_scene->engine()->allNodes().isEmpty()) return;
    auto reply = QMessageBox::question(this, "清空全部",
                                        "确定要清空所有节点和连线吗？",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        int n = m_scene->engine()->allNodes().size();
        m_scene->clearAll();
        m_logPanel->log(QString("清空了 %1 个节点").arg(n));
        statusBar()->showMessage("已清空", 3000);
    }
}

void MainWindow::onShowAbout()
{
    QMessageBox::about(this, "关于图像节点编辑器",
        "图像节点编辑器 v1.0\n\n"
        "基于节点的图像处理工作流工具。\n\n"
        "使用 C++ 和 Qt 构建。");
}
