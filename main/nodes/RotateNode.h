#pragma once

#include "core/Node.h"

class RotateNode : public Node {
    Q_OBJECT
public:
    RotateNode();
    QString category() const override { return "Filter"; }
    QString description() const override { return "Rotate image by arbitrary angle."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new RotateNode(); }
};
