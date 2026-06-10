#pragma once

#include "core/Node.h"

class PixelateNode : public Node {
    Q_OBJECT
public:
    PixelateNode();
    QString category() const override { return "Stylize"; }
    QString description() const override { return "Apply pixelation effect."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new PixelateNode(); }
};
