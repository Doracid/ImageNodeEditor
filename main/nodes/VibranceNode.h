#pragma once

#include "core/Node.h"

class VibranceNode : public Node {
    Q_OBJECT
public:
    VibranceNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "智能自然饱和度增强。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new VibranceNode(); }
};
