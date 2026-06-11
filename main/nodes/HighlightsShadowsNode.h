#pragma once

#include "core/Node.h"

class HighlightsShadowsNode : public Node {
    Q_OBJECT
public:
    HighlightsShadowsNode();
    QString category() const override { return "色彩调整"; }
    QString description() const override { return "独立调整高光和阴影区域。"; }
    bool process(const QVector<DataPacket> &inputs,
                 QVector<DataPacket> &outputs, QString &errorMsg) override;
    Node *clone() const override { return new HighlightsShadowsNode(); }
};
