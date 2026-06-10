#pragma once

#include "core/Node.h"

class InvertNode : public Node {
    Q_OBJECT
public:
    InvertNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "Invert image colors."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new InvertNode(); }
};
