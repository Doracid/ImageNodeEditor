#pragma once

#include "core/Node.h"
#include "algorithms/ImageAlgorithm.h"

class ImageInputNode : public Node {
    Q_OBJECT
public:
    ImageInputNode();
    QString category() const override { return "Input"; }
    QString description() const override { return "Load an image from file."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs,
                 QString &errorMsg) override;
    Node *clone() const override { return new ImageInputNode(); }
};
