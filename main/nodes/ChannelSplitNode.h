#pragma once

#include "core/Node.h"

class ChannelSplitNode : public Node {
    Q_OBJECT
public:
    ChannelSplitNode();
    QString category() const override { return "MultiPort"; }
    QString description() const override { return "Split image into R, G, B channels."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ChannelSplitNode(); }
};
