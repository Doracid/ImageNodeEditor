#pragma once

#include "core/Node.h"

class EdgeDetectionNode : public Node {
    Q_OBJECT
public:
    EdgeDetectionNode();
    QString category() const override { return "Filter"; }
    QString description() const override { return "Sobel edge detection."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new EdgeDetectionNode(); }
};
