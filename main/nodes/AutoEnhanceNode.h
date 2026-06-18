#pragma once

#include "core/Node.h"

class AutoEnhanceNode : public Node {
    Q_OBJECT
public:
    AutoEnhanceNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "自动美化：自动白平衡 + 色阶拉伸 + S 曲线对比度 + 饱和度微调。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new AutoEnhanceNode(); }
};
