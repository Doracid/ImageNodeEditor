#pragma once

#include "core/Node.h"

class CropNode : public Node {
    Q_OBJECT
public:
    CropNode();
    QString category() const override { return "几何变换"; }
    QString description() const override { return "Crop image to a region."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new CropNode(); }
};
