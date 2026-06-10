#pragma once

#include "core/Node.h"
#include <QComboBox>
#include <QDoubleSpinBox>

class RotateNode : public Node {
    Q_OBJECT
public:
    RotateNode();
    QString category() const override { return "几何变换"; }
    QString description() const override { return "Rotate image by arbitrary angle."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new RotateNode(); }
    QWidget *createParamWidget() override;

private:
    QDoubleSpinBox *m_angleSpin = nullptr;
    QComboBox *m_bgCombo = nullptr;
};
