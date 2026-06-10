#pragma once

#include "core/Node.h"

class ThresholdNode : public Node {
    Q_OBJECT
public:
    ThresholdNode();
    QString category() const override { return "转换"; }
    QString description() const override { return "Apply binary threshold."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ThresholdNode(); }
};
