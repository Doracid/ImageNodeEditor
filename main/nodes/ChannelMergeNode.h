#pragma once

#include "core/Node.h"

class ChannelMergeNode : public Node {
    Q_OBJECT
public:
    ChannelMergeNode();
    QString category() const override { return "MultiPort"; }
    QString description() const override { return "Merge R, G, B channels into color image."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ChannelMergeNode(); }
};
