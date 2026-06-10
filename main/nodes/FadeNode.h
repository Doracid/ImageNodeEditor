#pragma once

#include "core/Node.h"

class FadeNode : public Node {
    Q_OBJECT
public:
    FadeNode();
    QString category() const override { return "转换"; }
    QString description() const override { return "Fade image toward gray."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new FadeNode(); }
};
