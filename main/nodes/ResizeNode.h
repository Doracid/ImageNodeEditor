#pragma once

#include "core/Node.h"

class ResizeNode : public Node {
    Q_OBJECT
public:
    ResizeNode();
    QString category() const override { return "Filter"; }
    QString description() const override { return "Resize image dimensions."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ResizeNode(); }
};
