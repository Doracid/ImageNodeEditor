#pragma once

#include "core/Node.h"

class ExposureNode : public Node {
    Q_OBJECT
public:
    ExposureNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "EV 曝光补偿。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ExposureNode(); }
};
