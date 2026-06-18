#pragma once

#include "core/Node.h"

class PolarCoordsNode : public Node {
    Q_OBJECT
public:
    PolarCoordsNode();
    QString category() const override { return "风格化"; }
    QString description() const override { return "直角坐标↔极坐标转换，将图像在极坐标系中重新映射。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new PolarCoordsNode(); }

    QWidget *createParamWidget() override;
};
