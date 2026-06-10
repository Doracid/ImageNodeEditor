#pragma once

#include "core/Node.h"

class HueShiftNode : public Node {
    Q_OBJECT
public:
    HueShiftNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "Shift hue by an angle."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new HueShiftNode(); }
};
