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

    // Callback invoked when a node type is clicked (normal add mode)
    using AddNodeCallback = std::function<void(const QString&)>;
    void setAddNodeCallback(AddNodeCallback cb) { m_callback = cb; }

    // Picker mode — for node replacement
    using ReplaceNodeCallback = std::function<void(const QString&)>;
    void enterPickerMode(ReplaceNodeCallback cb);
    void exitPickerMode();
    bool isPickerMode() const { return m_pickerMode; }

signals:
    void pickerCancelled();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void populateButtons();
    void updateButtonStyles();

    AddNodeCallback m_callback;
    ReplaceNodeCallback m_replaceCallback;
    QVBoxLayout *m_layout;
    QScrollArea *m_scroll;
    QWidget *m_container;
    QLabel *m_debugLabel;
    QLabel *m_pickerLabel = nullptr;
    QVector<QPushButton*> m_buttons;
    bool m_pickerMode = false;
};
