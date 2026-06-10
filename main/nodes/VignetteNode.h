#pragma once

#include "core/Node.h"

class VignetteNode : public Node {
    Q_OBJECT
public:
    VignetteNode();
    QString category() const override { return "Stylize"; }
    QString description() const override { return "Add vignette darkening at edges."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new VignetteNode(); }
};
