#pragma once

#include "core/Node.h"

class SepiaNode : public Node {
    Q_OBJECT
public:
    SepiaNode();
    QString category() const override { return "Stylize"; }
    QString description() const override { return "Apply sepia tone effect."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new SepiaNode(); }
};
