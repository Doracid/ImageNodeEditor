#pragma once

#include "core/Node.h"

class MetalStyleNode : public Node {
    Q_OBJECT
public:
    MetalStyleNode();
    QString category() const override { return "风格化"; }
    QString description() const override { return "金属底板：金/银/青/古铜色调 + 凸版/凹版浮雕。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new MetalStyleNode(); }

    QWidget *createParamWidget() override;
};
