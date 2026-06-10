#pragma once

#include "core/Node.h"
#include <QSpinBox>
#include <QCheckBox>

class ResizeNode : public Node {
    Q_OBJECT
public:
    ResizeNode();
    QString category() const override { return "几何变换"; }
    QString description() const override { return "Resize image dimensions."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ResizeNode(); }
    QWidget *createParamWidget() override;

private:
    QSpinBox *m_wSpin = nullptr;
    QSpinBox *m_hSpin = nullptr;
    QCheckBox *m_keepCb = nullptr;
    double m_aspectRatio = 1.0;
    void updateHeightFromWidth();
    void updateWidthFromHeight();
};
