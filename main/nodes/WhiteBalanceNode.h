#pragma once

#include "core/Node.h"

class WhiteBalanceNode : public Node {
    Q_OBJECT
public:
    WhiteBalanceNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "白平衡：色温 + 色调 + 自动校正。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new WhiteBalanceNode(); }
};
