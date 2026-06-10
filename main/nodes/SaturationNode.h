#pragma once

#include "core/Node.h"

class SaturationNode : public Node {
    Q_OBJECT
public:
    SaturationNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "Adjust color saturation."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new SaturationNode(); }
};
