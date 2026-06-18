#pragma once

#include "core/Node.h"

class ComicStyleNode : public Node {
    Q_OBJECT
public:
    ComicStyleNode();
    QString category() const override { return "风格化"; }
    QString description() const override { return "漫画风格：黑色轮廓线 + 白色背景。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new ComicStyleNode(); }
};
