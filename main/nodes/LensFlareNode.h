#pragma once

#include "core/Node.h"

class LensFlareNode : public Node {
    Q_OBJECT
public:
    LensFlareNode();
    QString category() const override { return "风格化"; }
    QString description() const override { return "模拟镜头光晕效果，包含中心光斑、光环和色散。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new LensFlareNode(); }
};
