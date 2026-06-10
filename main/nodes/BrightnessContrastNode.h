#pragma once

#include "core/Node.h"

class BrightnessContrastNode : public Node {
    Q_OBJECT
public:
    BrightnessContrastNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "Adjust brightness and contrast."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new BrightnessContrastNode(); }
};
