#pragma once

#include "core/Node.h"

class GammaNode : public Node {
    Q_OBJECT
public:
    GammaNode();
    QString category() const override { return "Filter"; }
    QString description() const override { return "Apply gamma correction."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new GammaNode(); }
};
