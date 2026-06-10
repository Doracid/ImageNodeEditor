#pragma once

#include "core/Node.h"

class CartoonNode : public Node {
    Q_OBJECT
public:
    CartoonNode();
    QString category() const override { return "Stylize"; }
    QString description() const override { return "Apply cartoon effect."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new CartoonNode(); }
};
