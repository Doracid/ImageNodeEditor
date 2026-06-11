#pragma once

#include "core/Node.h"

class WhitesBlacksNode : public Node {
    Q_OBJECT
public:
    WhitesBlacksNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "调整白点和黑点（端点裁剪）。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new WhitesBlacksNode(); }
};
