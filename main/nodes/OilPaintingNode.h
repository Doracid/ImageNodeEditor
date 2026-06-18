#pragma once

#include "core/Node.h"

class OilPaintingNode : public Node {
    Q_OBJECT
public:
    OilPaintingNode();
    QString category() const override { return "风格化"; }
    QString description() const override { return "模拟油画笔触效果，通过量化亮度直方图统计生成笔触。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new OilPaintingNode(); }
};
