#pragma once

#include "core/Node.h"

class ColorTemperatureNode : public Node {
    Q_OBJECT
public:
    ColorTemperatureNode();
    QString category() const override { return "Stylize"; }
    QString description() const override { return "Adjust color temperature (warm/cool)."; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ColorTemperatureNode(); }
};
