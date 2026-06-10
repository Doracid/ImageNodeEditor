#pragma once

#include "core/Node.h"

class PencilSketchNode : public Node {
    Q_OBJECT
public:
    PencilSketchNode();
    QString category() const override { return "Stylize"; }
    QString description() const override { return "Convert to pencil sketch."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new PencilSketchNode(); }
};
