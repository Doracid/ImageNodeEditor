#pragma once

#include "core/Node.h"

class ThresholdNode : public Node {
    Q_OBJECT
public:
    ThresholdNode();
    QString category() const override { return "转换"; }
    QString description() const override { return "二值化：全局阈值/大津法/自适应均值/自适应高斯。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ThresholdNode(); }

    QWidget *createParamWidget() override;
};
