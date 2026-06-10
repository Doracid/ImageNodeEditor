#pragma once

#include "core/Node.h"

class SharpenNode : public Node {
    Q_OBJECT
public:
    SharpenNode();
    QString category() const override { return "Filter"; }
    QString description() const override { return "Sharpen the image."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new SharpenNode(); }
};
