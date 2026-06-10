#pragma once

#include "core/Node.h"

class GrayscaleNode : public Node {
    Q_OBJECT
public:
    GrayscaleNode();
    QString category() const override { return "Conversion"; }
    QString description() const override { return "Convert color image to grayscale."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new GrayscaleNode(); }
};
