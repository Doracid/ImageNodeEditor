#pragma once

#include "core/Node.h"

class ClarityNode : public Node {
    Q_OBJECT
public:
    ClarityNode();
    QString category() const override { return "滤波"; }
    QString description() const override { return "中频对比度增强（清晰度）。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ClarityNode(); }
};
