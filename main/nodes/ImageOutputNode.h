#pragma once

#include "core/Node.h"

class ImageOutputNode : public Node {
    Q_OBJECT
public:
    ImageOutputNode();
    QString category() const override { return "Output"; }
    QString description() const override { return "Save image to file."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs,
                 QString &errorMsg) override;
    Node *clone() const override { return new ImageOutputNode(); }
};
